#include <algorithm>

#include "spikey_nes.hpp"

namespace spkn
{
    void InitEmulatorLogs( sn::Level log_level )
    {
        // when multi-threading multiple emulator classes, it is HIGHLY recommended to have log_level = sn::None

        std::ofstream logFile ("simplenes.log"), cpuTraceFile;
        sn::TeeStream logTee (logFile, std::cout);

        if (logFile.is_open() && logFile.good())
            sn::Log::get().setLogStream(logTee);
        else
            sn::Log::get().setLogStream(std::cout);

        sn::Log::get().setLevel( log_level );
    }

    ColorHSL
    ConvertRGBtoHSL( const sf::Color& color )
    {
        // https://www.programmingalgorithms.com/algorithm/rgb-to-hsl?lang=C%2B%2B

        ColorHSL hsl = { 0.0f, 0.0f, 0.0f };

        float r = (float(color.r) / 255.0f);
        float g = (float(color.g) / 255.0f);
        float b = (float(color.b) / 255.0f);

        float min = std::min(std::min(r, g), b);
        float max = std::max(std::max(r, g), b);
        float delta = max - min;

        hsl.l = (max + min) / 2;

        if (delta == 0)
        {
            hsl.h = 0;
            hsl.s = 0.0f;
        }
        else
        {
            hsl.s = (hsl.l <= 0.5) ? (delta / (max + min)) : (delta / (2 - max - min));

            float hue;

            if (r == max)
            {
                hue = ((g - b) / 6) / delta;
            }
            else if (g == max)
            {
                hue = (1.0f / 3) + ((b - r) / 6) / delta;
            }
            else
            {
                hue = (2.0f / 3) + ((r - g) / 6) / delta;
            }

            while( hue < 0.0f )
            {
                hue += 1.0f;
            }
            while( hue > 1.0f )
            {
                hue -= 1.0f;
            }

            hsl.h = hue * 360.0f;
        }


        return hsl;
    }

    float
    ConvertHSLtoSingle( const ColorHSL& color, size_t rings, bool smooth )
    {
        // this function takes a HSL color and outputs an a single value that approximates the color,
        // by finding the closes point on a spiral that spirals through Hue and Lightness (it ignores Saturation)
        // the density of the spiral is determined by size_t rings

        if( color.l <= 0.0f )
        {
            return 0.0f;
        }

        if( color.l >= 1.0f )
        {
            return 1.0f;
        }

        if( rings < 1 )
        {
            rings = 1;
        }

        size_t total_rings = rings + 1; // 0.0 ... N ... 1.0

        float step_l = 1.0 / float(total_rings);

        float hue = color.h / 360.0f;

        float hue_offset = hue * step_l;

        float floor_l = floor( color.l * float(total_rings) ) / float(total_rings);

        if( !smooth )
        {
            return floor_l + hue_offset;
        }

        float lum_in_step = fmod( color.l, step_l );

        if( lum_in_step > hue_offset + step_l / 2.0f ) // up
        {
            return std::min( 1.0f, floor_l + hue_offset + step_l );
        }
        else if( lum_in_step <= hue_offset - step_l / 2.0f ) // down
        {
            return std::max( 0.0f, floor_l + hue_offset - step_l );
        }

        // stay
        return floor_l + hue_offset;
    }

}
