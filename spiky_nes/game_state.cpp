#include <cassert>

#include "game_state.hpp"

namespace spkn
{
    uint64_t
    getBCDValue( sn::Address begin, sn::Address end, const sn::Emulator& emulator, bool inclusive )
    {
        assert( begin <= end && "getBCDValue: addresses must not be reversed" );

        uint64_t value = 0;

        for( sn::Address it = begin; ( inclusive ? it <= end : it < end ); ++it )
        {
            value *= 10;
            value += emulator.peakMemory( it );
        }

        return value;
    }

    GameState_SuperMarioBros::GameState_SuperMarioBros( sn::Emulator& emu )
         : emulator( emu )
    {
        /*  */
    }

    void
    GameState_SuperMarioBros::InitGameToRunning()
    {
        // this code should only be run immediately after the ROM has been loaded
        // and should step the emulator to the first point in which the player
        // can actually control the character

        emulator.stepNFrames(30); // initial load
        // press start
        emulator.setControllerCallbackMap( { { sn::Controller::Start, []{ return true; } }, }, {} );
        emulator.stepNFrames(1); // for one frame
        emulator.setControllerCallbacks( {}, {} ); // release start

        // step until game init is done and passed the X lives left screen
        emulator.stepNFrames(7);
        while( emulator.peakMemory(0x07A0) > 1 )
        {
            emulator.stepNFrames(1);
        }
        while( emulator.peakMemory(0x07A0) != 6 )
        {
            emulator.stepNFrames(1);
        }
        //emulator.stepNFrames(153);
    }

    uint64_t
    GameState_SuperMarioBros::Score_High() const
    {
        return spkn::getBCDValue( 0x07D7, 0x07DC, emulator, true ) * 10;
    }

    uint64_t
    GameState_SuperMarioBros::Score_Mario() const
    {
        return spkn::getBCDValue( 0x07DD, 0x07E2, emulator, true ) * 10;
    }

    uint64_t
    GameState_SuperMarioBros::Score_Luigi() const
    {
        return spkn::getBCDValue( 0x07D3, 0x07D8, emulator, true ) * 10;
    }


    uint16_t
    GameState_SuperMarioBros::Lives() const
    {
        return uint16_t(uint8_t(emulator.peakMemory( 0x075A ) + 1));
    }

    uint64_t
    GameState_SuperMarioBros::Time() const
    {
        return spkn::getBCDValue( 0x07F8, 0x07FA, emulator, true );
    }


    uint16_t
    GameState_SuperMarioBros::World() const
    {
        return uint16_t(emulator.peakMemory( 0x075F )) + 1;
    }

    uint16_t
    GameState_SuperMarioBros::Level() const
    {
        return uint16_t(emulator.peakMemory( 0x0760 )) + 1;
    }

    uint16_t
    GameState_SuperMarioBros::WorldLevel() const
    {
        return World() * 10 + Level();
    }


    double
    GameState_SuperMarioBros::ScreenPosition() const
    {
        return double(emulator.peakMemory( 0x071A )) + double(emulator.peakMemory( 0x071C )) / 256.0;
    }

    uint8_t
    GameState_SuperMarioBros::ScreenNext() const
    {
        return emulator.peakMemory( 0x071B );
    }


    uint16_t
    GameState_SuperMarioBros::Coins_BCD() const
    {
        return spkn::getBCDValue( 0x07ED, 0x07EE, emulator, true );
    }

    uint8_t
    GameState_SuperMarioBros::Coins_Byte() const
    {
        return emulator.peakMemory( 0x075E );
    }

}
