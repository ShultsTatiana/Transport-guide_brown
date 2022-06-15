#include "transport_guide.h"

using namespace std;

//------------- TransportGuide::StopOnRout part -----------------------------------------
// Set Stop methods (public)
void TransportGuide::StopOnRout::setLocation(Location location_) {
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
void TransportGuide::Route::setType(bool is_roundtrip) {
    if (is_roundtrip) {
        type = Type::CIRCULAR;
    } 
    else {
        type = Type::DIRECT;
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
    else {//if (type == Type::CIRCULAR) {
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
// Process Updating base from read request
//from Stream ---------------------------------------------------------------------------
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
    buses[busIt->second].setType(*(busFromRequest.is_roundtrip));

    for (size_t i(0); i < busFromRequest.vectorRoute.size(); ++i) {
        hashStop stopIndex = addNewStop(move(busFromRequest.vectorRoute[i]));

        stops[stopIndex].addBus(busIt->first);

        hashStop stopBefore = buses[busIt->second].addStopOnRoute(stopIndex);

        updateDistanceRoute(stopBefore, stopIndex, busIt->second);
    }
}

//from JSON -----------------------------------------------------------------------------
void TransportGuide::addStop(const Json::Node& stopFromRequest) {
    using namespace Json;
    
    //проверяем соответствует ли запрос ожидаемому формату Map
    if (stopFromRequest.checkIndex(1)) {
        const map<string, Node>& request = stopFromRequest.AsMap();

        //если у остановки указаны имя и координаты - ее можно добавлять в базу
        //тк имя и координаты обязательно должны быть
        if (auto itName = request.find("name"), 
            itLat = request.find("latitude"), itLong = request.find("longitude");
            itName != request.end() &&
            itLat != request.end() && itLong != request.end()) {
                        
            hashStop stopFrom = addNewStop(itName->second.AsString());
            stops[stopFrom].setLocation(
                Location( itLat->second.returnDouble(), itLong->second.returnDouble() )
            );
            
            //проверяем наличие у остановки списка расстояний
            if (auto itDistances = request.find("road_distances");
                itDistances != request.end()) {

                //проверяем соответствует ли road_distances ожидаемому формату Map
                if (itDistances->second.checkIndex(1)) {

                    for (auto [stopToName, distance] : itDistances->second.AsMap()) {

                        //проверяем соответствует ли distance ожидаемому Int
                        if (distance.checkIndex(2)) {
                            hashStop stopTo = addNewStop(stopToName);
                            addDistance(stopFrom, stopTo, distance.AsInt());
                        }
                    }
                }
            }
        }
    }
}
void TransportGuide::addRoute(const Json::Node& busFromRequest) {
    using namespace Json;

    //проверяем соответствует ли запрос ожидаемому формату Map
    if (busFromRequest.checkIndex(1)) {

        const map<string, Node>& request = busFromRequest.AsMap();

        //если у автобуса указаны имя, маршрут и тип - его можно добавлять в базу
        //тк имя, маршрут и тип обязательно должны быть
        if (auto itName = request.find("name"), 
            itStops = request.find("stops"),
            itType = request.find("is_roundtrip");
            itName != request.end() &&
            itStops != request.end() && itType != request.end()) {
            
            //пока оставим без проверки формат имени, маршрута и типа
            auto busIt = addNewBus(itName->second.AsString());
            buses[busIt->second].setType(itType->second.AsBool());

            const vector<Node>& stopsOnRoute = itStops->second.AsArray();

            for (size_t i(0); i < stopsOnRoute.size(); ++i) {
                hashStop stopIndex = addNewStop(stopsOnRoute[i].AsString());

                stops[stopIndex].addBus(busIt->first);

                hashStop stopBefore = buses[busIt->second].addStopOnRoute(stopIndex);

                updateDistanceRoute(stopBefore, stopIndex, busIt->second);
            }
        }
    }
}

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
        string result;
        for (string_view bus : stop.getBuses()) {
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
Json::Node TransportGuide::getBusResult(string name, Json::Node id) const {
    using namespace Json;

    map<string, Node> result;
    result.emplace("request_id", move(id) );

    if (auto it = hashBuses.find(name); it != hashBuses.end()) {
        const Route& route = buses[it->second];
        //может потребоваться string("") для ключа
        result.emplace("stop_count", Node(int(route.getStopOnRouteCount())));
        result.emplace("unique_stop_count", Node(int(route.getUniqStopOnRouteCount())));
        result.emplace("route_length", Node(int(route.getLength())));
        result.emplace("curvature", Node(double(route.getCurvature())));
    }
    else {
        result.emplace("error_message", Node("not found"));
    }

    return Node(move(result));
}
Json::Node TransportGuide::getStopResult(string name, Json::Node id) const {
    using namespace Json;

    map<string, Node> result;
    result.emplace("request_id", move(id));

    if (auto it = hashStops.find(name); it != hashStops.end()) {
        const StopOnRout& stop = stops[it->second];
        vector<Node> busesResult;
        for (string_view bus : stop.getBuses()) {
            busesResult.push_back(Node(string(bus)));
        }
        result.emplace("buses", Node(busesResult));
    }
    else {
        result.emplace("error_message", Node(string("not found")));
    }

    return Node(move(result));
}

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
void TransportGuide::readRequests(Json::Document& document) {
    using namespace Json;
    // если среди запросов небыло запросов на заполнение базы или 
    auto root = document.GetRoot();
    if (root.checkIndex(1)) {
        auto requestsMap = root.AsMap();
        
        if (auto itRequests = requestsMap.find("base_requests");
            itRequests != requestsMap.end() && itRequests->second.checkIndex(0)) {

            const vector<Node>& requests = itRequests->second.AsArray();

            for (const Node& request : requests) {
                if (request.checkIndex(1)) {

                    if (auto itRequest = request.AsMap().find("type");
                        itRequest != request.AsMap().end() &&
                        itRequest->second.checkIndex(5)) {

                        if (itRequest->second.AsString() == "Stop") {
                            addStop(request);
                        }
                    }
                }
            }
            for (const Node& request : requests) {
                if (request.checkIndex(1)) {

                    if (auto itRequest = request.AsMap().find("type");
                        itRequest != request.AsMap().end() &&
                        itRequest->second.checkIndex(5)) {

                        if (itRequest->second.AsString() == "Bus") {
                            addRoute(request);
                        }
                    }
                }
            }
        }
    }
}

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
Json::Document TransportGuide::checkRequests(Json::Document& document) const {
    using namespace Json;

    vector<Node> results;
    auto root = document.GetRoot();

    //проверяем соответствует ли документ ожидаемому формату Map
    if (root.checkIndex(1)) {
        auto requestsMap = root.AsMap();

        //проверяем наличие в документе запросов типа stat_requests
        //проверяем соответствует ли запрос ожидаемому формату vector<Node>
        if (auto itRequests = requestsMap.find("stat_requests");
            itRequests != requestsMap.end() && itRequests->second.checkIndex(0)) {

            const vector<Node>& requests = itRequests->second.AsArray();
            //резервируем results (защита от релокации)
            results.reserve(requests.size());

            for (const Node& requestMap : requests) {
                //проверяем соответствует ли запрос ожидаемому формату Map
                if (requestMap.checkIndex(1)) {

                    const auto& request = requestMap.AsMap();

                    //проверяем наличие в запросе обязательных полей
                    if (
                        auto itType = request.find("type"),
                        itName = request.find("name"), 
                        itId = request.find("id");
                        itType != request.end() &&
                        itName != request.end() && 
                        itId != request.end()) 
                    {

                        const string& type = itType->second.AsString();
                        string name = itName->second.AsString();

                        if (type == "Bus") {
                            results.push_back(getBusResult(move(name), itId->second));
                        }
                        else {
                            results.push_back(getStopResult(move(name), itId->second));
                        }
                    }
                }
            }
        }
    }
    return Document{ Node{results} };
}

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