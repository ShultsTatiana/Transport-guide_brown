#pragma once

#include "helper.h"
#include "location.h"

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
#include <vector>
#include <typeinfo>
#include <cstdint>
#include <iterator>
#include <iomanip>

namespace Stream {
    //++++++++++++  Parsing Stop and Bus from Request from string +++++++++++++++++++++++++++
    struct Stop {
        std::string name;
        std::optional<Location> location;
        std::vector<std::pair<std::string, int>> distance;

        static Stop FromString(std::string_view& str);
    };

    struct Bus {
        std::string name;
        std::optional<bool> is_roundtrip;
        std::vector<std::string> vectorRoute;

        static Bus FromString(std::string_view& str);
    };


    //++++++++++++  Request  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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

    //------------- Parsing one Request -------------------------------------------------
    RequestHolder ParseRequest(std::string_view request_str);


    //++++++++++++  RequestResult  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    struct RequestResult {
        RequestResult() = default;
        RequestResult(std::string name_) : name(move(name_)) {}

        std::string name;
        virtual std::ostream& writingResult(std::ostream& out) const = 0;
    };

    struct Result {
        int amountStops = 0;
        int uniqStops = 0;
        int lenRoute = 0;
        double curvature = 0.0;
    };

    struct BusResult :RequestResult {
        BusResult() = default;
        BusResult(std::string name_, std::optional<Result> result_) :
            RequestResult(move(name_)), result(std::move(result_)) {}

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

    //------------- Writing Result in ostream -----------------------------------------------
    std::ostream& writingResult(
        const std::vector<std::unique_ptr<RequestResult>>& busesResult,
        std::ostream& out = std::cout);

}
