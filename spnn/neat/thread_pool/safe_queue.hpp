#ifndef POOL_SAFE_QUEUE_HPP_INCLUDED
#define POOL_SAFE_QUEUE_HPP_INCLUDED

#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>


namespace tpl
{
    template < typename T >
    class safe_queue
    {
        private:

            std::queue< T > queue;

            mutable std::mutex queue_mutex;
            std::condition_variable condition;
            std::atomic_bool valid{ true };

        public:

            safe_queue();
            ~safe_queue();

            void push( T value );

            bool try_pop( T& out );
            bool wait_pop( T& out );

            bool empty() const;
            void clear();
            size_t size() const;

            void invalidate();
            bool is_valid() const;
    };
}

#include "safe_queue.inl"

#endif // POOL_SAFE_QUEUE_HPP_INCLUDED
