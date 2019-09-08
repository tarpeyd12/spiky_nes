#ifndef HELPERS_HPP_INCLUDED
#define HELPERS_HPP_INCLUDED

namespace spkn
{
    inline void SecondsToHMS( long double seconds, uint64_t &h, uint8_t &m, long double &s );
    inline std::string SecondsToHMS( long double seconds, uint8_t precision = 2, bool prepad_zeros = true );

    inline std::string GetFileAsString( const std::string& file_path );

    inline std::string GetROMFileHashString( const std::string& rom_file_path );
}

#include "helpers.inl"

#endif // HELPERS_HPP_INCLUDED
