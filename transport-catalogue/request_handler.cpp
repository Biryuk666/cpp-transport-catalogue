#include "request_handler.h"

using namespace std;

namespace transport_catalogue {
    namespace request_handler {

        RequestHandler::RequestHandler(const TransportCatalogue& catalogue, const map_renderer::MapRenderer& renderer) : catalogue_(catalogue), renderer_(renderer) {}

        /*// Возвращает информацию о маршруте (запрос Bus)
        optional<domain::RouteInfo> RequestHandler::GetBusStat(const string_view& bus_name) const {
            auto bus = catalogue_.GetBus(bus_name);
            if (bus) {
                return catalogue_.GetRouteInfo(bus);
            }
            return nullopt;
        }

        // Возвращает маршруты, проходящие через
        const unordered_set<const domain::Bus*>* RequestHandler::GetBusesByStop(const string_view& stop_name) const {
            return catalogue_.GetBusesForStop(stop_name);
        }*/

        svg::Document RequestHandler::RenderMap() const {
            return renderer_.RenderMap(catalogue_.GetBusesList());
        }
        
    } // namespace request_handler
} // namespace transport_catalogue {