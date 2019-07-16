#ifndef POOL_FUTURE_HPP_INCLUDED
#define POOL_FUTURE_HPP_INCLUDED

#include <future>
#include <atomic>

namespace tpl
{
    template < typename T >
    class future
    {
        private:

            std::future< T > _future;
            bool block_on_destruction;

        public:

            future();
            future( std::future< T >&& f );
            ~future();

            // no copy
            future( const future& ) = delete;
            future& operator=( const future& ) = delete;

            // move OK
            future( future&& ) = default;
            future& operator=( future&& ) = default;

            T get();
            void release();
            bool valid() const;
            void wait() const;
    };
}

#include "future.inl"

#endif // POOL_FUTURE_HPP_INCLUDED
