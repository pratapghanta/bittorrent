#ifndef BINARYFILE_HANDLER_HPP
#define BINARYFILE_HANDLER_HPP

#include <iostream>
#include <fstream>
#include <string>

namespace BT {
	class BinaryFileHandler_t {
	public:
		BinaryFileHandler_t(std::string const& filename) {
			fileHndl.open(filename.c_str(), std::ios::binary);
		}

		~BinaryFileHandler_t() {
			if (fileHndl.is_open())
				fileHndl.close();
		}

		void seek(unsigned int const pos) {
			if (fileHndl.is_open())
				fileHndl.seekg(pos, fileHndl.beg);
		}

		std::string const get() {
			if (!fileHndl.is_open() || fileHndl.eof())
				return std::string({ '\0' });
			return std::string({ static_cast<char>(fileHndl.get()), '\0' });
		}

		void put(std::string const& data) {
			if (!fileHndl.is_open())
				return;

			fileHndl.write(data.c_str(), data.length());
			fileHndl.flush();
		}

	private:
		std::fstream fileHndl;
	};
}
#endif
