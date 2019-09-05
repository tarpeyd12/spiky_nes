#ifndef HELPERS_HPP_INCLUDED
#define HELPERS_HPP_INCLUDED

namespace spkn
{
    inline void SecondsToHMS( long double seconds, uint64_t &h, uint8_t &m, long double &s );
    inline std::string SecondsToHMS( long double seconds, uint8_t precision = 2, bool prepad_zeros = true );
}

#include "helpers.inl"

#endif // HELPERS_HPP_INCLUDED
