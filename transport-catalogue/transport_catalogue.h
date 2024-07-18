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

            struct RouteInfo {

                int total_stops_number;
                int unique_stops_number;
                size_t route_length;
                double route_curvature;
            };

            void AddBus(const std::string& bus_name, std::vector<std::string_view>& stops);
            void AddStop(const std::string& stop_name, Coordinates&& coordinates);
            void AddBusesForStop(std::string_view stop_name, const Bus* bus);
                    
            const Bus* GetBus(const std::string_view& bus_name) const;
            const Stop* GetStop(const std::string_view& stop_name) const;

            std::optional<std::unordered_set<const Bus*>> GetBusesForStop(const std::string_view& stop_name) const;

            void SetDistance(const Stop* from, const Stop* to, size_t distance);
            size_t GetDistance(const Stop* from, const Stop* to) const;
            
            RouteInfo GetRouteInfo(const Bus* bus) const;
            //void GetRouteInfoTest(const Bus* bus) const;

        private:

            struct PairHasher {
                size_t operator()(const std::pair<const Stop*, const Stop*> pair_of_stops) const noexcept
                {
                    return hasher_(pair_of_stops.first) * 17 + hasher_(pair_of_stops.second);
                }
                
            private:
                std::hash<const void*> hasher_;
            };

            std::deque<Bus> buses_;
            std::deque<Stop> stops_;
            std::unordered_map<std::string_view, const Stop*> stops_pointers_;
            std::unordered_map<std::string_view, const Bus*> buses_pointers_;
            std::unordered_map<std::string_view, std::unordered_set<const Bus*>> buses_for_stop_;
            std::unordered_map<std::pair<const Stop*, const Stop*>, size_t, PairHasher> distance_between_stops_;
        };

} // end of namespace transport_catalogue