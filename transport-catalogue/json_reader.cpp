#include <algorithm>
#include "json_reader.h"
#include <set>
#include <string>
#include <string_view>
#include <utility>

using namespace std;

namespace transport_catalogue {
    namespace json_reader {

        domain::Stop GetStopFromRequest(const json::Dict& stop_request) {
            return {stop_request.at("name"s).AsString(), {stop_request.at("latitude"s).AsDouble(), stop_request.at("longitude"s).AsDouble()}};
        }

        domain::Bus GetBusFromRequest(const TransportCatalogue& catalogue, const json::Dict& bus_request) {
            string bus_name = bus_request.at("name"s).AsString();
            vector<const domain::Stop*> stops;
            bool is_roundtrip = bus_request.at("is_roundtrip"s).AsBool();
            for (const auto& stop : bus_request.at("stops"s).AsArray()) {
                stops.push_back(catalogue.GetStop(stop.AsString()));
            }
            
            return {bus_name, stops, is_roundtrip};
        }

        void SetDistanceToStopsFromRequest (TransportCatalogue& catalogue, const json::Dict& stop_request) {
            string stop_from = stop_request.at("name"s).AsString();
            for (const auto& [stop_to, distance] : stop_request.at("road_distances"s).AsMap()) {
                catalogue.SetDistance(catalogue.GetStop(stop_from), catalogue.GetStop(stop_to), distance.AsInt());
            }
        }

        void BaseRequestProcess (TransportCatalogue& catalogue, const json::Array& base_request) {
            vector<const json::Dict*> buses_buffer, stops_buffer;
            for (const auto& request : base_request) {
                if (request.AsMap().at("type"s).AsString() == "Stop"s) {
                    catalogue.AddStop(move(GetStopFromRequest(request.AsMap())));
                    if (request.AsMap().count("road_distances"s)) {
                        stops_buffer.push_back(&request.AsMap());
                    }
                } else if (request.AsMap().at("type"s).AsString() == "Bus"s) {
                    buses_buffer.push_back(&request.AsMap());
                } else {
                    continue;
                }
            }

            for (const auto& request : buses_buffer) {
                catalogue.AddBus(move(GetBusFromRequest(catalogue, *request)));
            }

            for (const auto& request : stops_buffer) {
                SetDistanceToStopsFromRequest(catalogue, *request);
            }
        }

        json::Document StatRequestProcess (TransportCatalogue& catalogue, const vector<StatRequest>& stat_request) {
            json::Array result;
            for (const auto& request : stat_request) {
                json::Dict document;
                document["request_id"s] = request.id;
                switch (request.type) {
                    case RequestType::BUS: {
                        if (const auto& bus = catalogue.GetBus(request.name)) {
                            domain::RouteInfo route_info = catalogue.GetRouteInfo(bus);
                            document["curvature"s] = route_info.route_curvature;
                            document["route_length"s] = route_info.route_length;
                            document["stop_count"s] = route_info.total_stops_number;
                            document["unique_stop_count"s] = route_info.unique_stops_number;
                        } else {
                            document["error_message"s] = "not found"s;
                        }
                        break;
                    } case RequestType::STOP: {
                        if (const auto& stop = catalogue.GetStop(request.name)) {                            
                            auto buses_for_stop = catalogue.GetBusesForStop(stop->name);
                            json::Array buses;
                            if (buses_for_stop != nullopt) {
                                set<string> buses_names;
                                for (const auto& bus : *buses_for_stop) {
                                    buses_names.insert(bus->name);
                                }
                                
                                for (const auto& bus : buses_names) {
                                    buses.emplace_back(json::Node(bus));
                                }
                            }
                            document["buses"s] = buses;
                        } else {
                            document["error_message"s] = "not found"s;
                        }
                    } case RequestType::WTF: {
                        break;
                    }
                }
                result.push_back(document);          
            }
            return json::Document(result);
        }

        void JsonReader::RequestProcess(TransportCatalogue& catalogue, std::istream& input, std::ostream& output) {
            json::Document request = json::Load(input);
            for (const auto& [request_type, request_body] : request.GetRoot().AsMap()) {
                if (request_type == "base_requests"s) {
                    BaseRequestProcess(catalogue, request_body.AsArray());
                } else if (request_type == "stat_requests"s) {
                    for (const auto& query : request_body.AsArray()) {
                        StatRequest stat_request;
                        stat_request.id = query.AsMap().at("id").AsInt();
                        stat_request.type = GetRequestType(query.AsMap().at("type").AsString());
                        stat_request.name = query.AsMap().at("name").AsString();
                        stat_requests.push_back(stat_request);
                    }
                    json::Document document = StatRequestProcess(catalogue, stat_requests);
                    json::Print(document, output);
                } else {}
            }

        }

        RequestType GetRequestType(string_view request) {
            if (request == "Bus") {
                return RequestType::BUS;
            } else if (request == "Stop") {
                return RequestType::STOP;
            } else {
                return RequestType::WTF;
            }
        }

    } // namespace json_reader
} // namespace transport_catalogue