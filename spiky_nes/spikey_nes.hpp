#ifndef SPKN_SPIKEY_NES_HPP_INCLUDED
#define SPKN_SPIKEY_NES_HPP_INCLUDED

#include <SFML/Graphics.hpp>

#include "../simple_nes/include/Log.h"

namespace spkn
{
    void SetProcessPriority_low();
    void SetProcessPriority_lowest();

    void InitEmulatorLogs( sn::Level log_level = sn::None );
}

#include "color.hpp"
#include "fitness.hpp"
#include "game_state.hpp"

#endif // SPKN_SPIKEY_NES_HPP_INCLUDED
