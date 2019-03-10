/*! \file ViewSelectedImage.cpp
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
bool Render::operator()<ViewSelectedImage>(ViewSelectedImage & obj, StateApp & state) {
    bool res = false;

    const auto & referenceId = state.viewLoadedImages.referenceId;
    const auto & selectedId = state.viewLoadedImages.selectedId;

    if (selectedId >= 0) {
        auto & overlayImage = state.images[(StateApp::EImage)(obj.overlayId)];

        auto & referenceImage = state.loadedImages[referenceId].image;
        auto & selectedImage = state.loadedImages[selectedId].image;

        ImageRGBView view = { &obj.fov, &selectedImage };

        auto canvasPos = ImGui::GetCursorScreenPos();
        auto canvasSize = ImGui::GetContentRegionAvail();

        auto drawList = ImGui::GetWindowDrawList();

        {
            auto renderOverlay = obj.showOverlay && ::IsValid()(overlayImage);

            auto savePos = ImGui::GetCursorScreenPos();
            ImGui::Image((void *)(intptr_t) view.image->texture.id, canvasSize,
                         ImVec2(view.fov->centerX - 0.5f*view.fov->sizeX,
                                view.fov->centerY - 0.5f*view.fov->sizeY),
                         ImVec2(view.fov->centerX + 0.5f*view.fov->sizeX,
                                view.fov->centerY + 0.5f*view.fov->sizeY),
                         ImColor(255, 255, 255, renderOverlay ? (int) (255*(1.0f - obj.alphaOverlay)) : 255), ImColor(255, 255, 255, 0));

            if (renderOverlay) {
                ImGui::SetCursorScreenPos(savePos);
                ImGui::Image((void *)(intptr_t) overlayImage.texture.id, canvasSize,
                             ImVec2(view.fov->centerX - 0.5f*view.fov->sizeX,
                                    view.fov->centerY - 0.5f*view.fov->sizeY),
                             ImVec2(view.fov->centerX + 0.5f*view.fov->sizeX,
                                    view.fov->centerY + 0.5f*view.fov->sizeY),
                             ImColor(255, 255, 255, (int) (255*obj.alphaOverlay)), ImColor(255, 255, 255, 0));
            }
        }

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
                ::Erase()(obj.commonPointInput, selectedId);
            }
        }

        if (ImGui::BeginPopupContextWindow("Options", 1)) {
            ImGui::Checkbox("Show grid", &obj.showGrid);
            ImGui::Checkbox("Show overlay", &obj.showOverlay);

            ImGui::SliderFloat("Alpha overlay", &obj.alphaOverlay, 0.0f, 1.0f);

            if (ImGui::BeginCombo("Overlay image", std::to_string(obj.overlayId).c_str())) {
                { bool isSelected = obj.overlayId == 0; if (ImGui::Selectable("Projected", isSelected)) obj.overlayId = 0; }
                { bool isSelected = obj.overlayId == 1; if (ImGui::Selectable("Difference (standard)", isSelected)) obj.overlayId = 1; }
                { bool isSelected = obj.overlayId == 2; if (ImGui::Selectable("Difference (local diff)", isSelected)) obj.overlayId = 2; }
                { bool isSelected = obj.overlayId == 3; if (ImGui::Selectable("Difference (hist diff)", isSelected)) obj.overlayId = 3; }
                { bool isSelected = obj.overlayId == 4; if (ImGui::Selectable("Difference (SSIM)", isSelected)) obj.overlayId = 4; }
                ImGui::EndCombo();
            }

            ImGui::EndPopup();
        }

        if (state.curAction == StateApp::None) {
            for (const auto & p : state.commonPoints) {
                if (::Exist()(p, selectedId) == false) continue;

                auto & posInImage = p.posInImage.at(selectedId);

                auto cxy = ::toCanvas(posInImage, canvasPos, canvasSize, *view.fov);
                drawList->AddCircle(cxy, 10.0f, ImGui::ColorConvertFloat4ToU32({0.0f, 1.0f, 0.0f, 1.0f}));
            }

            // tmp
            if (obj.showGrid) {
                int gn = 64;
                for (int gi = gn/4; gi < gn - gn/4; ++gi) {
                    float fy = (((float)(gi) + 0.5f)/gn);
                    for (int gj = gn/4; gj < gn - gn/4; ++gj) {
                        float fx = (((float)(gj) + 0.5f)/gn);

                        if (selectedId == referenceId) {
                            auto cxy = ::toCanvas({ fx, fy }, canvasPos, canvasSize, *view.fov);
                            drawList->AddCircle(cxy, 2.0f, ImGui::ColorConvertFloat4ToU32({0.0f, 1.0f, 1.0f, 1.0f}));
                        } else {
                            const auto & h = state.homographies[referenceId][selectedId];

                            float tx = (h[0]*fx + h[1]*fy + h[2])/(h[6]*fx + h[7]*fy + h[8]);
                            float ty = (h[3]*fx + h[4]*fy + h[5])/(h[6]*fx + h[7]*fy + h[8]);

                            auto cxy = ::toCanvas({ tx, ty }, canvasPos, canvasSize, *view.fov);
                            drawList->AddCircle(cxy, 2.0f, ImGui::ColorConvertFloat4ToU32({0.0f, 1.0f, 1.0f, 1.0f}));
                        }
                    }
                }
            }
        }

        if (state.curAction == StateApp::AddingCommonPoint || state.curAction == StateApp::EditingCommonPoint) {
            if (::Exist()(obj.commonPointInput, selectedId)) {
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

    return res;
}
