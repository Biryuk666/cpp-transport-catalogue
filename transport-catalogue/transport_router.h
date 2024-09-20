#pragma once

#include <memory>
#include <optional>
#include "router.h"
#include <string>
#include <string_view>
#include "transport_catalogue.h"

namespace transport_catalogue {
    namespace transport_router {

        using Weight = double;

        struct StopVertexes {
            graph::VertexId wait;
            graph::VertexId bus;
        };

        class TransportRouter {
            public:
                struct Item {
                    std::string type;
                    std::string name;
                    double time = 0;
                    int span_count = 0;
                };

                struct RouteItems {
                    double total_time = 0;
                    std::vector<Item> items;
                };

                TransportRouter(const TransportCatalogue& catalogue) : catalogue_(catalogue) {}
                void SetWaitTime(int time);
                void SetBusVelocity(double velocity);
                int GetWaitTime() const;
                double GetBusVelocity() const;
                void BuildAllRoutes();
                std::optional<RouteItems> GetRouteByStops(std::string_view stop_from_name, std::string_view stop_to_name) const;

            private:
                int bus_wait_time_ = 0;
                double bus_velocity_ = 0;
                const TransportCatalogue& catalogue_;
                std::unique_ptr<graph::DirectedWeightedGraph<double>> graph_;
                std::unique_ptr<graph::Router<double>> router_;
                std::map<const domain::Stop*, StopVertexes> stop_to_stop_vertexes_;
                std::map<graph::EdgeId, Item> edge_to_item_;

                void AddStopsToGraph();               
                StopVertexes GetStopVertexes(const domain::Stop* stop_ptr) const;
                void AddBusEdge(const domain::Stop* start_stop, const domain::Stop* finish_stop, std::string_view bus_name, int span, double distance);
                void AddRouteToGraph(const domain::Bus* bus_ptr);
                
        };

    } // transport_router
} // namespace transport_catalogue
