#include "Controller.h"

namespace sn
{
    Controller::Controller() :
        m_buttonStates(0),
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

        for (size_t button = A; button < TotalButtons; ++button)
        {
            if( button < keys.size() )
            {
                m_buttonCallbacks[ button ] = [=](void) -> bool
                {
                    return sf::Keyboard::isKeyPressed(keys[button]);
                };
            }
            else
            {
                m_buttonCallbacks[ button ] = [](void) -> bool { return false; };
            }

        }
    }

    void Controller::setCallbacks(const std::vector<std::function<bool(void)>>& callbacks)
    {
        m_buttonCallbacks = callbacks;

        for( auto& callback : m_buttonCallbacks )
        {
            if( !callback )
            {
                callback = [](void) -> bool { return false; };
            }
        }

        while( m_buttonCallbacks.size() < TotalButtons )
        {
            m_buttonCallbacks.emplace_back( [](void) -> bool { return false; } );
        }
    }

    void Controller::setCallbackMap(const std::map<Buttons,std::function<bool(void)>>& callbacks)
    {
        for (size_t button = A; button < TotalButtons; ++button)
        {
            auto it = callbacks.find( static_cast<Buttons>(button) );
            if( it != callbacks.end() && it->second )
            {
                m_buttonCallbacks[ button ] = it->second;
            }
        }
    }

    void Controller::strobe(Byte b)
    {
        m_strobe = (b & 1);
        if (!m_strobe)
        {
            m_buttonStates = 0;
            int shift = 0;
            for( size_t button = A; button < TotalButtons; ++button )
            {
                m_buttonStates |= (m_buttonCallbacks[ button ]() << shift);
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
            ret = (m_buttonStates & 1);
            m_buttonStates >>= 1;
        }
        return ret | 0x40;
    }

}
