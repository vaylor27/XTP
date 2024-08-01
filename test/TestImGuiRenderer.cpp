//
// Created by Viktor Aylor on 14/07/2024.
//

#include "TestImGuiRenderer.h"

#include "Camera.h"
#include "imgui_internal.h"
#include "implot.h"
#include "implot_internal.h"
#include "TimeManager.h"
#include "XTPVulkan.h"
#include "renderable/MergedMeshRenderable.h"

#ifdef XTP_USE_IMGUI_UI
#ifdef XTP_USE_ADVANCED_TIMING
ImFont* TestImGuiRenderer::font;
std::deque<std::tuple<double, double, double>> TestImGuiRenderer::past60FrameTimes;
std::deque<double> TestImGuiRenderer::past60Frames;

void TestImGuiRenderer::init() {
    ImPlot::CreateContext();
    ImGui::GetIO().Fonts->AddFontFromFileTTF("./assets/Roboto-Regular.ttf", 13);
}

void TestImGuiRenderer::cleanUp() {
    ImPlot::DestroyContext();
}

void TestImGuiRenderer::drawLineThroughPlotAtMouseX() {
    //Draw A Vertical Line Through The Plot
    ImPlotContext* gp = ImPlot::GetCurrentContext();
    ImDrawList* drawList = ImPlot::GetPlotDrawList();

    //Set up line dimensions
    const ImVec2 xy = ImGui::GetIO().MousePos;
    const ImVec2 v1(xy.x, gp->CurrentPlot->PlotRect.Min.y);
    const ImVec2 v4(xy.x, gp->CurrentPlot->PlotRect.Max.y);

    //Draw The Line
    drawList->AddLine(v1, v4, ImGui::GetColorU32(ImVec4((.83137254902F), (0.0F), (0.0F), (1.0F))), 0.5);

    //Write Test (X Coord) To Char Buffer
    char buffer[128] = {};
    const double range_x = gp->CTicker.Ticks.Size > 1 ? (gp->CTicker.Ticks[1].PlotPos - gp->CTicker.Ticks[0].PlotPos) : gp->CurrentPlot->XAxis(0).Range.Size();
    snprintf(buffer, 128, "%.*f", ImPlot::Precision(range_x), ImPlot::GetPlotMousePos().x);

    //Compute Text Position And Width
    constexpr float txt_off = 5;
    const auto pos = ImVec2{ xy.x, gp->CurrentPlot->PlotRect.Max.y + txt_off };
    const auto labelWidth = ImGui::CalcTextSize(buffer, nullptr, true);

    //Compute Test Area
    const auto tooltip_content_bb = ImRect({pos.x - 1, pos.y - 1}, {pos.x + labelWidth.x + 1, pos.y + labelWidth.y + 1});

    //Draw Rectangle Around Text
    drawList->AddRectFilled(tooltip_content_bb.Min, tooltip_content_bb.Max, ImGui::GetColorU32(ImVec4((.83137254902F), (0.0F), (0.0F), (1.0F))));

    //Draw Text
    drawList->AddText(pos, IM_COL32_WHITE, buffer);
}

template <typename T> void TestImGuiRenderer::drawLinePlot(const std::string& name, const std::deque<T>& vals, bool shouldDrawTooltip) {
    ImPlot::PlotLine(name.c_str(), &vals[0], static_cast<int>(vals.size()), 1, 0, 0, 0);
    if (shouldDrawTooltip) {
        drawTooltip(&vals[0], vals.size(), 0, sizeof(vals[0])) ;
    }
}

void TestImGuiRenderer::drawTooltip(const double* values, const int count, const int offset, const int stride, bool drawForZeroValues) {
    double mouseX = ImPlot::GetPlotMousePos().x;
    ImPlotPoint pos = getPlotHeightAtPoint(values, count, offset, stride, static_cast<int>(round(mouseX)));
    if (!drawForZeroValues && pos.y <= 0) {
        return;
    }
    ImVec2 posVec =     ImPlot::PlotToPixels(pos);

    ImVec4 cc = ImPlot::GetLastItemColor();
    ImU32 col = IM_COL32(cc.x * 255, cc.y * 255, cc.z * 255, cc.w * 255);
    // get max width
    char label[64];
    snprintf(label, 64, "%.2f", static_cast<float>(pos.y));
    ImVec2 labelWidth = ImGui::CalcTextSize(label, nullptr, true);
    ImRect tooltip_content_bb = ImRect({posVec.x - 1, posVec.y - 1}, {posVec.x + labelWidth.x + 1, posVec.y + labelWidth.y + 1});
    // ImPlot::GetPlotDrawList()->AddCircleFilled(posVec, 2, col, 4);
    MarkerCircle(ImPlot::GetPlotDrawList(), posVec, 4, true, col, true, col, 2);

    ImPlot::GetPlotDrawList()->AddRectFilled(tooltip_content_bb.Min, tooltip_content_bb.Max, col);
    ImPlot::GetPlotDrawList()->AddText(posVec, ImPlot::CalcTextColor(ImGui::ColorConvertU32ToFloat4(col)), label);
}

//getter((T) idx, OffsetAndStride(vals, idx, count, offset, stride))

// return *(const T*)(const void*)((const unsigned char*)data + (size_t)PosMod(PosMod(offset, count) + idx, count) * stride);

// PosMod((l % r + r) % r)
ImPlotPoint TestImGuiRenderer::getPlotHeightAtPoint(const double* vals, const int count, const int offset, const int stride, int idx) {
    return {static_cast<double>(idx), *(const double*)(const void*)((const unsigned char*)vals + (size_t)posMod(posMod(offset, count) + idx, count) * stride)};
}

int TestImGuiRenderer::posMod(int l, int r) {
    return (l % r + r) % r;
}

void TestImGuiRenderer::render() {
    const uint64_t renderTime = XTPVulkan::renderTimeNanos[XTPVulkan::mostRecentFrameRendered];
    const long double cpuTime = static_cast<long double>(TimeManager::getDeltaTimeMilis()) / 1000000;
    const long double gpuTime = static_cast<long double>(renderTime) / 1000000.0;
    const long double totalTime = cpuTime + gpuTime;


    ImGui::Begin("XTP Debug Tool");

    if (ImGui::CollapsingHeader("Frame Info")) {
        ImGui::Indent(15);
        ImGui::Text(("Current FPS (60 Frame Average): " + std::to_string(1000 / totalTime)).c_str());

        ImGui::Text(("GPU Time (ms): " + (renderTime == -1 ? "Unavailable": std::to_string(gpuTime))).c_str());
        ImGui::Text(("CPU Time (ms): " + (renderTime == -1 ? "Unavailable": std::to_string(cpuTime))).c_str());
        ImGui::Text(("Total Frame Time (ms): " + std::to_string(totalTime)).c_str());

        ImGui::Unindent(15);
    }
    ImGui::End();
}
#endif
#endif