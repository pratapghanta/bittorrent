#if !defined(APP_CONDUCTOR_HPP)
#define APP_CONDUCTOR_HPP

#include "torrent/Torrent.hpp"
#include "StartParams.hpp"

namespace BT {
    class AppConductor {
    public:
        AppConductor(StartParams const& p);

        AppConductor(AppConductor const&) = default;
        AppConductor& operator=(AppConductor const&) = default;

        AppConductor(AppConductor&&) = default;
        AppConductor& operator=(AppConductor&&) = default;

        ~AppConductor() = default;

        void Start() const;

    private:
		void startSeeder(BT::Torrent_t const&) const;
		void startLeechers(BT::Torrent_t const&) const;
	
        StartParams mParams;
    };
}

#endif // !defined(APP_CONDUCTOR_HPP)
