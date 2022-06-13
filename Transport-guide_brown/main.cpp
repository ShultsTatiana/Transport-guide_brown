#include "TransportTest.h"
#include "transport_guide.h"
#include "location.h"
#include "fromStream.h"
#include "json.h"
#include "helper.h"

using namespace std;

int main() {
    poolOfTEsts();

	auto groundRequest = Stream::ReadRequests();
	TransportGuide guide;
	guide.readRequests(groundRequest);
	auto requests = Stream::ReadRequests();
	Stream::writingResult(guide.checkRequests(requests));

    return 0;
}
