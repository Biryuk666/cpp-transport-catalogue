#include "serialization.h"
#include "domain.h"

#include <algorithm>
#include <deque>
#include <fstream>
#include <iterator>
#include <map>
#include <set>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

using namespace std;

namespace transport_catalogue {
    namespace serialization {

		transport_catalogue_proto::CatalogueData Serializator::GetCatalogueDataForSerialization(const transport_catalogue::TransportCatalogue& catalogue) {
			transport_catalogue_proto::CatalogueData catalogue_data;
			
			const auto& stops_list = catalogue.GetAllStops();
			size_t id = 0;
			for (const domain::Stop& stop : stops_list) {
				transport_catalogue_proto::Stop stop_proto;
				// filling stop_proto
				{
					stop_proto.set_id(id);
					stop_proto.set_name(stop.name);
					stop_proto.set_latitude(stop.coordinates.lat);
					stop_proto.set_longitude(stop.coordinates.lng);
				}

				*catalogue_data.add_stops() = move(stop_proto);

				stop_name_to_stop_id_[stop.name] = id;
				++id;
			}

			const auto& buses_list = catalogue.GetAllBuses();
			id = 0;
			for (const domain::Bus& bus : buses_list) {
				transport_catalogue_proto::Bus bus_proto;
				// filling bus_proto
				{
					bus_proto.set_id(id);
					bus_proto.set_name(bus.name);

					for (const domain::Stop* stop : bus.stops) {
						bus_proto.add_stops(stop_name_to_stop_id_[stop->name]);
					}

					bus_proto.set_is_roundtrip(bus.is_roundtrip);
				}

				*catalogue_data.add_buses() = move(bus_proto);

				bus_name_to_bus_id_[bus.name] = id;
				++id;
			}

			auto distance_between_stops_list = catalogue.GetDistanceBetweenStopsList();
			for (const auto& [stops_pair, distance] : distance_between_stops_list) {
				transport_catalogue_proto::Distance distance_proto;
				// filling distance_proto
				{
					distance_proto.set_stop_from(stop_name_to_stop_id_[stops_pair.first->name]);
					distance_proto.set_stop_to(stop_name_to_stop_id_[stops_pair.second->name]);
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

		transport_catalogue_proto::RenderSettingsData Serializator::GetRenderSettingsDataForSerialization(const map_renderer::MapRenderer& renderer) const {
			map_renderer::RenderSettings render_settings = renderer.GetSettings();
			transport_catalogue_proto::RenderSettingsData render_settings_data;
			// filling render_settings_data
			{
				render_settings_data.set_width(render_settings.width);
				render_settings_data.set_height(render_settings.height);
				render_settings_data.set_padding(render_settings.padding);
				render_settings_data.set_line_width(render_settings.line_width);				
				render_settings_data.set_stop_radius(render_settings.stop_radius);
				render_settings_data.set_bus_label_font_size(render_settings.bus_label_font_size);
				render_settings_data.mutable_bus_label_offset()->set_x(render_settings.bus_label_offset.x);
				render_settings_data.mutable_bus_label_offset()->set_y(render_settings.bus_label_offset.y);
				render_settings_data.set_stop_label_font_size(render_settings.stop_label_font_size);
				render_settings_data.mutable_stop_label_offset()->set_x(render_settings.stop_label_offset.x);
				render_settings_data.mutable_stop_label_offset()->set_y(render_settings.stop_label_offset.y);
				*render_settings_data.mutable_underlayer_color() = GetColorData(render_settings.underlayer_color);
				render_settings_data.set_underlayer_width(render_settings.underlayer_width);

				for (const auto& color : render_settings.color_palette) {
					*render_settings_data.add_color_palette() = GetColorData(color);
				}				
			}

			return render_settings_data;
		}

		transport_catalogue_proto::DirectedWeightedGraph GetGraphData(const graph::DirectedWeightedGraph<double>& graph) {
			transport_catalogue_proto::DirectedWeightedGraph graph_proto;
				// filling graph_proto
			{
				const vector<graph::Edge<double>>& edges = graph.GetEdges();

				for (const auto& edge : edges) {
					transport_catalogue_proto::Edge edge_proto;
					// filling edge_proto
					{
						edge_proto.set_from(edge.from);
						edge_proto.set_to(edge.to);
						edge_proto.set_weight(edge.weight);
					}
					*graph_proto.add_edges() = move(edge_proto);
				}

				const vector<graph::IncidenceList>& incidence_lists = graph.GetIncidenceLists();

				for (const auto& incidence_list : incidence_lists) {
				transport_catalogue_proto::IncidenceList incidence_list_proto;

					for (auto edge_id : incidence_list) {
						incidence_list_proto.add_incidence_list(edge_id);
					}

					*graph_proto.add_incidence_lists() = move(incidence_list_proto);
				}
			}

			return graph_proto;
		}

		transport_catalogue_proto::Router GetRouterData(const graph::Router<double>& router) {
			transport_catalogue_proto::Router router_proto;
			graph::RoutesInternalData routes_iternal_data = router.GetRoutesInternalData();

			for (size_t i = 0; i < routes_iternal_data.size(); ++i) {
				transport_catalogue_proto::VectorOfRouteIternalData data_proto;
				//filling data_proto
				{
					for (size_t j = 0; j < routes_iternal_data[i].size(); ++j) {
						transport_catalogue_proto::OptionalRouteIternalData route_iternal_data_proto;
						// filling route_iternal_data_proto
						{
							if (routes_iternal_data[i][j]) {
								route_iternal_data_proto.mutable_route_iternal_data()->set_weight(routes_iternal_data[i][j]->weight);
								if (routes_iternal_data[i][j]->prev_edge) {
									route_iternal_data_proto.mutable_route_iternal_data()->set_value(routes_iternal_data[i][j]->prev_edge.value());
								} else {
									route_iternal_data_proto.mutable_route_iternal_data()->set_empty(true);
								}
							} else {
								route_iternal_data_proto.set_empty(true);
							}						
						}

						*data_proto.add_vector_of_route_iternal_data() = move(route_iternal_data_proto);
					}
				}
				*router_proto.add_routes_iternal_data() = move(data_proto);
			}

			return router_proto;
		}

		transport_catalogue_proto::RouterData Serializator::GetRouterDataForSerialization(const transport_router::TransportRouter& router) {
			transport_catalogue_proto::RouterData router_data;

			const transport_router::RouterSettings& router_settings = router.GetRouterSettings();
			transport_catalogue_proto::RouterSettings router_settings_proto;
			// filling router_settings_proto
			{
				router_settings_proto.set_bus_wait_time(router_settings.bus_wait_time);
				router_settings_proto.set_bus_velocity(router_settings.bus_velocity);
			}
			*router_data.mutable_router_settings() = move(router_settings_proto);

			const auto& graph_ptr = router.GetGraphPtr();
			*router_data.mutable_graph() = GetGraphData(*graph_ptr);

			const auto& router_ptr = router.GetRouterPtr();
			*router_data.mutable_router() = GetRouterData(*router_ptr);

			const auto& stop_to_stop_vertex = router.GetStopToStopVertexMap();			
			for (const auto& [stop_ptr, stop_vertex] : stop_to_stop_vertex) {
				transport_catalogue_proto::StopToStopVertex stop_to_stop_vertex_proto;
				// filling stop_to_stop_vertex_proto
				{
					stop_to_stop_vertex_proto.set_stop_name(stop_name_to_stop_id_[stop_ptr->name]);
					stop_to_stop_vertex_proto.mutable_stop_vertex()->set_wait(stop_vertex.wait);
					stop_to_stop_vertex_proto.mutable_stop_vertex()->set_bus(stop_vertex.bus);
				}

				*router_data.add_map_of_stop_to_stop_vertex() = move(stop_to_stop_vertex_proto);
			}

			const auto& edge_id_to_item = router.GetEdgeIdToItemMap();
			for (const auto& [edge_id, item] : edge_id_to_item) {
				transport_catalogue_proto::EdgeIdToItem edge_id_to_item_proto;
				// filling edge_id_to_item_proto
				{
					edge_id_to_item_proto.set_edge_id(edge_id);
					if (item.type == "Bus"s) {
						edge_id_to_item_proto.mutable_item()->set_type(0);
						edge_id_to_item_proto.mutable_item()->set_name(bus_name_to_bus_id_[item.name]);
					}
					else {
						edge_id_to_item_proto.mutable_item()->set_type(1);
						edge_id_to_item_proto.mutable_item()->set_name(stop_name_to_stop_id_[item.name]);
					}
					edge_id_to_item_proto.mutable_item()->set_time(item.time);
					edge_id_to_item_proto.mutable_item()->set_span_count(item.span_count);
				}
				*router_data.add_map_of_edge_id_to_item() = move(edge_id_to_item_proto);
			}

				return router_data;
		}

        void Serializator::SetSettings(std::string&& file_name) {
			settings_.file_name = move(file_name);
        }

        void Serializator::SerializeData(const TransportCatalogue &catalogue, const map_renderer::MapRenderer& renderer, const transport_router::TransportRouter& router)
        {
            auto catalogue_data = GetCatalogueDataForSerialization(catalogue);
			auto render_settings_data = GetRenderSettingsDataForSerialization(renderer);
			auto router_data = GetRouterDataForSerialization(router);
			transport_catalogue_proto::ProcessingData processing_data;
			*processing_data.mutable_catalogue() = move(catalogue_data);
			*processing_data.mutable_render_settings() = move(render_settings_data);
			*processing_data.mutable_router() = move(router_data);

			std::ofstream output(settings_.file_name, ios::binary);
			if (!output) throw ios_base::failure("Failed to create a file "s + settings_.file_name);
			processing_data.SerializePartialToOstream(&output);
        }

        TransportCatalogue Serializator::DeserializeCotalogueData (const transport_catalogue_proto::CatalogueData& catalogue_data) {
			TransportCatalogue catalogue;
			// filling catalogue
			{
				const auto& stops_proto = catalogue_data.stops();
				for (const transport_catalogue_proto::Stop& stop : stops_proto) {
					catalogue.AddStop(stop.name(), { stop.latitude(), stop.longitude() });
					stop_id_to_stop_name_[stop.id()] = stop.name();
				}

				const auto& buses_proto = catalogue_data.buses();
				for (const transport_catalogue_proto::Bus& bus : buses_proto) {
					vector<string> stops(bus.stops_size());
					for (int i = 0; i < stops.size(); ++i) {
						string stop_name = stop_id_to_stop_name_[bus.stops(i)];
						stops[i] = stop_name;
					}

					catalogue.AddBus(bus.name(), stops, bus.is_roundtrip());
					bus_id_to_bus_name_[bus.id()] = bus.name();
				}

				const auto& distance_between_stops_proto = catalogue_data.distance_between_stops();
				for (const auto& distance_data : distance_between_stops_proto) {
					const auto& stop_from = catalogue.GetStop(stop_id_to_stop_name_[distance_data.stop_from()]);
					const auto& stop_to = catalogue.GetStop(stop_id_to_stop_name_[distance_data.stop_to()]);
					catalogue.SetDistance(stop_from, stop_to, distance_data.distance());
				}
			}

			return catalogue;
		}

		svg::Color GetColorFromColorData(const transport_catalogue_proto::Color& color_data) {
			svg::Color color;
			// filling color
			{
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
			}

			return color;
		}

		map_renderer::RenderSettings Serializator::DeserializeRenderSettingsData(const transport_catalogue_proto::RenderSettingsData& render_settings_data) {
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

		void Serializator::DeserializeRouterData(const transport_catalogue_proto::RouterData& router_data, const TransportCatalogue& catalogue, transport_router::TransportRouter& router) {
			transport_router::RouterData import_data;

			const auto& router_settings_proto = router_data.router_settings();
			auto& router_settings = import_data.settings;
			// filling router_settings
			{
				router_settings.bus_wait_time = router_settings_proto.bus_wait_time();
				router_settings.bus_velocity = router_settings_proto.bus_velocity();
			}

			const auto& graph_proto = router_data.graph();
			auto& edges = import_data.edges;
			edges.resize(graph_proto.edges_size());			
			// filling edges
			{
				for (int i = 0; i < edges.size(); ++i) {
					edges[i].from = graph_proto.edges(i).from();
					edges[i].to = graph_proto.edges(i).to();
					edges[i].weight = graph_proto.edges(i).weight();
				}
			}

			auto& incidence_lists = import_data.incidence_lists;
			incidence_lists.resize(graph_proto.incidence_lists_size());
			// filling incidence_lists
			{
				for (int i = 0; i < incidence_lists.size(); ++i) {
					incidence_lists[i].resize(graph_proto.incidence_lists(i).incidence_list_size());
					for (int j = 0; j < incidence_lists[i].size(); ++j) {
						incidence_lists[i][j] = graph_proto.incidence_lists(i).incidence_list(j);
					}
				}
			}

			const auto& router_proto = router_data.router();
			auto& routes_iternal_data = import_data.routes_iternal_data;
			routes_iternal_data.resize(router_proto.routes_iternal_data_size());
			// filling routes_iternal_data
			{
				for (int i = 0; i < routes_iternal_data.size(); ++i) {
					routes_iternal_data[i].resize(router_proto.routes_iternal_data(i).vector_of_route_iternal_data_size());
					for (int j = 0; j < routes_iternal_data[i].size(); ++j) {
						const auto& data = router_proto.routes_iternal_data(i).vector_of_route_iternal_data(j);
						if (!data.has_empty()) {
							graph::RouteInternalData<double> iternal_data_temp;
							iternal_data_temp.weight = data.route_iternal_data().weight();
							if (!data.route_iternal_data().has_empty()) {
								iternal_data_temp.prev_edge = data.route_iternal_data().value();
							}
							routes_iternal_data[i][j] = move(iternal_data_temp);
						}
					}
				}
			}
;
			auto& stop_to_stop_vertex = import_data.stop_to_stop_vertex;
			// filling stop_to_stop_vertex
			{
				for (int i = 0; i < router_data.map_of_stop_to_stop_vertex_size(); ++i) {
					const auto& data = router_data.map_of_stop_to_stop_vertex(i);
					transport_router::StopVertex stop_vertex_temp;
					// filling stop_vertex_temp
					{
						stop_vertex_temp.wait = data.stop_vertex().wait();
						stop_vertex_temp.bus = data.stop_vertex().bus();
					}

					const auto& stop_ptr = catalogue.GetStop(stop_id_to_stop_name_[data.stop_name()]);
					stop_to_stop_vertex[stop_ptr] = move(stop_vertex_temp);
				}
			}

			auto& edge_id_to_item = import_data.edge_id_to_item;
			// filling edge_id_to_item
			{
				for (int i = 0; i < router_data.map_of_edge_id_to_item_size(); ++i) {
					const auto& data = router_data.map_of_edge_id_to_item(i);
					transport_router::Item item_temp;
					// filling item_temp
					{
						if (data.item().type() == 0) {
							item_temp.type = "Bus"s;
							item_temp.name = bus_id_to_bus_name_[data.item().name()];
						} else {
							item_temp.type = "Wait"s;
							item_temp.name = stop_id_to_stop_name_[data.item().name()];
						}

						item_temp.time = data.item().time();
						item_temp.span_count = data.item().span_count();
					}

					edge_id_to_item[data.edge_id()] = move(item_temp);
				}
			}

			router.SetRouterData(move(import_data));
		}

		void Serializator::DeserializeFile(TransportCatalogue& catalogue, map_renderer::MapRenderer& renderer, transport_router::TransportRouter& router) {
			ifstream input(settings_.file_name, ios::binary);
            if (!input) throw std::ios_base::failure("Failed to open a file " + settings_.file_name);
			transport_catalogue_proto::ProcessingData processing_data;
			if (!processing_data.ParseFromIstream(&input)) {
				throw std::runtime_error("The serialized file cannot be parsed from the istream");
			}

			catalogue = DeserializeCotalogueData(processing_data.catalogue());
			auto render_settings = DeserializeRenderSettingsData(processing_data.render_settings());
			renderer.SetSettings(move(render_settings));
			DeserializeRouterData(processing_data.router(), catalogue, router);
		}

    } // namespace serialization
} // namespace transport_catalogue