#include "transport_guide.h"

using namespace std;

//++++++++++++  Help part +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//------------- Parsing string by parts -------------------------------------------------
pair<string_view, optional<string_view>> SplitTwoStrict( 
    string_view s, 
    string_view delimiter
) {
    const size_t pos = s.find(delimiter);
    if (pos == s.npos) {
        return { s, nullopt };
    }
    else {
        return { s.substr(0, pos), s.substr(pos + delimiter.length()) };
    }
}
pair<string_view, string_view> SplitTwo( string_view s, string_view delimiter) {
    const auto [lhs, rhs_opt] = SplitTwoStrict(s, delimiter);
    return { lhs, rhs_opt.value_or("") };
}
string_view ReadToken(string_view& s, string_view delimiter) {
    const auto [lhs, rhs] = SplitTwo(s, delimiter);
    s = rhs;
    return lhs;
}

//------------- Convert string to Int and Double ----------------------------------------
int ConvertToInt(std::string_view str) {
    // use std::from_chars when available to git rid of string copy
    size_t pos;
    const int result = std::stoi(std::string(str), &pos);
    if (pos != str.length()) {
        std::stringstream error;
        error << "string " << str << " contains " << (str.length() - pos) << " trailing chars";
        throw std::invalid_argument(error.str());
    }
    return result;
}
double ConvertToDouble(std::string_view str) {
    // use std::from_chars when available to git rid of string copy
    size_t pos;
    const double result = std::stod(std::string(str), &pos);
    if (pos != str.length()) {
        std::stringstream error;
        error << "string " << str << " contains " << (str.length() - pos) << " trailing chars";
        throw std::invalid_argument(error.str());
    }
    return result;
}


//++++++++++++  Location part +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//------------- Parsing StopRequest and BusRequest --------------------------------------
Location Location::FromString(std::string_view& str) {
    double latitude = ConvertToDouble(ReadToken(str, ", "));
    double longitude = ConvertToDouble(ReadToken(str, ", "));
    return { latitude, longitude };
}
//------------- Parsing StopRequest and BusRequest --------------------------------------
double Location::arcLength(const Location& oher) const {
    return acos(
        sin(this->latitude) * sin(oher.latitude) +
        cos(this->latitude) * cos(oher.latitude) * 
        cos(abs(this->longitude - oher.longitude))
    ) * ERTH_RADIUS; // Delta in meters
}


//++++++++++++  Parsing Stop and Bus from Request from string +++++++++++++++++++++++++++
Stop Stop::FromString(string_view& str) {
    string name(ReadToken(str, ": "));
    if (!str.empty()) {
        Location location{ Location::FromString(str) };
        vector<pair<string, int>> stops;
        while (!str.empty()) {
            int distance = ConvertToInt(ReadToken(str, "m to "));
            stops.push_back({string(ReadToken(str, ", ")), distance });
        }
        return { name, location, stops };
    }
    else {
        return { name };
    }
}
Bus Bus::FromString(std::string_view& str) {
    string name(ReadToken(str, ": "));
    if (!str.empty()) {
        std::string_view delimiter(" > ");
        char ch('>');
        if (str.find(delimiter) == str.npos) {
            delimiter = " - ";
            ch = '-';
        }
        std::vector<std::string> routes;
        while (!str.empty()) {
            routes.push_back(std::string(ReadToken(str, delimiter)));
        }
        return Bus{ name, ch, routes };
    }
    else {
        return Bus{ name };
    }
}


//++++++++++++  Request  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//------------- Parsing StopRequest and BusRequest --------------------------------------
void StopRequest::ParseFrom(string_view str) {
    stop = Stop::FromString(str);
}
void BusRequest::ParseFrom(string_view str) {
    bus = Bus::FromString(str);
}

//------------- Parsing Type Request from string ----------------------------------------
std::optional<Request::Type> Request::FromString(std::string_view& str) {
    if (const auto it = STR_TO_REQUEST_TYPE.find(ReadToken(str));
        it != STR_TO_REQUEST_TYPE.end()) {
        return it->second;
    }
    else {
        return nullopt;
    }
}

