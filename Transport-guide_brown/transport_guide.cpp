#include "transport_guide.h"

using namespace std;

//------------- TransportGuide::StopOnRout part -----------------------------------------
// Set Stop methods (public)
void TransportGuide::StopOnRout::setLocation(Location& location_) {
    location.latitude = location_.latitude;
    location.longitude = location_.longitude;
}
void TransportGuide::StopOnRout::addDistanceToNextStop(hashStop stopTo, int distanse) {
    distanceTo[stopTo] = distanse;
}
void TransportGuide::StopOnRout::addBus(string_view busName) {
    busesOnStop.insert(busName);
}

// Get Stop methods (public)
const Location& TransportGuide::StopOnRout::getLocation() const {
    return location;
}
int TransportGuide::StopOnRout::getDistanceTo(
    TransportGuide::hashStop stopTo
) const {
    return distanceTo[stopTo];
}
bool TransportGuide::StopOnRout::checkBus(string_view busName) const {
    return busesOnStop.count(busName);
}
const set<string_view>& TransportGuide::StopOnRout::getBuses() const {
    return busesOnStop;
}

//------------- TransportGuide::Route part ----------------------------------------------
// Methods for update distance part of Rout (private)
void TransportGuide::Route::updateLength(int length) {
    distance.length += length;
}
void TransportGuide::Route::updateDirectLength(double directLength) {
    if (type == Type::DIRECT) {
        distance.directLength += (2. * directLength);
    }
    else if (type == Type::CIRCULAR) {
        distance.directLength += directLength;
    }
}
void TransportGuide::Route::updateCurvature() {
    distance.curvature = double(distance.length) / distance.directLength;
}

// Update Type, Rout and distance on route (public)
void TransportGuide::Route::setType(char chType) {
    if (chType == '-') {
        type = Type::DIRECT;
    }
    else if (chType == '>') {
        type = Type::CIRCULAR;
    }
}
TransportGuide::hashStop TransportGuide::Route::addStopOnRoute(hashStop stop) {
    route.push_back(stop);
    uniqStopsOnRoute.insert(stop);
    return route.size() == 1 ? -1 : *(next(route.rbegin()));
}
void TransportGuide::Route::updateDistances(int length, double directLength) {
    updateLength(length);
    updateDirectLength(directLength);
    updateCurvature();
}

// Get route parameters (public)
int TransportGuide::Route::getStopOnRouteCount() const {
    if (type == Type::DIRECT) {
        return int(route.size() * 2 - 1);
    }
    else if (type == Type::CIRCULAR) {
        return int(route.size());
    }
}
int TransportGuide::Route::getUniqStopOnRouteCount() const {
    return int(uniqStopsOnRoute.size());
}
int TransportGuide::Route::getLength() const {
    return distance.length;
}
double TransportGuide::Route::getDirectLength() const {
    return distance.directLength;
}

double TransportGuide::Route::getCurvature() const {
    return distance.curvature;
}
TransportGuide::Route::Type TransportGuide::Route::getType() const {
    return *type;
}

//------------- TransportGuide part -----------------------------------------------------
// prvate part
// small - update and create
TransportGuide::hashStop TransportGuide::addNewStop(string stopName) {
    hashStop stopIndex;
    if (auto it = hashStops.find(stopName);
        it == hashStops.end()) {
        stops.push_back(StopOnRout());
        stopIndex = int(stops.size()) - 1;
        hashStops[move(stopName)] = stopIndex;
    }
    else {
        stopIndex = it->second;
    }
    return stopIndex;
}
void TransportGuide::addDistance(hashStop from, hashStop to, int distance) {
    stops[from].addDistanceToNextStop(to, distance);
    //обновить, только если ранее расстояние по дорогам небыло задано
    if (stops[to].getDistanceTo(from) < 0) {
        stops[to].addDistanceToNextStop(from, distance);
    }
}
auto TransportGuide::addNewBus(string busName) {
    if (auto it = hashBuses.find(busName);
        it == hashBuses.end()) {
        buses.push_back(Route());
        return hashBuses.insert({ move(busName), int(buses.size() - 1) }).first;
    }
    else {
        return it;
    }
}
void TransportGuide::updateDistanceRoute(hashStop from, hashStop to, hashBus busIndex) {
    if (from != -1) {
        int length = stops[from].getDistanceTo(to);
        if (buses[busIndex].getType() == Route::Type::DIRECT) {
            length += stops[to].getDistanceTo(from);
        }

        double direct = stops[from].getLocation().arcLength(stops[to].getLocation());

        buses[busIndex].updateDistances(length, direct);
    }
}

