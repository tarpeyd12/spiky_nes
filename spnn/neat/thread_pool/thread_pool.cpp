#include <cassert>
#include <iostream>

#include "thread_pool.hpp"

namespace tpl
{
    pool::pool()
         : pool( std::max< size_t >( 2, std::thread::hardware_concurrency() ) - 1 )
    {
        /*  */
    }

    pool::pool( const size_t num_threads )
         : done( false ), worker_threads(), work_queue(), working_mutex(), working_condition(), max_workers_limit( num_threads ), num_working( 0 )
    {
        assert( num_threads > 0 );

        try
        {
            worker_threads.reserve( num_threads );
            while( worker_threads.size() < num_threads )
            {
                worker_threads.emplace_back( &pool::worker_method, this, worker_threads.size() );
            }
        }
        catch( ... )
        {
            destroy();
            throw;
        }
    }

    pool::~pool()
    {
        destroy();
    }

    void
    pool::limit_workers( size_t num_workers )
    {
        std::lock_guard<std::mutex> lock{ working_mutex };
        max_workers_limit = std::max<size_t>( 1, std::min<size_t>( num_workers, num_threads() ) );

        working_condition.notify_all();
    }

    void
    pool::destroy()
    {
        done = true;
        work_queue.invalidate();
        for( auto& thread : worker_threads )
        {
            if( thread.joinable() )
            {
                thread.join();
            }
        }
    }

    void
    pool::worker_method( const size_t __thread_num )
    {
        while( !done )
        {
            if( __thread_num < max_workers_limit )
            {
                std::unique_ptr< task_base > task{ nullptr };

                if( work_queue.wait_pop( task ) && task != nullptr )
                {
                    ++num_working;
                    task->execute();
                    --num_working;
                }
            }

            std::unique_lock<std::mutex> lock{ working_mutex };
            working_condition.wait( lock, [this,&__thread_num](){ return __thread_num < max_workers_limit || done; } );
        }
    }

}
