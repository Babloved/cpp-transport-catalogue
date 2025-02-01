#include "map_renderer.h"
// points_begin и points_end задают начало и конец интервала элементов geo::Coordinates


bool renderer::IsZero(double value){
return std::abs(value) < renderer::EPSILON;
}

template<typename PointInputIt>
renderer::SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                                           double max_width, double max_height, double padding)
        : padding_(padding) //
{
    // Если точки поверхности сферы не заданы, вычислять нечего
    if (points_begin == points_end){
        return;
    }

    // Находим точки с минимальной и максимальной долготой
    const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs){ return lhs.lng < rhs.lng; });
    min_lon_ = left_it->lng;
    const double max_lon = right_it->lng;

    // Находим точки с минимальной и максимальной широтой
    const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs){ return lhs.lat < rhs.lat; });
    const double min_lat = bottom_it->lat;
    max_lat_ = top_it->lat;

    // Вычисляем коэффициент масштабирования вдоль координаты x
    std::optional<double> width_zoom;
    if (!IsZero(max_lon - min_lon_)){
        width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
    }

    // Вычисляем коэффициент масштабирования вдоль координаты y
    std::optional<double> height_zoom;
    if (!IsZero(max_lat_ - min_lat)){
        height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
    }
    if (width_zoom && height_zoom){
        // Коэффициенты масштабирования по ширине и высоте ненулевые,
        // берём минимальный из них
        zoom_coeff_ = std::min(*width_zoom, *height_zoom);
    } else if (width_zoom){
        // Коэффициент масштабирования по ширине ненулевой, используем его
        zoom_coeff_ = *width_zoom;
    } else if (height_zoom){
        // Коэффициент масштабирования по высоте ненулевой, используем его
        zoom_coeff_ = *height_zoom;
    }
}

svg::Document renderer::MapRenderer::RenderDocument(){
    return svg::Document();
}
