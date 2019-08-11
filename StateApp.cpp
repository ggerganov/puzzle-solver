/*! \file StateApp.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "Types.h"
#include "Functions.h"

#include "imgui/imgui.h"

namespace {
    float frand() { return (float)(rand())/RAND_MAX; }
}

template <>
bool OnDragAndDrop::operator()<StateApp>(StateApp & obj, const char * fname) {
    if (::Exist()(obj.loadedImages, fname)) {
        return false;
    }

    ImageRGBWithTexture newImage;
    if (::LoadFromFile()(newImage, fname) == false) {
        return false;
    }

    bool res = true;

    res &= ::GenerateTexture()(newImage, true);
    obj.loadedImages.push_back({ fname, newImage });
    obj.viewLoadedImages.selectedId = obj.loadedImages.size() - 1;

    return res;
}

template <>
bool Render::operator()<StateApp>(StateApp & obj) {
    bool res = false;

    ImGui::SetNextWindowPos({ 0, 0 });
    ImGui::SetNextWindowSize({ obj.leftPanelSizeX, obj.loadedImagesSizeY });
    ImGui::Begin("Loaded images");
    {
        ::Render()(obj.viewLoadedImages, obj);
    }

    obj.leftPanelSizeX = ImGui::GetWindowSize().x;
    obj.loadedImagesSizeY = ImGui::GetWindowSize().y;
    ImGui::End();

    ImGui::SetNextWindowPos({ obj.leftPanelSizeX, 0 });
    ImGui::SetNextWindowSize({ ImGui::GetIO().DisplaySize.x - obj.leftPanelSizeX, ImGui::GetIO().DisplaySize.y });
    ImGui::Begin("Selected image");
    {
        ::Render()(obj.viewSelectedImage, obj);
    }
    ImGui::End();

    ImGui::SetNextWindowPos({ 0, obj.loadedImagesSizeY });
    ImGui::SetNextWindowSize({ obj.leftPanelSizeX, ImGui::GetIO().DisplaySize.y - obj.loadedImagesSizeY});
    ImGui::Begin("Actions");
    {
        ::Render()(obj.viewActions, obj);
    }
    ImGui::End();

    return res;
}
