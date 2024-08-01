//
// Created by Viktor Aylor on 14/07/2024.
//

#ifndef TESTIMGUIRENDERER_H
#define TESTIMGUIRENDERER_H
#include "XTPVulkan.h"
#include <deque>

#include "imgui.h"
#include "implot.h"

#ifdef XTP_USE_IMGUI_UI
#ifdef XTP_USE_ADVANCED_TIMING
class TestImGuiRenderer {
public:
    static ImFont* font;
    static std::deque<double> past60Frames;
    static std::deque<std::tuple<double, double, double>> past60FrameTimes;
    static constexpr int graphLengthSeconds = 60;

    static void init();

    static void cleanUp();

    static void drawLineThroughPlotAtMouseX();

    template <typename T> static void drawLinePlot(const std::string& name, const std::deque<T>& vals, bool shouldDrawTooltip);

    static void drawTooltip(const double *values, int count, int offset, int stride = sizeof(double), bool drawForZeroValues = false);

    static ImPlotPoint getPlotHeightAtPoint(const double*vals, int count, int offset, int stride, int idx);

    static void TransformMarker(ImVec2* points, int n, const ImVec2& c, float s) {
        for (int i = 0; i < n; ++i) {
            points[i].x = c.x + points[i].x * s;
            points[i].y = c.y + points[i].y * s;
        }
    }

    static void MarkerGeneral(ImDrawList* DrawList, ImVec2* points, int n, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
        TransformMarker(points, n, c, s);
        if (fill)
            DrawList->AddConvexPolyFilled(points, n, col_fill);
        if (outline && !(fill && col_outline == col_fill)) {
            for (int i = 0; i < n; ++i)
                DrawList->AddLine(points[i], points[(i+1)%n], col_outline, weight);
        }
    }

    static void MarkerCircle(ImDrawList* DrawList, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
        ImVec2 marker[10] = {ImVec2(1.0f, 0.0f),
                             ImVec2(0.809017f, 0.58778524f),
                             ImVec2(0.30901697f, 0.95105654f),
                             ImVec2(-0.30901703f, 0.9510565f),
                             ImVec2(-0.80901706f, 0.5877852f),
                             ImVec2(-1.0f, 0.0f),
                             ImVec2(-0.80901694f, -0.58778536f),
                             ImVec2(-0.3090171f, -0.9510565f),
                             ImVec2(0.30901712f, -0.9510565f),
                             ImVec2(0.80901694f, -0.5877853f)};
        MarkerGeneral(DrawList, marker, 10, c, s, outline, col_outline, fill, col_fill, weight);
    }

    static int posMod(int l, int r);


    static void render();
};

#endif
#endif


#endif //TESTIMGUIRENDERER_H
