#pragma once

#include <deque>
#include "geo.h"
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace transport_catalogue {

    class TransportCatalogue {
        public:

            struct Stop {
                std::string name;
                Coordinates coordinates;
            };

            struct Bus {
                std::string name;
                std::vector<const Stop*> stops;
             };
            
            void AddBus(const std::string& bus_name, std::vector<std::string_view>& stops);
            void AddStop(const std::string& stop_name, Coordinates&& coordinates);
            void AddBusesForStop(std::string_view stop_name, const Bus* bus);
                    
            const Bus* GetBus(const std::string_view& bus_name) const;
            const Stop* GetStop(const std::string_view& stop_name) const;

            std::optional<std::unordered_set<const Bus*>> GetBusesForStop(const std::string_view& stop_name) const;
            

            private:

            std::deque<Bus> buses_;
            std::deque<Stop> stops_;
            std::unordered_map<std::string_view, const Stop*> stops_pointers_;
            std::unordered_map<std::string_view, const Bus*> buses_pointers_;
            std::unordered_map<std::string_view, std::unordered_set<const Bus*>> buses_for_stop_;
        };

        struct RouteInfo {

            int total_stops_number;
            int unique_stops_number;
            double route_length;

        };

        RouteInfo GetRouteInfo(const TransportCatalogue::Bus& bus);

} // end of namespace transport_catalogue