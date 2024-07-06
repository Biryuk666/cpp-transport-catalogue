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

            struct Comporator {
                bool operator()(const TransportCatalogue::Bus* lhs, const TransportCatalogue::Bus* rhs) const;
            };
        
            struct Hasher {
                size_t operator()(const TransportCatalogue::Bus* bus) const;

                private:
                    std::hash<char> c_hasher;
            };

            std::optional<std::set<const Bus*, Comporator>> GetBusesForStop(const std::string_view& stop_name) const;
            

            private:

            std::deque<Bus> buses_;
            std::deque<Stop> stops_;
            std::unordered_map<std::string_view, const Stop*> stops_pointers_;
            std::unordered_map<std::string_view, const Bus*> buses_pointers_;
            // Считаю, что std::set в данном случае более подходящий, поскольку, для вывода информации о остановке, требуется отсортированный
            // по имени автобуса контейнер с уникальными объектами. Unordered_set в свою очередь предпологает
            // дальнейшие операции по сортировке, что увеличит затраты по используемой памяти, при той же временной сложности.
            std::unordered_map<std::string_view, std::set<const Bus*, Comporator>> buses_for_stop_;
        };

        struct RouteInfo {

            int total_stops_number;
            int unique_stops_number;
            double route_length;

        };

        RouteInfo GetRouteInfo(const TransportCatalogue::Bus& bus);

} // end of namespace transport_catalogue