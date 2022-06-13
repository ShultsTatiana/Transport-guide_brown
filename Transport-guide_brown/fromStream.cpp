#include "fromStream.h"

using namespace std;

namespace Stream {
    //++++++++++++  Parsing Stop and Bus from Request from string +++++++++++++++++++++++
    Stop Stop::FromString(string_view& str) {
        string name(ReadToken(str, ": "));
        if (!str.empty()) {
            Location location{ Location::FromString(str) };
            vector<pair<string, int>> stops;
            while (!str.empty()) {
                int distance = ConvertToInt(ReadToken(str, "m to "));
                stops.push_back({ string(ReadToken(str, ", ")), distance });
            }
            return { move(name), move(location), move(stops) };
        }
        else {
            return { move(name) };
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
            return Bus{ move(name), ch, move(routes) };
        }
        else {
            return Bus{ move(name) };
        }
    }


    //++++++++++++  Request  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //------------- Parsing StopRequest and BusRequest ----------------------------------
    void StopRequest::ParseFrom(string_view str) {
        stop = Stop::FromString(str);
    }
    void BusRequest::ParseFrom(string_view str) {
        bus = Bus::FromString(str);
    }

    //------------- Parsing Type Request from string ------------------------------------
    std::optional<Request::Type> Request::FromString(std::string_view& str) {
        if (const auto it = STR_TO_REQUEST_TYPE.find(ReadToken(str));
            it != STR_TO_REQUEST_TYPE.end()) {
            return it->second;
        }
        else {
            return nullopt;
        }
    }

    //------------- Create Request by Type of request -----------------------------------
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

    //------------- Parsing one Request -------------------------------------------------
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

    

    //++++++++++++  RequestResult  ++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
                out << "buses" << *result << "\n";
            }

        }
        else {
            out << "not found\n";
        }
        return out;
    }

    //------------- Writing Result in ostream -------------------------------------------
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

}
