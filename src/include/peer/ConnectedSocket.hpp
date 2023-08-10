#if !defined(I_CONNECTED_SOCKET_HPP)
#define I_CONNECTED_SOCKET_HPP

namespace BT 
{
    class IConnectedSocket
	{
	public:
		virtual unsigned int Send(void const * const, unsigned int) = 0;
        virtual unsigned int Receive(void*, unsigned int) = 0;
	};
}

#endif // !defined(I_CONNECTED_SOCKET_HPP)
