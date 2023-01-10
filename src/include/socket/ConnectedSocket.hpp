#if !defined(CONNECTED_SOCKET_HPP)
#define CONNECTED_SOCKET_HPP

#include <string>

namespace BT 
{
    struct ConnectedSocketParcel;
    class ConnectedSocket
    {
    private:
        ConnectedSocket() = default;

    public:
        ConnectedSocket(ConnectedSocketParcel const&);
        ConnectedSocket(ConnectedSocket const&) = delete;
        ConnectedSocket& operator=(ConnectedSocket const&) = delete;
        ConnectedSocket(ConnectedSocket&&);
        ConnectedSocket& operator=(ConnectedSocket&&);
        ~ConnectedSocket();

        virtual int Send(char*, unsigned int) const;
        virtual int Recieve(char*, unsigned int) const;

        virtual void Close();

    private:
        std::string fromIp;
        unsigned int fromPort;
        std::string toIp;
        unsigned int toPort;
        unsigned int connectedSockfd;
    };
}

#endif // !defined(CONNECTED_SOCKET_HPP)
