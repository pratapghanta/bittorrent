#ifndef STATUSCODES_H
#define STATUSCODES_H

namespace BT {
    enum STATUSCODE {
        SC_SUCCESS,
        SC_FAIL_UNKNOWN,
        SC_FAIL_BAD_TORRENT
    };

    inline bool SC_FAILED(STATUSCODE const status) { return status; }
}

#endif // #ifndef STATUSCODES_H