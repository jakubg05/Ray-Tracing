#pragma once

#include <imgui.h>

// don't want to include the whole ObjParser.h file so we just define the same enum here
#ifndef heuristicEnum
#define heuristicEnum
namespace BVH
{
    enum class Heuristic {
        OBJECT_MEDIAN_SPLIT,
        SPATIAL_MIDDLE_SPLIT,
        SURFACE_AREA_HEURISTIC,
        SURFACE_AREA_HEURISTIC_BUCKETS
    };
}
#endif


/**
* @brief Wrap code in an if statement and set imgui_was_input as true
* @param code - the code to be wrapped
*/
#define IMGUI_INPUT(code) \
    if (code) { \
        was_IMGUI_input = true; \
    }


/**
* @brief GUI for BVH settings
* @param display_BVH - whether to display the BVH
* @param active_heuristic - the heuristic to use for BVH construction
* @param displayed_layer - the layer of the BVH to display
* @param display_multiple - whether to display multiple layers of the BVH (till the displayed layer)
* @param was_IMGUI_input - whether there was IMGUI input (used in shader to tell when to restart the accumulation of rays)
* @param disabled - to disable the GUI when in the camera control mode
* */
void BVH_settings_GUI(bool& display_BVH, BVH::Heuristic& active_heuristic, int BVH_tree_depth, int& heatmap_color_limit, bool& showPixelData, bool& was_IMGUI_input, bool disabled) {
    ImGuiWindowFlags BVH_window_flags = 0;
    BVH_window_flags |= ImGuiWindowFlags_NoCollapse;
    BVH_window_flags |= ImGuiWindowFlags_NoTitleBar;

    if (disabled) { ImGui::BeginDisabled(); }
    ImGui::Begin("BVH Settings", NULL, BVH_window_flags);

    ImGui::Text("BVH Tree Depth: %d", BVH_tree_depth);
    if (active_heuristic == BVH::Heuristic::SURFACE_AREA_HEURISTIC_BUCKETS) {
        ImGui::Text("Active BVH Heuristic: Surface Area Heuristic Buckets");
    }
    else if (active_heuristic == BVH::Heuristic::SURFACE_AREA_HEURISTIC) {
        ImGui::Text("Active BVH Heuristic: %d", BVH::Heuristic::SURFACE_AREA_HEURISTIC);
    }
    else if (active_heuristic == BVH::Heuristic::OBJECT_MEDIAN_SPLIT) {
        ImGui::Text("Active BVH Heuristic: %d", BVH::Heuristic::OBJECT_MEDIAN_SPLIT);
    }
    else if (active_heuristic == BVH::Heuristic::SPATIAL_MIDDLE_SPLIT) {
        ImGui::Text("Active BVH Heuristic: %d", BVH::Heuristic::SPATIAL_MIDDLE_SPLIT);
    }

    ImGui::SeparatorText("Visual");
    if (ImGui::Checkbox("Show BVH heatmap", &display_BVH)) {
        was_IMGUI_input = true;
        showPixelData = true;
    }

    IMGUI_INPUT(ImGui::SliderInt("BVH Heatmap intersections limit", &heatmap_color_limit, 10.0f, 10000.0f, NULL, ImGuiSliderFlags_Logarithmic));

    if (!display_BVH) {
        ImGui::BeginDisabled();
        showPixelData = false; 
    }
    ImGui::Checkbox("Show pixel-data", &showPixelData);
    if (!display_BVH) { ImGui::EndDisabled(); }

    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted("The 'Show BVH' option must be ENABLED to display pixel-data. When this option is enabled and you hover over pixels in the viewport, a tooltip will appear. The tooltip provides detailed information about the pixel under the cursor, including its position, color values and the number of bounces.");

        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }

    ImGui::End();
	if (disabled) { ImGui::EndDisabled(); }
}
