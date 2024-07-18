#include <iostream>
#include <algorithm>
#include "transport_catalogue.h"
#include <utility>

#include "log_duration.h"

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

    void TransportCatalogue::SetDistance(const TransportCatalogue::Stop* from, const TransportCatalogue::Stop* to, size_t distance) {
        distance_between_stops_[{from, to}] = distance;
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

    double GetRouteLengthGeo(std::vector<const TransportCatalogue::Stop*> stops) {
        double result = 0;
        for (size_t i = 0; i < stops.size(); ++i) {
            if (i == stops.size() - 1) {
                break;
            }
            result += ComputeDistance(stops[i]->coordinates, stops[i + 1]->coordinates);
        }
        return result;
    }

    size_t TransportCatalogue::GetDistance(const TransportCatalogue::Stop* from, const TransportCatalogue::Stop* to) const {
        if (distance_between_stops_.count({from, to}) != 0) {
            return distance_between_stops_.at({from, to});
        } else if (distance_between_stops_.count({to, from}) != 0) {
            return distance_between_stops_.at({to, from});
        }
        return 0;
    }

    size_t GetRouteLength(const TransportCatalogue& transport_catalogue, std::vector<const TransportCatalogue::Stop*> stops) {
        size_t result = 0;
        for (size_t i = 0; i < stops.size() - 1; ++i) {
            result += transport_catalogue.GetDistance(stops[i], stops[i + 1]);
        }
        return result;
    }

    TransportCatalogue::RouteInfo TransportCatalogue::GetRouteInfo(const TransportCatalogue::Bus* bus) const {
        int total_stops_number = GetTotalStopsNumber(*bus);
        int unique_stops_number = GetUniqueStopsNumber(*bus);
        size_t route_length = GetRouteLength(*this, bus->stops);
        double route_length_geo = GetRouteLengthGeo(bus->stops);
        double route_curvature = static_cast<double>(route_length) / route_length_geo;

        return {total_stops_number, unique_stops_number, route_length, route_curvature};
    }

    /*void TransportCatalogue::GetRouteInfoTest(const TransportCatalogue::Bus* bus) const {
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
                GetRouteLength(*this, bus->stops);
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