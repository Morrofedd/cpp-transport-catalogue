#include <iostream>
#include <cstdio>
#include <string>

//#include "log_duration.h"
#include "json_reader.h"
#include "request_handler.h"
#include "map_renderer.h"

using namespace std;

int main() {
    TransportCatalogue::TransportCatalogue catalogue;
    json_reader reader(cin);
    request_handler rh(catalogue,reader);
    {
        //LOG_DURATION_STREAM("in", cerr);
        rh.MakeRequest(cout);
    }
}