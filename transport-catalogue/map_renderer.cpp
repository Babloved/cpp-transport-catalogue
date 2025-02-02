#include "map_renderer.h"
// points_begin и points_end задают начало и конец интервала элементов geo::Coordinates

using namespace std;

bool renderer::IsZero(double value){
    return std::abs(value) < renderer::EPSILON;
}

svg::Polyline renderer::MapRenderer::RenderPathLine(const Path &path, const SphereProjector &proj, const svg::Color &color) const{
    svg::Polyline polyline;
    for (const auto &stop: path.ordered_stops_){
        polyline.AddPoint(proj(stop->coordinates_));
    }
    if (!path.path_looped_){
        for (auto it = next(path.ordered_stops_.rbegin()); it != path.ordered_stops_.rend(); ++it){
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

std::vector<svg::Text>
renderer::MapRenderer::RenderPathName(const Path &path, const SphereProjector &proj, const svg::Color &fill_color) const{
    std::vector<svg::Text> texts;
    svg::Text first_stop;
    first_stop.SetPosition(proj(path.ordered_stops_.front()->coordinates_));
    //Основной текст
    texts.push_back(std::move(first_stop));
    //Подложка
    texts.push_back(texts.back());
    if (!path.path_looped_){
        svg::Text last_stop;
        last_stop.SetPosition(proj(path.ordered_stops_.back()->coordinates_));
        //Основной текст для конечной остановки
        texts.push_back(std::move(last_stop));
        //Подложка для конечной остановки
        texts.push_back(texts.back());
    }
    for (auto &text: texts){
        text.SetOffset({settings_.bus_label_offset_.first, settings_.bus_label_offset_.second})
                .SetFontSize(settings_.bus_label_font_size_)
                .SetFontFamily("Verdana")
                .SetFontWeight("bold")
                .SetData(path.path_name_);
    }
    for (size_t i = 0; i < texts.size(); i += 2){
        texts.at(i).SetFillColor(settings_.underlayer_color_)
                .SetStrokeColor(settings_.underlayer_color_)
                .SetStrokeWidth(settings_.underlayer_width_)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    }
    for (size_t i = 1; i < texts.size(); i += 2){
        texts.at(i).SetFillColor(fill_color);
    }
    return texts;
}

svg::Circle
renderer::MapRenderer::RenderStopCircle(const Stop &stop, const SphereProjector &proj) const{
    svg::Circle circle;
    circle.SetCenter(proj(stop.coordinates_))
            .SetRadius(settings_.stop_radius_)
            .SetFillColor("white");
    return circle;
}

const renderer::MapRenderer::RenderSettings &renderer::MapRenderer::GetRenderSettings() const{
    return settings_;
}

std::vector<svg::Text>
renderer::MapRenderer::RenderStopsName(const Stop &stop, const SphereProjector &proj) const{
    std::vector<svg::Text> texts;
    svg::Text first_stop;
    first_stop.SetPosition(proj(stop.coordinates_));
    //Основной текст
    texts.push_back(std::move(first_stop));
    //Подложка
    texts.push_back(texts.back());
    for (auto &text: texts){
        text.SetOffset({settings_.stop_label_offset_.first, settings_.stop_label_offset_.second})
                .SetFontSize(settings_.stop_label_font_size_)
                .SetFontFamily("Verdana")
                .SetData(stop.stop_name_);
    }
    texts.at(0).SetFillColor(settings_.underlayer_color_)
            .SetStrokeColor(settings_.underlayer_color_)
            .SetStrokeWidth(settings_.underlayer_width_)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    texts.at(1).SetFillColor("black");
    return texts;
}