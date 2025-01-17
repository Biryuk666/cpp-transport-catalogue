#include "request_handler.h"

using namespace std;

namespace transport_catalogue {
    namespace request_handler {

        RequestHandler::RequestHandler(const TransportCatalogue& catalogue, const map_renderer::MapRenderer& renderer) : catalogue_(catalogue), renderer_(renderer) {}

        svg::Document RequestHandler::RenderMap() const {
            return renderer_.RenderMap(catalogue_.GetBusesList());
        }
        
    } // namespace request_handler
} // namespace transport_catalogue {