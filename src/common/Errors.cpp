#include <iostream>

#include "common/Errors.hpp"

namespace BT
{
	void FatalError(std::string const& errorMsg) 
	{
		std::cerr << "Fatal: " << errorMsg << std::endl;
		exit(-1);
	}
}
