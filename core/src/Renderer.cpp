#include <iostream>

#include "core/Renderer.h"

struct SceneData;

Renderer::Renderer(SceneData& scene, BVH::BVH_data BVH_of_mesh)
	: m_Scene(scene),

	computeRtxShader(nullptr),
	computeRtxTexture(nullptr),

	computePostProcShader(nullptr),
	computePostProcTexture(nullptr),

	BVH_of_mesh(BVH_of_mesh)
{
	initComputeRtxStage();
	initComputePostProcStage();
}

Renderer::~Renderer()
{
	delete computeRtxShader;
	delete computeRtxTexture;

	delete computePostProcShader;
	delete computePostProcTexture;

	glDeleteBuffers(1, &rtx_parameters_UBO_ID);
	glDeleteBuffers(1, &postProcessing_parameters_UBO_ID);
	glDeleteBuffers(1, &sphereBuffer_UBO_ID);
}

void Renderer::setViewportSize(glm::vec2 viewportSize)
{
	GLCall(glViewport(0, 0, viewportSize.x, viewportSize.y));
	m_ViewportSize = viewportSize;

	delete computePostProcTexture;
	computePostProcTexture = new ComputeTexture(viewportSize.x, viewportSize.y, 1);

	delete computeRtxTexture;
	computeRtxTexture = new ComputeTexture(viewportSize.x, viewportSize.y, 0);
}


void Renderer::initComputeRtxStage()
{	
	// we would typically set the texture here but we dont know the texture size yet so we do it in setSize
	//computeRtxUBO = new UniformBuffer(sizeof(ComputeRtxUniforms), 0);
	computeRtxShader = new ComputeShader(CORE_RESOURCES_PATH "shaders/ComputeRayTracing.comp");
	computeRtxShader->Bind();
	configure_rtx_parameters_UBO_block();
	configure_sphereBuffer_UBO_block();
	configure_TrisMesh_SSBO_block();
	configure_BVH_SSBO_block();
	configure_PixelData_SSBO_block();
	
	update_sphereBuffer_UBO_block(); // only updated once in the beginning of the scene (assuming the scene is static)
	update_BVH_SSBO_block();
}

void Renderer::BeginComputeRtxStage()
{
	computeRtxShader->Bind();
}

ComputeTexture* Renderer::RenderComputeRtxStage()
{
	update_rtx_parameters_UBO_block();
	update_TrisMesh_SSBO_block();
	computeRtxShader->DrawCall(ceil(m_ViewportSize.x / 8), ceil(m_ViewportSize.y / 4), 1); // work_groups size

	read_PixelData_SSBO_block();

	return computeRtxTexture;
}


void Renderer::initComputePostProcStage()
{
	computePostProcShader = new ComputeShader(CORE_RESOURCES_PATH "shaders/ComputePostProcessing.comp");
	computePostProcShader->Bind();
	configure_postProcessing_parameters_UBO_block();
}

void Renderer::BeginComputePostProcStage()
{
	computePostProcShader->Bind();
}

ComputeTexture* Renderer::RenderComputePostProcStage()
{
	update_postProcessing_parameters_UBO_block();
	computePostProcShader->DrawCall(ceil(m_ViewportSize.x / 8), ceil(m_ViewportSize.y / 4), 1);
	return computePostProcTexture;
}

// binding point 0
void Renderer::configure_rtx_parameters_UBO_block() {	
	GLCall(glGenBuffers(1, &rtx_parameters_UBO_ID));
	GLCall(glBindBuffer(GL_UNIFORM_BUFFER, rtx_parameters_UBO_ID));
	GLCall(glBufferData(GL_UNIFORM_BUFFER, 188, nullptr, GL_DYNAMIC_DRAW));
	GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, 0, rtx_parameters_UBO_ID)); // binding the uniform buffer object to binding point 0
}

