#include <cmath>

#include "VirtualScreen.h"

namespace sn
{
    void VirtualScreen::create(unsigned int w, unsigned int h, float pixel_size, sf::Color color)
    {
        m_defaultColor = color;
        m_screenSize = {w, h};

        //m_sprite.setScale( m_screenSize.x * pixel_size, m_screenSize.y * pixel_size );
        m_sprite.setScale( pixel_size, pixel_size );

        m_texture.create( m_screenSize.x, m_screenSize.y );

        if( fmod( pixel_size, 1.0 ) != 0.0 )
        {
            m_texture.setSmooth( true );
        }

        flushScreenData();
    }

    void VirtualScreen::setPixel(std::size_t x, std::size_t y, sf::Color color)
    {
        if (x > m_pixelColors->getSize().x || y > m_pixelColors->getSize().y)
            return;

        m_pixelColors->setPixel( x, y, color );
    }

    std::shared_ptr<sf::Image> VirtualScreen::getScreenData() const
    {
        return m_pixelColors;
    }

    std::shared_ptr<sf::Image> VirtualScreen::flushScreenData()
    {
        auto currentScreenData = getScreenData();
        m_pixelColors = std::make_shared<sf::Image>();
        m_pixelColors->create( m_screenSize.x, m_screenSize.y, m_defaultColor );
        return currentScreenData;
    }

    void VirtualScreen::setScreenPosition( sf::Vector2f pos )
    {
        m_sprite.setPosition( pos );
    }

    void VirtualScreen::setScreenData( std::shared_ptr<sf::Image> data )
    {
        m_pixelColors = data;
        m_texture.update( *m_pixelColors );
    }

    sf::Vector2u VirtualScreen::screenSize() const
    {
        return m_screenSize;
    }

    void VirtualScreen::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        m_texture.update( *getScreenData() );
        m_sprite.setTexture( m_texture );
        target.draw( m_sprite, states );
    }
}

