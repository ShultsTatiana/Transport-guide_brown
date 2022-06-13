#pragma once

#include <string_view>
#include <iostream>
#include <sstream>
#include <utility>
#include <optional>
#include <exception>
#include <system_error>
#include <string>

//++++++++++++  Help part +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //------------- Parsing string by parts -------------------------------------------------
std::pair<std::string_view, std::optional<std::string_view>> SplitTwoStrict(
    std::string_view s,
    std::string_view delimiter = " ");

std::pair<std::string_view, std::string_view> SplitTwo(
    std::string_view s,
    std::string_view delimiter = " ");

std::string_view ReadToken(std::string_view& s, std::string_view delimiter = " ");

//------------- Convert string to Int and Double ----------------------------------------
template <typename Number>
Number ReadNumberOnLine(std::istream& stream) {
    Number number;
    stream >> number;
    std::string dummy;
    std::getline(stream, dummy);
    return number;
}
int ConvertToInt(std::string_view str);
double ConvertToDouble(std::string_view str);

template <typename Number>
void ValidateBounds(Number number_to_check, Number min_value, Number max_value) {
    if (number_to_check < min_value || number_to_check > max_value) {
        std::stringstream error;
        error << number_to_check << " is out of [" << min_value << ", " << max_value << "]";
        throw std::out_of_range(error.str());
    }
}