//------------- Create Request by Type of request ---------------------------------------
RequestHolder Request::Create(Request::Type type) {
    switch (type) {
    case Request::Type::STOP:
        return make_unique<StopRequest>();
    case Request::Type::BUS:
        return make_unique<BusRequest>();
    default:
        return nullptr;
    }
}

//------------- Parsing one Request from string (main) ----------------------------------
RequestHolder ParseRequest(string_view request_str) {
    const auto request_type = Request::FromString(request_str);
    if (!request_type) {
        return nullptr;
    }
    RequestHolder request = Request::Create(*request_type);
    if (request) {
        request->ParseFrom(request_str);
    };
    return request;
}

//------------- Parsing all Requests from string (main) ---------------------------------
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


//++++++++++++  RequestResult  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ostream& BusResult::writingResult(ostream& out) const {
    out << "Bus " << this->name << ": ";
    if (this->result) {
        out << this->result->amountStops << " stops on route, "
            << this->result->uniqStops << " unique stops, "
            << this->result->lenRoute << " route length, "
            << std::fixed
            << this->result->curvature << " curvature\n";
    }
    else {
        out << "not found\n";
    }
    return out;
}
ostream& StopResult::writingResult(ostream& out) const {
    out << "Stop " << this->name << ": ";
    if (this->result) {
        if (result->empty()) {
            out << "no buses\n";
        }
        else {
            out << "buses" << * result << "\n";
        }
        
    }
    else {
        out << "not found\n";
    }
    return out;
}

//------------- Writing Result in ostream -----------------------------------------------
ostream& writingResult(
    const vector<unique_ptr<RequestResult>>& busesResult,
    ostream& out
) {
    out.precision(6);
    for (const auto& result : busesResult) {
        result->writingResult(out);
    }
    return out;
}


//++++++++++++  TransportGuide  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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

// Process Updating base from read request
void TransportGuide::addStop(Stop& stopFromRequest) {
    hashStop stopFrom = addNewStop(move(stopFromRequest.name));
    stops[stopFrom].setLocation(*(stopFromRequest.location));

    for (auto [stopToName, distance] : stopFromRequest.distance) {
        hashStop stopTo = addNewStop(move(stopToName));
        addDistance(stopFrom, stopTo, distance);
    }
}
// заполняем маршруты после остановок
void TransportGuide::addRoute(Bus& busFromRequest) {
    auto busIt = addNewBus(move(busFromRequest.name));
    buses[busIt->second].setType(*(busFromRequest.routType_));

    for (size_t i(0); i < busFromRequest.vectorRoute.size(); ++i) {
        hashStop stopIndex = addNewStop(move(busFromRequest.vectorRoute[i]));

        stops[stopIndex].addBus(busIt->first);

        hashStop stopBefore = buses[busIt->second].addStopOnRoute(stopIndex);

        updateDistanceRoute(stopBefore, stopIndex, busIt->second);
    }
}

// Get result Get request
BusResult TransportGuide::getBusResult(string busName) const  {
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
StopResult TransportGuide::getStopResult(string stopName) const {
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

// public part
// Process Updating base from request
void TransportGuide::readRequests(vector<RequestHolder>& requests) {
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

// Process get info from base (Chek)
vector<unique_ptr<RequestResult>> TransportGuide::checkRequests(
    vector<RequestHolder>& requests
) const {
    vector<unique_ptr<RequestResult>> result;
    for (const auto& request : requests) {
        if (request->type == Request::Type::BUS) {
            result.push_back(
                make_unique<BusResult>(getBusResult(move(request->bus->name)))
            );
        }
        else if (request->type == Request::Type::STOP) {
            result.push_back(
                make_unique<StopResult>(getStopResult(move(request->stop->name)))
            );
        }
    }
    return result;
}