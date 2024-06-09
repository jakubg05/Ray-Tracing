#pragma once
#include <GL/glew.h>

#include "core/gl_util/ComputeShader.h"
#include "core/gl_util/ComputeTexture.h"

#include "imgui.h"

#include "core/gl_util/OpenGLdebugFuncs.h"
#include "core/camera/Camera.hpp"
#include "core/ObjParser/ObjParser.h"


/**
* @brief The SceneData struct
* This struct is used to pass the scene data to the renderer
* */
struct SceneData {
	const void* sceneObjects;
	size_t size;
	int numberOfObjects;
};

struct PixelData {
	glm::vec4 pixelColor; // .xyz = color, .w = TRI_intersect_count
	unsigned int AABB_intersect_count;
};

/**
* @brief The rtx_parameters_uniform_struct struct
* This struct is used to pass the uniform
* data to the compute shader for the ray tracing stage (in std140 layout)
* */
struct rtx_parameters_uniform_struct {
	unsigned int numAccumulatedFrames;      // offset 0  // alignment 4 // total 4 bytes
	unsigned int raysPerPixel;              // offset 4  // alignment 4 // total 8 bytes
	unsigned int bouncesPerRay;             // offset 8  // alignment 4 // total 12 bytes
	float FocalLength;						// offset 12 // alignment 4 // total 16 bytes

	glm::vec3 skyboxGroundColor;			// offset 16 // alignment 16 // total 32 bytes
	glm::vec3 skyboxHorizonColor;			// offset 32 // alignment 16 // total 48 bytes
	glm::vec3 skyboxZenithColor;			// offset 48 // alignment 16 // total 64 bytes                            
	glm::vec3 CameraPos;					// offset 64 // alignment 16 // total 80 bytes

	glm::vec3 pixelGlobalInvocationID;		// offset 80 // alignment 16 // total 96 bytes

	glm::mat4 ModelMatrix;					// offset 96 // alignment 16 // total 160 bytes

	bool WasInput;							// offset 160 // alignment 4 // total 164 bytes
	bool display_BVH = display_BVH;					// offset 164 // alignment 4 // total 168 bytes
	bool display_multiple = display_multiple;		// offset 168 // alignment 4 // total 172 bytes
	unsigned int displayed_layer = displayed_layer; // offset 172 // alignment 4 // total 176 bytes
	unsigned int BVH_tree_depth = BVH_tree_depth;	// offset 176 // alignment 4 // total 180 bytes
	bool show_skybox = show_skybox;					// offset 180 // alignment 4 // total 184 bytes
	int heatmap_color_limit = heatmap_color_limit;	// offset 184 // alignment 4 // total 188 bytes
};

/**
* @brief The postProcessing_parameters_uniform_struct struct
* This struct is used to pass the uniform
* data to the compute shader for the post processing stage (in std140 layout)
* */
struct postProcessing_parameters_uniform_struct {
	unsigned int numAccumulatedFrames;
};

/**
* @brief The Renderer class
* This class is used to render the scene using compute shaders
* */
class Renderer
{
private:
	SceneData m_Scene;

	glm::vec2 m_ViewportSize;

	void initComputeRtxStage();
	void initComputePostProcStage();

	// unifom buffer object setup and functions
	unsigned int rtx_parameters_UBO_ID, sphereBuffer_UBO_ID, postProcessing_parameters_UBO_ID, tris_SSBO_ID, BVH_SSBO_ID, pixelData_SSBO_ID;

	void configure_rtx_parameters_UBO_block();
	void update_rtx_parameters_UBO_block();

	void configure_sphereBuffer_UBO_block();
	void update_sphereBuffer_UBO_block();

	void configure_postProcessing_parameters_UBO_block();
	void update_postProcessing_parameters_UBO_block();

	void configure_TrisMesh_SSBO_block();
	void update_TrisMesh_SSBO_block();

	void configure_BVH_SSBO_block();
	void update_BVH_SSBO_block();

	void configure_PixelData_SSBO_block();
	void read_PixelData_SSBO_block();

public:
	Renderer(SceneData& scene, BVH::BVH_data BVH_of_mesh);
	~Renderer();

	void setViewportSize(glm::vec2 viewportSize);

	void BeginComputeRtxStage();
	ComputeTexture* RenderComputeRtxStage();
	rtx_parameters_uniform_struct rtx_uniform_parameters;

	PixelData pixelData; // should be read after rendering the compute rtx stage

	void BeginComputePostProcStage();
	ComputeTexture* RenderComputePostProcStage();
	postProcessing_parameters_uniform_struct postProcessing_uniform_parameters;

private:
	// compute rtx stage
	ComputeTexture* computeRtxTexture;
	ComputeShader* computeRtxShader;

	// compute post processing stage
	ComputeTexture* computePostProcTexture;
	ComputeShader* computePostProcShader;

	BVH::BVH_data BVH_of_mesh;
};