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
                double latitude, longitude;
            };

            struct Bus {
                std::string name;
                std::vector<const Stop*> stops;

                bool operator<(const Bus& other) const;
            };
            
            void AddBus(Bus&& bus);
            void AddStop(Stop&& stop);
            void AddBusesForStop(std::string_view stop_name, const Bus* bus);
                    
            const Bus* GetBus(const std::string_view& bus_name) const;
            const Stop* GetStop(const std::string_view& stop_name) const;

            struct BusNameCmp {
                bool operator()(const Bus* lhs, const Bus* rhs) const;
            };

            std::optional<std::set<const Bus*, BusNameCmp>> GetBusesForStop(const std::string_view& stop_name) const;
            

            private:

            std::deque<Bus> buses_;
            std::deque<Stop> stops_;
            std::unordered_map<std::string_view, const Stop*> stops_pointers_;
            std::unordered_map<std::string_view, const Bus*> buses_pointers_;        
            std::unordered_map<std::string_view, std::set<const Bus*, BusNameCmp>> buses_for_stop_;
        };

        int GetUniqueStopsNumber(const TransportCatalogue::Bus& bus);
        int GetStopsNumber(const TransportCatalogue::Bus& bus);
        double GetRouteLength(std::vector<const TransportCatalogue::Stop*> stops);
}