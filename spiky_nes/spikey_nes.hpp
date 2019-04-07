#ifndef SPKN_SPIKEY_NES_HPP_INCLUDED
#define SPKN_SPIKEY_NES_HPP_INCLUDED

#include <SFML/Graphics/Color.hpp>

#include "../simple_nes/include/Log.h"

namespace spkn
{
    void InitEmulatorLogs( sn::Level log_level = sn::None );

    struct ColorHSL
    {
        float h; // 0 to 360
        float s; // 0 to 1
        float l; // 0 to 1
    };

    ColorHSL ConvertRGBtoHSL( const sf::Color& color );

    float ConvertHSLtoSingle( const ColorHSL& color, size_t rings = 3, bool smooth = true );
}

#endif // SPKN_SPIKEY_NES_HPP_INCLUDED
