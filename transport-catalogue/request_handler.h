#pragma once

#include "map_renderer.h"
#include <string_view>
#include "transport_catalogue.h"
#include <optional>
#include <unordered_set>

namespace transport_catalogue {
    namespace request_handler {

        class RequestHandler {
        public:
            RequestHandler(const TransportCatalogue& catalogue, const map_renderer::MapRenderer& renderer);

            /*// Возвращает информацию о маршруте (запрос Bus)
            std::optional<domain::RouteInfo> GetBusStat(const std::string_view& bus_name) const;

            // Возвращает маршруты, проходящие через
            const std::unordered_set<const domain::Bus*>* GetBusesByStop(const std::string_view& stop_name) const;*/
            
            svg::Document RenderMap() const;

        private:
            // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
            const TransportCatalogue& catalogue_;
            const map_renderer::MapRenderer& renderer_;
        };

    } // namespace request_handler
} // namespace transport_catalogue {



