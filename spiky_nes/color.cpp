#include <algorithm>

#include "spikey_nes.hpp"

#include "color.hpp"

namespace spkn
{
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

            while( hue >= 1.0f )
            {
                hue -= 1.0f;
            }
            while( hue < 0.0f )
            {
                hue += 1.0f;
            }

            hsl.h = hue * 360.0f;
        }


        return hsl;
    }

    inline
    float
    ConvertRGBtoL( const sf::Color& color )
    {
        uint8_t min = std::min(std::min(color.r, color.g), color.b);
        uint8_t max = std::max(std::max(color.r, color.g), color.b);

        return (max/255.0f + min/255.0f) / 2.0f;
    }

    float
    ConvertHSLtoSingle( const ColorHSL& color, size_t rings, bool smooth )
    {
        // this function takes a HSL color and outputs an a single value that approximates the color,
        // by finding the closes point on a spiral that spirals through Hue and Lightness (it ignores Saturation)
        // the density of the spiral is determined by size_t rings
        // as size_t increases in value the closer the output resembles the Lightness component of the input

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

    sf::Vector3f
    ColorToVec3f( const sf::Color& color )
    {
        return { float(color.r)/255.0f, float(color.g)/255.0f, float(color.b)/255.0f };
    }

    sf::Color
    Vec3fToColor( const sf::Vector3f& vec3f )
    {
        return { uint8_t(vec3f.x*255), uint8_t(vec3f.y*255), uint8_t(vec3f.z*255), 255 };
    }

    inline
    sf::Vector3f
    __KernelOp( const sf::Image& image, size_t x, size_t y, const std::vector<std::vector<float>>& kernel, size_t width, size_t height )
    {
        if( width % 2 == 0 || height % 2 == 0 )
        {
            return {0.0f,0.0f,0.0f};
        }

        auto clampXY = [&]( int32_t& _x, int32_t& _y ) -> void { if( _x < 0 ) _x = 0; if( _y < 0 ) _y = 0; if( size_t(_x) >= image.getSize().x ) _x = image.getSize().x-1; if( size_t(_y) >= image.getSize().y ) _y = image.getSize().y-1; };
        auto getPixel = [&]( int32_t _x, int32_t _y ) -> sf::Vector3f { return ColorToVec3f( image.getPixel( _x, _y ) ); };

        sf::Vector3f out = { 0.0f, 0.0f, 0.0f };

        int32_t start_x = x - width/2;
        int32_t start_y = y - height/2;

        int32_t end_x = start_x + width - 1;
        int32_t end_y = start_y + height - 1;

        for( int32_t xv = start_x; xv <= end_x; ++xv )
        {
            for( int32_t yv = start_y; yv <= end_y; ++yv )
            {
                int32_t xvc = xv;
                int32_t yvc = yv;

                clampXY( xvc, yvc );

                out += getPixel( xvc, yvc ) * kernel[ xv-start_x ][ yv-start_y ];
            }
        }

        return out;
    }

    inline
    float
    __KernelOp( size_t isWidth, size_t isHeight, const std::vector<float>& image, size_t x, size_t y, const std::vector<std::vector<float>>& kernel, size_t width, size_t height )
    {
        if( width % 2 == 0 || height % 2 == 0 )
        {
            return 0.0f;
        }

        auto clampXY = [&]( int32_t& _x, int32_t& _y ) -> void
        {
            if( _x < 0 )
                _x = 0;
            if( _y < 0 )
                _y = 0;
            if( size_t(_x) >= isWidth )
                _x = isWidth-1;
            if( size_t(_y) >= isHeight )
                _y = isHeight-1;
        };

        float out = 0.0f;

        int32_t start_x = x - width/2;
        int32_t start_y = y - height/2;
        int32_t end_x = start_x + width - 1;
        int32_t end_y = start_y + height - 1;
        for( int32_t xv = start_x; xv <= end_x; ++xv )
        {
            for( int32_t yv = start_y; yv <= end_y; ++yv )
            {
                float k = kernel[ xv-start_x ][ yv-start_y ];
                if( !k ) { continue; }
                int32_t xvc = xv;
                int32_t yvc = yv;
                clampXY( xvc, yvc );
                out += image[ yvc * isWidth + xvc ] * k;
            }
        }

        return out;
    }

    sf::Image
    SobelEdgeDetection( const sf::Image& image )
    {
        std::vector<std::vector<float>> kgx = { {-1,0,1}, {-2,0,2}, {-1,0,1} };
        std::vector<std::vector<float>> kgy = { {-1,-2,-1}, {0,0,0}, {1,2,1} };

        sf::Image out;
        out.create( image.getSize().x, image.getSize().y );

        for( size_t y = 0; y < image.getSize().y; ++y )
        {
            for( size_t x = 0; x < image.getSize().x; ++x )
            {
                sf::Vector3f gx = __KernelOp( image, x, y, kgx, 3, 3 );
                sf::Vector3f gy = __KernelOp( image, x, y, kgy, 3, 3 );

                gx.x *= gx.x;
                gx.y *= gx.y;
                gx.z *= gx.z;

                gy.x *= gy.x;
                gy.y *= gy.y;
                gy.z *= gy.z;

                sf::Vector3f g = gx + gy;
                g.x = sqrt( g.x );
                g.y = sqrt( g.y );
                g.z = sqrt( g.z );

                out.setPixel( x, y, Vec3fToColor( g ) );
            }
        }

        return out;
    }

    sf::Image
    LaplacianEdgeDetection( const sf::Image& image )
    {
        std::vector<std::vector<float>> kernel = { {-1,-1,-1}, {-1,8,-1}, {-1,-1,-1} };

        sf::Image out;
        out.create( image.getSize().x, image.getSize().y );

        for( size_t y = 0; y < image.getSize().y; ++y )
        {
            for( size_t x = 0; x < image.getSize().x; ++x )
            {
                out.setPixel( x, y, Vec3fToColor( __KernelOp( image, x, y, kernel, 3, 3 ) ) );
            }
        }

        return out;
    }

    void
    ImageLaplacianEdgeDetectionToLightness( const sf::Image& image, std::vector<double>& destination, double scale, size_t startPos )
    {
        std::vector<float> temp( image.getSize().x * image.getSize().y, 0.0f );

        auto index = [&image]( size_t x, size_t y ) -> size_t { return y * image.getSize().x + x; };

        for( size_t y = 0; y < image.getSize().y; ++y )
        {
            size_t y1 = y - 1, y2 = y + 1;

            bool N = ( y > 0 );
            bool S = ( y < image.getSize().y - 2 );
            bool not_edge_y = ( N && S );

            for( size_t x = 0; x < image.getSize().x; ++x )
            {
                size_t x1 = x - 1, x2 = x + 1;

                float value = ConvertRGBtoL( image.getPixel( x, y ) );

                bool W = ( x > 0 );
                bool E = ( x < image.getSize().x - 2 );
                bool not_edge_x = ( W && E );

                size_t i_NW = index( x1, y1 ) + startPos;
                size_t i_N  = index( x,  y1 ) + startPos;
                size_t i_NE = index( x2, y1 ) + startPos;
                size_t i_W  = index( x1, y  ) + startPos;
                size_t i_C  = index( x,  y  ) + startPos;
                size_t i_E  = index( x2, y  ) + startPos;
                size_t i_SW = index( x1, y2 ) + startPos;
                size_t i_S  = index( x,  y2 ) + startPos;
                size_t i_SE = index( x2, y2 ) + startPos;

                temp[ i_C  ] += value * 8.0f;

                if( not_edge_x && not_edge_y )
                {
                    temp[ i_NW ] -= value;
                    temp[ i_N  ] -= value;
                    temp[ i_NE ] -= value;
                    temp[ i_W  ] -= value;
                    temp[ i_E  ] -= value;
                    temp[ i_SW ] -= value;
                    temp[ i_S  ] -= value;
                    temp[ i_SE ] -= value;
                }
                else
                {
                    if( N && W ) temp[ i_NW ] -= value;
                    if( N )      temp[ i_N  ] -= value;
                    if( N && E ) temp[ i_NE ] -= value;
                    if( W )      temp[ i_W  ] -= value;
                    if( E )      temp[ i_E  ] -= value;
                    if( S && W ) temp[ i_SW ] -= value;
                    if( S )      temp[ i_S  ] -= value;
                    if( S && E ) temp[ i_SE ] -= value;
                }
            }
        }

        neat::MinMax<float> minmax{ 0.0f };
        for( size_t i = 0; i < image.getSize().x * image.getSize().y; ++i )
        {
            minmax.expand( temp[ i ] );
        }

        for( size_t i = 0; i < image.getSize().x * image.getSize().y; ++i )
        {
            destination[ i + startPos ] = ( ( temp[ i ] - minmax.min ) / minmax.range() ) * scale;
        }
    }

    inline
    void
    __SobelKernelOp( bool doclamp, size_t isWidth, size_t isHeight, const std::vector<float>& image, size_t x, size_t y, float& gx, float& gy )
    {
        auto getPixel = [&]( int32_t _x, int32_t _y ) -> float
        {
            if( doclamp )
            {
                _x = neat::MinMax<int64_t>{ 0, int64_t(isWidth)-1 }.clamp( _x );
                _y = neat::MinMax<int64_t>{ 0, int64_t(isHeight)-1 }.clamp( _y );
            }
            return image[ _y * isWidth + _x ];
        };

        float N = getPixel( x, y-1 );
        float S = getPixel( x, y+1 );
        float E = getPixel( x+1, y );
        float W = getPixel( x-1, y );

        float NE = getPixel( x+1, y-1 );
        float SE = getPixel( x+1, y+1 );
        float NW = getPixel( x-1, y-1 );
        float SW = getPixel( x-1, y+1 );

        gx = ( NE + E + E + SE ) - ( NW + W + W + SW );
        gy = ( SW + S + S + SE ) - ( NW + N + N + NE );
    }

    void
    ImageSobelEdgeDetectionToLightness( const sf::Image& image, std::vector<double>& destination, double scale, size_t startPos )
    {
        std::vector<float> temp( image.getSize().x * image.getSize().y, 0.0 );

        for( size_t i = 0; i < temp.size(); ++i )
        {
            temp[ i ] = ConvertRGBtoL( image.getPixel( i % image.getSize().x, i / image.getSize().x ) );
        }

        neat::MinMax<float> minmax;
        for( size_t y = 0; y < image.getSize().y; ++y )
        {
            bool yedge = ( !y || y==image.getSize().y-1 );
            for( size_t x = 0; x < image.getSize().x; ++x )
            {
                bool xedge = ( !x || x==image.getSize().x-1 );
                float gx, gy;
                __SobelKernelOp( yedge || xedge, image.getSize().x, image.getSize().y, temp, x, y, gx, gy );
                float value = sqrt( gx * gx + gy * gy );
                minmax.expand( value );
                destination[ y * image.getSize().x + x + startPos ] = value;
            }
        }

        float range = minmax.range();
        for( size_t i = 0; i < temp.size(); ++i )
        {
            double& v = destination[ i + startPos ];
            v = ( ( v - minmax.min ) / range ) * scale;
        }
    }

    sf::Image
    ResizeImage( const sf::Image& image, size_t width, size_t height )
    {
        // https://en.sfml-dev.org/forums/index.php?topic=4119.0

        // use the render API of SFML to scale the image ( super slow )
        /*thread_local sf::Texture tex;
        tex.setSmooth( true );
        tex.loadFromImage( image );

        thread_local sf::Sprite spriteTmp;
        spriteTmp.setTexture( tex );
        spriteTmp.setScale( width, height );

        thread_local sf::RenderTexture rtex;
        rtex.create( width, height );
        rtex.setSmooth( true );
        rtex.clear();
        rtex.draw( spriteTmp );
        rtex.display();

        return rtex.getTexture().copyToImage();*/

        // nearest sampling, super fast but terrible
        /*sf::Image out;
        out.create( width, height );

        for( size_t y = 0; y < out.getSize().y; ++y )
        {
            for( size_t x = 0; x < out.getSize().x; ++x )
            {
                size_t sx = double(x)/double(out.getSize().x) * image.getSize().x;
                size_t sy = double(y)/double(out.getSize().y) * image.getSize().y;
                out.setPixel( x, y, image.getPixel( sx, sy ) );
            }
        }

        return out;*/

        // https://rosettacode.org/wiki/Bilinear_interpolation

        auto lerp =  [&]( float s, float e, float t ) -> float { return s + ( e - s ) * t; };
        auto blerp = [&]( float c00, float c10, float c01, float c11, float tx, float ty ) -> float { return lerp( lerp( c00, c10, tx ), lerp( c01, c11, tx ), ty ); };

        sf::Image out;
        out.create( width, height );

        size_t newWidth = width;
        size_t newHeight= height;
        size_t x = 0;
        size_t y = 0;
        for( ; y < newHeight; ++x )
        {
            if( x > newWidth )
            {
                x = 0;
                y++;
            }
            double gx = x / (double)(newWidth) * (image.getSize().x-1);
            double gy = y / (double)(newHeight) * (image.getSize().y-1);
            size_t gxi = (size_t)gx;
            size_t gyi = (size_t)gy;

            sf::Color c00 = image.getPixel( gxi, gyi );
            sf::Color c10 = image.getPixel( gxi + 1, gyi );
            sf::Color c01 = image.getPixel( gxi, gyi + 1 );
            sf::Color c11 = image.getPixel( gxi + 1, gyi + 1 );

            sf::Color result = { 0, 0, 0, 0 };
            result.r = blerp( c00.r, c10.r, c01.r, c11.r, gx - gxi, gy - gyi);
            result.g = blerp( c00.g, c10.g, c01.g, c11.g, gx - gxi, gy - gyi);
            result.b = blerp( c00.b, c10.b, c01.b, c11.b, gx - gxi, gy - gyi);
            result.a = blerp( c00.a, c10.a, c01.a, c11.a, gx - gxi, gy - gyi);

            out.setPixel( x, y, result );
        }

        return out;
    }

    sf::Image
    QuarterImage( const sf::Image& image )
    {
        sf::Image out;
        out.create( image.getSize().x / 2, image.getSize().y / 2 );

        for( size_t x = 0; x < out.getSize().x; ++x )
        {
            size_t _x = x * 2;
            for( size_t y = 0; y < out.getSize().y; ++y )
            {
                size_t _y = y * 2;
                sf::Vector3f val;
                val  = ColorToVec3f( image.getPixel( _x,     _y     ) );
                val += ColorToVec3f( image.getPixel( _x,     _y + 1 ) );
                val += ColorToVec3f( image.getPixel( _x + 1, _y     ) );
                val += ColorToVec3f( image.getPixel( _x + 1, _y + 1 ) );

                out.setPixel( x, y, Vec3fToColor( val / 4.0f ) );
            }
        }

        return out;
    }

    void
    ResizeImageVec( const std::vector<double>& image, size_t width, size_t height, std::vector<double>& destination, size_t newWidth, size_t newHeight, size_t offset )
    {
        auto index = []( size_t x, size_t y, size_t w ) -> size_t { return y * w + x; };
        auto lerp =  [&]( float s, float e, float t ) -> float { return s + ( e - s ) * t; };
        auto blerp = [&]( float c00, float c10, float c01, float c11, float tx, float ty ) -> float { return lerp( lerp( c00, c10, tx ), lerp( c01, c11, tx ), ty ); };

        //std::vector<double> out( newWidth*newHeight, 0.0 );

        size_t x = 0;
        size_t y = 0;
        for( ; y < newHeight; ++x )
        {
            if( x > newWidth )
            {
                x = 0;
                y++;
            }
            double gx = x / (double)(newWidth) * (width-1);
            double gy = y / (double)(newHeight) * (height-1);
            size_t gxi = (size_t)gx;
            size_t gyi = (size_t)gy;

            float c00 = image[ index( gxi,     gyi,     width ) ];
            float c10 = image[ index( gxi + 1, gyi,     width ) ];
            float c01 = image[ index( gxi,     gyi + 1, width ) ];
            float c11 = image[ index( gxi + 1, gyi + 1, width ) ];

            double result = blerp( c00, c10, c01, c11, gx - gxi, gy - gyi);

            destination[ offset + index( x, y, newWidth ) ] = result;
        }
    }


    std::vector<double>
    ResizeImageVec( const std::vector<double>& image, size_t width, size_t height, size_t newWidth, size_t newHeight, size_t offset )
    {
        std::vector<double> tmp( newWidth * newHeight, 0.0 );
        ResizeImageVec( image, width, height, tmp, newWidth, newHeight, 0 );
        return tmp;
    }


    sf::Image
    ImageToGreyscale( const sf::Image& image )
    {
        sf::Image out;
        out.create( image.getSize().x, image.getSize().y );

        for( size_t y = 0; y < image.getSize().y; ++y )
        {
            for( size_t x = 0; x < image.getSize().x; ++x )
            {
                uint8_t l = ConvertRGBtoHSL( image.getPixel( x, y ) ).l * 255;
                out.setPixel( x, y, { l, l, l, 255 } );
            }
        }

        return out;
    }

    std::vector<double>
    ImageToSingle( const sf::Image& image, size_t rings, bool smooth )
    {
        std::vector<double> out( image.getSize().x * image.getSize().y, 0.0 );

        ImageToSingle( image, rings, smooth, out, 1.0, 0 );

        return out;
    }

    void
    ImageToSingle( const sf::Image& image, size_t rings, bool smooth, std::vector<double>& destination, double scale, size_t startPos )
    {
        for( size_t y = 0; y < image.getSize().y; ++y )
        {
            for( size_t x = 0; x < image.getSize().x; ++x )
            {
                auto index = y * image.getSize().x + x;

                destination[ index + startPos ] = scale * ConvertHSLtoSingle( ConvertRGBtoHSL( image.getPixel( x, y ) ), rings, smooth );
            }
        }
    }

    sf::Image
    ImageVecToImage( const std::vector<double>& imageVec, size_t width, size_t height, float scale )
    {
        sf::Image image;
        image.create( width, height );

        for( size_t y = 0; y < image.getSize().y; ++y )
        {
            for( size_t x = 0; x < image.getSize().x; ++x )
            {
                auto index = y * image.getSize().x + x;

                uint8_t v = uint8_t( imageVec[ index ] /scale * 255.0f );

                image.setPixel( x, y, { v, v, v, 255 } );
            }
        }

        return image;
    }
}
