#pragma once
#include <imgui.h>
#include <glm/glm.hpp>

/**
* @brief Wrap code in an if statement and set imgui_was_input as true
* @param code - the code to be wrapped
*/
#define IMGUI_INPUT(code) \
    if (code) { \
        was_IMGUI_input = true; \
    }

void genSkyboxGUI(glm::vec3& groundColor, glm::vec3& skyColorHorizon, glm::vec3& skyColorZenith, bool& displaySkybox, bool& was_IMGUI_input, bool disabled)
{
	if (disabled) { ImGui::BeginDisabled(); }
	ImGui::Begin("Skybox Settings");

	IMGUI_INPUT(ImGui::Checkbox("Display Skybox", &displaySkybox));
	ImGui::Separator();
	IMGUI_INPUT(ImGui::ColorEdit3("GroundColor", &groundColor.x));
	IMGUI_INPUT(ImGui::ColorEdit3("SkyColorHorizon", &skyColorHorizon.x));
	IMGUI_INPUT(ImGui::ColorEdit3("SkyColorZenith", &skyColorZenith.x));

	ImGui::End();
	if (disabled) { ImGui::EndDisabled(); }
}