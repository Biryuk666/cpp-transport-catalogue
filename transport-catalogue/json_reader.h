#pragma once

#include <iostream>
#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include <vector>

namespace transport_catalogue {
    namespace json_reader {

        enum class RequestType {
            BUS,
            MAP,
            STOP,
            WTF
        };

        RequestType GetRequestType(std::string_view request);

        struct StatRequest {
            int id = 0;
            RequestType type;
            std::string name;
        };

        class JsonReader {
        public:
            
        void RequestProcess(TransportCatalogue& catalogue, std::istream& input, std::ostream& out, map_renderer::MapRenderer& renderer);

        private:
            std::vector<StatRequest> stat_requests;
            std::vector<json::Node> buses_request, stops_request;
        };

        void TestInnerFunction();

    } // namespace json_reader
} // namespace transport_catalogue