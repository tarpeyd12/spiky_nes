#ifndef RANDOM_FUNCTOR_HPP_INCLUDED
#define RANDOM_FUNCTOR_HPP_INCLUDED

#include <algorithm>
#include <random>
#include <mutex>

namespace Rand
{
    class RandomFunctor
    {
        public:
            virtual ~RandomFunctor() = default;

            virtual long double Float( long double min = 0.0, long double max = 1.0 ) = 0; // [min,max)
            virtual uintmax_t Int( uintmax_t min = 0, uintmax_t max = ~1 ) = 0; // [min,max]

            inline long double operator()( long double min = -1.0, long double max = 1.0 );
    };

    inline long double Float( long double min = 0.0, long double max = 1.0 );
    inline double Float( double min = 0.0, double max = 1.0 );
    inline float Float( float min = 0.0, float max = 1.0 );

    inline uintmax_t Int( uintmax_t min = 0, uintmax_t max = ~1 );

    inline void Seed( uintmax_t seed );

    // TODO(dot##9/17/2018): template the generator type
    class Random_Safe : public RandomFunctor
    {
        private:
            std::mutex randLock;

            //std::mt19937 generator;
            std::mt19937_64 generator;

        public:
            Random_Safe()
            : randLock(), generator()
            {
                std::lock_guard<std::mutex> lock( randLock );
                //std::random_device rd;
                //generator.seed( rd() );
                generator.seed( time(nullptr) ); // quick and dirty seed, highly portable
            }

            Random_Safe( uintmax_t _seed )
            : randLock(), generator( _seed )
            {

            }

            virtual ~Random_Safe() = default;

            inline
            long double
            Float( long double min = 0.0, long double max = 1.0 ) override
            {
                std::lock_guard<std::mutex> lock( randLock );
                return std::uniform_real_distribution<long double>( min, max )( generator );
            }

            inline
            uintmax_t
            Int( uintmax_t min = 0, uintmax_t max = ~1 ) override
            {
                std::lock_guard<std::mutex> lock( randLock );
                return std::uniform_int_distribution<uintmax_t>( min, max )( generator );
            }
    };

    class Random_Unsafe : public RandomFunctor
    {
        private:
            //std::mt19937 generator;
            std::mt19937_64 generator;

        public:
            Random_Unsafe()
            : generator()
            {
                //std::random_device rd;
                //generator.seed( rd() );
                generator.seed( time(nullptr) ); // quick and dirty seed, highly portable
            }

            Random_Unsafe( uintmax_t _seed )
            : generator( _seed )
            {

            }

            virtual ~Random_Unsafe() = default;

            inline
            long double
            Float( long double min = 0.0, long double max = 1.0 ) override
            {
                return std::uniform_real_distribution<long double>( min, max )( generator );
            }

            inline
            uintmax_t
            Int( uintmax_t min = 1, uintmax_t max = ~1 ) override
            {
                return std::uniform_int_distribution<uintmax_t>( min, max )( generator );
            }
    };
}

#include "random_functor.inl"

#endif // RANDOM_FUNCTOR_HPP_INCLUDED
