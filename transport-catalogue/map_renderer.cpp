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

        svg::Polyline MapRenderer::RenderRoutes(const domain::Bus* bus, SphereProjector& projector, size_t color_number) {
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

        svg::Document MapRenderer::RenderMap(const map<string_view, const domain::Bus*>* bus_list) {
            svg::Document map;

            auto comp = [] (const domain::Stop* lhs, const domain::Stop* rhs) {
                return lhs->name < rhs->name;
            };

            set<const domain::Stop*, decltype(comp)> stops(comp);
            for (const auto& bus : *bus_list) {
                stops.insert(bus.second->stops.begin(), bus.second->stops.end());
            }
            SphereProjector projector(stops.begin(), stops.end(), settings_.width, settings_.height, settings_.padding);
            size_t color_number = 0;
            for (const auto& bus : *bus_list) {
                if(bus.second->stops.empty()) continue;
                map.Add(RenderRoutes(bus.second, projector, color_number));
                ++color_number;
            }
            
            return map;
        }

    } // namespace map_renderer
} // namespace transport_catalogue