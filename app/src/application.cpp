// third party
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// standard
#include <iostream>
#include <fstream> 
#include <string>
#include <sstream> 
#include <vector>
#include <stdio.h>

// core
#include "core/Renderer.h" 
#include "core/gl_util/OpenGLdebugFuncs.h"
#include "core/camera/CameraHandler.hpp"

// app
#include "GUI/CameraComponentGUI.h"
#include "GUI/DockspaceGUI.h"
#include "GUI/PerformanceCounterGUI.h"
#include "GUI/InspectorGUI.h"
#include "GUI/SkyBoxGUI.h"
#include "GUI/BVHsettingsGUI.h"

#include "delta_lib/DeltaTime.h"
#include "scenes/Scene1.hpp"

const int SCREEN_WIDTH = 1920, SCREEN_HEIGHT = 1080;
int main() {
	//WINDOW SETUP
	GLFWwindow* window;
	if (!glfwInit()){
		glfwTerminate();
		ASSERT(false);
		return 1;}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, " ", NULL, NULL);
	if (!window) {
		glfwTerminate();
		ASSERT(false);
		return 1;}

	glfwMakeContextCurrent(window);

	std::cout << glGetString(GL_VERSION) << std::endl;
	glfwSwapInterval(false);

	if (glewInit() != GLEW_OK){
		if (!window){
			glfwDestroyWindow(window);
			glfwTerminate();
			ASSERT(false);
			return 1;}}

	{	
		float ASPECT = 16.0/9.0;
		float fov = 90;

		DeltaTime deltaTime;

		Camera camera = Camera(fov, ASPECT, deltaTime);
		CameraHandler cameraHandler(camera);
		SceneData sceneData = sponza_lights_scene();
		
		// set the active heuristic (SURFACE_AREA_HEURISTIC_BUCKETS, SURFACE_AREA_HEURISTIC, SPATIAL_MIDDLE_SPLIT, OBJECT_MEDIAN_SPLIT)
		BVH::Heuristic active_heuristic = BVH::Heuristic::SURFACE_AREA_HEURISTIC_BUCKETS;

		//BVH::BVH_data scene_BVH = BVH::construct(APP_RESOURCES_PATH "models/suzanne_high_poly_rotated.glb", active_heuristic);
		BVH::BVH_data scene_BVH = BVH::construct(APP_RESOURCES_PATH "models/stanford_dragon_pbr.glb", active_heuristic);
		//BVH::BVH_data scene_BVH = BVH::construct(APP_RESOURCES_PATH "models/sponza.obj", active_heuristic);
		//BVH::BVH_data scene_BVH = BVH::construct(APP_RESOURCES_PATH "models/stanford_bunny.obj", active_heuristic);
		std::cout << "BVH height: " << scene_BVH.BVH_tree_depth << std::endl;
		
		//camera.posVec = glm::vec3(3.027f, 46.893f, -134.682f); // set the initial camera position for stanford dragon
		//camera.posVec = glm::vec3(-116.479f, 84.908f, 86.822f);
		camera.posVec = glm::vec3(1.0f, 0.0f, 0.0f);
		Renderer renderer(sceneData, scene_BVH);
		
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		glfwSetWindowUserPointer(window, &cameraHandler);
		glfwSetKeyCallback(window, CameraHandler::GLFWKeyCallback);
		glfwSetCursorPosCallback(window, CameraHandler::GLFWMousePositionCallback);
		
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 460");

		//////////////////////////
		// ImGui widgets variables
		bool was_ImGui_Input;
		bool shouldPostProcess = false;
		bool shouldAccumulate = true;
		int heatmap_color_limit = 350;
		int raysPerPixel = 1;
		int bouncesPerRay = 5;
		
		//skybox
		bool show_skybox = true;

		glm::vec3 SkyGroundColor  = glm::vec3(0.6392156862f, 0.5803921f, 0.6392156862f);
		glm::vec3 SkyColorHorizon = glm::vec3(1.0f, 1.0f, 1.0f);
		glm::vec3 SkyColorZenith  = glm::vec3(0.486274509f, 0.71372549f, 234.0f / 255.0f);
		
		bool show_demo_window = true;
		int totalFrames = 0;
		double prevCamAspect = 0;
		ImVec2 prevViewportWindowSize = ImVec2(0.0f, 0.0f);
		ImVec2 prevViewportWindowPos = ImVec2(0.0f, 0.0f);
		ImVec2 viewportSize, topLeftTextureCoords, bottomLeftTextureCoords;
		
		// BVH settings
		bool display_BVH = false;
		bool showPixelData = true;
		int displayed_layer = 1;
		bool display_multiple = true;
		int BVH_height = 30; // the height of the BVH tree (the size of the traversal stack in the shader)

		unsigned int viewport_mouseX;
		unsigned int viewport_mouseY;
		unsigned int prev_viewport_mouseX = 0;
		unsigned int prev_viewport_mouseY = 0;
		unsigned int inverted_viewport_mouseY;

		// for averaging the number of intersects
		unsigned int AABB_intersect_count_sum = 0;
		unsigned int TRI_intersect_count_sum = 0;
		unsigned int same_mouse_pos_count = 0;

		while (!glfwWindowShouldClose(window)) {
			deltaTime.update();
			totalFrames += 1;
			was_ImGui_Input = false;

			GLCall(glClear(GL_COLOR_BUFFER_BIT));
			GLCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			
			// Widget windows
			genDockspace();
			genInspector(cameraHandler.CameraControllMode);
			component_cameraGUI(camera, was_ImGui_Input, cameraHandler.CameraControllMode, shouldAccumulate, shouldPostProcess, raysPerPixel, bouncesPerRay);
			genSkyboxGUI(SkyGroundColor, SkyColorHorizon, SkyColorZenith, show_skybox, was_ImGui_Input, cameraHandler.CameraControllMode);
			BVH_settings_GUI(display_BVH, active_heuristic, scene_BVH.BVH_tree_depth, heatmap_color_limit, showPixelData, was_ImGui_Input, cameraHandler.CameraControllMode);



			ImGui::ShowDemoWindow(&show_demo_window);
			if (camera.cameraKeybinds.camera_keybind_window_active) { genCameraChangeKeybindSplashScreen(); }
			
			
			// viewport
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
			ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar| ImGuiWindowFlags_NoCollapse);

			ImGui::SetNextWindowDockID(ImGui::GetID("MainDockspace"));
			
			ImVec2 viewportWindowSize = ImGui::GetContentRegionAvail();

			// check if the viewport size or aspect ratio changed
			if (viewportWindowSize.x != prevViewportWindowSize.x || viewportWindowSize.y != prevViewportWindowSize.y || camera.getAspect() != prevCamAspect)
			{
				prevViewportWindowSize = viewportWindowSize;
				prevCamAspect = camera.getAspect();
			
				double parentAspect = double(viewportWindowSize.x) / double(viewportWindowSize.y);
				// check if the parent aspect is smaller or equal to camera aspect (width is the limiting factor so the window will span the entire width)
				if (parentAspect <=  camera.getAspect())
				{
					viewportSize.x = viewportWindowSize.x; // width stays the same
					viewportSize.y = ceil(viewportWindowSize.x / camera.getAspect()); // calculate the height based on the aspect ratio
				}
				// (height is the limiting factor so the window will span the entire height)
				else if (parentAspect > camera.getAspect())
				{
					viewportSize.y = viewportWindowSize.y; // height stays the same
					viewportSize.x = ceil(viewportWindowSize.y * camera.getAspect()); // calculate the width based on the aspect ratio
				}
				
				renderer.setViewportSize(glm::vec2(viewportSize.x, viewportSize.y)); // sets the texture size to the viewport size
			
				camera.setScreenDimentions(int(viewportSize.x), int(viewportSize.y));
				was_ImGui_Input = true;
			
				topLeftTextureCoords.x = (viewportWindowSize.x - viewportSize.x) / 2.0f;
				topLeftTextureCoords.y = (viewportWindowSize.y - viewportSize.y) / 2.0f;
				// viewport offset
				topLeftTextureCoords.x += ImGui::GetWindowPos().x;
				topLeftTextureCoords.y += ImGui::GetWindowPos().y;
				
				bottomLeftTextureCoords.x = topLeftTextureCoords.x + viewportSize.x;
				bottomLeftTextureCoords.y = topLeftTextureCoords.y + viewportSize.y;
			}
			

			// check if the viewport window position changed
			ImVec2 viewportWindowPos = ImGui::GetWindowPos();
			if (viewportWindowPos.x != prevViewportWindowPos.x || viewportWindowPos.y != prevViewportWindowPos.y)
			{
				prevViewportWindowPos = viewportWindowPos;
			
				topLeftTextureCoords.x = (viewportWindowSize.x - viewportSize.x) / 2.0f;
				topLeftTextureCoords.y = (viewportWindowSize.y - viewportSize.y) / 2.0f;
				// viewport offset
				topLeftTextureCoords.x += viewportWindowPos.x;
				topLeftTextureCoords.y += viewportWindowPos.y;
			
				bottomLeftTextureCoords.x = topLeftTextureCoords.x + viewportSize.x;
				bottomLeftTextureCoords.y = topLeftTextureCoords.y + viewportSize.y;
			}
			
			camera.Update();
			
			static bool wasGlobalInput; // input from the camera, ImGui or any other source (which should reset accumulation)
			wasGlobalInput = (camera.wasCameraInput || was_ImGui_Input || !shouldAccumulate);
			static unsigned int m_NumAccumulatedFrames;
			if (wasGlobalInput) { m_NumAccumulatedFrames = 0; }
			else { m_NumAccumulatedFrames++; }

			viewport_mouseX = ImGui::GetMousePos().x - topLeftTextureCoords.x;
			viewport_mouseY = ImGui::GetMousePos().y - topLeftTextureCoords.y;
			inverted_viewport_mouseY = viewportSize.y - viewport_mouseY;
			// render the scene
			renderer.BeginComputeRtxStage();
			renderer.rtx_uniform_parameters.numAccumulatedFrames = m_NumAccumulatedFrames;
			renderer.rtx_uniform_parameters.raysPerPixel = raysPerPixel;
			renderer.rtx_uniform_parameters.bouncesPerRay = bouncesPerRay;
			renderer.rtx_uniform_parameters.FocalLength = camera.focalLength;
			renderer.rtx_uniform_parameters.skyboxGroundColor = SkyGroundColor;
			renderer.rtx_uniform_parameters.skyboxHorizonColor = SkyColorHorizon;
			renderer.rtx_uniform_parameters.skyboxZenithColor = SkyColorZenith;
			renderer.rtx_uniform_parameters.CameraPos = camera.posVec;
			renderer.rtx_uniform_parameters.ModelMatrix = glm::mat4(camera.getModelMatrix());
			renderer.rtx_uniform_parameters.WasInput = wasGlobalInput;

			renderer.rtx_uniform_parameters.display_BVH = display_BVH;
			renderer.rtx_uniform_parameters.displayed_layer = displayed_layer;
			renderer.rtx_uniform_parameters.display_multiple = display_multiple;
			renderer.rtx_uniform_parameters.BVH_tree_depth = scene_BVH.BVH_tree_depth;
			renderer.rtx_uniform_parameters.show_skybox = show_skybox;
			renderer.rtx_uniform_parameters.heatmap_color_limit = heatmap_color_limit;
			renderer.rtx_uniform_parameters.pixelGlobalInvocationID = glm::vec3(viewport_mouseX, inverted_viewport_mouseY, 1.0f); // invocations start from bottom left


			ComputeTexture* outputTexture = renderer.RenderComputeRtxStage();
			
			renderer.BeginComputePostProcStage();
			renderer.postProcessing_uniform_parameters.numAccumulatedFrames = m_NumAccumulatedFrames;
			ComputeTexture* postProcOutput = renderer.RenderComputePostProcStage();
			
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			drawList->AddImage((ImTextureID)postProcOutput->ID(), topLeftTextureCoords, bottomLeftTextureCoords, {0, 1}, {1, 0});

			if (ImGui::IsWindowHovered() && showPixelData && !cameraHandler.CameraControllMode) {
				if (viewport_mouseX >= 0 && viewport_mouseX < viewportSize.x && inverted_viewport_mouseY >= 0 && inverted_viewport_mouseY < viewportSize.y) {
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
						ImGui::BeginTooltip();
							ImGui::BeginGroup();
								ImGui::Spacing();
								ImGui::Text("Pixel Data");
								ImGui::Separator();
								ImGui::Text("X: %d, Y: %d", viewport_mouseX, inverted_viewport_mouseY);
								if (wasGlobalInput || viewport_mouseX != prev_viewport_mouseX || viewport_mouseY != prev_viewport_mouseY) {
									AABB_intersect_count_sum = 0;
									TRI_intersect_count_sum = 0;
									same_mouse_pos_count = 0;
								}
								prev_viewport_mouseX = viewport_mouseX;
								prev_viewport_mouseY = viewport_mouseY;

								AABB_intersect_count_sum += renderer.pixelData.AABB_intersect_count;
								TRI_intersect_count_sum += renderer.pixelData.pixelColor.w;
								same_mouse_pos_count++;


								glm::vec4 pclr = renderer.pixelData.pixelColor;
								
								ImVec4 pixelColor = ImVec4(pclr.r, pclr.g, pclr.b, 1.0f);
								ImGui::ColorButton("MyColor##3c", *(ImVec4*)&pclr, NULL, ImVec2(20, 20));
								ImGui::SameLine();
								ImGui::Text("RGB: (%f, %f, %f)", pclr.r, pclr.g, pclr.b);
								ImGui::Text("ray-AABB intersects: %u", AABB_intersect_count_sum / same_mouse_pos_count);
								ImGui::Text("ray-triangle intersects: %u", TRI_intersect_count_sum / same_mouse_pos_count);
							ImGui::EndGroup();
						ImGui::EndTooltip();
					ImGui::PopStyleVar();
				}
			}

			ImGui::PopStyleVar();
			ImGui::End();
			
			genPerformanceCounter();
			camera.ResetFlags();
			
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			glfwSwapBuffers(window);
			glfwPollEvents();
		}
	}
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}