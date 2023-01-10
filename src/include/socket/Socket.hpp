#if !defined(SOCKET_HPP)
#define SOCKET_HPP

namespace BT 
{
    class ISocket
    {
    public:
        virtual void Close() = 0;
    };
}

#endif // !defined(SOCKET_HPP)
