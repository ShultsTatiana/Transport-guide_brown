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
#include <memory>
#include <exception>
#include <system_error>
#include <vector>
#include <typeinfo>
#include <cmath>
#include <cstdint>
#include <iterator>
#include <cmath>


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

class Request;
using RequestHolder = std::unique_ptr<Request>;

struct Location {
    double latitude;
    double longitude;
};
struct RouteType {
    std::vector<std::string> vectorRoute;
    std::unordered_set<std::string> setRroute;
    std::optional<char> routType_;
};
struct ObjectRequest {
    std::string name;
    std::optional<Location> location;
    std::optional<RouteType> routeType;
};

class Request {
public:
    enum class Type {
        STOP,
        BUS,
    };

    Request(Type type) : type(type) {}
    static RequestHolder Create(Type type);
    virtual void ParseFrom(std::string_view) = 0;
    virtual std::string_view GetName() const = 0;
    virtual ~Request() = default;

    const Type type;
    using stopName = std::string;
    using busName = std::string;
    ObjectRequest object;
};

const std::unordered_map<std::string_view, Request::Type> STR_TO_REQUEST_TYPE = {
    {"Stop", Request::Type::STOP},
    {"Bus", Request::Type::BUS},
};

static const double PI(3.1415926535); //= acos(-1);
static const long ERTH_RADIUS(6371 * 1000);


class StopRequest :public Request {
public:
    StopRequest() : Request(Type::STOP) {}
    void ParseFrom(std::string_view str) override;
    std::string_view GetName() const override;
};
class BusRequest : public Request {
public:
    //mb rout type like enumClass
    BusRequest() : Request(Type::BUS) {}
    void ParseFrom(std::string_view str) override;
    std::string_view GetName() const override;
};

std::optional<Request::Type> ConvertRequestTypeFromString(std::string_view type_str);

RequestHolder ParseRequest(std::string_view request_str);

//std::vector<RequestHolder> FillBase(std::istream& in_stream = std::cin);
std::vector<RequestHolder> ReadRequests(std::istream& in_stream = std::cin);

double arcLength(const Location& loc1, const Location& loc2);

struct Result {
    size_t amountStops;
    size_t uniqStops;
    double lenRoute;
};

struct BusResult {
    std::string_view bus;
    std::optional<Result> result;
};

std::ostream& writingResult(const std::vector<BusResult>& busesResult, std::ostream& out = std::cout);

class Base {
    using busName = std::string_view;
    using stopName = std::string_view;


    std::unordered_map<stopName, RequestHolder> baseOfStop;
    std::unordered_map<busName, RequestHolder> baseOfBus;
public:
    Base(std::vector<RequestHolder>& groundRequest) {
        baseUpdating(groundRequest);
    }
    void baseUpdating(std::vector<RequestHolder>& groundRequest);

    std::vector<BusResult> checkRequests(const std::vector<RequestHolder>& checkRequest) const;

    BusResult findBus(busName name) const;
};
Base ProcessRequests(std::vector<RequestHolder>& base);