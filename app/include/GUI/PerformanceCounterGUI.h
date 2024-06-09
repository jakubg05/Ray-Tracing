#pragma once
#include <imgui.h>

void genPerformanceCounter()
{
	ImGuiIO& io = ImGui::GetIO();
	ImGui::Begin("FPS Counter", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking);
	ImVec2 widgetPos = ImGui::GetCursorScreenPos();
	ImVec2 widgetSize = ImGui::GetContentRegionAvail();
	ImVec2 mousePos = io.MousePos;

	if (mousePos.x >= widgetPos.x - 7 && mousePos.x <= (widgetPos.x + widgetSize.x + 7) &&
		mousePos.y >= widgetPos.y - 7 && mousePos.y <= (widgetPos.y + widgetSize.y + 7))
	{
		ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
	}
	else
	{
		ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
	}
	static const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
	ImGui::Text(version);
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::TextUnformatted("Current OpenGL Version");
		ImGui::EndTooltip();
	}
	ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
	ImGui::End();
}