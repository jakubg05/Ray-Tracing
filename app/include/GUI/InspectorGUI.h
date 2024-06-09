#pragma once
#include <imgui.h>

void genInspector(bool disabled)
{
	if (disabled) { ImGui::BeginDisabled(); }
	ImGui::Begin("Inspector");
	ImGui::End();
	if (disabled) { ImGui::EndDisabled(); }
}