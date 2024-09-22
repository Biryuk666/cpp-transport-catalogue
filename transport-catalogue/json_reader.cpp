#include <algorithm>
#include "json_reader.h"
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>

using namespace std;

namespace transport_catalogue {
    namespace json_reader {

        pair<string, geo::Coordinates> GetStopFromRequest(const json::Dict& stop_request) {
            return {stop_request.at("name"s).AsString(), {stop_request.at("latitude"s).AsDouble(), stop_request.at("longitude"s).AsDouble()}};
        }

        tuple<string, vector<string>, bool> GetBusFromRequest(const json::Dict& bus_request) {
            string bus_name = bus_request.at("name"s).AsString();
            vector<string> stops;
            bool is_roundtrip = bus_request.at("is_roundtrip"s).AsBool();
            for (const auto& stop : bus_request.at("stops"s).AsArray()) {
                stops.push_back(stop.AsString());
            }
            
            return tie(bus_name, stops, is_roundtrip);
        }

        void SetDistanceToStopsFromRequest (TransportCatalogue& catalogue, const json::Dict& stop_request) {
            string stop_from = stop_request.at("name"s).AsString();
            for (const auto& [stop_to, distance] : stop_request.at("road_distances"s).AsDict()) {
                catalogue.SetDistance(catalogue.GetStop(stop_from), catalogue.GetStop(stop_to), distance.AsInt());
            }
        }

        void BaseRequestProcess (TransportCatalogue& catalogue, const json::Array& base_request) {
            vector<const json::Dict*> buses_buffer, stops_buffer;
            for (const auto& request : base_request) {
                if (request.AsDict().at("type"s).AsString() == "Stop"s) {
                    auto [stop_name, coordinates] = GetStopFromRequest(request.AsDict());
                    catalogue.AddStop(stop_name, move(coordinates));
                    if (request.AsDict().count("road_distances"s)) {
                        stops_buffer.push_back(&request.AsDict());
                    }
                } else if (request.AsDict().at("type"s).AsString() == "Bus"s) {
                    buses_buffer.push_back(&request.AsDict());
                } else {
                    continue;
                }
            }

            for (const auto& request : buses_buffer) {
                auto bus = GetBusFromRequest(*request);
                string bus_name = get<string>(bus);
                auto stops = get<vector<string>>(bus);
                bool is_roundtrip = get<bool>(bus);
                catalogue.AddBus(bus_name, stops, is_roundtrip);
            }

            for (const auto& request : stops_buffer) {
                SetDistanceToStopsFromRequest(catalogue, *request);
            }
        }

        json::Document StatRequestProcess (TransportCatalogue& catalogue, const vector<StatRequest>& stat_request,  const request_handler::RequestHandler& handler, const transport_router::TransportRouter& router) {
            json::Array result;
            for (const auto& request : stat_request) {
                json::Builder builder;
                builder.StartDict().Key("request_id"s).Value(request.id);
                switch (request.type) {
                    case RequestType::BUS: {
                        if (const auto& bus = catalogue.GetBus(request.name)) {
                            domain::RouteInfo route_info = catalogue.GetRouteInfo(bus);
                            builder.Key("curvature"s).Value(route_info.route_curvature)
                            .Key("route_length"s).Value(route_info.route_length)
                            .Key("stop_count"s).Value(route_info.total_stops_number)
                            .Key("unique_stop_count"s).Value(route_info.unique_stops_number);
                        } else {
                            builder.Key("error_message"s).Value("not found"s);
                        }
                        break;
                    } case RequestType::MAP: {
                        ostringstream output;
                        handler.RenderMap().Render(output);
                        builder.Key("map"s).Value(output.str());
                        break;
                    } case RequestType::ROUTE: {
                        auto items = router.GetRouteByStops(request.from, request.to);
                        if (items) {
                            builder.Key("total_time").Value(items.value().total_time)
                            .Key("items").StartArray();
                            for (const auto& item : items.value().items) {
                                if (item.type == "Wait"s) {
                                    builder.StartDict()
                                    .Key("type"s).Value(item.type)
                                    .Key("stop_name"s).Value(item.name)
                                    .Key("time"s).Value(item.time)
                                    .EndDict();
                                } else {
                                    builder.StartDict()
                                    .Key("type"s).Value(item.type)
                                    .Key("bus"s).Value(item.name)
                                    .Key("span_count"s).Value(item.span_count)
                                    .Key("time"s).Value(item.time)
                                    .EndDict();
                                }
                            }
                            builder.EndArray();
                        } else {
                            builder.Key("error_message"s).Value("not found"s);
                        }
                        break;
                    } case RequestType::STOP: {
                        if (const auto& stop = catalogue.GetStop(request.name)) {                            
                            auto buses_for_stop = catalogue.GetBusesForStop(stop->name);
                            //json::Array buses;
                            builder.Key("buses"s).StartArray();
                            if (buses_for_stop) {
                                vector<string> buses_names;
                                for (const auto& bus : *buses_for_stop) {
                                    buses_names.push_back(bus->name);
                                }
                                sort(buses_names.begin(), buses_names.end());
                                buses_names.erase(unique(buses_names.begin(), buses_names.end()), buses_names.end());
                                
                                for (const auto& bus : buses_names) {
                                    builder.Value(bus);
                                }
                            }
                            builder.EndArray();
                        } else {
                            builder.Key("error_message"s).Value("not found"s);
                        }
                        break;
                    } case RequestType::WTF: {
                        break;
                    }
                }
                builder.EndDict();
                result.push_back(builder.Build());          
            }
            return json::Document(result);
        }

