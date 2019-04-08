/*#include <iostream>

using namespace std;

int main()
{
    cout << "Hello world!" << endl;
    return 0;
}
*/


#include "../spnn/tests/tests.hpp"


#include "../simple_nes/include/Emulator.h"
#include "../simple_nes/include/Log.h"
#include <string>
#include <sstream>
#include <future>

#include "spikey_nes.hpp"

namespace sn
{
    void parseControllerConf(std::string filepath,
                            std::vector<sf::Keyboard::Key>& p1,
                            std::vector<sf::Keyboard::Key>& p2);
}




int main(int argc, char** argv)
{
    spkn::InitEmulatorLogs();

    std::string path;

    //Default keybindings
    std::vector<sf::Keyboard::Key> p1 {sf::Keyboard::J, sf::Keyboard::K, sf::Keyboard::RShift, sf::Keyboard::Return,
                                       sf::Keyboard::W, sf::Keyboard::S, sf::Keyboard::A, sf::Keyboard::D},
                                   p2 {sf::Keyboard::Numpad5, sf::Keyboard::Numpad6, sf::Keyboard::Numpad8, sf::Keyboard::Numpad9,
                                       sf::Keyboard::Up, sf::Keyboard::Down, sf::Keyboard::Left, sf::Keyboard::Right};
    sn::Emulator emulator;
    sn::Emulator emulator2;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg (argv[i]);
        if (argv[i][0] != '-')
            path = argv[i];
        else
            std::cerr << "Unrecognized argument: " << argv[i] << std::endl;
    }

    if (path.empty())
    {
        std::cout << "Argument required: ROM path" << std::endl;
        return 1;
    }

    auto _up =    [](unsigned int j){ return sf::Joystick::hasAxis(j,sf::Joystick::Axis::Y) && sf::Joystick::getAxisPosition(j,sf::Joystick::Axis::Y) < -80.0; };
    auto _down =  [](unsigned int j){ return sf::Joystick::hasAxis(j,sf::Joystick::Axis::Y) && sf::Joystick::getAxisPosition(j,sf::Joystick::Axis::Y) > 80.0; };
    auto _left =  [](unsigned int j){ return sf::Joystick::hasAxis(j,sf::Joystick::Axis::X) && sf::Joystick::getAxisPosition(j,sf::Joystick::Axis::X) < -80.0; };
    auto _right = [](unsigned int j){ return sf::Joystick::hasAxis(j,sf::Joystick::Axis::X) && sf::Joystick::getAxisPosition(j,sf::Joystick::Axis::X) > 80.0; };

    std::map<sn::Controller::Buttons,std::function<bool(void)>> controller_map = {
        { sn::Controller::A,      [&]{ return sf::Joystick::isButtonPressed(0, 1) || sf::Joystick::isButtonPressed(0, 2) || sf::Keyboard::isKeyPressed(sf::Keyboard::J); } },
        { sn::Controller::B,      [&]{ return sf::Joystick::isButtonPressed(0, 0) || sf::Joystick::isButtonPressed(0, 3) || sf::Keyboard::isKeyPressed(sf::Keyboard::K); } },
        { sn::Controller::Select, [&]{ return sf::Joystick::isButtonPressed(0, 6) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift); } },
        { sn::Controller::Start,  [&]{ return sf::Joystick::isButtonPressed(0, 7) || sf::Keyboard::isKeyPressed(sf::Keyboard::Return); } },
        { sn::Controller::Up,     [&]{ return _up(0)    || sf::Keyboard::isKeyPressed(sf::Keyboard::W); } },
        { sn::Controller::Down,   [&]{ return _down(0)  || sf::Keyboard::isKeyPressed(sf::Keyboard::S); } },
        { sn::Controller::Left,   [&]{ return _left(0)  || sf::Keyboard::isKeyPressed(sf::Keyboard::A); } },
        { sn::Controller::Right,  [&]{ return _right(0) || sf::Keyboard::isKeyPressed(sf::Keyboard::D); } },
    };

    auto th = std::async( std::launch::async, [&]
    {
        sn::parseControllerConf("keybindings.conf", p1, p2);
        emulator.setKeys(p1, p2);
        //emulator2.setKeys( p1,p2 );

        //emulator.setControllerCallbackMap( controller_map, {} );
        //emulator2.setControllerCallbackMap( controller_map, {} );

        //emulator.run(path);

        {
            sn::VirtualScreen screen;
            sn::VirtualScreen screen2;
            sn::VirtualScreen screen3;
            screen.create(sn::NESVideoWidth,sn::NESVideoHeight,2.f,sf::Color::Magenta);
            screen2.create(sn::NESVideoWidth/2,sn::NESVideoHeight/2,2.f,sf::Color::Magenta);
            screen3.create(sn::NESVideoWidth/2,sn::NESVideoHeight/2,2.f,sf::Color::Magenta);
            screen2.setScreenPosition( { sn::NESVideoWidth*2.f+4.f,0.f } );
            screen3.setScreenPosition( { sn::NESVideoWidth*2.f+4.f,sn::NESVideoHeight } );

            emulator.init( path );
            //emulator2.init( path );

            screen.setScreenData( emulator.getScreenData() );
            //screen2.setScreenData( emulator2.getScreenData() );

            sf::RenderWindow window;
            window.create( sf::VideoMode((sn::NESVideoWidth*2.f+2.f)*2.f, sn::NESVideoHeight*2.f), "Spikey NES", sf::Style::Titlebar|sf::Style::Close );
            window.setVerticalSyncEnabled(true);

            bool run = true;

            auto emu_steps1 = std::async( std::launch::async, [&]
            {
                //emulator.stepNFrames(50);

                spkn::GameState_SuperMarioBros(emulator).InitGameToRunning();

                std::this_thread::sleep_for(std::chrono::milliseconds(2000));



                emulator.setControllerCallbackMap( controller_map, {} );

                auto frameTimer = std::chrono::high_resolution_clock::now();
                auto elapsed_time = std::chrono::duration<double>( frameTimer - frameTimer );

                while(run)
                {
                    auto currentTime = std::chrono::high_resolution_clock::now();
                    elapsed_time += currentTime - frameTimer;
                    frameTimer = currentTime;

                    while( run && elapsed_time > std::chrono::duration<double>( 1.0/60.0 ) )
                    {
                        emulator.stepNFrames(1);

                        elapsed_time -= std::chrono::duration<double>( 1.0/60.0 );
                    }

                    std::this_thread::sleep_for(std::chrono::nanoseconds(500));

                }
            } );

            auto emu_steps2 = std::async( std::launch::async, [&]
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));

                while(run)
                {
                    //emulator2.stepNFrames(1);
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));

                    for(size_t y = 0; y < screen.screenSize().y; ++y)
                    {
                        for(size_t x = 0; x < screen.screenSize().x; ++x)
                        {
                            auto index = y * screen.screenSize().x + x;
                            auto index2 = y/2 * screen2.screenSize().x + x/2;

                            if( !(x%2) && !(y%2) )
                            {
                                (*screen2.getScreenData())[ index2 ] = sf::Color::Black;
                            }

                            auto c = (*screen.getScreenData())[ index ];

                            c.r /= 4;
                            c.g /= 4;
                            c.b /= 4;

                            (*screen2.getScreenData())[ index2 ] +=  c;

                            if( (x%2) && (y%2) )
                            {
                                auto clr = (*screen2.getScreenData())[ index2 ];
                                auto hsl = spkn::ConvertRGBtoHSL( clr );
                                float v = spkn::ConvertHSLtoSingle( hsl, 5, true );
                                //float v = spkn::ConvertRGBtoHSL( (*screen2.getScreenData())[ index2 ] ).l;
                                uint8_t b = ( v )*255.0f;
                                (*screen3.getScreenData())[ index2 ] = sf::Color{ b, b, b, 255 };
                            }
                        }
                    }
                }
            } );

            while (window.isOpen())
            {
                sf::Event event;
                while (window.pollEvent(event))
                {
                    if (event.type == sf::Event::Closed )
                    {
                        run = false;
                        window.close();
                        emu_steps1.get();
                        emu_steps2.get();
                        return;
                    }
                }

                /*emulator.stepNFrames(1);
                emulator2.stepNFrames(2);*/

                window.draw(screen);
                window.draw(screen2);
                window.draw(screen3);
                window.display();
            }
            emu_steps1.get();
            emu_steps2.get();
        }
    });

    auto th2 = std::async( std::launch::async, [&]
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));

        auto start_time = std::chrono::high_resolution_clock::now();
        auto prev_time = start_time;

        uint64_t prev_numFrames = 0;

        std::cout << "\n\n";
        while(th.valid())
        {
            system("cls");

            auto current_time = std::chrono::high_resolution_clock::now();

            uint64_t numFrames = emulator.getNumVBlank() + emulator2.getNumVBlank();

            double dt = std::chrono::duration<double>( current_time - prev_time ).count();
            uint64_t dframes = numFrames - prev_numFrames;

            std::cout << "numVBlanks = " << numFrames << " ";
            std::cout << "(" << double(dframes)/dt << "/s)";

            std::cout << "\n";

            spkn::GameState_SuperMarioBros state( emulator );

            std::cout << "High = ";
            std::cout << state.Score_High();
            std::cout << " Mario = ";
            std::cout << state.Score_Mario();
            std::cout << " Luigi = ";
            std::cout << state.Score_Luigi();
            std::cout << " coins = ";
            std::cout << state.Coins_BCD();
            std::cout << "(" << int(state.Coins_Byte()) << ")";
            std::cout << " time = ";
            std::cout << state.Time();
            std::cout << " World-Level = ";
            std::cout << state.World();
            std::cout << "-";
            std::cout << state.Level();
            std::cout << " Lives = ";
            std::cout << state.Lives();
            std::cout << " screen = ";
            std::cout << state.ScreenPosition() << "," << int(emulator.peakMemory(0x07A0));

            std::cout << "         \n";

            std::cout << " " << std::string(sf::Joystick::getIdentification(0).name);
            std::cout << " " << sf::Joystick::getButtonCount(0) << "\n\t";

            for( size_t i = 0; i < sf::Joystick::getButtonCount(0); ++i )
            {
                std::cout << " [" << i << ":" << sf::Joystick::isButtonPressed(0, i) << "], ";
            }

            std::cout << "\n\t";

            std::cout << "[X:" << sf::Joystick::hasAxis(0,sf::Joystick::Axis::X) << ":" << sf::Joystick::getAxisPosition(0,sf::Joystick::Axis::X) << "], ";
            std::cout << "[Y:" << sf::Joystick::hasAxis(0,sf::Joystick::Axis::Y) << ":" << sf::Joystick::getAxisPosition(0,sf::Joystick::Axis::Y) << "], ";

            std::cout << "\n\n";

            std::cout << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(25));

            prev_time = current_time;
            prev_numFrames = numFrames;
        }

        std::cout << "\n" << std::endl;
    });

    //_tests::Test7();

    th.get();
    th2.get();

    return 0;
}
