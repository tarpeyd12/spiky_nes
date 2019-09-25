#ifndef POOL_THREAD_POOL_INL_INCLUDED
#define POOL_THREAD_POOL_INL_INCLUDED

#include <functional>
#include <future>
#include <list>

#include "future.hpp"

namespace tpl
{
    template < typename FuncType, typename ... ArgsType >
    decltype(auto)
    pool::submit( FuncType&& func, ArgsType&& ... args )
    {
        auto bound_task = std::bind( std::forward< FuncType >( func ), std::forward< ArgsType >( args ) ... );
        using func_result_type = std::result_of_t< decltype( bound_task )() >;
        using packaged_task_type = std::packaged_task< func_result_type() >;
        using task_type = thread_task< packaged_task_type >;

        packaged_task_type task{ std::move( bound_task ) };
        future< func_result_type > result_future{ task.get_future() };
        work_queue.push( std::make_unique< task_type >( std::move( task ) ) );
        return result_future;
    }

    size_t
    pool::num_threads() const
    {
        return worker_threads.size();
    }

    size_t
    pool::size() const
    {
        return work_queue.size();
    }

    bool
    pool::empty() const
    {
        return work_queue.empty();
    }

    size_t
    pool::workers_limit() const
    {
        //std::lock_guard<std::mutex> lock{ working_mutex };
        return max_workers_limit;
    }

    size_t
    pool::workers_active() const
    {
        return num_working;
    }

    template< class InputIt, class UnaryFunction >
    UnaryFunction
    for_each( pool& thread_pool, InputIt first, InputIt last, UnaryFunction f )
    {
        std::list< future< void > > futures; // to store the futures

        for( ; first != last; ++first )
        {
            futures.emplace_back( thread_pool.submit( [&f,first]{ f(*first); } ) );
        }

        while( !futures.empty() )
        {
            futures.pop_front();
        }

        return f;
    }
}

#endif // POOL_THREAD_POOL_INL_INCLUDED
