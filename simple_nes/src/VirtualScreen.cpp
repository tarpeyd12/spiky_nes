#include "VirtualScreen.h"

namespace sn
{
    void VirtualScreen::create(unsigned int w, unsigned int h, float pixel_size, sf::Color color)
    {
        m_defaultColor = color;
        m_pixelSize = pixel_size;
        m_screenSize = {w, h};

        flushScreenData();
    }

    void VirtualScreen::setPixel(std::size_t x, std::size_t y, sf::Color color)
    {
        auto index = y * screenSize().x + x;

        if (index >= m_pixelColors->size())
            return;

        (*m_pixelColors)[index] = color;
    }

    std::shared_ptr<std::vector<sf::Color>> VirtualScreen::getScreenData() const
    {
        return m_pixelColors;
    }

    std::shared_ptr<std::vector<sf::Color>> VirtualScreen::flushScreenData()
    {
        auto currentScreenData = getScreenData();
        m_pixelColors = std::make_shared<std::vector<sf::Color>>( screenSize().x * screenSize().y, m_defaultColor );

        return currentScreenData;
    }

    void VirtualScreen::setScreenPosition( sf::Vector2f pos )
    {
        m_screenPosition = pos;
    }

    void VirtualScreen::setScreenData( std::shared_ptr<std::vector<sf::Color>> data )
    {
        m_pixelColors = data;
    }

    sf::Vector2u VirtualScreen::screenSize() const
    {
        return m_screenSize;
    }

    void VirtualScreen::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        sf::VertexArray vertices( sf::Quads, screenSize().x * screenSize().y * 4 );

        for(size_t y = 0; y < screenSize().y; ++y)
        {
            for(size_t x = 0; x < screenSize().x; ++x)
            {
                auto index = y * screenSize().x + x;

                auto color = (*m_pixelColors)[index];

                index *= 4;

                sf::Vector2f coord2d( x * m_pixelSize, y * m_pixelSize );
                coord2d += m_screenPosition;

                //quad
                //top-left
                vertices[index].position     = coord2d;
                vertices[index].color        = color;

                //top-right
                vertices[index + 1].position = coord2d + sf::Vector2f{ m_pixelSize, 0 };
                vertices[index + 1].color    = color;

                //bottom-right
                vertices[index + 2].position = coord2d + sf::Vector2f{ m_pixelSize, m_pixelSize };
                vertices[index + 2].color    = color;

                //bottom-left
                vertices[index + 3].position = coord2d + sf::Vector2f{ 0, m_pixelSize };
                vertices[index + 3].color    = color;

            }
        }

        target.draw(vertices, states);
    }
}

