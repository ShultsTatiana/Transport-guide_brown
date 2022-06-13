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
#include <exception>
#include <iomanip>
#include <list>


#include "helper.h"
#include "location.h"
#include "fromStream.h"
#include "json.h"


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
        void setLocation(Location location_);
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
        void        setType(bool is_roundtrip);
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
    //from Stream -----------------------------------------------------------------------
    void addStop(Stream::Stop& stopFromRequest);
    // заполняем маршруты после остановок
    void addRoute(Stream::Bus& busFromRequest);
    //from JSON -------------------------------------------------------------------------
    void addStop(const Json::Node& stopFromRequest);
    // заполняем маршруты после остановок
    void addRoute(const Json::Node& busFromRequest);

    // Get result
    //from Stream -----------------------------------------------------------------------
    Stream::BusResult getBusResultStream(std::string busName) const;
    Stream::StopResult getStopResultStream(std::string stopName) const;
    //from JSON -------------------------------------------------------------------------
    Json::Node getBusResult(std::string stopName, Json::Node id) const;
    Json::Node getStopResult(std::string stopName, Json::Node id) const;

public:
    TransportGuide() {
        stops.reserve(N_REQUESTS);
        buses.reserve(N_REQUESTS);
    }

    // Process Updating base from request
    //from Stream -----------------------------------------------------------------------
    void readRequests(std::vector<Stream::RequestHolder>& requests);
    //from JSON -------------------------------------------------------------------------
    void readRequests(Json::Document& document);

    // Process get info from base (Chek)
    //from Stream -----------------------------------------------------------------------
    std::vector<std::unique_ptr<Stream::RequestResult>> checkRequests(
        std::vector<Stream::RequestHolder>& requests
    ) const;
    //from JSON -------------------------------------------------------------------------
    Json::Document checkRequests(Json::Document& document) const;
};

//------------- Parsing all Requests ----------------------------------------------------
//from Stream ---------------------------------------------------------------------------
namespace Stream {
    std::vector<RequestHolder> ReadRequests(std::istream& in_stream = std::cin);
}
//from JSON -----------------------------------------------------------------------------
namespace Json {
     //ReadRequests(Document& document);
}