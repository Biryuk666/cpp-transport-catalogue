#pragma once

#include <cmath>

namespace transport_catalogue {
    namespace geo {

        struct Coordinates {
            double lat;
            double lng;
            bool operator==(const Coordinates& other) const;
            bool operator!=(const Coordinates& other) const;
        };

        double ComputeDistance(Coordinates from, Coordinates to);

    } // namespace geo
} // namespace transpotr_catalogue