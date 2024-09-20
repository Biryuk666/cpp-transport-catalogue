#include "transport_router.h"
#include <utility>

using namespace std;

namespace transport_catalogue {
    namespace transport_router {
        
        void TransportRouter::SetWaitTime(int time) {
            bus_wait_time_ = time;
        }

        void TransportRouter::SetBusVelocity(double velocity) {
            bus_velocity_ = velocity;
        }

        int TransportRouter::GetWaitTime() const {
            return bus_wait_time_;
        }

        double TransportRouter::GetBusVelocity() const {
            return bus_velocity_;
        }

        StopVertexes TransportRouter::GetStopVertexes(const domain::Stop* stop) const {
            return stop_to_stop_vertexes_.at(stop);
        }

        void TransportRouter::BuildAllRoutes() {
            size_t stops_number = catalogue_.GetStopsList()->size();
            graph_ = std::make_unique<graph::DirectedWeightedGraph<double>>(stops_number * 2);
            AddStopsToGraph();
            for(const auto [name, bus] : *catalogue_.GetBusesList()) {
                AddRouteToGraph(bus);
            }
            router_ = std::make_unique<graph::Router<double>>(*graph_);
        }

        optional<TransportRouter::RouteItems> TransportRouter::GetRouteByStops(string_view stop_from_name, string_view stop_to_name) const {
            RouteItems items_info;
            auto stop_from = catalogue_.GetStop(stop_from_name);
            auto stop_to = catalogue_.GetStop(stop_to_name);
            auto router_info = router_->BuildRoute(GetStopVertexes(stop_from).wait, GetStopVertexes(stop_to).wait);
            if (router_info) {
                items_info.total_time = router_info.value().weight;
                for (const auto& edge : router_info.value().edges) {
                    items_info.items.push_back(edge_to_item_.at(edge));
                }
                return items_info;
            } else {
                return {};
            }
        }

        void TransportRouter::AddStopsToGraph() {
            graph::VertexId vertex_id = 0;
            for (const auto& [name, stop] : *catalogue_.GetStopsList()) {
                stop_to_stop_vertexes_[stop] = {vertex_id, vertex_id + 1};
                auto edge_id = graph_->AddEdge({vertex_id, vertex_id + 1, static_cast<double>(bus_wait_time_)});
                Item item;
                item.type = "Wait"s;
                item.name = name;
                item.time = static_cast<double>(bus_wait_time_);
                item.span_count = 1;
                edge_to_item_[edge_id] = move(item);
                vertex_id += 2;
            }
        }


        void TransportRouter::AddBusEdge(const domain::Stop* from, const domain::Stop* to, string_view bus_name, int span, double distance) {
            Item item;
            item.type = "Bus"s;
            item.name = bus_name;
            item.time = distance /(bus_velocity_ *  1000 / 60);
            item.span_count = span;
            auto vertexes_from = stop_to_stop_vertexes_.at(from);
            auto vertexes_to = stop_to_stop_vertexes_.at(to);
            auto edge_id = graph_->AddEdge({vertexes_from.bus, vertexes_to.wait, item.time});
            edge_to_item_[edge_id] = move(item);
        }

        void TransportRouter::AddRouteToGraph(const domain::Bus* bus) {
            for (size_t i = 0; i < bus->stops.size() - 1; ++i) {
                double forward_distance = 0;
                double backward_distance = 0;
                for (size_t j = i; j < bus->stops.size() - 1; ++j) {
                    forward_distance += catalogue_.GetDistance(bus->stops[j], bus->stops[j + 1]);
                    AddBusEdge(bus->stops[i], bus->stops[j + 1], bus->name, j - i + 1, forward_distance);
                    if (!bus->is_roundtrip){
                        backward_distance += catalogue_.GetDistance(bus->stops[j + 1], bus->stops[j]);
                        AddBusEdge(bus->stops[j + 1], bus->stops[i], bus->name, j - i + 1, backward_distance);
                    }
                }
            }
        }

    } // transport_router
} // namespace transport_catalogue