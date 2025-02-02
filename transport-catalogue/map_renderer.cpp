#include "map_renderer.h"
// points_begin и points_end задают начало и конец интервала элементов geo::Coordinates

using namespace std;

bool renderer::IsZero(double value){
    return std::abs(value) < renderer::EPSILON;
}

svg::Polyline renderer::MapRenderer::RenderPath(const Path &path, const SphereProjector &proj, const svg::Color &color) const{
    svg::Polyline polyline;
    for (const auto &stop: path.ordered_stops_){
        polyline.AddPoint(proj(stop->coordinates_));
    }
    if (!path.path_looped_){
        for (auto it = next(path.ordered_stops_.rbegin()); it != path.ordered_stops_.rend(); ++it) {
            polyline.AddPoint(proj((*it)->coordinates_));
        }
    }
    polyline
            .SetStrokeColor(color)
            .SetFillColor(svg::NoneColor)
            .SetStrokeWidth(settings_.line_width_)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    return polyline;
}

const renderer::MapRenderer::RenderSettings &renderer::MapRenderer::GetRenderSettings() const{
    return settings_;
}
