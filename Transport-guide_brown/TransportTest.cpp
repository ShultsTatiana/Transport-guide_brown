#include "TransportTest.h"

using namespace std;

bool doubleCompare(double x1, double x2, double epsilon = 0.000001) {
	double delta = abs(x1 - x2);
	double delta1 = x1 * epsilon;
	double delta2 = x2 * epsilon;
	return delta <= max(delta1, delta2);
}

bool operator==(const StopRequest& lhs, const StopRequest& rhs) {
	return(
		lhs.stop->name == rhs.stop->name &&
		doubleCompare(lhs.stop->location->latitude, rhs.stop->location->latitude, 0.000001) &&
		doubleCompare(lhs.stop->location->longitude, rhs.stop->location->longitude, 0.000001)
		);
}

bool operator==(const Stop& lhs, const Stop& rhs) {
	if (lhs.distance.size() == rhs.distance.size()) {
		bool vectorCheck = true;
		for (size_t i(0); i < lhs.distance.size(); ++i) {
			vectorCheck = (lhs.distance[i] == rhs.distance[i]);
		}
		return(
			vectorCheck &&
			lhs.name == rhs.name &&
			doubleCompare(lhs.location->latitude, rhs.location->latitude, 0.000001) &&
			doubleCompare(lhs.location->longitude, rhs.location->longitude, 0.000001)
		);
	}
	else { return false; }
}
bool operator==(const Bus& lhs, const Bus& rhs) {
	return(
		lhs.name == rhs.name &&
		lhs.vectorRoute == rhs.vectorRoute &&
		lhs.routType_ == rhs.routType_
		);
}

ostream& operator<<(ostream& out, const StopRequest& lhs) {
	out << lhs.stop->name << ": "
		<< lhs.stop->location->latitude << ", "
		<< lhs.stop->location->longitude;
	return out;
}
ostream& operator<<(ostream& out, const Stop& lhs) {
	out << lhs.name << ": "
		<< lhs.location->latitude << ", "
		<< lhs.location->longitude;
	return out;
}

ostream& operator<<(ostream& out, const BusRequest& lhs) {
	out << lhs.bus->name << ": ";
	if (!lhs.bus->vectorRoute.empty()) {
		for (size_t i(0); i < lhs.bus->vectorRoute.size() - 1; ++i) {
			out << lhs.bus->vectorRoute[i] << " " << lhs.bus->routType_.value() << " ";
		}
		out << lhs.bus->vectorRoute.back();
	}
	return out;
}
ostream& operator<<(ostream& out, const Bus& lhs) {
	out << lhs.name << ": ";
	if (!lhs.vectorRoute.empty()) {
		for (size_t i(0); i < lhs.vectorRoute.size() - 1; ++i) {
			out << lhs.vectorRoute << " " << lhs.routType_.value() << " ";
		}
		out << lhs.vectorRoute.back();
	}
	return out;
}

bool operator==(const BusResult& lhs, const BusResult& rhs) {
	if (lhs.result && rhs.result) {
		return (
			lhs.name == rhs.name &&
			lhs.result->amountStops == rhs.result->amountStops &&
			lhs.result->uniqStops == rhs.result->uniqStops &&
			doubleCompare(lhs.result->lenRoute, rhs.result->lenRoute, 0.0001)
			);
	}
	else if ((!lhs.result && rhs.result) || (lhs.result && !rhs.result)) {
		return false;
	}
	else { return lhs.name == rhs.name; }
}
ostream& operator<<(ostream& out, const unique_ptr<RequestResult>& result) {
	out.precision(6);
	if (result != nullptr) {
		result->writingResult(out);
	}
	return out;
}
bool operator==(const StopResult& lhs, const StopResult& rhs) {
	return (lhs.name == rhs.name && lhs.result == rhs.result);
}

void test1() {
	string inStr{
		"13\n"
		"Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino\n"
		"Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka\n"
		"Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n"
		"Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka\n"
		"Stop Rasskazovka: 55.632761, 37.333324\n"
		"Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam\n"
		"Stop Biryusinka: 55.581065, 37.64839, 750m to Universam\n"
		"Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, 900m to Biryulyovo Tovarnaya\n"
		"Stop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo Passazhirskaya\n"
		"Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to Biryulyovo Zapadnoye\n"
		"Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye\n"
		"Stop Rossoshanskaya ulitsa: 55.595579, 37.605757\n"
		"Stop Prazhskaya: 55.611678, 37.603831\n"
		"6\n"
		"Bus 256\n"
		"Bus 750\n"
		"Bus 751\n"
		"Stop Samara\n"
		"Stop Prazhskaya\n"
		"Stop Biryulyovo Zapadnoye\n"
	};
	stringstream input;
	input << inStr;
	auto groundRequest = ReadRequests(input);
	TransportGuide guide;
	guide.readRequests(groundRequest);
	auto requests = ReadRequests(input);
	auto answer = guide.checkRequests(requests);
	string expected{
		"Bus 256: 6 stops on route, 5 unique stops, 5950 route length, 1.361239 curvature\n"
		"Bus 750: 5 stops on route, 3 unique stops, 27600 route length, 1.318084 curvature\n"
		"Bus 751: not found\n"
		"Stop Samara: not found\n"
		"Stop Prazhskaya: no buses\n"
		"Stop Biryulyovo Zapadnoye: buses 256 828\n"
	};
	stringstream out;
	writingResult(answer, out);
	ASSERT_EQUAL(out.str(), expected);
}

