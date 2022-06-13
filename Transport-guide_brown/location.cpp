#include "location.h"

using namespace std;

static const double PI = 3.1415926535; //= acos(-1);
static const double ERTH_RADIUS = 6'371'000.0; // в метрах
static const double PI_TO_DEGREE = PI / 180.0;


Location::Location(double latitudeDegree, double longitudeDegree) :
    latitude(latitudeDegree * PI_TO_DEGREE),
    longitude(longitudeDegree * PI_TO_DEGREE) {
}

Location::Location(const Location& location)  noexcept :
    latitude(location.latitude),
    longitude(location.longitude) {
}

Location::Location(Location&& location) noexcept :
    latitude(location.latitude),
    longitude(location.longitude)  {
}

Location& Location::operator = (Location&& location) noexcept {
    latitude = location.latitude;
    longitude = location.longitude;
    return *this;
}

//------------- Parsing from stream -----------------------------------------------------
Location Location::FromString(string_view& str) {
    double latitude = ConvertToDouble(ReadToken(str, ", "));
    double longitude = ConvertToDouble(ReadToken(str, ", "));
    return Location{ latitude, longitude };
}

//------------- Direct distance by Earth surface ----------------------------------------
double Location::arcLength(const Location& oher) const {
    return acos(
        sin(this->latitude) * sin(oher.latitude) +
        cos(this->latitude) * cos(oher.latitude) *
        cos(abs(this->longitude - oher.longitude))
    ) * ERTH_RADIUS; // Delta in meters
}


/**/