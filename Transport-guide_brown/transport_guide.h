#pragma once

#include <string_view>
#include <iostream>
#include <sstream>
#include <string>
#include <optional>
#include <utility>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <map>
#include <memory>
#include <exception>
#include <system_error>
#include <vector>
#include <typeinfo>
#include <cmath>
#include <cstdint>
#include <iterator>
#include <cmath>
#include <exception>
#include <iomanip>
#include <list>


//++++++++++++  Help part +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//------------- Parsing string by parts -------------------------------------------------
std::pair<std::string_view, std::optional<std::string_view>> SplitTwoStrict(
    std::string_view s,
    std::string_view delimiter = " ");

std::pair<std::string_view, std::string_view> SplitTwo(
    std::string_view s, 
    std::string_view delimiter = " ");

std::string_view ReadToken(std::string_view& s, std::string_view delimiter = " ");

//------------- Convert string to Int and Double ----------------------------------------
template <typename Number>
Number ReadNumberOnLine(std::istream& stream) {
    Number number;
    stream >> number;
    std::string dummy;
    std::getline(stream, dummy);
    return number;
}
int ConvertToInt(std::string_view str);
double ConvertToDouble(std::string_view str);

template <typename Number>
void ValidateBounds(Number number_to_check, Number min_value, Number max_value) {
    if (number_to_check < min_value || number_to_check > max_value) {
        std::stringstream error;
        error << number_to_check << " is out of [" << min_value << ", " << max_value << "]";
        throw std::out_of_range(error.str());
    }
}


//++++++++++++  Location part +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static const double PI = 3.1415926535; //= acos(-1);
static const double ERTH_RADIUS = 6'371'000.0; // в метрах

struct Location {
    double latitude = 0.0;
    double longitude = 0.0;

    Location(double latitudeDegree = 0.0, double longitudeDegree = 0.0) :
        latitude((latitudeDegree* PI) / 180),
        longitude((longitudeDegree* PI) / 180) {
    }

    Location(const Location& location) :
        latitude(location.latitude),
        longitude(location.longitude) {
    }

    Location& operator = (Location&& location) noexcept {
        latitude = location.latitude;
        longitude = location.longitude;
        return *this;
    }

    static Location FromString(std::string_view& str);

    double arcLength(const Location& oher) const;
};


//++++++++++++  Parsing Stop and Bus from Request from string +++++++++++++++++++++++++++
struct Stop {
    std::string name;
    std::optional<Location> location;
    std::vector<std::pair<std::string, int>> distance;

    static Stop FromString(std::string_view& str);
};

struct Bus {
    std::string name;
    std::optional<char> routType_;
    std::vector<std::string> vectorRoute;

    static Bus FromString(std::string_view& str);
};


//++++++++++++  Request  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class Request;
using RequestHolder = std::unique_ptr<Request>;

class Request {
public:
    enum class Type {
        STOP,
        BUS,
    };

    Request(Type type) : type(type) {}
    static RequestHolder Create(Type type);
    virtual void ParseFrom(std::string_view) = 0;
    virtual ~Request() = default;

    static std::optional<Request::Type> FromString(std::string_view& str);

    const Type type;
    std::optional<Stop> stop;
    std::optional<Bus> bus;
};

class StopRequest : public Request {
public:
    StopRequest() : Request(Type::STOP) {}
    void ParseFrom(std::string_view str) override;
};

class BusRequest : public Request {
public:
    BusRequest() : Request(Type::BUS) {}
    void ParseFrom(std::string_view str) override;
};

static const std::unordered_map<std::string_view, Request::Type> STR_TO_REQUEST_TYPE = {
    {"Stop", Request::Type::STOP},
    {"Bus", Request::Type::BUS},
};

//------------- Parsing one Request from string (main) ----------------------------------
RequestHolder ParseRequest(std::string_view request_str);

//------------- Parsing all Requests from string (main) ---------------------------------
std::vector<RequestHolder> ReadRequests(std::istream& in_stream = std::cin);


//++++++++++++  RequestResult  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
struct RequestResult {
    RequestResult() = default;
    RequestResult(std::string name_) : name(move(name_)){}

    std::string name;
    virtual std::ostream& writingResult(std::ostream& out) const = 0 ;
};

