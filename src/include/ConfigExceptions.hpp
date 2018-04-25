#ifndef CONFIG_EXCEPTIONS_HPP
#define CONFIG_EXCEPTIONS_HPP

#include <exception>

namespace BT {
	struct UnknownOptionException : std::exception {
		const char * what() const throw () {
			return "Exception: Unknown option.";
		}
	};

	struct PeerLimitExceededException : std::exception {
		const char * what() const throw () {
			return "Exception: Peer limit exceeded.";
		}
	};

	struct TorrentFileMissingException : std::exception {
		const char * what() const throw () {
			return "Exception: .torrent file missing.";
		}
	};

	struct UnknownPeerExpressionException : std::exception {
		const char * what() const throw () {
			return "Exception: Peer description is incorrect.";
		}
	};

	struct InvalidIPAddressException : std::exception {
		const char * what() const throw () {
			return "Exception: Invalid Peer's address.";
		}
	};

	struct InvalidPortNumberException : std::exception {
		const char * what() const throw () {
			return "Exception: Invalid Peer's port number. Reserved ports are not accepted.";
		}
	};
}

#endif
