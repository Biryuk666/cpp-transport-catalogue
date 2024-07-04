#include <algorithm>
#include "transport_catalogue.h"
#include <utility>

using namespace std;

namespace transport_catalogue {

    void TransportCatalogue::AddBus(Bus&& bus) {
        buses_.push_back(move(bus));
        buses_pointers_[buses_.back().name] = &buses_.back();
    }

    void TransportCatalogue::AddStop(Stop&& stop) {
        stops_.push_back(move(stop));
        stops_pointers_[stops_.back().name] = &stops_.back();
    }

    void TransportCatalogue::AddBusesForStop(std::string_view stop_name, const TransportCatalogue::Bus* bus) {
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

    optional<set<const TransportCatalogue::Bus*, TransportCatalogue::BusNameCmp>> TransportCatalogue::GetBusesForStop(const std::string_view& stop_name) const {
        if (buses_for_stop_.count(stop_name) == 0) {
            return nullopt;
        }
        return buses_for_stop_.at(stop_name);
    }

    bool TransportCatalogue::Bus::operator<(const Bus& other) const {
        return name < other.name;
    }

    bool TransportCatalogue::BusNameCmp::operator()(const Bus* lhs, const Bus* rhs) const {
        return *lhs < *rhs;
    }

    int GetUniqueStopsNumber(const TransportCatalogue::Bus& bus) {
        unordered_set<string_view> unique_stops;
        for (const auto& stop : bus.stops) {
            unique_stops.insert(stop->name);
        }

        return static_cast<int>(unique_stops.size());
    }

    int GetStopsNumber(const TransportCatalogue::Bus& bus) {
        return static_cast<int>(bus.stops.size());
    }

    double GetRouteLength(std::vector<const TransportCatalogue::Stop*> stops) {
        double result = 0;
        for (size_t i = 0; i < stops.size(); ++i) {
            if (i == stops.size() - 1) {
                break;
            }
            Coordinates from, to;
            from.lat = stops[i]->latitude;
            from.lng = stops[i]->longitude;
            to.lat = stops[i + 1]->latitude;
            to.lng = stops[i + 1]->longitude;
            result += ComputeDistance(from, to);
        }
        return result;
    }

} // end of namespace transport_catalogue