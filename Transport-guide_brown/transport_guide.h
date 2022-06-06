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

    std::string_view GetName();

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

RequestHolder ParseRequest(std::string_view request_str);

std::vector<RequestHolder> ReadRequests(std::istream& in_stream = std::cin);

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

std::ostream& writingResult(const std::vector<std::unique_ptr<RequestResult>>& busesResult, std::ostream& out = std::cout);
/*
class Base {
    using busName = std::string_view;
    using stopName = std::string_view;


    std::unordered_map<stopName, RequestHolder> baseOfStop;
    std::unordered_map<stopName, std::set<busName>> baseOfStorBus;
    std::unordered_map<busName, RequestHolder> baseOfBus;
    std::unordered_map<stopName, std::unordered_map<stopName, double>> baseOfDistance;

    double distance(stopName from, stopName to) const;
    double direct(stopName from, stopName to) const;
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
*/
static const int N_REQUESTS = 2000;

class TransportGuide {
    using hashStop = int;
    using hashBus = int;

    class StopOnRout {
        Location location;
        std::vector<double> distanceTo; // max ~ N_REQUESTS остановок
        // Гарантируется, что для любых двух соседних остановок любого маршрута
        // так или иначе задано расстояние по дорогам.
        std::set<std::string_view> busesOnStop; // max ~ N_REQUESTS автобусов
    public:
        StopOnRout() : distanceTo(N_REQUESTS, -1.) {
        }
        void setLocation(Location& location_) {
            location.latitude = location_.latitude;
            location.longitude = location_.longitude;
        }
        void addDistanceToNextStop(hashStop stopTo, double distanse) {
            distanceTo[stopTo] = distanse;
        }
        void addBus(std::string_view busName) {
            busesOnStop.insert(busName);
        }
        const Location& getLocation() const {
            return location;
        }
        double getDistanceTo(hashStop stopTo) const {
            return distanceTo[stopTo];
        }
        bool checkBus(std::string_view busName) const {
            return busesOnStop.count(busName);
        }
        const std::set<std::string_view>& getBuses() const {
            return busesOnStop;
        }
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

        void updateLength(double length) {
            distance.length += length;
        }
        void updateDirectLength(double directLength) {
            if (type == Type::DIRECT) {
                distance.directLength += (2. * directLength);
            }
            else if (type == Type::CIRCULAR) {
                distance.directLength += directLength;
            }
        }
        void updateCurvature() {
            distance.curvature = distance.length / distance.directLength;
        }
    public:
        void setType(char chType) {
            if (chType == '-') {
                type = Type::DIRECT;
            }
            else if (chType == '>') {
                type = Type::CIRCULAR;
            }
        }
        hashStop addStopOnRoute(hashStop stop) {
            route.push_back(stop);
            uniqStopsOnRoute.insert(stop);
            return route.size() == 1 ? -1 : *(next(route.rbegin()));
        }
        void updateDistances(double length, double directLength) {
            updateLength(length);
            updateDirectLength(directLength);
            updateCurvature();
        }
        int getStopOnRouteCount() const {
            if (type == Type::DIRECT) {
                return route.size()*2 - 1;
            }
            else if (type == Type::CIRCULAR) {
                return route.size();
            }
        }
        int getUniqStopOnRouteCount() const {
            return uniqStopsOnRoute.size();
        }
        double getDirectLength() const {
            return distance.directLength;
        }
        int getLength() const {
            return distance.length;
        }
        double getCurvature() const {
            return distance.curvature;
        }
        Type getType() const {
            return *type;
        }
    };

    std::vector<StopOnRout> stops;
    std::unordered_map<std::string, hashStop> hashStops;

    std::vector<Route> buses;
    std::unordered_map<std::string, hashBus> hashBuses;

