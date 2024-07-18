#include <cassert>
#include <iostream>
#include "tests.h"
#include "transport_catalogue.h"

using namespace std;
using namespace transport_catalogue;

void TestGetRouteLength() {
    TransportCatalogue catalogue;
    catalogue.AddStop("from", {55.85623777240991, 38.965963307835146});
    catalogue.AddStop("to", {55.856631639749466, 38.96792060733355});
    vector<string_view> stops {"from", "to"};
    catalogue.AddBus("bus", stops);
    catalogue.SetDistance(catalogue.GetStop("from"), catalogue.GetStop("to"), 1500);
    auto route_info = catalogue.GetRouteInfo(catalogue.GetBus("bus"));
    assert(route_info.route_length == 1500);       
}

void TestDuration() {
    TransportCatalogue catalogue;

    {
        LOG_DURATION("Add Stop");
        for (int i = 1; i < 101; ++i) {
            catalogue.AddStop("stop" + i, {55.85623777240991, 38.965963307835146});
        }
    }

    {
        vector<string_view> stops {"stop1", "stop2", "stop3", "stop4"};
        LOG_DURATION("Add Bus");
        for (int i = 1; i < 101; ++i) {
            catalogue.AddBus("bus" + i, stops);
        }
    }

    {
        LOG_DURATION("Get Bus");
        for (int i = 1; i < 101; ++i) {
            catalogue.GetBus("bus" + i);
        }
    }

    {
        LOG_DURATION("Get Stop");
        for (int i = 1; i < 101; ++i) {
            catalogue.GetStop("stop" + i);
        }
    }

    {   
        const TransportCatalogue::Stop* from = catalogue.GetStop("stop1");
        const TransportCatalogue::Stop* to = catalogue.GetStop("stop2");
        LOG_DURATION("Set Distance");
        for (int i = 1; i < 101; ++i) {
            catalogue.SetDistance(from, to, 10000);
        }
    }

    /*{ //тест сломан
        const TransportCatalogue::Bus* bus = catalogue.GetBus("bus1");
        LOG_DURATION("Get Route Info");
        for (int i = 1; i < 101; ++i) {
            catalogue.GetRouteInfo(bus);
        }
    }*/

    {
        const TransportCatalogue::Stop* from = catalogue.GetStop("stop10");
        const TransportCatalogue::Stop* to = catalogue.GetStop("stop11");
        LOG_DURATION("Get Distance");
        for (int i = 1; i < 101; ++i) {
            catalogue.GetDistance(from, to);
        }
    }
}

/*void TestGetRouteInfo(){
    TransportCatalogue catalogue;
    catalogue.AddStop("stop1", {55.85623777240991, 38.965963307835146});
    catalogue.AddStop("stop2", {55.85623777240991, 38.965963307835146});
    vector<string_view> stops {"stop1", "stop2"};
    catalogue.AddBus("Bus", stops);
    const TransportCatalogue::Stop* from = catalogue.GetStop("stop1");
    const TransportCatalogue::Stop* to = catalogue.GetStop("stop2");
    catalogue.SetDistance(from, to, 1500);
    const TransportCatalogue::Bus* bus = catalogue.GetBus("Bus");
    catalogue.GetRouteInfoTest(bus);
}*/

void Tests() {
    //TestGetRouteLength();
    TestDuration();
    //TestGetRouteInfo();
}