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
        {
            mFileHandle.close();
        }
    }

    void CBinaryFileHandler::Seek(unsigned int const pos)
    {
        if (!mFileHandle.is_open())
        {
            return;
        }

        mFileHandle.seekg(pos, mFileHandle.beg);
    }

    void CBinaryFileHandler::Get(char& ch)
    {
        if (!mFileHandle.is_open() || mFileHandle.eof())
        {
            return;
        }

        mFileHandle.get(ch);
    }

    void CBinaryFileHandler::Get(unsigned int const n, 
                                 char* buffer,
                                 unsigned int& bufferLen)
    {
        bufferLen = 0;
        while (mFileHandle.is_open() && 
               !mFileHandle.eof() && 
               bufferLen < n)
        {
            mFileHandle.get(buffer[bufferLen]);    
            bufferLen++;
        }
    }

    void CBinaryFileHandler::Put(char const& data)
    {
        if (!mFileHandle.is_open())
        {
            return;
        }

        mFileHandle.put(data);
        mFileHandle.flush();
    }
}
