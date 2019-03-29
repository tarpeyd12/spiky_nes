#include <windows.h>

#include "tests.hpp"

namespace _tests
{
    void
    SetProcessPriority_low()
    {
        SetPriorityClass( GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS );
    }

    void
    SetProcessPriority_lowest()
    {
        SetPriorityClass( GetCurrentProcess(), IDLE_PRIORITY_CLASS );
    }
}
