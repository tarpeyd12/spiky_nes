#if defined( _WIN32 )
#include <windows.h>
#endif

#include "tests.hpp"

namespace _tests
{
    void
    SetProcessPriority_low()
    {
        #if defined( _WIN32 )
        SetPriorityClass( GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS );
        #endif
    }

    void
    SetProcessPriority_lowest()
    {
        #if defined( _WIN32 )
        SetPriorityClass( GetCurrentProcess(), IDLE_PRIORITY_CLASS );
        #endif
    }
}
