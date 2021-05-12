#ifndef SPKN_GAME_STATE_HPP_INCLUDED
#define SPKN_GAME_STATE_HPP_INCLUDED

#include "../simple_nes/include/Emulator.h"

namespace spkn
{
    uint64_t getBCDValue( sn::Address begin, sn::Address end, const sn::Emulator& emulator, bool inclusive = false );

    class GameState_SuperMarioBros
    {
        private:

            sn::Emulator& emulator;

        public:

            GameState_SuperMarioBros( sn::Emulator& emu );

            void InitGameToRunning();

            uint64_t Score_High() const;
            uint64_t Score_Mario() const;
            uint64_t Score_Luigi() const;

            uint16_t Lives() const;
            uint64_t Time() const;

            uint16_t PowerupState() const;

            uint16_t World() const;
            uint16_t Level() const;
            uint16_t WorldLevel() const;

            double   ScreenPosition() const;
            uint8_t  ScreenNext() const;

            uint16_t Coins_BCD() const;
            uint8_t  Coins_Byte() const;
    };


}

#endif // SPKN_GAME_STATE_HPP_INCLUDED
