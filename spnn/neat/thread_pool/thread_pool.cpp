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
         : done( false ), worker_threads(), work_queue()
    {
        assert( num_threads > 0 );

        try
        {
            worker_threads.reserve( num_threads );
            while( worker_threads.size() < num_threads )
            {
                worker_threads.emplace_back( &pool::worker_method, this );
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
    pool::worker_method()
    {
        while( !done )
        {
            std::unique_ptr< task_base > task{ nullptr };

            if( work_queue.wait_pop( task ) && task != nullptr )
            {
                task->execute();
            }
        }
    }

}
