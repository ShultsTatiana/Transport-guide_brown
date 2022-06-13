#pragma once

#include "helper.h"

#include <string_view>
#include <cmath>
#include <optional>
#include <utility>
#include <type_traits>
#include <typeinfo>
#include <cstdint>
#include <iterator>

struct Location {
    double latitude = 0.0;
    double longitude = 0.0;

    //Location() {}

    Location(double latitudeDegree = 0.0, double longitudeDegree = 0.0);

    Location(const Location& location)  noexcept;

    Location(Location&& location) noexcept;

    Location& operator = (Location&& location) noexcept;

    //------------- Direct distance by Earth surface ------------------------------------
    double arcLength(const Location& oher) const;

    static Location FromString(std::string_view& str);
};