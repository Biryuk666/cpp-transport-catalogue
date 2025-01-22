#pragma once

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <transport_catalogue.pb.h>
#include <vector>

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

namespace transport_catalogue {
    namespace transport_router {

        struct StopVertex {
            graph::VertexId wait;
            graph::VertexId bus;
        };

        struct RouterSettings {
            int bus_wait_time = 0;
            double bus_velocity = 0;
        };

        struct Item {
            std::string type;
            std::string name;
            double time = 0;
            int span_count = 0;
        };

        struct RouterData {
            RouterSettings settings;
            std::vector<graph::Edge<double>> edges;
            std::vector<graph::IncidenceList> incidence_lists;
            graph::RoutesInternalData routes_iternal_data;
            std::map <const domain::Stop*, StopVertex> stop_to_stop_vertex;
            std::map<graph::EdgeId, Item> edge_id_to_item;
        };

        class TransportRouter {
            public:
                struct RouteItems {
                    double total_time = 0;
                    std::vector<Item> items;
                };

                TransportRouter(const TransportCatalogue& catalogue);
                TransportRouter(const TransportCatalogue& catalogue, const RouterSettings& settings);

                std::optional<RouteItems> GetRouteByStops(std::string_view stop_from_name, std::string_view stop_to_name) const;

                void SetRouterData(RouterData&& import_data);

                const RouterSettings& GetRouterSettings() const;
                const std::unique_ptr<graph::DirectedWeightedGraph<double>>& GetGraphPtr() const;
                const std::unique_ptr<graph::Router<double>>& GetRouterPtr() const;
                const std::map<const domain::Stop*, StopVertex>& GetStopToStopVertexMap() const;
                const std::map<graph::EdgeId, Item>& GetEdgeIdToItemMap() const;

            private:
                const TransportCatalogue& catalogue_;
                RouterSettings router_settings_;
                std::unique_ptr<graph::DirectedWeightedGraph<double>> graph_;
                std::unique_ptr<graph::Router<double>> router_;
                std::map<const domain::Stop*, StopVertex> stop_to_stop_vertex_;
                std::map<graph::EdgeId, Item> edge_id_to_item_;
                
                StopVertex GetStopVertex(const domain::Stop* stop) const;
                void AddStopsToGraph();
                void AddBusEdge(const domain::Stop* from, const domain::Stop* to, std::string_view bus_name, int span, double distance);
                void AddRouteToGraph(const domain::Bus* bus);
                void BuildAllRoutes();                
        };

    } // transport_router
} // namespace transport_catalogue
