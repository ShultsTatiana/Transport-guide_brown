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
    double latitude = ConvertToDouble(ReadToken(str, ","));
    double longitude = ConvertToDouble(str);
    ValidateBounds(latitude, 0.0, 360.0);
    ValidateBounds(longitude, 0.0, 360.0);
    return { latitude, longitude };
}

RouteType RouteType::FromString(std::string_view& str) {
    std::string_view delimiter(" > ");
    char ch('>');
    if (str.find(delimiter) == str.npos) {
        delimiter = " - ";
        ch = '-';
    }
    std::vector<std::string> routes;
    std::unordered_set<std::string> setRroute;
    while (!str.empty()) {
        routes.push_back(std::string(ReadToken(str, delimiter)));
        setRroute.insert(routes.back());
    }
    return RouteType{ routes, setRroute, ch };
}

void StopRequest::ParseFrom(string_view str) {
    object.name = ReadToken(str, ":");
    if (!str.empty()) {
        object.location = Location::FromString(str);
    }
}

void BusRequest::ParseFrom(string_view str) {
    object.name = ReadToken(str, ": ");
    if (!str.empty()) {
        object.routeType = RouteType::FromString(str);
    }
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

string_view Request::GetName() const {
    return object.name;
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

double Location::arcLength(const Location& oher) const {
    return acos(
        sin(this->latitude) * sin(oher.latitude) +
        cos(this->latitude) * cos(oher.latitude) * cos(abs(this->longitude - oher.longitude))
    ) * ERTH_RADIUS; // Delta in meters
}


ostream& BusResult::writingResult(ostream& out) {
    out << "Bus " << this->name << ": ";
    if (this->result) {
        out << this->result->amountStops << " stops on route, "
            << this->result->uniqStops << " unique stops, "
            << this->result->lenRoute << " route length\n";
    }
    else {
        out << "not found\n";
    }
    return out;
}
ostream& StopResult::writingResult(ostream& out) {
    out << "Stop " << this->name << ": ";
    if (this->result) {
        out << *result << "\n";
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

void Base::baseUpdating(std::vector<RequestHolder>& groundRequest) {
    for (auto& request : groundRequest) {
        if (request->type == Request::Type::STOP) {
            auto stop = request->GetName();
            baseOfStorBus[stop];
            baseOfStop[stop] = move(request);
        }
        else if (request->type == Request::Type::BUS) {
            auto bus = request->GetName();
            for (stopName stop : request->object.routeType->setRroute) {
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

BusResult Base::findBus(busName name) const {
    BusResult result;
    result.name = name;
    auto it = baseOfBus.find(name);
    if (it != baseOfBus.end()) {
        Result data{0, 0, 0.0};
        auto& route = it->second->object.routeType;
        data.amountStops = (route->routType_ == '>' ? route->vectorRoute.size() :
                                                     (route->vectorRoute.size() * 2) - 1);
        data.uniqStops = route->setRroute.size();
        for (size_t i(0); i < route->vectorRoute.size() - 1; ++i) {
            data.lenRoute +=
                baseOfStop.find(route->vectorRoute[i])->second->object.location->arcLength(
                    *(baseOfStop.find(route->vectorRoute[i + 1])->second->object.location)
                );
        }
        if (route->routType_ == '-') {
            data.lenRoute *= 2.0;
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
            result.push_back(make_unique<BusResult>(findBus(request->object.name)));
        }
        else if (request->type == Request::Type::STOP) {
            result.push_back(make_unique<StopResult>(findStop(request->object.name)));
        }
    }

    return result;
}