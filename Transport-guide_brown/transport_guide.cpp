#include "transport_guide.h"

using namespace std;

pair<string_view, optional<string_view>> SplitTwoStrict( string_view s, string_view delimiter) {
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

Location Location::FromString(std::string_view& str) {
    double latitude = ConvertToDouble(ReadToken(str, ", "));
    double longitude = ConvertToDouble(ReadToken(str, ", "));
    return { latitude, longitude };
}

double Location::arcLength(const Location& oher) const {
    return acos(
        sin(this->latitude) * sin(oher.latitude) +
        cos(this->latitude) * cos(oher.latitude) * 
        cos(abs(this->longitude - oher.longitude))
    ) * ERTH_RADIUS; // Delta in meters
}

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


void StopRequest::ParseFrom(string_view str) {
    stop = Stop::FromString(str);
}
void BusRequest::ParseFrom(string_view str) {
    bus = Bus::FromString(str);
}

std::optional<Request::Type> Request::FromString(std::string_view& str) {
    if (const auto it = STR_TO_REQUEST_TYPE.find(ReadToken(str));
        it != STR_TO_REQUEST_TYPE.end()) {
        return it->second;
    }
    else {
        return nullopt;
    }
}

std::string_view Request::GetName() {
    if (type == Type::STOP) {
        return stop->name;
    }
    else if (type == Type::BUS) {
        return bus->name;
    }
}

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


ostream& writingResult(const vector<unique_ptr<RequestResult>>& busesResult, ostream& out) {
    out.precision(6);
    for (const auto& result : busesResult) {
        result->writingResult(out);
    }
    return out;
}
/*
void Base::baseUpdating(std::vector<RequestHolder>& groundRequest) {
    for (auto& request : groundRequest) {
        if (request->type == Request::Type::STOP) {
            std::string_view stop = request->GetName();
            baseOfStorBus[stop];
            for (auto& [stopTo, distance] : request->stop->distance) {
                baseOfDistance[stop].insert({ stopTo, distance });
                if (auto it = baseOfDistance[stopTo].find(stop); it == baseOfDistance[stopTo].end()) {
                    baseOfDistance[stopTo].insert({ stop, distance });
                }
                
            }
            baseOfStop[stop] = move(request);
        }
        else if (request->type == Request::Type::BUS) {
            string_view bus = request->GetName();
            for (stopName stop : request->bus->setRroute) {
                baseOfStorBus[stop].insert(bus);
            }
            baseOfBus[bus] = move(request);
        }
    }
}
Base ProcessRequests(vector<RequestHolder>& base) {
    Base returnBase(base);

    return returnBase;
}

double Base::distance(stopName from, stopName to) const {
    auto it = baseOfDistance.find(from);
    if (it != baseOfDistance.end()) {
        auto it2 = it->second.find(to);
        if ( it2 != it->second.end()) {
            return it2->second;
        }
    }
    else {
        return baseOfStop.find(from)->second->stop->location->arcLength(
            *(baseOfStop.find(to)->second->stop->location)
        );
    }
}

double Base::direct(stopName from, stopName to) const {
    return baseOfStop.find(from)->second->stop->location->arcLength(
        *(baseOfStop.find(to)->second->stop->location)
    );
}

BusResult Base::findBus(busName name) const {
    BusResult result;
    result.name = name;
    auto it = baseOfBus.find(name);
    if (it != baseOfBus.end()) {
        Result data{0, 0, 0.0};
        auto& route = it->second->bus;
        data.amountStops = (route->routType_ == '>' ? route->vectorRoute.size() :
                                                     (route->vectorRoute.size() * 2) - 1);
        data.uniqStops = route->setRroute.size();
        
        for (size_t i(0); i < route->vectorRoute.size() - 1; ++i) {
            data.lenRoute += distance(route->vectorRoute[i], route->vectorRoute[i + 1]);
            data.direct += direct(route->vectorRoute[i], route->vectorRoute[i + 1]);
        }
        if (route->routType_ == '-') {
            for (size_t i(0); i < route->vectorRoute.size() - 1; ++i) {
                data.lenRoute += distance(route->vectorRoute[i], route->vectorRoute[i + 1]);
                data.direct += direct(route->vectorRoute[i], route->vectorRoute[i + 1]);
            }
        }
        result.result = move(data);
    }
    return result;
}

StopResult Base::findStop(stopName name) const {
    StopResult result;
    result.name = name;
    auto it = baseOfStorBus.find(name);
    if (it != baseOfStorBus.end()) {
        string data{""};
        if (it->second.empty()) {
            data = "no buses";
        }
        else {
            data += "buses ";
            for (auto bus(it->second.begin()); bus != it->second.end(); ++bus) {
                data += string(*bus);
                if (bus != prev(it->second.end())) {
                    data += " ";
                }
            }
        }
        result.result = move(data);
    }
    //else { *result.result = "not found"; }
    return result;
}

vector<unique_ptr<RequestResult>> Base::checkRequests(const vector<RequestHolder>& checkRequest) const {
    vector<unique_ptr<RequestResult>> result;
    for (const auto& request : checkRequest) {
        if (request->type == Request::Type::BUS) {
            result.push_back(make_unique<BusResult>(findBus(request->GetName())));
        }
        else if (request->type == Request::Type::STOP) {
            result.push_back(make_unique<StopResult>(findStop(request->GetName())));
        }
    }

    return result;
}*/