void test2() {
	string inStr{
		"3\n"
		"Stop A: 0.5, -1.0, 100000m to B\n"
		"Stop B: 0.0, -1.1\n"
		"Bus 256: B - A\n"
		"4\n"
		"Bus 256\n"
		"Stop A\n"
		"Stop B\n"
		"Stop C\n"
	};
	stringstream input;
	input << inStr;
	auto groundRequest = ReadRequests(input);
	TransportGuide guide;
	guide.readRequests(groundRequest);
	auto requests = ReadRequests(input);
	auto answer = guide.checkRequests(requests);
	string expected{
		"Bus 256: 3 stops on route, 2 unique stops, 200000 route length, 1.763716 curvature\n"
		"Stop A: buses 256\n"
		"Stop B: buses 256\n"
		"Stop C: not found\n"
	};
	stringstream out;
	writingResult(answer, out);
	ASSERT_EQUAL(out.str(), expected);
}

void test3() {
	string inStr{
		"3\n"
		"Stop A: 0.5, -1.0, 100000m to B\n"
		"Stop B: 0.0, -1.1, 300000m to A\n"
		"Bus 256: B - A\n"
		"4\n"
		"Bus 256\n"
		"Stop A\n"
		"Stop B\n"
		"Stop C\n"
	};
	stringstream input;
	input << inStr;
	auto groundRequest = ReadRequests(input);
	TransportGuide guide;
	guide.readRequests(groundRequest);
	auto requests = ReadRequests(input);
	auto answer = guide.checkRequests(requests);
	string expected{
		"Bus 256: 3 stops on route, 2 unique stops, 400000 route length, 3.527431 curvature\n"
		"Stop A: buses 256\n"
		"Stop B: buses 256\n"
		"Stop C: not found\n"
	};
	stringstream out;
	writingResult(answer, out);
	ASSERT_EQUAL(out.str(), expected);
}
void test4() {
	string inStr{
		"5\n"
		"Stop A: 0.5, -1.0, 100000m to B\n"
		"Stop B: 0.0, -1.1, 300000m to A\n"
		"Bus 256: B - A\n"
		"Bus 25: B > A > B\n"
		"Bus 2: B > A\n"
		"6\n"
		"Bus 256\n"
		"Bus 25\n"
		"Bus 2\n"
		"Stop A\n"
		"Stop B\n"
		"Stop C\n"
	};
	stringstream input;
	input << inStr;
	auto groundRequest = ReadRequests(input);
	TransportGuide guide;
	guide.readRequests(groundRequest);
	auto requests = ReadRequests(input);
	auto answer = guide.checkRequests(requests);
	string expected{
		"Bus 256: 3 stops on route, 2 unique stops, 400000 route length, 3.527431 curvature\n"
		"Bus 25: 3 stops on route, 2 unique stops, 400000 route length, 3.527431 curvature\n"
		"Bus 2: 2 stops on route, 2 unique stops, 300000 route length, 5.291147 curvature\n"
		"Stop A: buses 2 25 256\n"
		"Stop B: buses 2 25 256\n"
		"Stop C: not found\n"
	};
	stringstream out;
	writingResult(answer, out);
	ASSERT_EQUAL(out.str(), expected);
}


void poolOfTEsts() {
	TestRunner tr;
	RUN_TEST(tr, test1);
	RUN_TEST(tr, test2);
	RUN_TEST(tr, test3);
	RUN_TEST(tr, test4);
}

/*
3
Stop A: 0.5, -1.0, 100000m to B
Stop B: 0.0, -1.1
Bus 256: B - A
4
Bus 256
Stop A
Stop B
Stop C

Bus 256: 3 stops on route, 2 unique stops, 200000 route length, 1.763715637 curvature
Stop A: buses 256
Stop B: buses 256
Stop C: not found

*/