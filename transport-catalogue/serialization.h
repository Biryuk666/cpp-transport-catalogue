#pragma once

#include <iostream>
#include <string>
#include <transport_catalogue.pb.h>

#include "transport_catalogue.h"

namespace transport_catalogue {
    namespace serialization {

        struct SerializationSettings {
            std::string file_name;
        };

        void SerializeData(const transport_catalogue::TransportCatalogue& catalogue, const SerializationSettings& settings);

        transport_catalogue::TransportCatalogue DeserializeFile(std::istream& input);

    } // namespace serialization
} // namespace transport_catalogue