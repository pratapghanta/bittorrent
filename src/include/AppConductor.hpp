#if !defined(APP_CONDUCTOR_HPP)
#define APP_CONDUCTOR_HPP

#include "torrent/Torrent.hpp"
#include "StartParams.hpp"

namespace BT {
    class AppConductor_t {
    public:
        AppConductor_t(StartParams const& p);

        AppConductor_t(AppConductor_t const&) = default;
        AppConductor_t& operator=(AppConductor_t const&) = default;

        AppConductor_t(AppConductor_t&&) = default;
        AppConductor_t& operator=(AppConductor_t&&) = default;

        ~AppConductor_t() = default;

        void Start() const;

    private:
		void startSeeder(BT::Torrent_t const&) const;
		void startLeechers(BT::Torrent_t const&) const;
	
        StartParams mParams;
    };
}

#endif // !defined(APP_CONDUCTOR_HPP)
