#include "TransportTest.h"
#include "transport_guide.h"

using namespace std;


int main() {
    poolOfTEsts();

	auto groundRequest = ReadRequests();
	TransportGuide guide;
	guide.readRequests(groundRequest);
	auto requests = ReadRequests();
	auto answer = guide.checkRequests(requests);
	writingResult(answer);
	      
    return 0;
}
