#ifndef RANDOM_FUNCTOR_INL_INCLUDED
#define RANDOM_FUNCTOR_INL_INCLUDED

namespace Rand
{
    long double
    RandomFunctor::operator()( long double min, long double max )
    {
        return Float( min, max );
    }

    extern Random_Unsafe __random;
    extern std::mutex    __random_lock;

    long double
    Float( long double min, long double max )
    {
        std::lock_guard<std::mutex> lock( __random_lock );
        return __random.Float( min, max );
    }

    double
    Float( double min, double max )
    {
        std::lock_guard<std::mutex> lock( __random_lock );
        return __random.Float( min, max );
    }

    float
    Float( float min, float max )
    {
        std::lock_guard<std::mutex> lock( __random_lock );
        return __random.Float( min, max );
    }

    uintmax_t
    Int( uintmax_t min, uintmax_t max )
    {
        std::lock_guard<std::mutex> lock( __random_lock );
        return __random.Int( min, max );
    }

    inline
    void Seed( uintmax_t seed )
    {
        std::lock_guard<std::mutex> lock( __random_lock );
        __random = Random_Unsafe( seed );
    }
}

#endif // RANDOM_FUNCTOR_INL_INCLUDED
