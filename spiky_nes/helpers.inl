#ifndef HELPERS_INL_INCLUDED
#define HELPERS_INL_INCLUDED

#include <sstream>

namespace spkn
{
    void
    SecondsToHMS( long double seconds, uint32_t &h, uint32_t &m, long double &s )
    {
        h = uint32_t( seconds / 3600 );
        m = uint32_t( seconds / 60 ) % 60;
        s = fmod( seconds, 60.0 );
    }

    std::string
    SecondsToHMS( long double seconds, uint8_t precision, bool prepad_zeros )
    {
        std::ostringstream ss;

        uint32_t h, m;
        long double s;

        SecondsToHMS( seconds, h, m, s );

        if( seconds >= 3600.0 )
        {
            ss << h << "h";

        }

        bool display_minutes = seconds >= 60.0;

        if( prepad_zeros && display_minutes )
        {
            ss << std::setw(2) << std::setfill('0') << m << "m";
        }
        else if( display_minutes )
        {
            ss << m << "m";
        }

        if( prepad_zeros )
        {
            ss << std::fixed << std::setprecision( precision ) << std::setw( 2 + precision + ( precision > 0 ? 1 : 0 ) ) << std::setfill('0') << s << "s";
        }
        else
        {
            ss << std::fixed << std::setprecision( precision ) << s << "s";
        }

        return ss.str();
    }
}

#endif // HELPERS_INL_INCLUDED
