#pragma once

#include <iosfwd>
#include <string_view>
#include "transport_catalogue.h"

namespace transport_catalogue {
    namespace stat_reader {

        void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request, std::ostream& output);

    } // end of namespace stat_reader
} // end of namespace transport_catalogue