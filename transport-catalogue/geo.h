#pragma once
#include <cmath>

namespace geo{

    struct Coordinates{
        double lat = NAN;
        double lng = NAN;

        bool operator==(const Coordinates &other) const{
            return lat == other.lat && lng == other.lng;
        }

        bool operator!=(const Coordinates &other) const{
            return !(*this == other);
        }

        [[nodiscard]] bool ValidateData() const{
            return !(std::isnan(lat) && std::isnan(lng));
        }
    };

    double ComputeDistance(Coordinates from, Coordinates to);
}

