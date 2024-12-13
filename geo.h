#pragma once

#include <cmath>

namespace geo{
    static const double EMPTY_DATA_VALUE = NAN;

    struct Coordinates {
        double lat = EMPTY_DATA_VALUE;
        double lng = EMPTY_DATA_VALUE;
        bool operator==(const Coordinates& other) const {
            return lat == other.lat && lng == other.lng;
        }
        bool operator!=(const Coordinates& other) const {
            return !(*this == other);
        }
        [[nodiscard]] bool ValidateData() const{
            return !(std::isnan(lat) && std::isnan(lng));
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
}

