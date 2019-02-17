/*! \file LoadedImages.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "Types.h"
#include "Functions.h"

#include "imgui/imgui.h"

template <>
bool Render::operator()<LoadedImages>(LoadedImages & obj) {
    bool res = false;

    for (int id = 0; id < (int) obj.size(); ++id) {
        auto & item = obj[id];
        ImGui::PushID(item.fname.c_str());
        if (ImGui::Button("Delete")) {
            res = true;
            ::Free()(item);
            obj.erase(obj.begin() + id);
            obj.selectedId = -1;
            --id;
            continue;
        }
        ImGui::SameLine();
        if (id == obj.selectedId) {
            ImGui::TextColored({0.0f, 1.0f, 0.0f, 1.0f}, "%s", item.fname.c_str());
        } else {
            ImGui::Text("%s", item.fname.c_str());
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
bool Exist::operator()<LoadedImages>(const char * fname, const LoadedImages & obj) {
    bool res = false;

    for (const auto & cur : obj) {
        if (cur.fname == fname) {
            res = true;
            break;
        }
    }

    return res;
}
