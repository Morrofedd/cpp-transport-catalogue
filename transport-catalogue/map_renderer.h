#pragma once

#include "domain.h"
#include "json_reader.h"

#include <algorithm>
#include <cstdlib>
#include <optional>

inline const double EPSILON = 1e-6;

static bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

class SphereProjector {
public:

    SphereProjector() = default;
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates


    SphereProjector(const SphereProjector& other) {
        padding_ = other.padding_;
        min_lon_ = other.min_lon_;
        max_lat_ = other.max_lat_;
        zoom_coeff_ = other.zoom_coeff_;
    }

    SphereProjector& operator= (const SphereProjector& other) {
        if (&other != this)  // избегаем самоприсваивания
        {
            padding_ = other.padding_;
            min_lon_ = other.min_lon_;
            max_lat_ = other.max_lat_;
            zoom_coeff_ = other.zoom_coeff_;
        }
        return *this;
    }

    explicit SphereProjector(geo::MinMaxCoords minmax,
        double max_width, double max_height, double padding)
        : padding_(padding), min_lon_(minmax.min.lng), max_lat_(minmax.max.lat) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (minmax.max == minmax.min) {
            return;
        }

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(minmax.max.lng - minmax.min.lng)) {
            width_zoom = (max_width - 2 * padding) / (minmax.max.lng - minmax.min.lng);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(minmax.max.lat - minmax.min.lat)) {
            height_zoom = (max_height - 2 * padding) / (minmax.max.lat - minmax.min.lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

enum LayerType {
    UPPER,
    LOWER
};

class map_renderer
{
public:
    map_renderer() = delete;
    map_renderer(const TransportCatalogue::TransportCatalogue&, const json_reader&);

    void MapRander(std::ostream& output);

private:
    svg::Text MakeText(std::string, int, LayerType, RequestType);
    svg::Document MakeRenderDocument();
    std::set<std::string_view> MakeRoute(svg::Document&, std::string_view, int);
    void MakeNameOfRoute(svg::Document&, std::string_view, int);
    void MakeNameOfStop(svg::Document&, std::string_view);
    void MakeCircle(std::string_view,svg::Document&);

private:
    SphereProjector sp_;
    const TransportCatalogue::TransportCatalogue& catalogue_;
    const Settings settings_;
};

