#if !defined(MESSAGING_SOCKET_HPP)
#define MESSAGING_SOCKET_HPP

#include <memory>
#include <string>

#include "peer/ConnectedSocket.hpp"
#include "peer/MessageParcel.hpp"

namespace BT 
{
    struct ConnectedSocketParcel;
	
	using ConnectedSocketPtr = std::unique_ptr<IConnectedSocket>;

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

        unsigned int Send(char const * const, unsigned int) const;
        unsigned int Receive(char*, unsigned int) const;

        void SendMessage(MessageParcel const&) const;
        MessageParcel const ReceiveMessage() const;

	private:
        std::string fromId;
        std::string toId;
        ConnectedSocketPtr connectedSocketPtr;
	};
}

#endif // !defined(MESSAGING_SOCKET_HPP)
