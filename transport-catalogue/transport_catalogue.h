#pragma once

#include <deque>
#include "domain.h"
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

            void AddBus(const std::string& bus_name, std::vector<std::string>& stops, bool is_roundtrip);
            void AddStop(const std::string& stop_name, geo::Coordinates&& coordinates);
                                
            const domain::Bus* GetBus(std::string_view bus_name) const;
            const domain::Stop* GetStop(std::string_view stop_name) const;

            const std::unordered_set<const domain::Bus*>* GetBusesForStop(std::string_view stop_name) const;

            void SetDistance(const domain::Stop* from, const domain::Stop* to, int distance);
            int GetDistance(const domain::Stop* from, const domain::Stop* to) const;
            
            domain::RouteInfo GetRouteInfo(const domain::Bus* bus) const;

            const std::map<std::string_view, const domain::Bus*>* GetBusesList() const;
            const std::unordered_map<std::string_view, const domain::Stop*>* GetStopsList() const;

            const std::deque<domain::Bus>& GetAllBuses() const;
            const std::deque<domain::Stop>& GetAllStops() const;
            const std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>, int, detail::PairHasher>& GetDistanceBetweenStopsList() const;

        private:
            std::deque<domain::Bus> buses_;
            std::deque<domain::Stop> stops_;
            std::unordered_map<std::string_view, const domain::Stop*> stops_pointers_;
            std::map<std::string_view, const domain::Bus*> buses_pointers_;
            std::unordered_map<std::string_view, std::unordered_set<const domain::Bus*>> buses_for_stop_;
            std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>, int, detail::PairHasher> distance_between_stops_;

            void AddBusesForStop(std::string_view stop_name, const domain::Bus* bus);
            int GetUniqueStopsNumber(const domain::Bus& bus) const;
            int GetTotalStopsNumber(const domain::Bus& bus) const;
            double GetRouteLengthGeo(std::vector<const domain::Stop*> stops) const;
            int GetRouteLength(const std::vector<const domain::Stop*>& stops, bool is_roundtrip) const;
        };

} // end of namespace transport_catalogue