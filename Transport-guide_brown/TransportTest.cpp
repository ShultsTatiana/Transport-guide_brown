#include "TransportTest.h"


using namespace std;

static string inStr{
"10\n\
Stop Tolstopaltsevo: 55.611087, 37.20829\n\
Stop Marushkino: 55.595884, 37.209755\n\
Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n\
Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka\n\
Stop Rasskazovka: 55.632761, 37.333324\n\
Stop Biryulyovo Zapadnoye: 55.574371, 37.6517\n\
Stop Biryusinka: 55.581065, 37.64839\n\
Stop Universam: 55.587655, 37.645687\n\
Stop Biryulyovo Tovarnaya: 55.592028, 37.653656\n\
Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164\n\
3\n\
Bus 256\n\
Bus 750\n\
Bus 751\n"
};

bool doubleCompare(double x1, double x2, double epsilon = 0.00001) {
	double delta = abs(x1 - x2);
	double delta1 = x1 * epsilon;
	double delta2 = x2 * epsilon;
	return delta <= max(delta1, delta2);
}

bool operator==(const StopRequest& lhs, const StopRequest& rhs) {
	return(
		lhs.object.name == rhs.object.name &&
		doubleCompare(lhs.object.location->latitude, rhs.object.location->latitude, 0.000001) &&
		doubleCompare(lhs.object.location->longitude, rhs.object.location->longitude, 0.000001)
		);
}

struct Stop {
	Request::stopName stopName_;
	Location location;
};
struct Bus {
	Request::busName busNumb_;
	std::vector<Request::stopName> rout_;
	std::optional<char> routType_;
};

bool operator==(const StopRequest& lhs, const Stop& rhs) {
	return(
		lhs.object.name == rhs.stopName_ &&
		doubleCompare(lhs.object.location->latitude, rhs.location.latitude, 0.000001) &&
		doubleCompare(lhs.object.location->longitude, rhs.location.longitude, 0.000001)
		);
}
bool operator==(const BusRequest& lhs, const Bus& rhs) {
	return(
		lhs.object.name == rhs.busNumb_ &&
		lhs.object.routeType->vectorRoute == rhs.rout_ &&
		lhs.object.routeType->routType_ == rhs.routType_
		);
}

ostream& operator<<(ostream& out, const StopRequest& lhs) {
	out << lhs.object.name << ": "
		<< lhs.object.location->latitude << ", "
		<< lhs.object.location->longitude;
	return out;
}
ostream& operator<<(ostream& out, const Stop& lhs) {
	out << lhs.stopName_ << ": "
		<< lhs.location.latitude << ", "
		<< lhs.location.longitude;
	return out;
}

ostream& operator<<(ostream& out, const BusRequest& lhs) {
	out << lhs.object.name << ": ";
	if (!lhs.object.routeType->vectorRoute.empty()) {
		for (size_t i(0); i < lhs.object.routeType->vectorRoute.size() - 1; ++i) {
			out << lhs.object.routeType->vectorRoute[i] << " " << lhs.object.routeType->routType_.value() << " ";
		}
		out << lhs.object.routeType->vectorRoute.back();
	}
	return out;
}
ostream& operator<<(ostream& out, const Bus& lhs) {
	out << lhs.busNumb_ << ": ";
	if (!lhs.rout_.empty()) {
		for (size_t i(0); i < lhs.rout_.size() - 1; ++i) {
			out << lhs.rout_ << " " << lhs.routType_.value() << " ";
		}
		out << lhs.rout_.back();
	}
	return out;
}

void testParseInputStopRequest() {
	vector<string> input{
		"Tolstopaltsevo: 55.611087, 37.20829",
		"Marushkino: 55.595884, 37.209755",
		"Biryulyovo Tovarnaya: 55.592028, 37.653656",
		"Biryulyovo Passazhirskaya: 55.580999, 37.659164"
	};
	vector<Stop> expect{
		{"Tolstopaltsevo", 55.611087, 37.20829},
		{"Marushkino", 55.595884, 37.209755},
		{"Biryulyovo Tovarnaya", 55.592028, 37.653656},
		{"Biryulyovo Passazhirskaya", 55.580999, 37.659164}
	};
	for (size_t i(0); i < input.size(); ++i) {
		StopRequest request = StopRequest();
		request.ParseFrom(input[i]);
		ASSERT_EQUAL( request, expect[i] );
	}

}
void testParseInputBusRequest() {
	vector<string> input{
		"256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye",
		"750: Tolstopaltsevo - Marushkino - Rasskazovka",
	};
	vector<Bus> expect{
		{
			"256", 
			{
			"Biryulyovo Zapadnoye",
			"Biryusinka", 
			"Universam",
			"Biryulyovo Tovarnaya",
			"Biryulyovo Passazhirskaya",
			"Biryulyovo Zapadnoye"
			},
			'>'
		},
		{"750", {"Tolstopaltsevo", "Marushkino", "Rasskazovka"}, '-'},
	};
	for (size_t i(0); i < input.size(); ++i) {
		BusRequest request = BusRequest();
		request.ParseFrom(input[i]);
		ASSERT_EQUAL(request, expect[i]);
	}

}

