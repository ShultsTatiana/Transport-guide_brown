#include "TransportTest.h"
#include "transport_guide.h"

using namespace std;

int main() {
    
	poolOfTEsts();
	/*
	{
		using namespace Stream;

		auto groundRequest = Stream::ReadRequests();

		TransportGuide guide;
		guide.readRequests(groundRequest);

		auto requests = Stream::ReadRequests();

		Stream::writingResult(guide.checkRequests(requests));
	}
	*/

	{
		using namespace Json;

		auto groundRequest = Load(cin);

		TransportGuide guide;
		guide.readRequests(groundRequest);

		auto answer = guide.checkRequests(groundRequest);

		UnloadDoc(cout, answer);
	}

    return 0;
}
