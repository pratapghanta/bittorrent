#if !defined(SOCKET_HPP)
#define SOCKET_HPP

namespace BT 
{
    class ISocket
    {
    public:
        virtual void Send(char*, unsigned int) = 0;
        virtual void Receive(char*, unsigned int) = 0;
        virtual void Destroy() = 0;
    };
}

#endif // !defined(SOCKET_HPP)
