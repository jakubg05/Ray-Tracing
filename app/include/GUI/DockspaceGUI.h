#pragma once
#include <imgui.h>

void genDockspace()
{

	ImVec2 window_size = ImGui::GetIO().DisplaySize;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::SetNextWindowSize(window_size);
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::Begin("dockspace", nullptr, ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

	ImGuiID dockspace_id = ImGui::GetID("Dockspace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f));

	ImGui::PopStyleVar();
	ImGui::End();
}
