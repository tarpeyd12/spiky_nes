#ifndef POOL_SAFE_QUEUE_INL_INCLUDED
#define POOL_SAFE_QUEUE_INL_INCLUDED

#include <utility>

namespace tpl
{
    template < typename T >
    safe_queue< T >::safe_queue()
         : queue(), queue_mutex(), condition(), valid( true )
    {
        /*  */
    }

    template < typename T >
    safe_queue< T >::~safe_queue()
    {
        // invalidate the queue, which halts all pushes and pops
        invalidate();
    }

    template < typename T >
    void
    safe_queue< T >::push( T value )
    {
        // we safely push the value

        std::lock_guard<std::mutex> lock{ queue_mutex };

        queue.push( std::move( value ) );

        // notify only one waiting pop
        condition.notify_one();
    }

    template < typename T >
    bool
    safe_queue< T >::try_pop( T& out )
    {
        // safely pop one value, if there is a value to pop

        std::lock_guard<std::mutex> lock{ queue_mutex };

        if( queue.empty() || !valid )
        {
            return false;
        }

        out = std::move( queue.front() );
        queue.pop();

        return true;
    }

    template < typename T >
    bool
    safe_queue< T >::wait_pop( T& out )
    {
        // safely pop a value if it exists, otherwise wait for a value to pop

        std::unique_lock<std::mutex> lock{ queue_mutex };

        condition.wait( lock, [this](){ return !queue.empty() || !valid; } );

        if( queue.empty() || !valid )
        {
            return false;
        }

        out = std::move( queue.front() );
        queue.pop();

        return true;
    }

    template < typename T >
    bool
    safe_queue< T >::empty() const
    {
        std::lock_guard<std::mutex> lock{ queue_mutex };

        return queue.empty();
    }

    template < typename T >
    void
    safe_queue< T >::clear()
    {
        // safely clear all values in the queue

        std::lock_guard<std::mutex> lock{ queue_mutex };

        while( !queue.empty() )
        {
            queue.pop();
        }

        condition.notify_all();
    }

    template < typename T >
    size_t
    safe_queue< T >::size() const
    {
        std::lock_guard<std::mutex> lock{ queue_mutex };

        return queue.size();
    }

    template < typename T >
    void
    safe_queue< T >::invalidate()
    {
        // invalidate the whole queue, wakes up all waiting pops

        std::lock_guard<std::mutex> lock{ queue_mutex };

        valid = false;

        condition.notify_all();
    }

    template < typename T >
    bool
    safe_queue< T >::is_valid() const
    {
        std::lock_guard<std::mutex> lock{ queue_mutex };

        return valid;
    }

}

#endif // POOL_SAFE_QUEUE_INL_INCLUDED
