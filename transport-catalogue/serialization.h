#pragma once

#include <iostream>
#include <string>

#include <svg.pb.h>
#include <map_renderer.pb.h>
#include <transport_catalogue.pb.h>

#include "map_renderer.h"
#include "request_handler.h"
#include "svg.h"
#include "transport_catalogue.h"

namespace transport_catalogue {
    namespace serialization {

        struct SerializationSettings {
            std::string file_name;
        };

        class Serializator {
        public:
            void SetSettings(std::string&& file_name);

            void SerializeData(const TransportCatalogue& catalogue, const map_renderer::MapRenderer& renderer);
            void DeserializeFile(TransportCatalogue& catalogue, map_renderer::MapRenderer& renderer);

        private:
            SerializationSettings settings_;

            transport_catalogue_proto::CatalogueData GetCatalogueDataForSerialization(const TransportCatalogue& catalogue);
            transport_catalogue_proto::RenderSettings GetRenderSettingsDataForSerialization(const map_renderer::MapRenderer& renderer);
            TransportCatalogue DeserializeCotalogueData (const transport_catalogue_proto::CatalogueData& catalogue_data);
            map_renderer::RenderSettings DeserializeRenderSettingsData (const transport_catalogue_proto::RenderSettings& render_settings_data);
        };        

    } // namespace serialization
} // namespace transport_catalogue