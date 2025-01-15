#include "serialization.h"

#include <algorithm>
#include <deque>
#include <fstream>
#include <iterator>
#include <map>
#include <set>
#include <stdexcept>
#include <string_view>
#include <vector>

using namespace std;

namespace transport_catalogue {
    namespace serialization {

		template <typename It>
		int CalculateStopId(It begin, It end, const string& name) {
			auto it = find_if(begin, end, [&name](const domain::Stop& stop) {
				return stop.name == name;
			});
		
			return distance(begin, it);
		}

		void SerializeData(const TransportCatalogue& catalogue, const SerializationSettings& settings) {
			transport_catalogue_proto::Catalogue serialization_catalogue;

			auto stops_list_ptr = catalogue.GetAllStops();
			int id = 0;
			for (const domain::Stop& stop : *stops_list_ptr) {
				transport_catalogue_proto::Stop stop_proto;
				stop_proto.set_id(id);
				stop_proto.set_name(stop.name);
				stop_proto.set_latitude(stop.coordinates.lat);
				stop_proto.set_longitude(stop.coordinates.lng);

				*serialization_catalogue.add_stops() = move(stop_proto);
				++id;
			}

			auto buses_list_ptr = catalogue.GetAllBuses();
			for (const domain::Bus& bus : *buses_list_ptr) {
				transport_catalogue_proto::Bus bus_proto;
				bus_proto.set_name(bus.name);

				for (const domain::Stop* stop : bus.stops) {
					int id = CalculateStopId(stops_list_ptr->cbegin(), stops_list_ptr->cend(), stop->name);
					bus_proto.add_stops(id);
				}
				
				bus_proto.set_is_roundtrip(bus.is_roundtrip);

				*serialization_catalogue.add_buses() = move(bus_proto);
			}

			auto distance_between_stops_list = catalogue.GetDistanceBetweenStopsList();
			for (const auto& [stops_pair, distance] : *distance_between_stops_list) {
				transport_catalogue_proto::Distance distance_proto;
				int id_from = CalculateStopId(stops_list_ptr->begin(), stops_list_ptr->end(), stops_pair.first->name);
				distance_proto.set_stop_from(id_from);
				int id_to = CalculateStopId(stops_list_ptr->begin(), stops_list_ptr->end(), stops_pair.second->name);
				distance_proto.set_stop_to(id_to);
				distance_proto.set_distance(distance);

				*serialization_catalogue.add_distance_between_stops() = move(distance_proto);
			}


			ofstream output (settings.file_name, ios::binary);
			if (!output) throw ios_base::failure("Failed to create a file "s + settings.file_name);

			serialization_catalogue.SerializeToOstream(&output);

		}

		transport_catalogue::TransportCatalogue DeserializeFile(std::istream& input)
		{
			transport_catalogue_proto::Catalogue deserialization_catalogue;
			if (!deserialization_catalogue.ParseFromIstream(&input)) {
				throw std::runtime_error("The serialized file cannot be parsed from the istream");
			}

			TransportCatalogue catalogue;

			const auto& stops = deserialization_catalogue.stops();
			for (const transport_catalogue_proto::Stop& stop : stops) {
				catalogue.AddStop(stop.name(), {stop.latitude(), stop.longitude()});
			}

			auto stops_list_ptr = catalogue.GetAllStops();

			const auto& buses = deserialization_catalogue.buses();
			for (const transport_catalogue_proto::Bus& bus : buses) {
				vector<string> stops(bus.stops_size());
				for (int i = 0; i < stops.size(); ++i) {
					string stop_name = stops_list_ptr->at(bus.stops(i)).name;
					stops[i] = stop_name;
				}

				catalogue.AddBus(bus.name(), stops, bus.is_roundtrip());
			}

			const auto& distance_between_stops = deserialization_catalogue.distance_between_stops();
			for (const transport_catalogue_proto::Distance& distance_data : distance_between_stops) {
				auto stop_from = &stops_list_ptr->at(distance_data.stop_from());
				auto stop_to = &stops_list_ptr->at(distance_data.stop_to());
				catalogue.SetDistance(stop_from, stop_to, distance_data.distance());
			}

			return catalogue;
		}

    } // namespace serialization
} // namespace transport_catalogue