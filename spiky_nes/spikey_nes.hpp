#ifndef SPKN_SPIKEY_NES_HPP_INCLUDED
#define SPKN_SPIKEY_NES_HPP_INCLUDED

#include <SFML/Graphics.hpp>

#include "../simple_nes/include/Log.h"

namespace spkn
{
    void SetProcessPriority_low();
    void SetProcessPriority_lowest();

    void InitEmulatorLogs( sn::Level log_level = sn::None );

    struct ColorHSL
    {
        float h; // 0 to 360
        float s; // 0 to 1
        float l; // 0 to 1
    };

    ColorHSL ConvertRGBtoHSL( const sf::Color& color );

    float ConvertHSLtoSingle( const ColorHSL& color, size_t rings = 3, bool smooth = true );

    sf::Image ResizeImage( const sf::Image& image, size_t width, size_t height );
}

#include "fitness.hpp"
#include "game_state.hpp"

#endif // SPKN_SPIKEY_NES_HPP_INCLUDED
