#ifndef EMULATOR_H
#define EMULATOR_H
#include <SFML/Graphics.hpp>
#include <chrono>

#include "CPU.h"
#include "PPU.h"
#include "MainBus.h"
#include "PictureBus.h"
#include "Controller.h"

namespace sn
{
    const int NESVideoWidth = ScanlineVisibleDots;
    const int NESVideoHeight = VisibleScanlines;

    class Emulator
    {
    public:
        Emulator();
        bool init(const std::string& rom_path);
        void stepFrame();
        void stepNFrames( uint64_t n );
        void run(const std::string& rom_path);

        void setVideoWidth(int width);
        void setVideoHeight(int height);
        void setVideoScale(float scale);
        void setKeys(const std::vector<sf::Keyboard::Key>& p1, const std::vector<sf::Keyboard::Key>& p2);
        void setControllerCallbacks(const std::vector<std::function<bool(void)>>& p1, const std::vector<std::function<bool(void)>>& p2);
        void setControllerCallbackMap(const std::map<Controller::Buttons,std::function<bool(void)>>& p1, const std::map<Controller::Buttons,std::function<bool(void)>>& p2);

        Byte peakMemory(Address addr) const;
        std::shared_ptr<sf::Image> getScreenData() const;
        uint64_t getNumVBlank() const;
    private:
        void DMA(Byte page);

        MainBus m_bus;
        PictureBus m_pictureBus;
        CPU m_cpu;
        PPU m_ppu;
        uint64_t m_vblankCounter;
        bool m_vblankFlag;
        Cartridge m_cartridge;
        std::unique_ptr<Mapper> m_mapper;

        Controller m_controller1, m_controller2;

        VirtualScreen m_emulatorScreen;
        float m_screenScale;

        std::chrono::nanoseconds m_cpuCycleDuration;
    };
}
#endif // EMULATOR_H
