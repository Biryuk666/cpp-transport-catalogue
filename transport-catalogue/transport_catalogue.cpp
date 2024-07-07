#include <iostream>
#include <algorithm>
#include "transport_catalogue.h"
#include <utility>

using namespace std;

namespace transport_catalogue {

    void TransportCatalogue::AddBus(const string& bus_name, vector<string_view>& stops) {
        Bus new_bus;
        new_bus.name = bus_name;
        for (const auto& stop : stops) {
            new_bus.stops.push_back(GetStop(stop));
        }
        buses_.push_back(move(new_bus));
        buses_pointers_[buses_.back().name] = &buses_.back();
        for (const auto& stop : stops) {
            AddBusesForStop(GetStop(stop)->name, &buses_.back());
        }
    }

    void TransportCatalogue::AddStop(const string& stop_name, Coordinates&& coordinates) {
        TransportCatalogue::Stop new_stop{stop_name, move(coordinates)};
        stops_.push_back(move(new_stop));
        stops_pointers_[stops_.back().name] = &stops_.back();
    }

    void TransportCatalogue::AddBusesForStop(std::string_view stop_name, const TransportCatalogue::Bus* bus) {
        if (buses_for_stop_.count(stop_name) != 0 && buses_for_stop_.at(stop_name).count(bus) != 0) {
            return;
        }
        buses_for_stop_[stop_name].insert(bus);
    }

    const TransportCatalogue::Bus* TransportCatalogue::GetBus(const string_view& bus_name) const {
        if (buses_pointers_.count(bus_name) == 0) {

            return nullptr;
        }
        return buses_pointers_.at(bus_name);
    }

    const TransportCatalogue::Stop* TransportCatalogue::GetStop(const string_view& stop_name) const {
        if (stops_pointers_.count(stop_name) == 0) {
            return nullptr;
        }
        return stops_pointers_.at(stop_name);
    }

    optional<unordered_set<const TransportCatalogue::Bus*>> TransportCatalogue::GetBusesForStop(const std::string_view& stop_name) const {
        if (buses_for_stop_.count(stop_name) == 0) {
            return nullopt;
        }
        return buses_for_stop_.at(stop_name);
    }

    int GetUniqueStopsNumber(const TransportCatalogue::Bus& bus) {
        unordered_set<string_view> unique_stops;
        for (const auto& stop : bus.stops) {
            unique_stops.insert(stop->name);
        }
        return static_cast<int>(unique_stops.size());
    }

    int GetTotalStopsNumber(const TransportCatalogue::Bus& bus) {
        return static_cast<int>(bus.stops.size());
    }

    double GetRouteLength(std::vector<const TransportCatalogue::Stop*> stops) {
        double result = 0;
        for (size_t i = 0; i < stops.size(); ++i) {
            if (i == stops.size() - 1) {
                break;
            }
            result += ComputeDistance(stops[i]->coordinates, stops[i + 1]->coordinates);
        }
        return result;
    }

    RouteInfo GetRouteInfo(const TransportCatalogue::Bus& bus) {
        int total_stops_number = GetTotalStopsNumber(bus);
        int unique_stops_number = GetUniqueStopsNumber(bus);
        double route_length = GetRouteLength(bus.stops);
        return {total_stops_number, unique_stops_number, route_length};
    }

} // end of namespace transport_catalogue