/*void simpleTestFill(){
	stringstream input;
	input << inStr;
	auto request = ReadRequests(input);
	Base base(ProcessRequests(request));
}*/

bool operator==(const BusResult& lhs, const BusResult& rhs) {
	if (lhs.result && rhs.result) {
		return (
			lhs.bus == rhs.bus &&
			lhs.result->amountStops == rhs.result->amountStops &&
			lhs.result->uniqStops == rhs.result->uniqStops &&
			doubleCompare(lhs.result->lenRoute, rhs.result->lenRoute, 0.0001)
			);
	}
	else if ((!lhs.result && rhs.result) || (lhs.result && !rhs.result)) {
		return false;
	}
	else { return lhs.bus == rhs.bus; }
}
ostream& operator<<(ostream& out, const BusResult& result) {
	out.precision(6);
	out << "Bus " << result.bus << ": ";
	if (result.result) {
		out << result.result->amountStops << " stops on route, "
			<< result.result->uniqStops << " unique stops, "
			<< result.result->lenRoute << " route length\n";
	}
	else {
		out << "not found\n";
	}
	return out;
}

void testLeght() {
	double length = (arcLength({ 55.611087, 37.20829 }, { 55.595884, 37.209755 }) +
		arcLength({ 55.595884, 37.209755 }, { 55.632761, 37.333324 })) * 2;
	ASSERT(doubleCompare(length, 20939.5, 0.0001));
}

void testWrite() {
	vector<BusResult> input = {
		BusResult{"256", Result{6, 5, 4371.02}},
		BusResult{"750", Result{5, 3, 20939.5}},
		BusResult{"751", nullopt}
	};
	string expected{
"Bus 256: 6 stops on route, 5 unique stops, 4371.02 route length\n\
Bus 750: 5 stops on route, 3 unique stops, 20939.5 route length\n\
Bus 751: not found\n"
	};
	stringstream out;
	writingResult(input, out);
	ASSERT_EQUAL(out.str(), expected);
}

void simpleTestRead() {
	stringstream input;
	input << inStr;
	auto groundRequest = ReadRequests(input);
	Base base(ProcessRequests(groundRequest));
	const auto requests = ReadRequests(input);
	auto answer = base.checkRequests(requests);
	vector<BusResult> expected = {
		BusResult{"256", Result{6, 5, 4371.02}},
		BusResult{"750", Result{5, 3, 20939.5}},
		BusResult{"751", nullopt}
	};

	for (size_t i(0); i < expected.size(); ++i) {
		ASSERT_EQUAL(answer[i], expected[i]);
	}
}

void fullTest() {
	stringstream input;
	input << inStr;
	auto groundRequest = ReadRequests(input);
	Base base(ProcessRequests(groundRequest));
	const auto requests = ReadRequests(input);
	auto answer = base.checkRequests(requests);
	string expected{
"Bus 256: 6 stops on route, 5 unique stops, 4371.02 route length\n\
Bus 750: 5 stops on route, 3 unique stops, 20939.5 route length\n\
Bus 751: not found\n"
	};
	stringstream out;
	writingResult(answer, out);
	ASSERT_EQUAL(out.str(), expected);
}


void poolOfTEsts() {
	TestRunner tr;
	RUN_TEST(tr, testParseInputStopRequest);
	RUN_TEST(tr, testParseInputBusRequest);
	//RUN_TEST(tr, simpleTestFill);
	RUN_TEST(tr, testLeght);
	RUN_TEST(tr, testWrite);
	RUN_TEST(tr, simpleTestRead);
	RUN_TEST(tr, fullTest);

}