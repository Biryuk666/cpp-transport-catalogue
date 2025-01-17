#include "serialization.h"

#include <algorithm>
#include <deque>
#include <fstream>
#include <iterator>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
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

		transport_catalogue_proto::CatalogueData Serializator::GetCatalogueDataForSerialization(const transport_catalogue::TransportCatalogue& catalogue) {
			transport_catalogue_proto::CatalogueData catalogue_data;

			auto stops_list_ptr = catalogue.GetAllStops();
			int id = 0;
			for (const domain::Stop& stop : *stops_list_ptr) {
				transport_catalogue_proto::Stop stop_proto;
				// filling stop_proto
				{
					stop_proto.set_id(id);
					stop_proto.set_name(stop.name);
					stop_proto.set_latitude(stop.coordinates.lat);
					stop_proto.set_longitude(stop.coordinates.lng);
				}

				*catalogue_data.add_stops() = move(stop_proto);
				++id;
			}

			auto buses_list_ptr = catalogue.GetAllBuses();
			for (const domain::Bus& bus : *buses_list_ptr) {
				transport_catalogue_proto::Bus bus_proto;
				// filling bus_proto
				{
					bus_proto.set_name(bus.name);

					for (const domain::Stop* stop : bus.stops) {
						int id = CalculateStopId(stops_list_ptr->cbegin(), stops_list_ptr->cend(), stop->name);
						bus_proto.add_stops(id);
				}

				bus_proto.set_is_roundtrip(bus.is_roundtrip);
				}

				*catalogue_data.add_buses() = move(bus_proto);
			}

			auto distance_between_stops_list = catalogue.GetDistanceBetweenStopsList();
			for (const auto& [stops_pair, distance] : *distance_between_stops_list) {
				transport_catalogue_proto::Distance distance_proto;
				// filling distance_proto
				{
					int id_from = CalculateStopId(stops_list_ptr->begin(), stops_list_ptr->end(), stops_pair.first->name);
					distance_proto.set_stop_from(id_from);
					int id_to = CalculateStopId(stops_list_ptr->begin(), stops_list_ptr->end(), stops_pair.second->name);
					distance_proto.set_stop_to(id_to);
					distance_proto.set_distance(distance);
				}

				*catalogue_data.add_distance_between_stops() = move(distance_proto);
			}

			return catalogue_data;
		}

		transport_catalogue_proto::Color GetColorData(const svg::Color& color) {
			transport_catalogue_proto::Color color_proto;

			if (holds_alternative<monostate>(color)) {
				color_proto.set_none(true);
			} else if (holds_alternative<svg::Rgb>(color)) {
				svg::Rgb rgb = get<svg::Rgb>(color);
				// filling rgb	
				{
					color_proto.mutable_rgb()->set_red(rgb.red);
					color_proto.mutable_rgb()->set_green(rgb.green);
					color_proto.mutable_rgb()->set_blue(rgb.blue);
				}
			} else if (holds_alternative<svg::Rgba>(color)) {
				svg::Rgba rgba = get<svg::Rgba>(color);
				// filling rgba
				{
					color_proto.mutable_rgba()->set_red(rgba.red);
					color_proto.mutable_rgba()->set_green(rgba.green);
					color_proto.mutable_rgba()->set_blue(rgba.blue);
					color_proto.mutable_rgba()->set_opacity(rgba.opacity);
				}				
			} else {
				string s_color = get<string>(color);
				color_proto.set_s_color(s_color);
			}

			return color_proto;
		} 

		transport_catalogue_proto::RenderSettings Serializator::GetRenderSettingsDataForSerialization(const map_renderer::MapRenderer& renderer) {
			map_renderer::RenderSettings render_settings = renderer.GetSettings();
			transport_catalogue_proto::RenderSettings render_settings_proto;
			// filling render_settings_proto
			{
				render_settings_proto.set_width(render_settings.width);
				render_settings_proto.set_height(render_settings.height);
				render_settings_proto.set_padding(render_settings.padding);
				render_settings_proto.set_line_width(render_settings.line_width);				
				render_settings_proto.set_stop_radius(render_settings.stop_radius);
				render_settings_proto.set_bus_label_font_size(render_settings.bus_label_font_size);
				render_settings_proto.mutable_bus_label_offset()->set_x(render_settings.bus_label_offset.x);
				render_settings_proto.mutable_bus_label_offset()->set_y(render_settings.bus_label_offset.y);
				render_settings_proto.set_stop_label_font_size(render_settings.stop_label_font_size);
				render_settings_proto.mutable_stop_label_offset()->set_x(render_settings.stop_label_offset.x);
				render_settings_proto.mutable_stop_label_offset()->set_y(render_settings.stop_label_offset.y);
				*render_settings_proto.mutable_underlayer_color() = GetColorData(render_settings.underlayer_color);
				render_settings_proto.set_underlayer_width(render_settings.underlayer_width);

				for (const auto& color : render_settings.color_palette) {
					*render_settings_proto.add_color_palette() = GetColorData(color);
				}				
			}

			return render_settings_proto;
		}

        void Serializator::SetSettings(std::string&& file_name) {
			settings_.file_name = move(file_name);
        }

        void Serializator::SerializeData(const TransportCatalogue &catalogue, const map_renderer::MapRenderer& renderer)
        {
            auto catalogue_data = GetCatalogueDataForSerialization(catalogue);
			auto render_settings_data = GetRenderSettingsDataForSerialization(renderer);
			transport_catalogue_proto::ProcessingData processing_data;
			*processing_data.mutable_catalogue() = move(catalogue_data);
			*processing_data.mutable_render_settings() = move(render_settings_data);

			std::ofstream output(settings_.file_name, ios::binary);
			if (!output) throw ios_base::failure("Failed to create a file "s + settings_.file_name);
			processing_data.SerializePartialToOstream(&output);
        }

        TransportCatalogue Serializator::DeserializeCotalogueData (const transport_catalogue_proto::CatalogueData& catalogue_data) {
			TransportCatalogue catalogue;

			const auto& stops = catalogue_data.stops();
			for (const transport_catalogue_proto::Stop& stop : stops) {
				catalogue.AddStop(stop.name(), {stop.latitude(), stop.longitude()});
			}

			auto stops_list_ptr = catalogue.GetAllStops();

			const auto& buses = catalogue_data.buses();
			for (const transport_catalogue_proto::Bus& bus : buses) {
				vector<string> stops(bus.stops_size());
				for (int i = 0; i < stops.size(); ++i) {
					string stop_name = stops_list_ptr->at(bus.stops(i)).name;
					stops[i] = stop_name;
				}

				catalogue.AddBus(bus.name(), stops, bus.is_roundtrip());
			}

			const auto& distance_between_stops = catalogue_data.distance_between_stops();
			for (const transport_catalogue_proto::Distance& distance_data : distance_between_stops) {
				auto stop_from = &stops_list_ptr->at(distance_data.stop_from());
				auto stop_to = &stops_list_ptr->at(distance_data.stop_to());
				catalogue.SetDistance(stop_from, stop_to, distance_data.distance());
			}

			return catalogue;
		}

		svg::Color GetColorFromColorData(const transport_catalogue_proto::Color& color_data) {
			svg::Color color;
			if (color_data.has_rgb()) {
				svg::Rgb rgb;
				// fill rgb
				{
					rgb.red = color_data.rgb().red();
					rgb.green = color_data.rgb().green();
					rgb.blue = color_data.rgb().blue();
				}

				color = move(rgb);
			} else if (color_data.has_rgba()) {
				svg::Rgba rgba;
				// fill rgba
				{
					rgba.red = color_data.rgba().red();
					rgba.green = color_data.rgba().green();
					rgba.blue = color_data.rgba().blue();
					rgba.opacity = color_data.rgba().opacity();
				}

				color = move(rgba);
			} else if (color_data.has_s_color()) {
				color = color_data.s_color();
			}

			return color;
		}

		map_renderer::RenderSettings Serializator::DeserializeRenderSettingsData (const transport_catalogue_proto::RenderSettings& render_settings_data) {
			map_renderer::RenderSettings render_settings;
			// filling render_settings
			{
				render_settings.width = render_settings_data.width();
				render_settings.height = render_settings_data.height();
				render_settings.padding = render_settings_data.padding();
				render_settings.line_width = render_settings_data.line_width();				
				render_settings.stop_radius = render_settings_data.stop_radius();	
				render_settings.bus_label_font_size = render_settings_data.bus_label_font_size();
				render_settings.bus_label_offset.x = render_settings_data.bus_label_offset().x();
				render_settings.bus_label_offset.y = render_settings_data.bus_label_offset().y();
				render_settings.stop_label_font_size = render_settings_data.stop_label_font_size();
				render_settings.stop_label_offset.x = render_settings_data.stop_label_offset().x();
				render_settings.stop_label_offset.y = render_settings_data.stop_label_offset().y();
				render_settings.underlayer_color = GetColorFromColorData(render_settings_data.underlayer_color());
				render_settings.underlayer_width = render_settings_data.underlayer_width();

				vector<svg::Color> color_palette(render_settings_data.color_palette_size());

				for (int i = 0; i < color_palette.size(); ++i) {
					color_palette[i] = GetColorFromColorData(render_settings_data.color_palette(i));
				}

				render_settings.color_palette = move(color_palette);
			}
			
			return render_settings;
		}

		void Serializator::DeserializeFile(TransportCatalogue& catalogue, map_renderer::MapRenderer& renderer) {
			ifstream input(settings_.file_name, ios::binary);
            if (!input) throw std::ios_base::failure("Failed to open a file " + settings_.file_name);
			transport_catalogue_proto::ProcessingData processing_data;
			if (!processing_data.ParseFromIstream(&input)) {
				throw std::runtime_error("The serialized file cannot be parsed from the istream");
			}

			catalogue = DeserializeCotalogueData(processing_data.catalogue());
			auto render_settings = DeserializeRenderSettingsData(*processing_data.mutable_render_settings());
			renderer.SetSettings(move(render_settings));
		}

    } // namespace serialization
} // namespace transport_catalogue