void Renderer::update_rtx_parameters_UBO_block() {
	GLCall(glBindBuffer(GL_UNIFORM_BUFFER, rtx_parameters_UBO_ID));
	GLCall(glBufferSubData(GL_UNIFORM_BUFFER, 0,   sizeof(unsigned int), &rtx_uniform_parameters.numAccumulatedFrames));
	GLCall(glBufferSubData(GL_UNIFORM_BUFFER, 4,   sizeof(unsigned int), &rtx_uniform_parameters.raysPerPixel));
	GLCall(glBufferSubData(GL_UNIFORM_BUFFER, 8,   sizeof(unsigned int), &rtx_uniform_parameters.bouncesPerRay));
	GLCall(glBufferSubData(GL_UNIFORM_BUFFER, 12,  sizeof(float), &rtx_uniform_parameters.FocalLength));
	
	GLCall(glBufferSubData(GL_UNIFORM_BUFFER, 16,  sizeof(glm::vec3), &rtx_uniform_parameters.skyboxGroundColor));
	GLCall(glBufferSubData(GL_UNIFORM_BUFFER, 32,  sizeof(glm::vec3), &rtx_uniform_parameters.skyboxHorizonColor));
	GLCall(glBufferSubData(GL_UNIFORM_BUFFER, 48,  sizeof(glm::vec3), &rtx_uniform_parameters.skyboxZenithColor));
	GLCall(glBufferSubData(GL_UNIFORM_BUFFER, 64,  sizeof(glm::vec3), &rtx_uniform_parameters.CameraPos));
	
	GLCall(glBufferSubData(GL_UNIFORM_BUFFER, 80, sizeof(glm::vec3), &rtx_uniform_parameters.pixelGlobalInvocationID));
	
	GLCall(glBufferSubData(GL_UNIFORM_BUFFER, 96,  sizeof(glm::mat4), &rtx_uniform_parameters.ModelMatrix));

	GLCall(glBufferSubData(GL_UNIFORM_BUFFER, 160, sizeof(bool), &rtx_uniform_parameters.WasInput));
	GLCall(glBufferSubData(GL_UNIFORM_BUFFER, 164, sizeof(bool), &rtx_uniform_parameters.display_BVH));
	GLCall(glBufferSubData(GL_UNIFORM_BUFFER, 168, sizeof(bool), &rtx_uniform_parameters.display_multiple));
	GLCall(glBufferSubData(GL_UNIFORM_BUFFER, 172, sizeof(unsigned int), &rtx_uniform_parameters.displayed_layer));
	GLCall(glBufferSubData(GL_UNIFORM_BUFFER, 176, sizeof(unsigned int), &rtx_uniform_parameters.BVH_tree_depth));
	GLCall(glBufferSubData(GL_UNIFORM_BUFFER, 180, sizeof(bool), &rtx_uniform_parameters.show_skybox));
	GLCall(glBufferSubData(GL_UNIFORM_BUFFER, 184, sizeof(int), &rtx_uniform_parameters.heatmap_color_limit));
	
	GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

// binding point 1
void Renderer::configure_sphereBuffer_UBO_block() {
	GLCall(glGenBuffers(1, &sphereBuffer_UBO_ID));
	GLCall(glBindBuffer(GL_UNIFORM_BUFFER, sphereBuffer_UBO_ID));
	GLCall(glBufferData(GL_UNIFORM_BUFFER, m_Scene.size, nullptr, GL_STATIC_DRAW));
	GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, 1, sphereBuffer_UBO_ID)); 
}

void Renderer::update_sphereBuffer_UBO_block() {
	GLCall(glBindBuffer(GL_UNIFORM_BUFFER, sphereBuffer_UBO_ID));
	GLCall(glBufferSubData(GL_UNIFORM_BUFFER, 0, m_Scene.size, m_Scene.sceneObjects)); // sceneObjects are already a const void* pointer
	GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

// binding point 2
void Renderer::configure_postProcessing_parameters_UBO_block() {
	unsigned int postProcessing_buffer_size_bytes = 4;
	GLCall(glGenBuffers(1, &postProcessing_parameters_UBO_ID));
	GLCall(glBindBuffer(GL_UNIFORM_BUFFER, postProcessing_parameters_UBO_ID));
	GLCall(glBufferData(GL_UNIFORM_BUFFER, postProcessing_buffer_size_bytes, nullptr, GL_DYNAMIC_DRAW));
	GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, 2, postProcessing_parameters_UBO_ID)); // binding point 0
}

void Renderer::update_postProcessing_parameters_UBO_block() {
	GLCall(glBindBuffer(GL_UNIFORM_BUFFER, postProcessing_parameters_UBO_ID));
	GLCall(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(unsigned int), &postProcessing_uniform_parameters.numAccumulatedFrames));
	GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

void Renderer::configure_TrisMesh_SSBO_block()
{
	GLCall(glGenBuffers(1, &tris_SSBO_ID));
	GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, tris_SSBO_ID));
	GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Triangle) * BVH_of_mesh.TRIANGLES_size, nullptr, GL_STATIC_DRAW));
	GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, tris_SSBO_ID));
}

void Renderer::update_TrisMesh_SSBO_block()
{
	GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, tris_SSBO_ID));
	GLCall(glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Triangle) * BVH_of_mesh.TRIANGLES_size, BVH_of_mesh.TRIANGLES.data()))
	GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0)); // unbind
}

void Renderer::configure_BVH_SSBO_block()
{
	GLCall(glGenBuffers(1, &BVH_SSBO_ID));
	GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, BVH_SSBO_ID));
	GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(BVH::Node) * BVH_of_mesh.BVH_size, nullptr, GL_STATIC_DRAW));
	GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, BVH_SSBO_ID));
}

void Renderer::update_BVH_SSBO_block()
{
	GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, BVH_SSBO_ID));
	for (size_t i = 0; i < 3; i++) {
		std::cout << BVH_of_mesh.BVH[i] << std::endl;
	}
	GLCall(glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(BVH::Node) * BVH_of_mesh.BVH_size, BVH_of_mesh.BVH.data()));
	GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0)); // unbind
}

void Renderer::configure_PixelData_SSBO_block()
{
	GLCall(glGenBuffers(1, &pixelData_SSBO_ID));
	GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, pixelData_SSBO_ID));
	GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, 20, nullptr, GL_DYNAMIC_READ));
	GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, pixelData_SSBO_ID));
}

void Renderer::read_PixelData_SSBO_block() {
    GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, pixelData_SSBO_ID));

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    PixelData* SSBO_pixelData_ptr = (PixelData*)glMapBufferRange(
										GL_SHADER_STORAGE_BUFFER, 0,  20,
										GL_MAP_READ_BIT);

    if (SSBO_pixelData_ptr != nullptr) {
		pixelData.pixelColor = SSBO_pixelData_ptr->pixelColor;
		pixelData.AABB_intersect_count = SSBO_pixelData_ptr->AABB_intersect_count;
		

		GLCall(glUnmapBuffer(GL_SHADER_STORAGE_BUFFER));
    } 
	else { std::cout << "Error mapping buffer" << std::endl; }

    GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
}
