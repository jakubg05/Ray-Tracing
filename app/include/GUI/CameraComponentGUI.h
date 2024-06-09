#pragma once
#include "core/camera/Camera.hpp"
#include <imgui.h>
#include <unordered_map>

std::unordered_map<int, std::string> keyMap = {
{GLFW_KEY_A, "A"},
{GLFW_KEY_B, "B"},
{GLFW_KEY_C, "C"},
{GLFW_KEY_D, "D"},
{GLFW_KEY_E, "E"},
{GLFW_KEY_F, "F"},
{GLFW_KEY_G, "G"},
{GLFW_KEY_H, "H"},
{GLFW_KEY_I, "I"},
{GLFW_KEY_J, "J"},
{GLFW_KEY_K, "K"},
{GLFW_KEY_L, "L"},
{GLFW_KEY_M, "M"},
{GLFW_KEY_N, "N"},
{GLFW_KEY_O, "O"},
{GLFW_KEY_P, "P"},
{GLFW_KEY_Q, "Q"},
{GLFW_KEY_R, "R"},
{GLFW_KEY_S, "S"},
{GLFW_KEY_T, "T"},
{GLFW_KEY_U, "U"},
{GLFW_KEY_V, "V"},
{GLFW_KEY_W, "W"},
{GLFW_KEY_X, "X"},
{GLFW_KEY_Y, "Y"},
{GLFW_KEY_Z, "Z"},
{GLFW_KEY_LEFT, "LEFT ARROW"},
{GLFW_KEY_RIGHT, "RIGHT ARROW"},
{GLFW_KEY_UP, "UP ARROW"},
{GLFW_KEY_DOWN, "DOWN ARROW"},
{GLFW_KEY_LEFT_SHIFT, "LEFT SHIFT"},
{GLFW_KEY_RIGHT_SHIFT, "RIGHT SHIFT"},
{GLFW_KEY_LEFT_CONTROL, "LEFT CONTROL"},
{GLFW_KEY_RIGHT_CONTROL, "RIGHT CONTROL"},
{GLFW_KEY_SPACE, "SPACE"},
{GLFW_KEY_0, "0"},
{GLFW_KEY_1, "1"},
{GLFW_KEY_2, "2"},
{GLFW_KEY_3, "3"},
{GLFW_KEY_4, "4"},
{GLFW_KEY_5, "5"},
{GLFW_KEY_6, "6"},
{GLFW_KEY_7, "7"},
{GLFW_KEY_8, "8"},
{GLFW_KEY_9, "9"}
};

void scheduleKeybindChange(Camera& camera, int& keybind) {
	camera.cameraKeybinds.camera_keybind_window_active = true;
	camera.cameraKeybinds.keybind_to_be_changed = &keybind;
}

