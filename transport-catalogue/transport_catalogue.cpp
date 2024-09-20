#include <algorithm>
#include <iostream>
#include <numeric>
#include "transport_catalogue.h"
#include <utility>

#include "log_duration.h"

using namespace std;

namespace transport_catalogue {

    void TransportCatalogue::AddBus(const string& bus_name, vector<string>& stops, bool is_roundtrip) {
        domain::Bus new_bus;
        new_bus.name = bus_name;
        for (const auto& stop : stops) {
            new_bus.stops.push_back(GetStop(stop));
        }
        new_bus.is_roundtrip = is_roundtrip;
        buses_.push_back(move(new_bus));
        buses_pointers_[buses_.back().name] = &buses_.back();
        for (const auto& stop : stops) {
            AddBusesForStop(GetStop(stop)->name, &buses_.back());
        }
    }

    void TransportCatalogue::AddStop(const string& stop_name, geo::Coordinates&& coordinates) {
        domain::Stop new_stop{stop_name, move(coordinates)};
        stops_.push_back(move(new_stop));
        stops_pointers_[stops_.back().name] = &stops_.back();
    }

    void TransportCatalogue::AddBusesForStop(std::string_view stop_name, const domain::Bus* bus) {
        if (buses_for_stop_.count(stop_name) != 0 && buses_for_stop_.at(stop_name).count(bus) != 0) {
            return;
        }
        buses_for_stop_[stop_name].insert(bus);
    }

    const domain::Bus* TransportCatalogue::GetBus(string_view bus_name) const {
        if (buses_pointers_.count(bus_name) == 0) {

            return nullptr;
        }
        return buses_pointers_.at(bus_name);
    }

    const domain::Stop* TransportCatalogue::GetStop(string_view stop_name) const {
        if (stops_pointers_.count(stop_name) == 0) {
            return nullptr;
        }
        return stops_pointers_.at(stop_name);
    }

    const unordered_set<const domain::Bus*>* TransportCatalogue::GetBusesForStop(string_view stop_name) const {
        if (buses_for_stop_.count(stop_name) == 0) {
            return nullptr;
        }
        return &buses_for_stop_.at(stop_name);
    }

    void TransportCatalogue::SetDistance(const domain::Stop* from, const domain::Stop* to, int distance) {
        distance_between_stops_[{from, to}] = distance;
    }

    int GetUniqueStopsNumber(const domain::Bus& bus) {
        unordered_set<string_view> unique_stops;
        for (const auto& stop : bus.stops) {
            unique_stops.insert(stop->name);
        }
        return static_cast<int>(unique_stops.size());
    }

    int GetTotalStopsNumber(const domain::Bus& bus) {
        if (bus.is_roundtrip) {
            return static_cast<int>(bus.stops.size());
        }
        return static_cast<int>(bus.stops.size()) * 2 - 1;
    }

    double GetRouteLengthGeo(std::vector<const domain::Stop*> stops) {
        double result = 0;
        for (size_t i = 0; i < stops.size(); ++i) {
            if (i == stops.size() - 1) {
                break;
            }
            result += ComputeDistance(stops[i]->coordinates, stops[i + 1]->coordinates);
        }
        return result;
    }

    int TransportCatalogue::GetDistance(const domain::Stop* from, const domain::Stop* to) const {
        if (distance_between_stops_.count({from, to}) != 0) {
            return distance_between_stops_.at({from, to});
        } else if (distance_between_stops_.count({to, from}) != 0) {
            return distance_between_stops_.at({to, from});
        }
        return 0;
    }

    int TransportCatalogue::GetRouteLength(const vector<const domain::Stop*>& stops, bool is_roundtrip) const {
        int result = 0;
        for (size_t i = 0; i < stops.size() - 1; ++i) {
            result += GetDistance(stops[i], stops[i + 1]);
        }
        if (!is_roundtrip) {
            for (size_t i = stops.size() - 1; i > 0; --i) {
                result += GetDistance(stops[i], stops[i - 1]);
            }
        }
        return result;
    }

    domain::RouteInfo TransportCatalogue::GetRouteInfo(const domain::Bus* bus) const {
        int total_stops_number = GetTotalStopsNumber(*bus);
        int unique_stops_number = GetUniqueStopsNumber(*bus);
        int route_length = GetRouteLength(bus->stops, bus->is_roundtrip);
        double route_length_geo = GetRouteLengthGeo(bus->stops) * (bus->is_roundtrip ? 1 : 2);
        double route_curvature = static_cast<double>(route_length) / route_length_geo;

        return {total_stops_number, unique_stops_number, route_length, route_curvature};
    }

    const map<string_view, const domain::Bus*>* TransportCatalogue::GetBusesList() const {
        return &buses_pointers_;
    }

    const std::unordered_map<std::string_view, const domain::Stop*>* TransportCatalogue::GetStopsList() const {
        return &stops_pointers_;
    }

    size_t detail::PairHasher::operator()(const std::pair<const domain::Stop*, const domain::Stop*> pair_of_stops) const noexcept {
        return hasher_(pair_of_stops.first) * 17 + hasher_(pair_of_stops.second);
    }

    /*void TransportCatalogue::GetRouteInfoTest(const domain::Bus* bus) const {
        {
            LOG_DURATION("Get Total Stops Number");
            for (int i = 1; i < 101; ++i) {
                GetTotalStopsNumber(*bus);
            }
        }

        {
            LOG_DURATION("Get Unique Stops Number");
            for (int i = 1; i < 101; ++i) {
                GetUniqueStopsNumber(*bus);
            }
        }

        {
            LOG_DURATION("Get Route Length");
            for (int i = 1; i < 101; ++i) {
                GetRouteLength(bus->stops, bus->is_roundtrip);
            }
        }

        {
            LOG_DURATION("Get Route Length Geo");
            for (int i = 1; i < 101; ++i) {
                GetRouteLengthGeo(bus->stops);
            }
        }
    }*/
    
    

} // end of namespace transport_catalogue