//++++++++++++++++++++++ Parsing request part +++++++++++++++++++++++++++++++++++++++++++
//from Stream ---------------------------------------------------------------------------
// Process Updating base from read request
void TransportGuide::addStop(Stream::Stop& stopFromRequest) {
    hashStop stopFrom = addNewStop(move(stopFromRequest.name));
    stops[stopFrom].setLocation(*(stopFromRequest.location));

    for (auto [stopToName, distance] : stopFromRequest.distance) {
        hashStop stopTo = addNewStop(move(stopToName));
        addDistance(stopFrom, stopTo, distance);
    }
}
// заполняем маршруты после остановок
void TransportGuide::addRoute(Stream::Bus& busFromRequest) {
    auto busIt = addNewBus(move(busFromRequest.name));
    buses[busIt->second].setType(*(busFromRequest.routType_));

    for (size_t i(0); i < busFromRequest.vectorRoute.size(); ++i) {
        hashStop stopIndex = addNewStop(move(busFromRequest.vectorRoute[i]));

        stops[stopIndex].addBus(busIt->first);

        hashStop stopBefore = buses[busIt->second].addStopOnRoute(stopIndex);

        updateDistanceRoute(stopBefore, stopIndex, busIt->second);
    }
}
//from JSON -----------------------------------------------------------------------------

// Get result Get request
//from Stream ---------------------------------------------------------------------------
Stream::BusResult TransportGuide::getBusResultStream(string busName) const {
    if (auto it = hashBuses.find(busName); it != hashBuses.end()) {
        const Route& route = buses[it->second];
        return Stream::BusResult{
            move(busName),
            Stream::Result{
                .amountStops = route.getStopOnRouteCount(),
                .uniqStops = route.getUniqStopOnRouteCount(),
                .lenRoute = route.getLength(),
                .curvature = route.getCurvature()
            }
        };
    }
    else {
        return Stream::BusResult{ move(busName), std::nullopt };
    }
}
Stream::StopResult TransportGuide::getStopResultStream(string stopName) const {
    if (auto it = hashStops.find(stopName); it != hashStops.end()) {
        const StopOnRout& stop = stops[it->second];
        std::string result;
        for (std::string_view bus : stop.getBuses()) {
            result += ' ';
            result += bus;
        }
        return Stream::StopResult{ move(stopName), move(result) };
    }
    else {
        return Stream::StopResult{ move(stopName), std::nullopt };
    }
}
//from JSON -----------------------------------------------------------------------------


// public part
// Process Updating base from request
//from Stream ---------------------------------------------------------------------------
void TransportGuide::readRequests(vector<Stream::RequestHolder>& requests) {
    using namespace Stream;
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
//from JSON -----------------------------------------------------------------------------


// Process get info from base (Chek)
//from Stream ---------------------------------------------------------------------------
vector<unique_ptr<Stream::RequestResult>> TransportGuide::checkRequests(
    vector<Stream::RequestHolder>& requests
) const {
    using namespace Stream;
    vector<unique_ptr<RequestResult>> result;
    for (const auto& request : requests) {
        if (request->type == Request::Type::BUS) {
            result.push_back(
                make_unique<BusResult>(getBusResultStream(move(request->bus->name)))
            );
        }
        else if (request->type == Request::Type::STOP) {
            result.push_back(
                make_unique<StopResult>(getStopResultStream(move(request->stop->name)))
            );
        }
    }
    return result;
}
//from JSON -----------------------------------------------------------------------------


namespace Stream {
    //------------- Parsing all Requests ----------------------------------------------------
    vector<RequestHolder> ReadRequests(istream& in_stream) {
        const size_t request_count = ReadNumberOnLine<size_t>(in_stream);

        vector<RequestHolder> requests;
        requests.reserve(request_count);

        for (size_t i = 0; i < request_count; ++i) {
            string request_str;
            getline(in_stream, request_str);
            if (auto request = ParseRequest(request_str)) {
                requests.push_back(move(request));
            }
        }
        return requests;
    }
}