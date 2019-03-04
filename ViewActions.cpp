/*! \file ViewActions.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "Types.h"
#include "Functions.h"

#include "ggimg/ggimg.h"

#include "imgui/imgui.h"

template <>
bool Render::operator()<ViewActions>(ViewActions & obj, StateApp & state) {
    bool res = false;

    if (state.curAction == StateApp::LoadingImages) {
        ImGui::PushTextWrapPos(ImGui::GetContentRegionAvailWidth());
        ImGui::Text("Load images to analyze by drag & dropping them in the application window.");
        if (state.loadedImages.size() > 1) {
            if (ImGui::Button("READY")) {
                state.curAction = StateApp::None;
            }
        }
        ImGui::PopTextWrapPos();
    }

    if (state.curAction == StateApp::AddingCommonPoint || state.curAction == StateApp::EditingCommonPoint) {
        ImGui::PushTextWrapPos(ImGui::GetContentRegionAvailWidth());
        ImGui::Text("Select the positions of a point that is visible in at least 2 images. "
                    "Right click on an image to clear the position in that image");

        {
            int nImages = state.loadedImages.size();
            for (int i = 0; i < nImages; ++i) {
                if (::Exist()(state.viewSelectedImage.commonPointInput, i)) {
                    ImGui::Text("Image %d : %4.2f %4.2f", i, state.viewSelectedImage.commonPointInput.posInImage[i].x, state.viewSelectedImage.commonPointInput.posInImage[i].y);
                } else {
                    ImGui::Text("Image %d : ---", i);
                }
            }
        }
        ImGui::PopTextWrapPos();

        if (ImGui::Button("Cancel")) {
            state.curAction = StateApp::None;
        }
        if (::Count()(state.viewSelectedImage.commonPointInput) > 1) {
            ImGui::SameLine();
            if (state.curAction == StateApp::AddingCommonPoint) {
                if (ImGui::Button("Add")) {
                    state.commonPoints.push_back(state.viewSelectedImage.commonPointInput);
                    state.curAction = StateApp::None;
                }
            } else {
                if (ImGui::Button("Edit")) {
                    state.commonPoints[state.viewLoadedImages.editId] = state.viewSelectedImage.commonPointInput;
                    state.curAction = StateApp::None;
                }
            }
        }
    }

    if (state.curAction == StateApp::None) {
        auto & referenceId = state.viewLoadedImages.referenceId;
        auto & selectedId = state.viewLoadedImages.selectedId;

        if (ImGui::Button("Add common point")) {
            state.viewLoadedImages.selectedId = 0;
            state.curAction = StateApp::AddingCommonPoint;
        }

        {
            auto nPts = ::Count()(state.commonPoints);
            ImGui::Text("Common points: %d", nPts);
            for (int i = 0; i < nPts; ++i) {
                ImGui::PushID(i);
                if (ImGui::SmallButton("Edit")) {
                    state.viewLoadedImages.editId = i;
                    state.viewSelectedImage.commonPointInput = state.commonPoints[i];
                    state.curAction = StateApp::EditingCommonPoint;
                }
                ImGui::SameLine();
                ImGui::Text("   Point %d - %d image(s)", i, ::Count()(state.commonPoints[i]));
                ImGui::PopID();
            }
        }

        if (ImGui::Button("Load common points")) {
            ::LoadFromFile()(state.commonPoints, "CommonPoints.dat");
        }

        if (::Count()(state.commonPoints) > 0) {
            if (ImGui::Button("Save common points")) {
                ::SaveToFile()(state.commonPoints, "CommonPoints.dat");
            }
        }

        ImGui::Separator();

        ImGui::Text("Reference image: %d", referenceId);

        if (ImGui::Button("Set reference image")) {
            referenceId = selectedId;
        }

        if (::Count()(state.commonPoints) > 0) {
            if (ImGui::Button("Calculate homographies")) {
                int nImages = state.loadedImages.size();
                for (int i = 0; i < nImages; ++i) {
                    for (int j = 0; j < nImages; ++j) {
                        if (i == j) continue;

                        int np = 0;
                        std::array<Point2D, 4> pi;
                        std::array<Point2D, 4> pj;
                        for (auto & p : state.commonPoints) {
                            if (::Exist()(p, i) && ::Exist()(p, j)) {
                                pi[np] = p.posInImage[i];
                                pj[np] = p.posInImage[j];
                                ++np;
                            }

                            if (np == 4) break;
                        }

                        if (np == 4) {
                            auto homography = ::ComputeHomography()(pi, pj);
                            printf("Homography %d -> %d\n", i, j);
                            printf("    %6.2f %6.2f %6.2f\n", homography[0], homography[1], homography[2]);
                            printf("    %6.2f %6.2f %6.2f\n", homography[3], homography[4], homography[5]);
                            printf("    %6.2f %6.2f %6.2f\n", homography[6], homography[7], homography[8]);
                            printf("\n");

                            state.homographies[i][j] = homography;
                        }
                    }
                }
            }
        }

        ImGui::Checkbox("Show grid", &state.viewSelectedImage.showGrid);
        ImGui::Checkbox("Show projected", &state.viewSelectedImage.showProjected);

        if (ImGui::Button("Project")) {
            const auto & referenceImage = state.loadedImages[referenceId].image;
            const auto & selectedImage = state.loadedImages[selectedId].image;

            auto & projectedImage = state.projectedImage;
            auto & homography = state.homographies[selectedId][referenceId];

            int nx = referenceImage.nx;
            int ny = referenceImage.ny;

            ::Resize()(projectedImage, nx, ny);
            ::ggimg::transform_homography_rgb_nn(nx, ny, referenceImage.pixels.data(), homography, nx, ny, projectedImage.pixels.data());
            ::GenerateTexture()(projectedImage, true);
        }

    }

    state.leftPanelSizeX = ImGui::GetWindowSize().x;

    return res;
}
