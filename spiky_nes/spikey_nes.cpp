#include <algorithm>

#include "spikey_nes.hpp"

#include "../spnn/tests/tests.hpp"

namespace spkn
{
    void
    SetProcessPriority_low()
    {
        _tests::SetProcessPriority_low();
    }

    void
    SetProcessPriority_lowest()
    {
        _tests::SetProcessPriority_lowest();
    }

    void InitEmulatorLogs( sn::Level log_level )
    {
        // when multi-threading multiple sn::Emulator classes, it is HIGHLY recommended to have log_level = sn::None

        std::ofstream logFile ("simplenes.log"), cpuTraceFile;
        sn::TeeStream logTee (logFile, std::cout);

        if (logFile.is_open() && logFile.good())
            sn::Log::get().setLogStream(logTee);
        else
            sn::Log::get().setLogStream(std::cout);

        sn::Log::get().setLevel( log_level );
    }
}
