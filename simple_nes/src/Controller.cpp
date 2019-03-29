#include "Controller.h"

namespace sn
{
    Controller::Controller() :
        m_keyStates(0),
        m_buttonCallbacks(TotalButtons)
    {
//         m_keyBindings[A] = sf::Keyboard::J;
//         m_keyBindings[B] = sf::Keyboard::K;
//         m_keyBindings[Select] = sf::Keyboard::RShift;
//         m_keyBindings[Start] = sf::Keyboard::Return;
//         m_keyBindings[Up] = sf::Keyboard::W;
//         m_keyBindings[Down] = sf::Keyboard::S;
//         m_keyBindings[Left] = sf::Keyboard::A;
//         m_keyBindings[Right] = sf::Keyboard::D;
    }

    void Controller::setKeyBindings(const std::vector<sf::Keyboard::Key>& keys)
    {
        //m_keyBindings = keys;

        for (int button = A; button < TotalButtons; ++button)
        {
            m_buttonCallbacks[ button ] = [=](void)
            {
                return sf::Keyboard::isKeyPressed(keys[static_cast<Buttons>(button)]);
            };
        }
    }

    void Controller::setCallbacks(const std::vector<std::function<bool(void)>>& callbacks)
    {
        m_buttonCallbacks = callbacks;
    }

    void Controller::strobe(Byte b)
    {
        m_strobe = (b & 1);
        if (!m_strobe)
        {
            m_keyStates = 0;
            int shift = 0;
            for (int button = A; button < TotalButtons; ++button)
            {
                m_keyStates |= (m_buttonCallbacks[static_cast<Buttons>(button)]() << shift);
                ++shift;
            }
        }
    }

    Byte Controller::read()
    {
        Byte ret;
        if (m_strobe)
            ret = m_buttonCallbacks[A]();
        else
        {
            ret = (m_keyStates & 1);
            m_keyStates >>= 1;
        }
        return ret | 0x40;
    }

}
