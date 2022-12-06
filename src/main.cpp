#include <vector>
#include <string>
#include <cstring>
#include <csignal>
#include <unistd.h>

#include "common/Helpers.hpp"
#include "AppConductor.hpp"
#include "StartParams.hpp"

namespace {
	void handleInterruptSignal()
	{
		struct sigaction act;
		memset(&act, 0, sizeof(act));
		act.sa_handler = SIG_IGN;
		act.sa_flags = SA_RESTART;
		sigaction(SIGPIPE, &act, NULL);
	}

	std::vector<std::string> getVectorOfStrings(int const argc, char** argv)
	{
		std::vector<std::string> data;
		data.reserve(argc);
		for (int i = 1; i < argc; i++)
		{
			data.push_back(std::string(argv[i]));
		}
		return data;
	}
}

int main (int argc, char * argv[]) 
{
	handleInterruptSignal();

	BT::StartParams const params(getVectorOfStrings(argc, argv));
	BT::AppConductor_t const ac(params);
	ac.Start();

	return 0;
}

