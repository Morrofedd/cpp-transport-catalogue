#pragma once

#include <cmath>
#include <algorithm>

namespace geo {
    struct Coordinates {
        double lat;
        double lng;
        bool operator==(const Coordinates& other) const {
            return lat == other.lat && lng == other.lng;
        }
        bool operator!=(const Coordinates& other) const {
            return !(*this == other);
        }
    };

    inline double ComputeDistance(Coordinates from, Coordinates to) {
        using namespace std;
        if (from == to) {
            return 0;
        }
        static const double dr = 3.1415926535 / 180.;
        return acos(sin(from.lat * dr) * sin(to.lat * dr)
            + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
            * 6371000;
    }

    struct MinMaxCoords {
        Coordinates max = { -180,-90 };
        Coordinates min = { 180,90 };
        void Upload(Coordinates c) {
            max.lat = std::max(max.lat, c.lat);
            max.lng = std::max(max.lng, c.lng); 
            min.lat = std::min(min.lat, c.lat);
            min.lng = std::min(min.lng, c.lng);
        }
    };
}
