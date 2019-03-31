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

namespace sn
{
    void parseControllerConf(std::string filepath,
                            std::vector<sf::Keyboard::Key>& p1,
                            std::vector<sf::Keyboard::Key>& p2);
}

int main(int argc, char** argv)
{
    std::ofstream logFile ("simplenes.log"), cpuTraceFile;
    sn::TeeStream logTee (logFile, std::cout);

    if (logFile.is_open() && logFile.good())
        sn::Log::get().setLogStream(logTee);
    else
        sn::Log::get().setLogStream(std::cout);

    sn::Log::get().setLevel(sn::Info);

    std::string path;

    //Default keybindings
    std::vector<sf::Keyboard::Key> p1 {sf::Keyboard::J, sf::Keyboard::K, sf::Keyboard::RShift, sf::Keyboard::Return,
                                       sf::Keyboard::W, sf::Keyboard::S, sf::Keyboard::A, sf::Keyboard::D},
                                   p2 {sf::Keyboard::Numpad5, sf::Keyboard::Numpad6, sf::Keyboard::Numpad8, sf::Keyboard::Numpad9,
                                       sf::Keyboard::Up, sf::Keyboard::Down, sf::Keyboard::Left, sf::Keyboard::Right};
    sn::Emulator emulator;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg (argv[i]);
        if (arg == "-h" || arg == "--help")
        {
            std::cout << "SimpleNES is a simple NES emulator.\n"
                      << "It can run off .nes images.\n"
                      << "Set keybindings with keybindings.conf\n\n"
                      << "Usage: SimpleNES [options] rom-path\n\n"
                      << "Options:\n"
                      << "-h, --help             Print this help text and exit\n"
                      << "-s, --scale            Set video scale. Default: 2.\n"
                      << "                       Scale of 1 corresponds to " << sn::NESVideoWidth << "x" << sn::NESVideoHeight << std::endl
                      << "-w, --width            Set the width of the emulation screen (height is\n"
                      << "                       set automatically to fit the aspect ratio)\n"
                      << "-H, --height           Set the height of the emulation screen (width is\n"
                      << "                       set automatically to fit the aspect ratio)\n"
                      << "                       This option is mutually exclusive to --width\n"
                      << std::endl;
            return 0;
        }
        else if (std::strcmp(argv[i], "--log-cpu") == 0)
        {
            sn::Log::get().setLevel(sn::CpuTrace);
            cpuTraceFile.open("sn.cpudump");
            sn::Log::get().setCpuTraceStream(cpuTraceFile);
            LOG(sn::Info) << "CPU logging set." << std::endl;
        }
        else if (std::strcmp(argv[i], "-s") == 0 || std::strcmp(argv[i], "--scale") == 0)
        {
            float scale;
            std::stringstream ss;
            if (i + 1 < argc && ss << argv[i + 1] && ss >> scale)
                emulator.setVideoScale(scale);
            else
                LOG(sn::Error) << "Setting scale from argument failed" << std::endl;
            ++i;
        }
        else if (std::strcmp(argv[i], "-w") == 0 || std::strcmp(argv[i], "--width") == 0)
        {
            int width;
            std::stringstream ss;
            if (i + 1 < argc && ss << argv[i + 1] && ss >> width)
                emulator.setVideoWidth(width);
            else
                LOG(sn::Error) << "Setting width from argument failed" << std::endl;
            ++i;
        }
        else if (std::strcmp(argv[i], "-H") == 0 || std::strcmp(argv[i], "--height") == 0)
        {
            int height;
            std::stringstream ss;
            if (i + 1 < argc && ss << argv[i + 1] && ss >> height)
                emulator.setVideoHeight(height);
            else
                LOG(sn::Error) << "Setting height from argument failed" << std::endl;
            ++i;
        }
        else if (argv[i][0] != '-')
            path = argv[i];
        else
            std::cerr << "Unrecognized argument: " << argv[i] << std::endl;
    }

    if (path.empty())
    {
        std::cout << "Argument required: ROM path" << std::endl;
        return 1;
    }

    auto th = std::async( std::launch::async, [&]
    {
        sn::parseControllerConf("keybindings.conf", p1, p2);
        emulator.setKeys(p1, p2);

        auto _up =    [](unsigned int j){ return sf::Joystick::hasAxis(j,sf::Joystick::Axis::Y) && sf::Joystick::getAxisPosition(j,sf::Joystick::Axis::Y) < -80.0; };
        auto _down =  [](unsigned int j){ return sf::Joystick::hasAxis(j,sf::Joystick::Axis::Y) && sf::Joystick::getAxisPosition(j,sf::Joystick::Axis::Y) > 80.0; };
        auto _left =  [](unsigned int j){ return sf::Joystick::hasAxis(j,sf::Joystick::Axis::X) && sf::Joystick::getAxisPosition(j,sf::Joystick::Axis::X) < -80.0; };
        auto _right = [](unsigned int j){ return sf::Joystick::hasAxis(j,sf::Joystick::Axis::X) && sf::Joystick::getAxisPosition(j,sf::Joystick::Axis::X) > 80.0; };

        emulator.setControllerCallbackMap(
        {
            { sn::Controller::A,      [&]{ return sf::Joystick::isButtonPressed(0, 1) || sf::Keyboard::isKeyPressed(sf::Keyboard::J); } },
            { sn::Controller::B,      [&]{ return sf::Joystick::isButtonPressed(0, 0) || sf::Keyboard::isKeyPressed(sf::Keyboard::K); } },
            { sn::Controller::Select, [&]{ return sf::Joystick::isButtonPressed(0, 6) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift); } },
            { sn::Controller::Start,  [&]{ return sf::Joystick::isButtonPressed(0, 7) || sf::Keyboard::isKeyPressed(sf::Keyboard::Return); } },
            { sn::Controller::Up,     [&]{ return _up(0)    || sf::Keyboard::isKeyPressed(sf::Keyboard::W); } },
            { sn::Controller::Down,   [&]{ return _down(0)  || sf::Keyboard::isKeyPressed(sf::Keyboard::S); } },
            { sn::Controller::Left,   [&]{ return _left(0)  || sf::Keyboard::isKeyPressed(sf::Keyboard::A); } },
            { sn::Controller::Right,  [&]{ return _right(0) || sf::Keyboard::isKeyPressed(sf::Keyboard::D); } },
        }, {} );

        emulator.run(path);
    });

    auto th2 = std::async( std::launch::async, [&]
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        std::cout << "\n\n";
        while(th.valid())
        {
            std::cout << "High = ";
            std::cout << int(emulator.peakMemory( 0x07D7 ));
            std::cout << int(emulator.peakMemory( 0x07D8 ));
            std::cout << int(emulator.peakMemory( 0x07D9 ));
            std::cout << int(emulator.peakMemory( 0x07DA ));
            std::cout << int(emulator.peakMemory( 0x07DB ));
            std::cout << int(emulator.peakMemory( 0x07DC ));
            std::cout << "0 Mario = ";
            std::cout << int(emulator.peakMemory( 0x07DD ));
            std::cout << int(emulator.peakMemory( 0x07DE ));
            std::cout << int(emulator.peakMemory( 0x07DF ));
            std::cout << int(emulator.peakMemory( 0x07E0 ));
            std::cout << int(emulator.peakMemory( 0x07E1 ));
            std::cout << int(emulator.peakMemory( 0x07E2 ));
            std::cout << "0 Luigi = ";
            std::cout << int(emulator.peakMemory( 0x07D3 ));
            std::cout << int(emulator.peakMemory( 0x07D4 ));
            std::cout << int(emulator.peakMemory( 0x07D5 ));
            std::cout << int(emulator.peakMemory( 0x07E6 ));
            std::cout << int(emulator.peakMemory( 0x07E7 ));
            std::cout << int(emulator.peakMemory( 0x07E8 ));
            std::cout << "0 coins = ";
            std::cout << int(emulator.peakMemory( 0x07ED ));
            std::cout << int(emulator.peakMemory( 0x07EE ));
            std::cout << "(" << int(emulator.peakMemory( 0x075E )) << ")";
            std::cout << " time = ";
            std::cout << int(emulator.peakMemory( 0x07F8 ));
            std::cout << int(emulator.peakMemory( 0x07F9 ));
            std::cout << int(emulator.peakMemory( 0x07FA ));
            std::cout << " World-Level = ";
            std::cout << int(emulator.peakMemory( 0x075F ));
            std::cout << "-";
            std::cout << int(emulator.peakMemory( 0x0760 ));
            std::cout << " Lives = ";
            std::cout << int(emulator.peakMemory( 0x075A ));
            std::cout << " screen = ";
            std::cout << int(emulator.peakMemory( 0x071A )) << ",";
            std::cout << int(emulator.peakMemory( 0x071B )) << ",";
            std::cout << int(emulator.peakMemory( 0x071C ));

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
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            system("cls");
        }

        std::cout << "\n" << std::endl;
    });

    //_tests::Test7();

    th.get();
    th2.get();

    return 0;
}
