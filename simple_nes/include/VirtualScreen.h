#ifndef VIRTUALSCREEN_H
#define VIRTUALSCREEN_H
#include <memory>
#include <SFML/Graphics.hpp>

namespace sn
{
    class VirtualScreen : public sf::Drawable
    {
    public:
        void create (unsigned int width = 256, unsigned int height = 240, float pixel_size = 2.f, sf::Color color = sf::Color::Magenta);
        void setPixel (std::size_t x, std::size_t y, sf::Color color);

        std::shared_ptr<std::vector<sf::Color>> getScreenData() const;
        std::shared_ptr<std::vector<sf::Color>> flushScreenData();

        void setScreenPosition( sf::Vector2f pos );

        void setScreenData( std::shared_ptr<std::vector<sf::Color>> data );

        sf::Vector2u screenSize() const;

    private:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

        std::shared_ptr<std::vector<sf::Color>> m_pixelColors;
        sf::Color m_defaultColor;
        sf::Vector2u m_screenSize;
        sf::Vector2f m_screenPosition; // offset from {0,0} in real pixels
        float m_pixelSize; //virtual pixel size in real pixels
    };
}
#endif // VIRTUALSCREEN_H