struct Result {
    int amountStops = 0;
    int uniqStops = 0;
    int lenRoute = 0;
    double curvature = 0.0;
};

struct BusResult:RequestResult {
    BusResult() = default;
    BusResult(std::string name_, std::optional<Result> result_):
        RequestResult(move(name_)), result(std::move(result_)){}

    std::optional<Result> result = std::nullopt;
    std::ostream& writingResult(std::ostream& out) const override;
};
struct StopResult :RequestResult {
    StopResult() = default;
    StopResult(std::string name_, std::optional<std::string> result_) :
        RequestResult(move(name_)), result(move(result_)) {}

    std::optional<std::string> result;
    std::ostream& writingResult(std::ostream& out) const override;
};

//------------- Writing Result in ostream -----------------------------------------------
std::ostream& writingResult(
    const std::vector<std::unique_ptr<RequestResult>>& busesResult, 
    std::ostream& out = std::cout);


//++++++++++++  TransportGuide  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static const int N_REQUESTS = 2000; // max Query count in this part

class TransportGuide {
    using hashStop = int;
    using hashBus = int;

    class StopOnRout {
        Location location;
        std::vector<int> distanceTo; // max ~ N_REQUESTS остановок
        // Гарантируется, что для любых двух соседних остановок любого маршрута
        // так или иначе задано расстояние по дорогам.
        std::set<std::string_view> busesOnStop; // max ~ N_REQUESTS автобусов
    public:
        StopOnRout() : distanceTo(N_REQUESTS, -1) {
        }
        // Set Stop methods
        void setLocation(Location& location_);
        void addDistanceToNextStop(hashStop stopTo, int distanse);
        void addBus(std::string_view busName);

        // Get Stop methods 
        const Location& getLocation() const;
        int             getDistanceTo(hashStop stopTo) const;
        bool            checkBus(std::string_view busName) const;
        const  std::set<std::string_view>&  getBuses() const;
    };

    class Route {
    public:
        enum class Type {
            CIRCULAR = 0,   // круговой
            DIRECT = 1,     // туда и обратно
        };
    private:
        struct Distance {
            int length = 0;         // N_остановок*D_пути = 100*10^6 = max 10^8
            double directLength = 0;   // географическое расстояние
            double curvature = 1;      // length/directLength > 1
        };

        Distance distance;    
        std::optional<Type> type;       // тип маршрута
        std::list<hashStop> route;              // max 100 остановок в маршруте
        std::set<hashStop> uniqStopsOnRoute;    // max 100 остановок в маршруте

        // Methods for update distance part of Rout
        void updateLength(int length);
        void updateDirectLength(double directLength);
        void updateCurvature();
    public:
        // Update Type, Rout and distance on route
        void        setType(char chType);
        hashStop    addStopOnRoute(hashStop stop);
        void        updateDistances(int length, double directLength);

        // Get route parameters
        int     getStopOnRouteCount() const;
        int     getUniqStopOnRouteCount() const;
        int     getLength() const;
        double  getDirectLength() const;
        double  getCurvature() const;
        Type    getType() const;
    };

    std::vector<StopOnRout> stops;
    std::unordered_map<std::string, hashStop> hashStops;

    std::vector<Route> buses;
    std::unordered_map<std::string, hashBus> hashBuses;

    // small - update and create
    hashStop addNewStop(std::string stopName);
    void addDistance(hashStop from, hashStop to, int distance);
    auto addNewBus(std::string busName);
    void updateDistanceRoute(hashStop from, hashStop to, hashBus busIndex);
    
    // Process base Update request
    void addStop(Stop& stopFromRequest);
    // заполняем маршруты после остановок
    void addRoute(Bus& busFromRequest);

    // Get result
    BusResult getBusResult(std::string busName) const;
    StopResult getStopResult(std::string stopName) const;

public:
    TransportGuide() {
        stops.reserve(N_REQUESTS);
        buses.reserve(N_REQUESTS);
    }

    // Process Updating base from request
    void readRequests(std::vector<RequestHolder>& requests);

    // Process get info from base (Chek)
    std::vector<std::unique_ptr<RequestResult>> checkRequests(
        std::vector<RequestHolder>& requests
    ) const;
};