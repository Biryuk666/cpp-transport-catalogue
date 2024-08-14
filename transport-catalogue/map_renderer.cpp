#include "map_renderer.h"
#include <set>

using namespace std;

namespace transport_catalogue {
    namespace map_renderer {

        bool IsZero(double value) {
            return std::abs(value) < EPSILON;
        }

        void MapRenderer::SetSetting(RenderSettings&& settings) {
                settings_ = std::move(settings);
            }

        svg::Polyline MapRenderer::RenderRoutes(const domain::Bus* bus, SphereProjector& projector, size_t color_number) const {
            svg::Polyline route;
            for (const auto& stop : bus->stops) {
                route.AddPoint(projector(stop->coordinates));
            }
            if (!bus->is_roundtrip) {
                for (int i = static_cast<int>(bus->stops.size()) - 2; i >= 0; --i) {
                    route.AddPoint(projector(bus->stops[static_cast<size_t>(i)]->coordinates));
                }
            }
            route.SetFillColor("none");
            route.SetStrokeColor(settings_.color_palette[color_number % settings_.color_palette.size()]);
            route.SetStrokeWidth(settings_.line_width);
            route.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            route.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            return route;
        }

        svg::Text MapRenderer::RenderRouteName(const domain::Bus* bus, svg::Point stop_coordinates, size_t color_number = 0) const {
            svg::Text route_name;
            route_name.SetData(bus->name);
            route_name.SetPosition({stop_coordinates.x, stop_coordinates.y});
            route_name.SetOffset(settings_.bus_label_offset);
            route_name.SetFontSize(static_cast<uint32_t>(settings_.bus_label_font_size));
            route_name.SetFontFamily("Verdana"s);
            route_name.SetFontWeight("bold"s);
            route_name.SetFillColor(settings_.color_palette[color_number % settings_.color_palette.size()]);

            return route_name;
        }

        svg::Text MapRenderer::RenderRouteNameBase(const domain::Bus* bus, svg::Point stop_coordinates) const {
            svg::Text route_name_base = RenderRouteName(bus, stop_coordinates);
            route_name_base.SetFillColor(settings_.underlayer_color);
            route_name_base.SetStrokeColor(settings_.underlayer_color);
            route_name_base.SetStrokeWidth(settings_.underlayer_width);
            route_name_base.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            route_name_base.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            return route_name_base;
        }

        svg::Circle MapRenderer::RenderStopCircle(const domain::Stop* stop, SphereProjector& projector) const {
            svg::Circle stop_circle;
            stop_circle.SetCenter(projector(stop->coordinates));
            stop_circle.SetRadius(settings_.stop_radius);
            stop_circle.SetFillColor("white"s);

            return stop_circle;
        }

        svg::Text MapRenderer::RenderStopName(const domain::Stop* stop, SphereProjector& projector) const {
            svg::Text stop_name;
            stop_name.SetData(stop->name);
            stop_name.SetPosition(projector(stop->coordinates));
            stop_name.SetOffset(settings_.stop_label_offset);
            stop_name.SetFontSize(static_cast<uint32_t>(settings_.stop_label_font_size));
            stop_name.SetFontFamily("Verdana"s);
            stop_name.SetFillColor("black");

            return stop_name;
        }

        svg::Text MapRenderer::RenderStopNameBase(const domain::Stop* stop, SphereProjector& projector) const {
            svg::Text stop_name_base = RenderStopName(stop, projector);
            stop_name_base.SetFillColor(settings_.underlayer_color);
            stop_name_base.SetStrokeColor(settings_.underlayer_color);
            stop_name_base.SetStrokeWidth(settings_.underlayer_width);
            stop_name_base.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            stop_name_base.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            return stop_name_base;
        }

        svg::Document MapRenderer::RenderMap(const map<string_view, const domain::Bus*>* bus_list) const {
            svg::Document map;

            auto comp = [] (const domain::Stop* lhs, const domain::Stop* rhs) {
                return lhs->name < rhs->name;
            };

            set<const domain::Stop*, decltype(comp)> stops_list(comp);
            for (const auto& bus : *bus_list) {
                stops_list.insert(bus.second->stops.begin(), bus.second->stops.end());
            }
            SphereProjector projector(stops_list.begin(), stops_list.end(), settings_.width, settings_.height, settings_.padding);
            size_t color_number = 0;
            for (const auto& bus : *bus_list) {
                if(bus.second->stops.empty()) continue;
                map.Add(RenderRoutes(bus.second, projector, color_number));
                ++color_number;
            }

            color_number = 0;
            for (const auto& bus : *bus_list) {
                auto stops = bus.second->stops;
                if(stops.empty()) continue;
                map.Add(RenderRouteNameBase(bus.second, projector(stops[0]->coordinates)));
                map.Add(RenderRouteName(bus.second, projector(stops[0]->coordinates), color_number));
                if (!bus.second->is_roundtrip && stops[0] != stops[stops.size() - 1]) {
                    map.Add(RenderRouteNameBase(bus.second, projector(stops[stops.size() - 1]->coordinates)));
                    map.Add(RenderRouteName(bus.second, projector(stops[stops.size() - 1]->coordinates), color_number));
                }
                ++color_number;
            }
            
            for (const auto& stop : stops_list) {
                map.Add(RenderStopCircle(stop, projector));
            }

            for (const auto& stop : stops_list) {
                map.Add(RenderStopNameBase(stop, projector));
                map.Add(RenderStopName(stop, projector));
            }
            
            return map;
        }

    } // namespace map_renderer
} // namespace transport_catalogue