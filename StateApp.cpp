/*! \file StateApp.cpp
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

    ImVec2 toCanvas(const ::Point2D & p, const ImVec2 & canvasPos, const ImVec2 & canvasSize, const ::FieldOfView & fov ) {
        auto [cx, cy] = p;

        cx = (cx - fov.centerX + 0.5f*fov.sizeX)/fov.sizeX;
        cy = (cy - fov.centerY + 0.5f*fov.sizeY)/fov.sizeY;

        float x = canvasPos.x + cx*canvasSize.x;
        float y = canvasPos.y + cy*canvasSize.y;

        return { x, y };
    }
}

template <>
bool OnDragAndDrop::operator()<StateApp>(const char * fname, StateApp & obj) {
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
bool Render::operator()<StateApp>(StateApp & obj) {
    bool res = false;

    ImGui::SetNextWindowPos({ 0, 0 });
    ImGui::SetNextWindowSize({ obj.leftPanelSizeX, obj.loadedImagesSizeY });
    ImGui::Begin("Loaded images");
    {
        auto & images = obj.loadedImages;
        auto & selectedId = images.selectedId;
        for (int id = 0; id < (int) images.size(); ++id) {
            auto & item = images[id];
            ImGui::PushID(item.fname.c_str());
            if (obj.curAction == StateApp::LoadingImages) {
                if (ImGui::Button("Delete")) {
                    ::Free()(item);
                    images.erase(images.begin() + id);
                    selectedId = -1;
                    --id;
                    continue;
                }
                ImGui::SameLine();
            }
            if (id == selectedId) {
                ImGui::TextColored({1.0f, 1.0f, 0.0f, 1.0f}, "%s", item.fname.c_str());
            } else {
                if (obj.curAction == StateApp::AddingCommonPoint) {
                    if (::Exist()(id, obj.commonPointInput)) {
                        ImGui::TextColored({0.0f, 1.0f, 0.0f, 1.0f}, "%s", item.fname.c_str());
                    } else {
                        ImGui::TextColored({1.0f, 0.0f, 0.0f, 1.0f}, "%s", item.fname.c_str());
                    }
                } else {
                    ImGui::Text("%s", item.fname.c_str());
                }
                if (ImGui::IsItemHovered()) {
                    if (ImGui::IsMouseDown(0)) {
                        selectedId = id;
                    }
                }
            }
            ImGui::PopID();
        }
    }

    obj.leftPanelSizeX = ImGui::GetWindowSize().x;
    obj.loadedImagesSizeY = ImGui::GetWindowSize().y;
    ImGui::End();

    ImGui::SetNextWindowPos({ obj.leftPanelSizeX, 0 });
    ImGui::SetNextWindowSize({ ImGui::GetIO().DisplaySize.x - obj.leftPanelSizeX, ImGui::GetIO().DisplaySize.y });
    ImGui::Begin("Selected image");
    {
        const auto & selectedId = obj.loadedImages.selectedId;
        if (selectedId >= 0) {
            auto & selectedImage = obj.loadedImages[selectedId].image;
            ImageRGBView view = { &obj.fovSelectedImage, &selectedImage };

            auto canvasPos = ImGui::GetCursorScreenPos();
            auto canvasSize = ImGui::GetContentRegionAvail();

            auto drawList = ImGui::GetWindowDrawList();

            ImGui::Image((void *)(intptr_t) view.image->texture.id, canvasSize,
                         ImVec2(view.fov->centerX - 0.5f*view.fov->sizeX,
                                view.fov->centerY - 0.5f*view.fov->sizeY),
                         ImVec2(view.fov->centerX + 0.5f*view.fov->sizeX,
                                view.fov->centerY + 0.5f*view.fov->sizeY),
                         ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 0));

            if (ImGui::IsItemHovered()) {
                int signW = ::sign(ImGui::GetIO().MouseWheel);

                float mxCanvas = (ImGui::GetIO().MousePos.x - canvasPos.x)/canvasSize.x;
                float myCanvas = (ImGui::GetIO().MousePos.y - canvasPos.y)/canvasSize.y;

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

                if (ImGui::IsMouseClicked(0)) {
                    Point2D p = {
                        view.fov->centerX - 0.5f*view.fov->sizeX + mxCanvas*view.fov->sizeX,
                        view.fov->centerY - 0.5f*view.fov->sizeY + myCanvas*view.fov->sizeY,
                    };

                    obj.commonPointInput.posInImage[selectedId] = p;
                }

                if (ImGui::IsMouseClicked(1)) {
                    ::Erase()(selectedId, obj.commonPointInput);
                }
            }

            if (obj.curAction == StateApp::None) {
                for (const auto & p : obj.commonPoints) {
                    if (::Exist()(selectedId, p) == false) continue;

                    auto & posInImage = p.posInImage.at(selectedId);
                    auto cxy = ::toCanvas(posInImage, canvasPos, canvasSize, *view.fov);

                    drawList->AddCircle(cxy, 10.0f, ImGui::ColorConvertFloat4ToU32({0.0f, 1.0f, 0.0f, 1.0f}));
                }
            }

            if (obj.curAction == StateApp::AddingCommonPoint) {
                if (::Exist()(selectedId, obj.commonPointInput)) {
                    auto & posInImage = obj.commonPointInput.posInImage[selectedId];
                    auto cxy = ::toCanvas(posInImage, canvasPos, canvasSize, *view.fov);

                    drawList->AddCircle(cxy, 10.0f, ImGui::ColorConvertFloat4ToU32({0.0f, 1.0f, 0.0f, 1.0f}));

                    if (((int)(ImGui::GetTime()*10))%2 == 0) {
                        drawList->AddLine({ canvasPos.x, cxy.y }, { canvasPos.x + canvasSize.x, cxy.y }, ImGui::ColorConvertFloat4ToU32({0.0f, 1.0f, 0.0f, 1.0f}));
                        drawList->AddLine({ cxy.x, canvasPos.y }, { cxy.x, canvasPos.y + canvasSize.y }, ImGui::ColorConvertFloat4ToU32({0.0f, 1.0f, 0.0f, 1.0f}));
                    }
                }

            }
        }
    }
    ImGui::End();

    ImGui::SetNextWindowPos({ 0, obj.loadedImagesSizeY });
    ImGui::SetNextWindowSize({ obj.leftPanelSizeX, ImGui::GetIO().DisplaySize.y - obj.loadedImagesSizeY});
    ImGui::Begin("Actions");
    if (obj.curAction == StateApp::LoadingImages) {
        ImGui::PushTextWrapPos(ImGui::GetContentRegionAvailWidth());
        ImGui::Text("Load images to analyze by drag & dropping them in the application window.");
        if (obj.loadedImages.size() > 1) {
            if (ImGui::Button("READY")) {
                obj.curAction = StateApp::None;
            }
        }
        ImGui::PopTextWrapPos();
    }

    if (obj.curAction == StateApp::AddingCommonPoint) {
        ImGui::PushTextWrapPos(ImGui::GetContentRegionAvailWidth());
        ImGui::Text("Select the positions of a point that is visible in at least 2 images. "
                    "Right click on an image to clear the position in that image");

        {
            int nImages = obj.loadedImages.size();
            for (int i = 0; i < nImages; ++i) {
                if (::Exist()(i, obj.commonPointInput)) {
                    ImGui::Text("Image %d : %4.2f %4.2f", i, obj.commonPointInput.posInImage[i].x, obj.commonPointInput.posInImage[i].y);
                } else {
                    ImGui::Text("Image %d : ---", i);
                }
            }
        }
        ImGui::PopTextWrapPos();

        if (ImGui::Button("Cancel")) {
            obj.curAction = StateApp::None;
        }
        if (::Count()(obj.commonPointInput) > 1) {
            ImGui::SameLine();
            if (ImGui::Button("Add")) {
                obj.commonPoints.push_back(obj.commonPointInput);
                obj.curAction = StateApp::None;
            }
        }
    }

    if (obj.curAction == StateApp::None) {
        if (ImGui::Button("Add common point")) {
            obj.loadedImages.selectedId = 0;
            obj.curAction = StateApp::AddingCommonPoint;
        }

        {
            auto nPts = ::Count()(obj.commonPoints);
            ImGui::Text("Common points: %d", nPts);
            for (int i = 0; i < nPts; ++i) {
                ImGui::Text("   Point %d - %d image", i, ::Count()(obj.commonPoints[i]));
            }
        }

        if (ImGui::Button("Load common points")) {
            ::LoadFromFile()("CommonPoints.dat", obj.commonPoints);
        }

        if (::Count()(obj.commonPoints)) {
            if (ImGui::Button("Save common points")) {
                ::SaveToFile()("CommonPoints.dat", obj.commonPoints);
            }
        }
    }

    obj.leftPanelSizeX = ImGui::GetWindowSize().x;
    ImGui::End();

    return res;
}
