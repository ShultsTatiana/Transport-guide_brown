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


template <typename Number>
Number ReadNumberOnLine(std::istream& stream) {
    Number number;
    stream >> number;
    std::string dummy;
    std::getline(stream, dummy);
    return number;
}

std::pair<std::string_view, std::optional<std::string_view>> SplitTwoStrict(
    std::string_view s,
    std::string_view delimiter = " ");

std::pair<std::string_view, std::string_view> SplitTwo(
    std::string_view s, 
    std::string_view delimiter = " ");

std::string_view ReadToken(std::string_view& s, std::string_view delimiter = " ");

int ConvertToInt(std::string_view str);
double ConvertToDouble(std::string_view str);



static const double PI = 3.1415926535; //= acos(-1);
static const double ERTH_RADIUS = 6'371'000.0; // в метрах

template <typename Number>
void ValidateBounds(Number number_to_check, Number min_value, Number max_value) {
    if (number_to_check < min_value || number_to_check > max_value) {
        std::stringstream error;
        error << number_to_check << " is out of [" << min_value << ", " << max_value << "]";
        throw std::out_of_range(error.str());
    }
}

struct Location {
    double latitude = 0.0;
    double longitude = 0.0;

    Location(double latitudeDegree = 0.0, double longitudeDegree = 0.0) :
        latitude((latitudeDegree* PI) / 180),
        longitude((longitudeDegree* PI) / 180) {}

    Location(const Location& location) :
        latitude(location.latitude),
        longitude(location.longitude) {}

    static Location FromString(std::string_view& str);

    double arcLength(const Location& oher) const;
};

struct RouteType {
    std::vector<std::string> vectorRoute;
    std::unordered_set<std::string> setRroute;
    std::optional<char> routType_;

    static RouteType FromString(std::string_view& str);
};

struct Object {
    std::string name;
    //Object (std::string _name) : name(_name) {}
};

struct Stop : Object {
    Stop() : Object() {}
    std::optional<Location> location;
};
struct Bus : Object {
    Bus() : Object() {}
    std::optional<RouteType> routeType;
};

template <typename ObjectHolder>
class Request {
public:
    enum class Type {
        STOP,
        BUS,
    };

    Request(Type type, ObjectHolder object) : type(type), object(object){}
    static RequestHolder Create(Type type);
    virtual void ParseFrom(std::string_view) = 0;
    virtual ~Request() = default;

    std::string_view GetName() const;
    static std::optional<Request::Type> FromString(std::string_view& str);

    const Type type;
    ObjectHolder object;
};

//using ObjectHolder = std::unique_ptr<Object>;
using RequestHolder = std::unique_ptr<Request<std::unique_ptr<Object>>>;

class StopRequest :public Request<Object> {
public:
    StopRequest() : Request<Object>(Type::STOP, Stop()) {}
    void ParseFrom(std::string_view str) override;
};

class BusRequest : public Request<Object> {
public:
    BusRequest() : Request<Object>(Type::BUS, Bus()) {}
    void ParseFrom(std::string_view str) override;
};
static const std::unordered_map<std::string_view, Request<Object>::Type> STR_TO_REQUEST_TYPE = {
    {"Stop", Request<Object>::Type::STOP},
    {"Bus", Request<Object>::Type::BUS},
};

RequestHolder ParseRequest(std::string_view request_str);

std::vector<RequestHolder> ReadRequests(std::istream& in_stream = std::cin);

struct RequestResult {
    RequestResult() = default;
    RequestResult(std::string_view name_) : name(name_){}

    std::string_view name;
    virtual std::ostream& writingResult(std::ostream& out) = 0;
};

struct Result {
    size_t amountStops = 0;
    size_t uniqStops = 0;
    double lenRoute = 0.0;
};

struct BusResult:RequestResult {
    BusResult() = default;
    BusResult(std::string_view name_, std::optional<Result> result_):
        RequestResult(name_), result(std::move(result_)){}

    std::optional<Result> result = std::nullopt;
    std::ostream& writingResult(std::ostream& out) override;
};
struct StopResult :RequestResult {
    StopResult() = default;
    StopResult(std::string_view name_, std::optional<std::string> result_) :
        RequestResult(name_), result(std::move(result_)) {}

    std::optional<std::string> result = std::nullopt;
    std::ostream& writingResult(std::ostream& out) override;
};

std::ostream& writingResult(const std::vector<std::unique_ptr<RequestResult>>& busesResult, std::ostream& out = std::cout);

class Base {
    using busName = std::string_view;
    using stopName = std::string_view;


    std::unordered_map<stopName, RequestHolder> baseOfStop;
    std::unordered_map<stopName, std::set<busName>> baseOfStorBus;
    std::unordered_map<busName, RequestHolder> baseOfBus;
public:
    Base(std::vector<RequestHolder>& groundRequest) {
        baseUpdating(groundRequest);
    }
    void baseUpdating(std::vector<RequestHolder>& groundRequest);

    std::vector<std::unique_ptr<RequestResult>> checkRequests(const std::vector<RequestHolder>& checkRequest) const;

    BusResult findBus(busName name) const;
    StopResult findStop(stopName name) const;
};
Base ProcessRequests(std::vector<RequestHolder>& base);