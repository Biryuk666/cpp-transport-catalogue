#pragma once

#include <iostream>
#include "json.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include <vector>

namespace transport_catalogue {
    namespace json_reader {

        enum class RequestType {
            BUS,
            MAP,
            ROUTE,
            STOP,
            WTF
        };

        RequestType GetRequestType(std::string_view request);

        struct StatRequest {
            int id = 0;
            RequestType type;
            std::string name;
            std::string from;
            std::string to;
        };

        class JsonReader {
        public:
            
        void RequestProcess(TransportCatalogue& catalogue, std::istream& input, std::ostream& out, map_renderer::MapRenderer& renderer, const request_handler::RequestHandler& handler, transport_router::TransportRouter& router);

        private:
            std::vector<StatRequest> stat_requests;
        };

        void TestInnerFunction();

    } // namespace json_reader
} // namespace transport_catalogue