    hashStop addNewStop(std::string stopName) {
        hashStop stopIndex;
        if (auto it = hashStops.find(stopName);
            it == hashStops.end()) {
            stops.push_back(StopOnRout());
            stopIndex = stops.size() - 1;
            hashStops[move(stopName)] = stopIndex;
        }
        else {
            stopIndex = it->second;
        }
        return stopIndex;
    }
    void addDistance(hashStop from, hashStop to, double distance) {
        stops[from].addDistanceToNextStop(to, distance);
        if (stops[to].getDistanceTo(from) < 0.) {
            stops[to].addDistanceToNextStop(from, distance);
        }
    }
    auto addNewBus(std::string busName) {
        if (auto it = hashBuses.find(busName);
            it == hashBuses.end()) {
            buses.push_back(Route());
            return hashBuses.insert({ move(busName), buses.size() - 1 }).first;
        }
        else {
            return it;
        }
    }
    void updateDistanceRoute(hashStop from, hashStop to, hashBus busIndex) {
        if (from != -1) {
            double length = stops[from].getDistanceTo(to);
            if (buses[busIndex].getType() == Route::Type::DIRECT) {
                length += stops[to].getDistanceTo(from);
            }

            double direct =
                stops[from].getLocation().arcLength(stops[to].getLocation());

            buses[busIndex].updateDistances(length, direct);
        }
    }
    
    void addStop(Stop& stopFromRequest) {
        hashStop stopFrom = addNewStop(move(stopFromRequest.name));
        stops[stopFrom].setLocation(*(stopFromRequest.location));

        for (auto [stopToName, distance] : stopFromRequest.distance) {
            hashStop stopTo = addNewStop(move(stopToName));
            addDistance(stopFrom, stopTo, distance);
        }
    }
    // заполняем маршруты после остановок
    void addRoute(Bus& busFromRequest) {
        auto busIt = addNewBus(move(busFromRequest.name)); 
        buses[busIt->second].setType(*(busFromRequest.routType_));

        for (size_t i(0); i < busFromRequest.vectorRoute.size(); ++i) {
            hashStop stopIndex = addNewStop(move(busFromRequest.vectorRoute[i]));

            stops[stopIndex].addBus(busIt->first);

            hashStop stopBefore = buses[busIt->second].addStopOnRoute(stopIndex);

            updateDistanceRoute(stopBefore, stopIndex, busIt->second);
        }
    }

    BusResult getBusResult(std::string busName) {
        if (auto it = hashBuses.find(busName); it != hashBuses.end()) {
            const Route& route = buses[it->second];
            return { 
                busName, 
                Result{
                    .amountStops = route.getStopOnRouteCount(),
                    .uniqStops = route.getUniqStopOnRouteCount(),
                    .lenRoute = route.getLength(),
                    .curvature = route.getCurvature()
                }
            };
        }
        else {
            return { busName, std::nullopt };
        }
    }
    StopResult getStopResult(std::string stopName) {
        if (auto it = hashStops.find(stopName); it != hashStops.end()) {
            const StopOnRout& stop = stops[it->second];
            std::string result;
            for (std::string_view bus : stop.getBuses()) {
                result += ' ';
                result += bus;
            }
            return { stopName, move(result) };
        }
        else {
            return { stopName, std::nullopt };
        }
    }

public:
    TransportGuide() {
        stops.reserve(N_REQUESTS);
        buses.reserve(N_REQUESTS);
    }
    
    void readRequests(std::vector<RequestHolder>& requests) {
        for (RequestHolder& request : requests) {
            if (request->type == Request::Type::STOP) {
                addStop(*(request->stop));
            }
        }
        for (RequestHolder& request : requests) {
            if (request->type == Request::Type::BUS) {
                addRoute(*(request->bus));
            }
        }
    }

    std::vector<std::unique_ptr<RequestResult>> checkRequests(std::vector<RequestHolder>& requests) {
        std::vector<std::unique_ptr<RequestResult>> result;
        for (const auto& request : requests) {
            if (request->type == Request::Type::BUS) {
                result.push_back(
                    std::make_unique<BusResult>(getBusResult(move(request->bus->name)))
                );
            }
            else if (request->type == Request::Type::STOP) {
                result.push_back(
                    std::make_unique<StopResult>(getStopResult(move(request->stop->name)))
                );
            }
        }
        return result;
    }
};