        svg::Color GetColorFromRequest(const json::Node& color_node) {
            svg::Color color;
            if (color_node.IsString()) {
                color = color_node.AsString();
            } else if (color_node.AsArray().size() == 3) {
                color = svg::Rgb(static_cast<uint8_t>(color_node.AsArray()[0].AsInt()),
                                 static_cast<uint8_t>(color_node.AsArray()[1].AsInt()),
                                 static_cast<uint8_t>(color_node.AsArray()[2].AsInt()));
            } else if (color_node.AsArray().size() == 4) {
                color = svg::Rgba(static_cast<uint8_t>(color_node.AsArray()[0].AsInt()),
                                  static_cast<uint8_t>(color_node.AsArray()[1].AsInt()),
                                  static_cast<uint8_t>(color_node.AsArray()[2].AsInt()),
                                  color_node.AsArray()[3].AsDouble());
            }
            return color;
        }

        void SetRenderSettings(map_renderer::MapRenderer& renderer, const json::Dict& render_settings) {
            map_renderer::RenderSettings settings;
            settings.width = render_settings.at("width").AsDouble();
            settings.height = render_settings.at("height").AsDouble();
            settings.padding = render_settings.at("padding").AsDouble();
            settings.line_width = render_settings.at("line_width").AsDouble();
            settings.stop_radius = render_settings.at("stop_radius").AsDouble();
            settings.bus_label_font_size = render_settings.at("bus_label_font_size").AsInt();
            auto bus_label_offset = render_settings.at("bus_label_offset").AsArray();
            settings.bus_label_offset.x = bus_label_offset[0].AsDouble();
            settings.bus_label_offset.y = bus_label_offset[1].AsDouble();
            settings.stop_label_font_size = render_settings.at("stop_label_font_size").AsInt();
            auto stop_label_offset = render_settings.at("stop_label_offset").AsArray();
            settings.stop_label_offset.x = stop_label_offset[0].AsDouble();
            settings.stop_label_offset.y = stop_label_offset[1].AsDouble();
            settings.underlayer_color = GetColorFromRequest(render_settings.at("underlayer_color"));
            settings.underlayer_width = render_settings.at("underlayer_width").AsDouble();               
            for (const auto& color_nodes : render_settings.at("color_palette").AsArray()) {
                settings.color_palette.push_back(GetColorFromRequest(color_nodes));
            }
            renderer.SetSetting(move(settings));
        }

        void SetRoutingSettings(transport_router::TransportRouter& router, const json::Dict& route_request) {
            router.SetWaitTime(route_request.at("bus_wait_time"s).AsInt());
            router.SetBusVelocity(route_request.at("bus_velocity"s).AsDouble());
        }

        void JsonReader::RequestProcess(TransportCatalogue& catalogue, std::istream& input, std::ostream& output, map_renderer::MapRenderer& renderer, const request_handler::RequestHandler& handler, transport_router::TransportRouter& router) {
            json::Document request = json::Load(input);
            for (const auto& [request_type, request_body] : request.GetRoot().AsDict()) {
                if (request_type == "base_requests"s && !request_body.AsArray().empty()) {
                    BaseRequestProcess(catalogue, request_body.AsArray());
                } else if (request_type == "stat_requests"s && !request_body.AsArray().empty()) {
                    for (const auto& query : request_body.AsArray()) {
                        StatRequest stat_request;
                        stat_request.id = query.AsDict().at("id").AsInt();
                        stat_request.type = GetRequestType(query.AsDict().at("type").AsString());
                        if (stat_request.type == RequestType::BUS || stat_request.type == RequestType::STOP) {
                            stat_request.name = query.AsDict().at("name").AsString();
                        } else if (stat_request.type == RequestType::ROUTE) {
                            stat_request.from = query.AsDict().at("from").AsString();
                            stat_request.to = query.AsDict().at("to").AsString();
                        }
                        stat_requests.push_back(stat_request);
                    }
                    json::Document document = StatRequestProcess(catalogue, stat_requests, handler, router);
                    json::Print(document, output);
                } else if (request_type == "render_settings"s && !request_body.AsDict().empty()) {
                    SetRenderSettings(renderer, request_body.AsDict());
                } else if (request_type == "routing_settings"s && !request_body.AsDict().empty()) {
                    SetRoutingSettings(router, request_body.AsDict());
                }
            }

        }

        RequestType GetRequestType(string_view request) {
            if (request == "Bus") {
                return RequestType::BUS;
            } else if (request == "Stop") {
                return RequestType::STOP;
            } else if (request == "Map") {
                return RequestType::MAP;
            } else if (request == "Route") {
                return RequestType::ROUTE;
            } else {
                return RequestType::WTF;
            }
        }

    } // namespace json_reader
} // namespace transport_catalogue