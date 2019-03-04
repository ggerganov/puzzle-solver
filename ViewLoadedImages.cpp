/*! \file ViewLoadedImages.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "Types.h"
#include "Functions.h"

#include "imgui/imgui.h"

template <>
bool Render::operator()<ViewLoadedImages>(ViewLoadedImages & obj, StateApp & state) {
    bool res = false;

    for (int id = 0; id < (int) state.loadedImages.size(); ++id) {
        auto & item = state.loadedImages[id];
        ImGui::PushID(item.fname.c_str());
        if (state.curAction == StateApp::LoadingImages) {
            if (ImGui::Button("Delete")) {
                res = true;
                ::Free()(item);
                state.loadedImages.erase(state.loadedImages.begin() + id);
                obj.selectedId = -1;
                --id;
                continue;
            }
            ImGui::SameLine();
        }
        if (id == obj.selectedId) {
            ImGui::TextColored({1.0f, 1.0f, 0.0f, 1.0f}, "%s", item.fname.c_str());
        } else {
            if (state.curAction == StateApp::AddingCommonPoint || state.curAction == StateApp::EditingCommonPoint) {
                if (::Exist()(state.viewSelectedImage.commonPointInput, id)) {
                    ImGui::TextColored({0.0f, 1.0f, 0.0f, 1.0f}, "%s", item.fname.c_str());
                } else {
                    ImGui::TextColored({1.0f, 0.0f, 0.0f, 1.0f}, "%s", item.fname.c_str());
                }
            } else {
                ImGui::Text("%s", item.fname.c_str());
            }
            if (ImGui::IsItemHovered()) {
                if (ImGui::IsMouseDown(0)) {
                    res = true;
                    obj.selectedId = id;
                }
            }
        }
        ImGui::PopID();
    }

    return res;
}

template <>
bool Exist::operator()<std::vector<LoadedImage>>(const std::vector<LoadedImage> & obj, const char * fname) {
    bool res = false;

    for (const auto & cur : obj) {
        if (cur.fname == fname) {
            res = true;
            break;
        }
    }

    return res;
}
