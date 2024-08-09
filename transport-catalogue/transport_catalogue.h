#pragma once

#include <deque>
#include "domain.h"
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
    namespace detail {

            struct PairHasher {
                size_t operator()(const std::pair<const domain::Stop*, const domain::Stop*> pair_of_stops) const noexcept;
                
            private:
                std::hash<const void*> hasher_;
            };

        } // namespace detail

    class TransportCatalogue {
        public:
            

            void AddBus(const std::string& bus_name, std::vector<std::string_view>& stops);
            void AddBus(domain::Bus&& bus);
            void AddStop(const std::string& stop_name, geo::Coordinates&& coordinates);            
            void AddStop(domain::Stop&& stop);
            void AddBusesForStop(std::string_view stop_name, const domain::Bus* bus);
                    
            const domain::Bus* GetBus(const std::string_view& bus_name) const;
            const domain::Stop* GetStop(const std::string_view& stop_name) const;

            std::optional<std::unordered_set<const domain::Bus*>> GetBusesForStop(const std::string_view& stop_name) const;

            void SetDistance(const domain::Stop* from, const domain::Stop* to, int distance);
            int GetDistance(const domain::Stop* from, const domain::Stop* to) const;
            
            domain::RouteInfo GetRouteInfo(const domain::Bus* bus) const;

            //тест производительности внутренних методов GetRouteInfo
            //void GetRouteInfoTest(const Bus* bus) const; 

        private:

            

            std::deque<domain::Bus> buses_;
            std::deque<domain::Stop> stops_;
            std::unordered_map<std::string_view, const domain::Stop*> stops_pointers_;
            std::unordered_map<std::string_view, const domain::Bus*> buses_pointers_;
            std::unordered_map<std::string_view, std::unordered_set<const domain::Bus*>> buses_for_stop_;
            std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>, int, detail::PairHasher> distance_between_stops_;
        };

} // end of namespace transport_catalogue