/*! \file StateUI.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "Types.h"
#include "Functions.h"

#include "imgui/imgui.h"

namespace {
    template <typename T> int sign(T val) {
        return (T(0) < val) - (val < T(0));
    }
}

template <>
bool OnDragAndDrop::operator()<StateUI>(const char * fname, StateUI & obj) {
    if (::Exist()(fname, obj.loadedImages)) {
        return false;
    }

    ImageRGB newImage;
    if (::LoadFromFile()(fname, newImage) == false) {
        return false;
    }

    bool res = true;

    res &= ::GenerateTexture()(true, newImage);
    obj.loadedImages.push_back({ fname, newImage });
    obj.loadedImages.selectedId = obj.loadedImages.size() - 1;

    return res;
}

template <>
bool Render::operator()<StateUI>(StateUI & obj) {
    bool res = false;

    ImGui::Begin("Loaded images");
    res |= this->operator()(obj.loadedImages);
    ImGui::End();

    ImGui::Begin("Selected image");
    if (obj.loadedImages.selectedId >= 0) {
        auto & selectedImage = obj.loadedImages[obj.loadedImages.selectedId].image;
        ImageRGBView view = { &obj.fovSelectedImage, &selectedImage };

        auto savePos = ImGui::GetCursorScreenPos();
        auto canvasSize = ImVec2(512, 512);

        ImGui::Image((void *)(intptr_t) view.image->texture.id, canvasSize,
                     ImVec2(view.fov->centerX - 0.5f*view.fov->sizeX,
                            view.fov->centerY - 0.5f*view.fov->sizeY),
                     ImVec2(view.fov->centerX + 0.5f*view.fov->sizeX,
                            view.fov->centerY + 0.5f*view.fov->sizeY),
                     ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 0));

        int signW = ::sign(ImGui::GetIO().MouseWheel);
        float mxCanvas = (ImGui::GetIO().MousePos.x - savePos.x)/canvasSize.x;
        float myCanvas = (ImGui::GetIO().MousePos.y - savePos.y)/canvasSize.y;

        if (signW) {
            float oldSX = view.fov->sizeX;
            float oldSY = view.fov->sizeY;

            view.fov->sizeX = std::min(1.0f, view.fov->sizeX*(1.0f - 0.1f*signW));
            view.fov->sizeY = std::min(1.0f, view.fov->sizeY*(1.0f - 0.1f*signW));

            view.fov->sizeX = std::max(0.25f, view.fov->sizeX);
            view.fov->sizeY = std::max(0.25f, view.fov->sizeY);

            view.fov->centerX = (mxCanvas - 0.5f)*(oldSX - view.fov->sizeX) + view.fov->centerX;
            view.fov->centerY = (myCanvas - 0.5f)*(oldSY - view.fov->sizeY) + view.fov->centerY;
        }

        {
            float diff = view.fov->centerX - 0.5f*view.fov->sizeX;
            if (diff < 0.0f) view.fov->centerX -= diff;
        }
        {
            float diff = view.fov->centerY - 0.5f*view.fov->sizeY;
            if (diff < 0.0f) view.fov->centerY -= diff;
        }
        {
            float diff = view.fov->centerX + 0.5f*view.fov->sizeX;
            if (diff >= 1.0f) view.fov->centerX -= (diff - 1.0f);
        }
        {
            float diff = view.fov->centerY + 0.5f*view.fov->sizeY;
            if (diff >= 1.0f) view.fov->centerY -= (diff - 1.0f);
        }
    }
    ImGui::End();

    return res;
}
