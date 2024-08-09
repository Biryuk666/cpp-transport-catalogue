#pragma once

#include "geo.h"
#include <string>
#include <vector>

namespace transport_catalogue {
    namespace domain {

        struct Stop {
            std::string name;
            geo::Coordinates coordinates;
        };

        struct Bus {
            std::string name;
            std::vector<const Stop*> stops;
            bool is_roundtrip;
        };

        struct RouteInfo {
            int total_stops_number;
            int unique_stops_number;
            int route_length;
            double route_curvature;
        };
        
    } // namespace domain
} // namespace transport_catalogue