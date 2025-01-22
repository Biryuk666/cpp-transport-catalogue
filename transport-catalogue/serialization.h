#pragma once

#include <iostream>
#include <string>
#include <unordered_map>

#include <graph.pb.h>
#include <map_renderer.pb.h>
#include <svg.pb.h>
#include <router.pb.h>
#include <transport_catalogue.pb.h>
#include <transport_router.pb.h>

#include "graph.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "router.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace transport_catalogue {
    namespace serialization {

        struct SerializationSettings {
            std::string file_name;
        };

        class Serializator {
        public:
            void SetSettings(std::string&& file_name);

            void SerializeData(const TransportCatalogue& catalogue, const map_renderer::MapRenderer& renderer, const transport_router::TransportRouter& router);
            void DeserializeFile(TransportCatalogue& catalogue, map_renderer::MapRenderer& renderer, transport_router::TransportRouter& router);

        private:
            SerializationSettings settings_;
            std::unordered_map<size_t, std::string> stop_id_to_stop_name_, bus_id_to_bus_name_;
            std::unordered_map<std::string, size_t> stop_name_to_stop_id_, bus_name_to_bus_id_;


            transport_catalogue_proto::CatalogueData GetCatalogueDataForSerialization(const TransportCatalogue& catalogue);
            transport_catalogue_proto::RenderSettingsData GetRenderSettingsDataForSerialization(const map_renderer::MapRenderer& renderer) const;
            transport_catalogue_proto::RouterData GetRouterDataForSerialization(const transport_router::TransportRouter& router);
            TransportCatalogue DeserializeCotalogueData (const transport_catalogue_proto::CatalogueData& catalogue_data);
            map_renderer::RenderSettings DeserializeRenderSettingsData (const transport_catalogue_proto::RenderSettingsData& render_settings_data);
            void DeserializeRouterData(const transport_catalogue_proto::RouterData& router_data, const TransportCatalogue& catalogue, transport_router::TransportRouter& router);
        };        

    } // namespace serialization
} // namespace transport_catalogue