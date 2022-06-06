#include "TransportTest.h"


using namespace std;

static string inStrC{
"13\n\
Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino\n\
Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka\n\
Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n\
Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka\n\
Stop Rasskazovka: 55.632761, 37.333324\n\
Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam\n\
Stop Biryusinka: 55.581065, 37.64839, 750m to Universam\n\
Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, 900m to Biryulyovo Tovarnaya\n\
Stop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo Passazhirskaya\n\
Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to Biryulyovo Zapadnoye\n\
Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye\n\
Stop Rossoshanskaya ulitsa: 55.595579, 37.605757\n\
Stop Prazhskaya: 55.611678, 37.603831\n\
6\n\
Bus 256\n\
Bus 750\n\
Bus 751\n\
Stop Samara\n\
Stop Prazhskaya\n\
Stop Biryulyovo Zapadnoye\n"
};

static string inStr{
	"11\n"
	"Stop Tolstopaltsevo: 55.611087, 37.20829\n"
	"Stop Marushkino: 55.595884, 37.209755\n"
	"Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n"
	"Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka\n"
	"Stop Rasskazovka: 55.632761, 37.333324\n"
	"Stop Biryulyovo Zapadnoye: 55.574371, 37.6517\n"
	"Stop Biryusinka: 55.581065, 37.64839\n"
	"Stop Universam: 55.587655, 37.645687\n"
	"Stop Biryulyovo Tovarnaya: 55.592028, 37.653656\n"
	"Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164\n"
	"Stop Prazhskaya: 55.611678, 37.603831\n"
	"6\n"
	"Bus 256\n"
	"Bus 750\n"
	"Bus 751\n"
	"Stop Tolstopaltsevo\n"
	"Stop Samara\n"
	"Stop Prazhskaya\n"
};

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
/*
void testParseInputStopRequest() {
	
	vector<string> input{
		"Tolstopaltsevo: 55.611087, 37.20829",
		"Marushkino: 55.595884, 37.209755",
		"Biryulyovo Tovarnaya: 55.592028, 37.653656",
		"Biryulyovo Passazhirskaya: 55.580999, 37.659164"
	};
	vector<Stop> expect {
		{"Tolstopaltsevo", Location(55.611087, 37.20829) },
		{"Marushkino", Location(55.595884, 37.209755) },
		{"Biryulyovo Tovarnaya", Location(55.592028, 37.653656) },
		{"Biryulyovo Passazhirskaya", Location(55.580999, 37.659164) }
	};
	for (size_t i(0); i < input.size(); ++i) {
		StopRequest request = StopRequest();
		request.ParseFrom(input[i]);
		ASSERT_EQUAL( *(request.stop), expect[i] );
	}
}

void testParseInputStopRequestC() {

	vector<string> input{
		"Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino",
		"Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka",
		"Rasskazovka: 55.632761, 37.333324",
		"Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam",
	};
	vector<Stop> expect{
		{"Tolstopaltsevo", Location(55.611087, 37.20829), {{"Marushkino", 3900}} },
		{"Marushkino", Location(55.595884, 37.209755), {{"Rasskazovka", 9900}}  },
		{"Rasskazovka", Location(55.632761, 37.333324), {} },
		{"Biryulyovo Zapadnoye", Location(55.574371, 37.6517), {{"Rossoshanskaya ulitsa", 7500}, {"Biryusinka", 1800}, {"Universam", 2400}}  },
	};
	for (size_t i(0); i < input.size(); ++i) {
		StopRequest request = StopRequest();
		request.ParseFrom(input[i]);
		ASSERT_EQUAL(*(request.stop), expect[i]);
	}

}

void testParseInputBusRequest() {
	vector<string> input{
		"256: "
		"Biryulyovo Zapadnoye > "
		"Biryusinka > "
		"Universam > "
		"Biryulyovo Tovarnaya > "
		"Biryulyovo Passazhirskaya > "
		"Biryulyovo Zapadnoye",
		"750: Tolstopaltsevo - Marushkino - Rasskazovka",
	};
	vector<Bus> expect{
		{ "256",
			'>', {
			"Biryulyovo Zapadnoye",
			"Biryusinka", 
			"Universam",
			"Biryulyovo Tovarnaya",
			"Biryulyovo Passazhirskaya",
			"Biryulyovo Zapadnoye"
			}
		},
		{"750", '-', {"Tolstopaltsevo", "Marushkino", "Rasskazovka"}},
	};
	for (size_t i(0); i < input.size(); ++i) {
		BusRequest request = BusRequest();
		request.ParseFrom(input[i]);
		ASSERT_EQUAL(*(request.bus), expect[i]);
	}

}

void testLeght() {
	double length = ( Location(55.611087, 37.20829).arcLength(Location( 55.595884, 37.209755 )) +
		Location(55.595884, 37.209755).arcLength(Location( 55.632761, 37.333324 )) ) * 2;
	ASSERT(doubleCompare(length, 20939.5, 0.0001));
}

void testWrite() {
	auto un = make_unique<BusResult>( "256", Result{6, 5, 4371.02} );
	BusResult bus2{ "750", Result{5, 3, 20939.5} };
	BusResult bus3{ "751", nullopt };
	vector<unique_ptr<RequestResult>> res;
	res.push_back(move(un));
	vector<unique_ptr<RequestResult>> input;
	input.push_back(make_unique<BusResult>(BusResult{ "256", Result{6, 5, 4371.02} }));
	input.push_back(make_unique<BusResult>(BusResult{ "750", Result{5, 3, 20939.5} }));
	input.push_back(make_unique<BusResult>(BusResult{ "751", nullopt }));
	input.push_back(make_unique<StopResult>(StopResult{ "Tolstopaltsevo", "buses 750" }));
	input.push_back(make_unique<StopResult>(StopResult{ "Samara", "not found" }));
	input.push_back(make_unique<StopResult>(StopResult{ "Prazhskaya", "no buses" }));
	string expected{
"Bus 256: 6 stops on route, 5 unique stops, 4371.02 route length, 1.000000 curvature\n\
Bus 750: 5 stops on route, 3 unique stops, 20939.5 route length, 1.000000 curvature\n\
Bus 751: not found\n\
Stop Tolstopaltsevo: buses 750\n\
Stop Samara: not found\n\
Stop Prazhskaya: no buses\n"
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
	vector<unique_ptr<RequestResult>> expected;
	expected.push_back(make_unique<BusResult>(BusResult{ "256", Result{6, 5, 4371.02} }));
	expected.push_back(make_unique<BusResult>(BusResult{ "750", Result{5, 3, 20939.5} }));
	expected.push_back(make_unique<BusResult>(BusResult{ "751", nullopt }));
	expected.push_back(make_unique<StopResult>(StopResult{ "Tolstopaltsevo", "buses 750" }));
	expected.push_back(make_unique<StopResult>(StopResult{ "Samara", nullopt }));
	expected.push_back(make_unique<StopResult>(StopResult{ "Prazhskaya", "no buses" }));


	for (size_t i(0); i < expected.size(); ++i) {
		ASSERT_EQUAL(answer[i], expected[i]);
	}
}

void simpleTestReadC() {
	stringstream input;
	input << inStrC;
	auto groundRequest = ReadRequests(input);
	Base base(ProcessRequests(groundRequest));
	const auto requests = ReadRequests(input);
	auto answer = base.checkRequests(requests);
	vector<unique_ptr<RequestResult>> expected;
	expected.push_back(make_unique<BusResult>(BusResult{ "256", Result{6, 5, 5950} }));
	expected.push_back(make_unique<BusResult>(BusResult{ "256", Result{6, 5, 27600} }));
	expected.push_back(make_unique<BusResult>(BusResult{ "751", nullopt }));
	expected.push_back(make_unique<StopResult>(StopResult{ "Samara", nullopt }));
	expected.push_back(make_unique<StopResult>(StopResult{ "Prazhskaya", "no buses" }));
	expected.push_back(make_unique<StopResult>(StopResult{ "Biryulyovo Zapadnoye", "buses 256 828" }));


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
"Bus 256: 6 stops on route, 5 unique stops, 4371.02 route length, 1.00000 curvature\n\
Bus 750: 5 stops on route, 3 unique stops, 20939.5 route length, 1.00000 curvature\n\
Bus 751: not found\n\
Stop Tolstopaltsevo: buses 750\n\
Stop Samara: not found\n\
Stop Prazhskaya: no buses\n"
	};
	stringstream out;
	writingResult(answer, out);
	ASSERT_EQUAL(out.str(), expected);
}

void fullTestC() {
	stringstream input;
	input << inStrC;
	auto groundRequest = ReadRequests(input);
	Base base(ProcessRequests(groundRequest));
	const auto requests = ReadRequests(input);
	auto answer = base.checkRequests(requests);
	string expected{
"Bus 256: 6 stops on route, 5 unique stops, 5950 route length, 1.361239 curvature\n\
Bus 750: 5 stops on route, 3 unique stops, 27600 route length, 1.318084 curvature\n\
Bus 751: not found\n\
Stop Samara: not found\n\
Stop Prazhskaya: no buses\n\
Stop Biryulyovo Zapadnoye: buses 256 828\n"
	};
	stringstream out;
	writingResult(answer, out);
	ASSERT_EQUAL(out.str(), expected);
}
*/
void jastTest() {
	stringstream input;
	input << inStrC;
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

void poolOfTEsts() {
	TestRunner tr;
	//RUN_TEST(tr, testParseInputStopRequest);
	//RUN_TEST(tr, testParseInputStopRequestC);
	//RUN_TEST(tr, testParseInputBusRequest);
	//////RUN_TEST(tr, simpleTestFill);
	//RUN_TEST(tr, testLeght);
	////RUN_TEST(tr, testWrite);
	////RUN_TEST(tr, simpleTestRead);
	////RUN_TEST(tr, simpleTestReadC);
	////RUN_TEST(tr, fullTest);
	////RUN_TEST(tr, fullTestC);

	RUN_TEST(tr, jastTest);
}

/*
Bus 256: 6 stops on route, 5 unique stops, 5950 route length, 1.361239 curvature
Bus 750: 5 stops on route, 3 unique stops, 27600 route length, 1.318084 curvature
Bus 751: not found
Stop Samara: not found
Stop Prazhskaya: no buses
Stop Biryulyovo Zapadnoye: buses 256 828

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