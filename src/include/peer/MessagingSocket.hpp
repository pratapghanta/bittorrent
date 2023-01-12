#if !defined(MESSAGING_SOCKET_HPP)
#define MESSAGING_SOCKET_HPP

#include <string>

#include "peer/MessageParcel.hpp"
#include "socket/ConnectedSocket.hpp"

namespace BT 
{
    struct ConnectedSocketParcel;

	class MessagingSocket
	{
	public:
		MessagingSocket(ConnectedSocketParcel const&);
		MessagingSocket(MessagingSocket const&) = delete;
		MessagingSocket& operator=(MessagingSocket const&) = delete;
		MessagingSocket(MessagingSocket&&) = default;
		MessagingSocket& operator=(MessagingSocket&&) = default;
		~MessagingSocket() = default;

        std::string GetFromId() const { return fromId; }
        std::string GetToId() const { return toId; }

        int Send(char const * const, unsigned int) const;
        int Receive(char*, unsigned int) const;

        void SendMessage(MessageParcel const&) const;
        MessageParcel const ReceiveMessage() const;

	private:
        std::string fromId;
        std::string toId;
        ConnectedSocket connectedSocket;
	};
}

#endif // !defined(MESSAGING_SOCKET_HPP)
