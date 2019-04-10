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

        std::shared_ptr<sf::Image> getScreenData() const;
        std::shared_ptr<sf::Image> flushScreenData();

        void setScreenPosition( sf::Vector2f pos );

        void setScreenData( std::shared_ptr<sf::Image> data );

        sf::Vector2u screenSize() const;

    private:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

        std::shared_ptr<sf::Image> m_pixelColors; // turn this into a std::shared_ptr<sf::Image> it *should* be simpler for other stuff
        mutable sf::Texture m_texture;
        mutable sf::Sprite m_sprite;
        sf::Color m_defaultColor;
        sf::Vector2u m_screenSize;
    };
}
#endif // VIRTUALSCREEN_H
