#ifndef SPKN_COLOR_HPP_INCLUDED
#define SPKN_COLOR_HPP_INCLUDED

#include <SFML/Graphics.hpp>

namespace spkn
{
    struct ColorHSL
    {
        float h; // 0 to 360
        float s; // 0 to 1
        float l; // 0 to 1
    };

    ColorHSL ConvertRGBtoHSL( const sf::Color& color );
    //float ConvertRGBtoL( const sf::Color& color );

    float ConvertHSLtoSingle( const ColorHSL& color, size_t rings = 3, bool smooth = true );

    sf::Vector3f ColorToVec3f( const sf::Color& color );
    sf::Color Vec3fToColor( const sf::Vector3f& vec3f );

    sf::Image SobelEdgeDetection( const sf::Image& image );
    sf::Image LaplacianEdgeDetection( const sf::Image& image );

    void ImageLaplacianEdgeDetectionToLightness( const sf::Image& image, std::vector<double>& destination, double scale, size_t startPos );
    void ImageSobelEdgeDetectionToLightness( const sf::Image& image, std::vector<double>& destination, double scale, size_t startPos );

    sf::Image ResizeImage( const sf::Image& image, size_t width, size_t height );

    sf::Image QuarterImage( const sf::Image& image );


    void ResizeImageVec( const std::vector<double>& image, size_t width, size_t height, std::vector<double>& destination, size_t newWidth, size_t newHeight, size_t offset );
    std::vector<double> ResizeImageVec( const std::vector<double>& image, size_t width, size_t height, size_t newWidth, size_t newHeight, size_t offset );

    sf::Image ImageToGreyscale( const sf::Image& image );
    std::vector<double> ImageToSingle( const sf::Image& image, size_t rings = 3, bool smooth = true );
    void ImageToSingle( const sf::Image& image, size_t rings, bool smooth, std::vector<double>& destination, double scale, size_t startPos );

    sf::Image ImageVecToImage( const std::vector<double>& imageVec, size_t width, size_t height, float scale );
}

#endif // SPKN_COLOR_HPP_INCLUDED
