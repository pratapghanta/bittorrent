#if !defined(STATUSCODE_HPP)
#define STATUSCODE_HPP

enum STATUSCODE 
{
    SC_SUCCESS,
    SC_FAIL_UNKNOWN,
    SC_FAIL_BAD_PARAMETER,
    SC_FAIL_BAD_TORRENT
};

inline bool SC_FAILED(STATUSCODE const status) { return status; }

#endif // !defined(STATUSCODE_HPP)