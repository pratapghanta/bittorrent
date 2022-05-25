#if !defined(BINARYFILE_HANDLER_HPP)
#define BINARYFILE_HANDLER_HPP

#include <iostream>
#include <fstream>
#include <string>

namespace BT 
{
	class CBinaryFileHandler
	{
	public:
		CBinaryFileHandler(std::string const& filename);
		CBinaryFileHandler(CBinaryFileHandler const&) = delete;
		CBinaryFileHandler& operator=(CBinaryFileHandler const&) = delete;
		CBinaryFileHandler(CBinaryFileHandler&&) = default;
		CBinaryFileHandler& operator=(CBinaryFileHandler&&) = default;
		~CBinaryFileHandler();

		void Seek(unsigned int const pos);
		std::string const Get();
		void Put(std::string const& data);

	private:
		std::fstream mFileHandle;
	};
}

#endif // !defined(BINARYFILE_HANDLER_HPP)
