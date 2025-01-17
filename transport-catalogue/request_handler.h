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
            
            svg::Document RenderMap() const;

        private:
            // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
            const TransportCatalogue& catalogue_;
            const map_renderer::MapRenderer& renderer_;
        };

    } // namespace request_handler
} // namespace transport_catalogue {