void component_cameraGUI(Camera& camera, bool& was_IMGUI_Input, bool disabled, bool& shouldAccumulate, bool& doPostProc, int& raysPerPixel, int& bouncesPerRay) {
	if (disabled) { ImGui::BeginDisabled(); }
	ImGui::Begin("Camera");
	ImVec2 windowSize = ImGui::GetContentRegionAvail();
	ImGui::Indent(20.0f);

	ImGui::Spacing();
	ImGui::SeparatorText("TRANSFORM");
	static int currentMode = WORLD_RELATIVE;
	if (ImGui::RadioButton("WORLD RELATIVE", &currentMode, WORLD_RELATIVE)) {
		camera.movementType = WORLD_RELATIVE;
		camera.rotAroundZ = 0.0f;
		camera.flags.scheduleRotUpdate = true;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("CAMERA RELATIVE", &currentMode, CAMERA_RELATIVE)) {
		camera.movementType = CAMERA_RELATIVE;
	}
	ImGui::SameLine();
	ImGui::Dummy(ImVec2(60.0f, 0.0f));
	ImGui::SameLine();

	if (ImGui::SmallButton("Reset")) {
		camera.posVec = glm::vec3(0.0f, 0.0f, 0.0f);
		camera.rotAroundX = 0.0f;
		camera.rotAroundY = 0.0f;
		camera.rotAroundZ = 0.0f;
		camera.flags.scheduleRotUpdate = true;
		camera.flags.schedulePosUpdate = true;
	}

	ImGui::Spacing();
	glm::vec3 prevPos = camera.posVec;
	ImGui::Text("Position");
	ImGui::SameLine();
	float sliderMaxWidth = 100;
	float sliders_width = windowSize.x <= 3 * sliderMaxWidth ? windowSize.x : 3 * sliderMaxWidth;

	// Calculate the width for each item (equal distribution)
	float itemWidth = (sliders_width / 3.0f);

	ImGui::PushItemWidth(itemWidth);
	ImGui::PushID("translate_x");
	if (ImGui::DragFloat("x", &camera.posVec.x, 0.05f, -FLT_MAX, FLT_MAX, "%.3f")) {
		camera.flags.schedulePosUpdate = true;
	}
	ImGui::PopID();
	ImGui::SameLine(0, 10); // Add some spacing between sliders
	ImGui::PushItemWidth(itemWidth);
	ImGui::PushID("translate_y");
	if (ImGui::DragFloat("y", &camera.posVec.y, 0.05f, -FLT_MAX, FLT_MAX, "%.3f")) {
		camera.flags.schedulePosUpdate = true;
	}
	ImGui::PopID();
	ImGui::SameLine(0, 10); // Add some spacing between sliders
	ImGui::PushItemWidth(itemWidth);
	ImGui::PushID("translate_z");
	if (ImGui::DragFloat("z", &camera.posVec.z, 0.05f, -FLT_MAX, FLT_MAX, "%.3f")) {
		camera.flags.schedulePosUpdate = true;
	}
	ImGui::PopID();

	ImGui::PopItemWidth();
	ImGui::PopItemWidth();
	ImGui::PopItemWidth();

	glm::vec3 prevRot = glm::vec3(camera.rotAroundX, camera.rotAroundY, camera.rotAroundZ);
	ImGui::Text("Rotation");
	ImGui::SameLine();
	ImGui::PushItemWidth(itemWidth);
	ImGui::PushID("rotate_x");
	ImGui::DragFloat("x", &camera.rotAroundX, 0.05f, -FLT_MAX, FLT_MAX, "%.3f");
	ImGui::PopID();
	ImGui::SameLine(0, 10); // Add some spacing between sliders
	ImGui::PushItemWidth(itemWidth);
	ImGui::PushID("rotate_y");
	ImGui::DragFloat("y", &camera.rotAroundY, 0.05f, -FLT_MAX, FLT_MAX, "%.3f");
	ImGui::PopID();
	ImGui::SameLine(0, 10); // Add some spacing between sliders
	ImGui::PushItemWidth(itemWidth);
	if (camera.movementType == WORLD_RELATIVE) { ImGui::BeginDisabled(); }
	ImGui::PushID("rotate_z");
	ImGui::DragFloat("z", &camera.rotAroundZ, 0.05f, -FLT_MAX, FLT_MAX, "%.3f");
	ImGui::PopID();
	if (camera.movementType == WORLD_RELATIVE) { ImGui::EndDisabled(); }

	ImGui::PopItemWidth();
	ImGui::PopItemWidth();
	ImGui::PopItemWidth();

	ImGui::Spacing();
	ImGui::Spacing();

	if (prevPos != camera.posVec || prevRot != glm::vec3(camera.rotAroundX, camera.rotAroundY, camera.rotAroundZ))
	{
		was_IMGUI_Input = true;
		camera.flags.scheduleRotUpdate = true;
	}

	static const char* items[] = { "Aspect: 21/9", "Aspect: 16/9", "Aspect: 3/2", "Aspect: 4/3", "Aspect: 1/1" };
	static int selectedItem = 1;
	float values[] = { 21.0f / 9.0f, 16.0f / 9.0f, 3.0f / 2.0f, 4.0f / 3.0f, 1.0f / 1.0f };
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x / 2.0f);
	if (ImGui::Combo("##AspectCombo", &selectedItem, items, IM_ARRAYSIZE(items))) {
		camera.setAspect(values[selectedItem]);
		camera.flags.scheduleScreenUpdate = true;
	}
	ImGui::PopItemWidth();

	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x / 2.0f);
	if (ImGui::SliderFloat("FOV", &camera.FOV_deg, 50.0f, 110.0f, "%.1f")) {
		camera.flags.scheduleScreenUpdate = true;
		was_IMGUI_Input = true;
	}
	ImGui::PopItemWidth();
	ImGui::Spacing();
	ImGui::SeparatorText("RAY TRACING & POST PROCESSING");

	ImGui::Checkbox("Accumulate", &shouldAccumulate);
	ImGui::Checkbox("Post Processing", &doPostProc);
	static int step = 1;
	static int step_fast = 5;
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x / 2.0f);
	if (ImGui::InputScalar("Rays Per Pixel", ImGuiDataType_S32, &raysPerPixel, &step, &step_fast, NULL, ImGuiInputTextFlags_CharsDecimal)) {
		if (raysPerPixel < 0) {
			raysPerPixel = 0;
		}
		else if (raysPerPixel > 100) {
			raysPerPixel = 100;
		}
		camera.flags.schedulePosUpdate = true; // only to refresh screen
		was_IMGUI_Input = true;
	}
	ImGui::PopItemWidth();

	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x / 2.0f);
	if (ImGui::InputScalar("Bounces Per Ray", ImGuiDataType_S32, &bouncesPerRay, &step, &step_fast, NULL, ImGuiInputTextFlags_CharsDecimal)) {
		if (bouncesPerRay < 0) {
			bouncesPerRay = 0;
		}
		else if (bouncesPerRay > 100) {
			bouncesPerRay = 100;
		}
		camera.flags.schedulePosUpdate = true; // only to refresh screen
		was_IMGUI_Input = true;
	}
	ImGui::PopItemWidth();

	ImGui::Spacing();
	ImGui::SeparatorText("CONTROLLS & SETTINGS");


	if (ImGui::TreeNode("Keybinds")) 
	{
		if (ImGui::BeginTable("KeybindTable", 2))
		{
			ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_None);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::Text("Forward"); ImGui::TableSetColumnIndex(1);
			if (ImGui::Button((keyMap[camera.cameraKeybinds.FORWARD]).c_str())) {
				scheduleKeybindChange(camera, camera.cameraKeybinds.FORWARD);
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::Text("Backward"); ImGui::TableSetColumnIndex(1);
			if (ImGui::Button((keyMap[camera.cameraKeybinds.BACKWARD]).c_str())) {
				scheduleKeybindChange(camera, camera.cameraKeybinds.BACKWARD);
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::Text("Left"); ImGui::TableSetColumnIndex(1);
			if (ImGui::Button((keyMap[camera.cameraKeybinds.LEFT]).c_str())) {
				scheduleKeybindChange(camera, camera.cameraKeybinds.LEFT);
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::Text("Right"); ImGui::TableSetColumnIndex(1);
			if (ImGui::Button((keyMap[camera.cameraKeybinds.RIGHT]).c_str())) {
				scheduleKeybindChange(camera, camera.cameraKeybinds.RIGHT);
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::Text("Up"); ImGui::TableSetColumnIndex(1);
			if (ImGui::Button((keyMap[camera.cameraKeybinds.UP]).c_str())) {
				scheduleKeybindChange(camera, camera.cameraKeybinds.UP);
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::Text("Down"); ImGui::TableSetColumnIndex(1);
			if (ImGui::Button((keyMap[camera.cameraKeybinds.DOWN]).c_str())) {
				scheduleKeybindChange(camera, camera.cameraKeybinds.DOWN);
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::Text("Roll left"); ImGui::TableSetColumnIndex(1);
			if (ImGui::Button((keyMap[camera.cameraKeybinds.ROLL_LEFT]).c_str())) {
				scheduleKeybindChange(camera, camera.cameraKeybinds.ROLL_LEFT);
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::Text("Roll right"); ImGui::TableSetColumnIndex(1);
			if (ImGui::Button((keyMap[camera.cameraKeybinds.ROLL_RIGHT]).c_str())) {
				scheduleKeybindChange(camera, camera.cameraKeybinds.ROLL_RIGHT);
			}

			ImGui::EndTable();
		}
	ImGui::TreePop();
	}




	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x / 2.0f);
	float logSpeed = log10(camera.speed);
	if (ImGui::SliderFloat("Velocity", &camera.speed, 0.1, 1500, "%.1f", ImGuiSliderFlags_Logarithmic)) { was_IMGUI_Input = true; }
	ImGui::SliderFloat("Sensitivity", &camera.sensitivity, 10.0f, 50.0f, "%.2f");
	ImGui::PopItemWidth();

	ImGui::Unindent(20.0f);

	ImGui::End(); // Close Main Window
	if (disabled) { ImGui::EndDisabled(); }
}

void genCameraChangeKeybindSplashScreen()
{
	ImVec2 window_size = ImGui::GetIO().DisplaySize;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::SetNextWindowSize(window_size);
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::Begin("windowKeybinds", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
	ImVec2 textSize = ImGui::CalcTextSize("Press A Key");
	ImVec2 textPos = ImVec2((ImGui::GetWindowSize().x - textSize.x) * 0.5f, (ImGui::GetWindowSize().y - textSize.y) * 0.5f);

	// Set the cursor position to center the text
	ImGui::SetCursorPos(textPos);

	// Display the centered text
	ImGui::Text("Press A Key (?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::TextUnformatted("\n  Press any key (already bound keys are unavaible) you can rebind the same key though  \n  Pressing ESC closes the window  \n ");
		ImGui::EndTooltip();
	}
	ImGui::PopStyleVar();
	ImGui::End();
}