#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <SFML/Window.hpp>
#include <cstdint>
#include <vector>
#include <functional>

namespace sn
{
    using Byte = std::uint8_t;
    class Controller
    {
    public:
        Controller();
        enum Buttons
        {
            A,
            B,
            Select,
            Start,
            Up,
            Down,
            Left,
            Right,
            TotalButtons,
        };

        void strobe(Byte b);
        Byte read();
        void setKeyBindings(const std::vector<sf::Keyboard::Key>& keys);
        void setCallbacks(const std::vector<std::function<bool(void)>>& callbacks);
    private:
        bool m_strobe;
        unsigned int m_keyStates;

        std::vector<std::function<bool(void)>> m_buttonCallbacks;
//        std::vector<sf::Keyboard::Key> m_keyBindings;
    };
}

#endif // CONTROLLER_H
