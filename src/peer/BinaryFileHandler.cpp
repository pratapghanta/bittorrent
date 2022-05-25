#include "peer/BinaryFileHandler.hpp"

namespace BT
{
    CBinaryFileHandler::CBinaryFileHandler(std::string const& filename)
    {
        mFileHandle.open(filename.c_str(), std::ios::binary);
    }

    CBinaryFileHandler::~CBinaryFileHandler()
    {
        if (mFileHandle.is_open())
            mFileHandle.close();
    }

    void CBinaryFileHandler::Seek(unsigned int const pos)
    {
        if (!mFileHandle.is_open())
            return;

        mFileHandle.seekg(pos, mFileHandle.beg);
    }

    std::string const CBinaryFileHandler::Get()
    {
        if (!mFileHandle.is_open() || mFileHandle.eof())
            return "";

        return std::string({ static_cast<char>(mFileHandle.get()), '\0' });
    }

    void CBinaryFileHandler::Put(std::string const& data)
    {
        if (!mFileHandle.is_open())
            return;

        mFileHandle.write(data.c_str(), data.length());
        mFileHandle.flush();
    }
}
