#include "random_functor.hpp"

namespace Rand
{
    Random_Unsafe __random;
    std::mutex    __random_lock;
}
