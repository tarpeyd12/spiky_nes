#ifndef HELPERS_INL_INCLUDED
#define HELPERS_INL_INCLUDED

#include <sstream>

namespace spkn
{
    void
    SecondsToHMS( long double seconds, uint64_t &h, uint8_t &m, long double &s )
    {
        h = uint64_t( seconds / 3600l );
        m = uint8_t( uint64_t( seconds / 60l ) % 60l );
        s = fmodl( seconds, 60.0L );
    }

    std::string
    SecondsToHMS( long double seconds, uint8_t precision, bool prepad_zeros )
    {
        std::ostringstream ss;

        uint64_t    h;
        uint8_t     m;
        long double s;

        SecondsToHMS( seconds, h, m, s );

        if( seconds >= 3600.0L )
        {
            ss << h << "h";

        }

        bool display_minutes = seconds >= 60.0L;

        if( prepad_zeros && display_minutes )
        {
            ss << std::setw( 2 ) << std::setfill( '0' ) << uint16_t( m ) << "m";
        }
        else if( display_minutes )
        {
            ss << uint16_t( m ) << "m";
        }

        if( prepad_zeros )
        {
            ss << std::fixed << std::setprecision( precision ) << std::setw( 2 + precision + ( precision > 0 ? 1 : 0 ) ) << std::setfill( '0' ) << s << "s";
        }
        else
        {
            ss << std::fixed << std::setprecision( precision ) << s << "s";
        }

        return ss.str();
    }
}

#endif // HELPERS_INL_INCLUDED
