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
         : done( false ), worker_threads(), work_queue(), working_mutex(), working_condition(), max_workers_limit( 0 ), num_working( 0 ), num_executing( 0 )
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

        limit_workers( num_threads );
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

        // tell all threads that may be waiting to execute to re-evaluate if they are allowed to execute
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
    pool::worker_method( const size_t /*__thread_num*/ )
    {
        while( !done )
        {
            // block execution until the number of working threads falls under the number of threads allowed to be working
            std::unique_lock<std::mutex> lock{ working_mutex };
            working_condition.wait( lock, [this](){ return ( num_working < workers_limit() ) || done; } );

            // track the change in the number of threads working
            ++num_working;

            // release the lock so other threads can see if they are allowed to execute within the max_workers_limit limit
            lock.unlock();

            {
                std::unique_ptr< task_base > task{ nullptr };

                // block thread execution until a task is retrieved
                if( work_queue.wait_pop( task ) && task != nullptr && !done )
                {
                    ++num_executing;

                    // execute the retrieved task
                    task->execute();

                    --num_executing;
                }
            }

            // reacquire the lock to safely modify num_working and signal other threads
            lock.lock();

            // we are finished executing so we update the count of working threads
            --num_working;

            // notify the other threads that one of them may retrieve a task
            working_condition.notify_one();
        }
        // thread done
    }

}
