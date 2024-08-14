#include <cassert>
#include <iostream>
#include "json_reader.h"
#include <fstream>
#include <sstream>
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
        LOG_DURATION("Add Stop x100");
        for (int i = 1; i < 101; ++i) {
            catalogue.AddStop("stop" + std::to_string(i), {55.85623777240991, 38.965963307835146});
        }
    }

    {
        vector<string_view> stops {"stop1", "stop2", "stop3", "stop4"};
        LOG_DURATION("Add Bus x100");
        for (int i = 1; i < 101; ++i) {
            catalogue.AddBus("bus" + std::to_string(i), stops);
        }
    }

    {
        LOG_DURATION("Get Bus x100");
        for (int i = 1; i < 101; ++i) {
            catalogue.GetBus("bus" + std::to_string(i));
        }
    }

    {
        LOG_DURATION("Get Stop x100");
        for (int i = 1; i < 101; ++i) {
            catalogue.GetStop("stop" + std::to_string(i));
        }
    }

    {   
        const domain::Stop* from = catalogue.GetStop("stop1");
        const domain::Stop* to = catalogue.GetStop("stop2");
        LOG_DURATION("Set Distance x100");
        for (int i = 1; i < 101; ++i) {
            catalogue.SetDistance(from, to, 10000);
        }
    }

    {
        const domain::Bus* bus = catalogue.GetBus("bus1");
        LOG_DURATION("Get Route Info x100");
        for (int i = 1; i < 101; ++i) {
            catalogue.GetRouteInfo(bus);
        }
    }

    {
        const domain::Stop* from = catalogue.GetStop("stop10");
        const domain::Stop* to = catalogue.GetStop("stop11");
        LOG_DURATION("Get Distance x100");
        for (int i = 1; i < 101; ++i) {
            catalogue.GetDistance(from, to);
        }
    }

    /*{
        TransportCatalogue catalogue;
        LOG_DURATION("rvalue AddBus x100");
        for (int i = 1; i < 101; ++i) {
            domain::Bus bus;
            bus.name = i;
            catalogue.AddBus(move(bus));
        }
    }

    {
        TransportCatalogue catalogue;
        LOG_DURATION("rvalue AddStop x100");
        for (int i = 1; i < 101; ++i) {
            domain::Stop stop;
            stop.name = i;
            catalogue.AddStop(move(stop));
        }
    }

    {
        TransportCatalogue catalogue;
        for (int i = 1; i < 101; ++i) {
            domain::Bus bus;
            bus.name = i;
            catalogue.AddBus(move(bus));
        }
        LOG_DURATION("Get Buses List x100");
        for (int i = 1; i < 101; ++i) {
            catalogue.GetBusesList();
        }
    }*/
}

void TestRequestProcessDuration() {
    {

        json_reader::JsonReader reader;
        map_renderer::MapRenderer renderer;
        ifstream input("input2.txt", ios::in);
        ostringstream output;
        TransportCatalogue catalogue;
    
        LOG_DURATION("Request process");
        //reader.RequestProcess(catalogue, input, output, renderer);
 
    }
/*
    {
        ifstream input("base_requests.txt", ios::in);
        ostringstream output;
        json_reader::JsonReader reader;
        map_renderer::MapRenderer renderer;
        TransportCatalogue catalogue;

        LOG_DURATION("Base Requests");
        reader.RequestProcess(catalogue, input, output, renderer);

    }

    {
        ifstream input1("base_requests.txt", ios::in);
        ifstream input2("stat_requests.txt", ios::in);

        ostringstream output;
        json_reader::JsonReader reader;
        map_renderer::MapRenderer renderer;
        TransportCatalogue catalogue;
        reader.RequestProcess(catalogue, input1, output, renderer);

        LOG_DURATION("Stat Requests");
        reader.RequestProcess(catalogue, input2, output, renderer);

    }

    {
        ifstream input("render_settings.txt", ios::in);
        ostringstream output;
        json_reader::JsonReader reader;
        map_renderer::MapRenderer renderer;
        TransportCatalogue catalogue;

        LOG_DURATION("Render Settings");         
        reader.RequestProcess(catalogue, input, output, renderer);

    }
    */

}

void TestRenderMapDuration() {
    
    json_reader::JsonReader reader;
    map_renderer::MapRenderer renderer;
    ifstream input("input2.txt", ios::in);
    ostringstream output;

    TransportCatalogue catalogue;
    //reader.RequestProcess(catalogue, input, output, renderer);

    LOG_DURATION("Render map");
    svg::Document doc = renderer.RenderMap(catalogue.GetBusesList());

}


/*void TestGetRouteInfo(){
    TransportCatalogue catalogue;
    catalogue.AddStop("stop1", {55.85623777240991, 38.965963307835146});
    catalogue.AddStop("stop2", {55.85623777240991, 38.965963307835146});
    vector<string_view> stops {"stop1", "stop2"};
    catalogue.AddBus("Bus", stops);
    const domain::Stop* from = catalogue.GetStop("stop1");
    const domain::Stop* to = catalogue.GetStop("stop2");
    catalogue.SetDistance(from, to, 1500);
    const domain::Bus* bus = catalogue.GetBus("Bus");
    catalogue.GetRouteInfoTest(bus);
}*/

void Tests() {
    //TestGetRouteLength();
    //TestDuration();
    TestRequestProcessDuration();
    TestRenderMapDuration();
    //TestGetRouteInfo();
}