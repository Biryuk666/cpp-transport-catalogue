#include "transport_router.h"
#include <utility>

using namespace std;

namespace transport_catalogue {
    namespace transport_router {
        TransportRouter::TransportRouter(const TransportCatalogue& catalogue) : catalogue_(catalogue) {}
        TransportRouter::TransportRouter(const TransportCatalogue& catalogue, const RouterSettings& settings)
            : catalogue_(catalogue), router_settings_(settings) {
            BuildAllRoutes();
        }

        StopVertex TransportRouter::GetStopVertex(const domain::Stop* stop) const {
            return stop_to_stop_vertex_.at(stop);
        }

        optional<TransportRouter::RouteItems> TransportRouter::GetRouteByStops(string_view stop_from_name, string_view stop_to_name) const {
            RouteItems items_info;
            auto stop_from = catalogue_.GetStop(stop_from_name);
            auto stop_to = catalogue_.GetStop(stop_to_name);
            auto router_info = router_->BuildRoute(GetStopVertex(stop_from).wait, GetStopVertex(stop_to).wait);
            if (router_info) {
                items_info.total_time = router_info.value().weight;
                for (const auto& edge : router_info.value().edges) {
                    items_info.items.push_back(edge_id_to_item_.at(edge));
                }
                return items_info;
            } else {
                return {};
            }
        }

        void TransportRouter::SetRouterData(RouterData&& import_data) {
            router_settings_ = move(import_data.settings);
            graph_ = make_unique<graph::DirectedWeightedGraph<double>>(move(import_data.edges), move(import_data.incidence_lists));
            router_ = make_unique<graph::Router<double>>(*graph_, move(import_data.routes_iternal_data));
            stop_to_stop_vertex_ = move(import_data.stop_to_stop_vertex);
            edge_id_to_item_ = move(import_data.edge_id_to_item);
        }

        const RouterSettings& TransportRouter::GetRouterSettings() const {
            return router_settings_;
        }

        const std::unique_ptr<graph::DirectedWeightedGraph<double>>& TransportRouter::GetGraphPtr() const {
            return graph_;
        }

        const std::unique_ptr<graph::Router<double>>& TransportRouter::GetRouterPtr() const {
            return router_;
        }

        const std::map<const domain::Stop*, StopVertex>& TransportRouter::GetStopToStopVertexMap() const {
            return stop_to_stop_vertex_;
        }

        const std::map<graph::EdgeId, Item>& TransportRouter::GetEdgeIdToItemMap() const {
            return edge_id_to_item_;
        }

        void TransportRouter::AddStopsToGraph() {
            graph::VertexId vertex_id = 0;
            for (const auto& [name, stop] : *catalogue_.GetStopsList()) {
                stop_to_stop_vertex_[stop] = {vertex_id, vertex_id + 1};
                auto edge_id = graph_->AddEdge({vertex_id, vertex_id + 1, static_cast<double>(router_settings_.bus_wait_time)});
                Item item;
                item.type = "Wait"s;
                item.name = name;
                item.time = static_cast<double>(router_settings_.bus_wait_time);
                item.span_count = 1;
                edge_id_to_item_[edge_id] = move(item);
                vertex_id += 2;
            }
        }

        void TransportRouter::AddBusEdge(const domain::Stop* from, const domain::Stop* to, string_view bus_name, int span, double distance) {
            Item item;
            item.type = "Bus"s;
            item.name = bus_name;
            item.time = distance /(router_settings_.bus_velocity *  1000 / 60);
            item.span_count = span;
            auto vertex_from = stop_to_stop_vertex_.at(from);
            auto vertex_to = stop_to_stop_vertex_.at(to);
            auto edge_id = graph_->AddEdge({vertex_from.bus, vertex_to.wait, item.time});
            edge_id_to_item_[edge_id] = move(item);
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

        void TransportRouter::BuildAllRoutes() {
            size_t stops_number = catalogue_.GetStopsList()->size();
            graph_ = std::make_unique<graph::DirectedWeightedGraph<double>>(stops_number * 2);
            AddStopsToGraph();
            for(const auto [name, bus] : *catalogue_.GetBusesList()) {
                AddRouteToGraph(bus);
            }
            router_ = std::make_unique<graph::Router<double>>(*graph_);
        }

    } // transport_router
} // namespace transport_catalogue