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
        auto index = x * m_screenSize.y + y;

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
        m_pixelColors = std::make_shared<std::vector<sf::Color>>();
        m_pixelColors->resize(m_screenSize.x * m_screenSize.y);
        for( auto& c : *m_pixelColors )
        {
            c = m_defaultColor;
        }

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

    void VirtualScreen::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        sf::VertexArray vertices;
        {
            vertices.resize(m_screenSize.x * m_screenSize.y * 6);
            vertices.setPrimitiveType(sf::Triangles);

            for (std::size_t x = 0; x < m_screenSize.x; ++x)
            {
                for (std::size_t y = 0; y < m_screenSize.y; ++y)
                {
                    auto index = x * m_screenSize.y + y;

                    auto color = (*m_pixelColors)[index];

                    index *= 6;

                    sf::Vector2f coord2d (x * m_pixelSize, y * m_pixelSize);
                    coord2d += m_screenPosition;

                    //Triangle-1
                    //top-left
                    vertices[index].position = coord2d;
                    vertices[index].color    = color;

                    //top-right
                    vertices[index + 1].position = coord2d + sf::Vector2f{m_pixelSize, 0};
                    vertices[index + 1].color = color;

                    //bottom-right
                    vertices[index + 2].position = coord2d + sf::Vector2f{m_pixelSize, m_pixelSize};
                    vertices[index + 2].color = color;

                    //Triangle-2
                    //bottom-right
                    vertices[index + 3].position = coord2d + sf::Vector2f{m_pixelSize, m_pixelSize};
                    vertices[index + 3].color = color;

                    //bottom-left
                    vertices[index + 4].position = coord2d + sf::Vector2f{0, m_pixelSize};
                    vertices[index + 4].color = color;

                    //top-left
                    vertices[index + 5].position = coord2d;
                    vertices[index + 5].color    = color;
                }
            }
        }

        target.draw(vertices, states);
    }
}

