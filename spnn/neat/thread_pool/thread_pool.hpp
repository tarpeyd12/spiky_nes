#ifndef POOL_THREAD_POOL_HPP_INCLUDED
#define POOL_THREAD_POOL_HPP_INCLUDED

#include <atomic>
#include <vector>
#include <thread>
#include <condition_variable>

#include "safe_queue.hpp"
#include "task.hpp"

namespace tpl
{
    class pool
    {
        private:

            std::atomic_bool done;
            std::vector< std::thread > worker_threads;
            safe_queue< std::unique_ptr< task_base > > work_queue;

            std::mutex working_mutex;
            std::condition_variable working_condition;
            size_t max_workers_limit;
            size_t num_working;

            std::atomic< size_t > num_executing;

        public:

            pool();
            explicit pool( const size_t num_threads );
            ~pool();

            // no copy
            pool( const pool& ) = delete;
            pool& operator=( const pool& ) = delete;

            // job submission
            template < typename FuncType, typename ... ArgsType >
            decltype(auto) submit( FuncType&& func, ArgsType&& ... args ); // returns a tpl::future< T > where T is the return type of func

            inline size_t num_threads() const;
            inline size_t size() const;
            inline bool empty() const;

            void limit_workers( size_t num_workers );
            inline size_t workers_limit() const;
            inline size_t workers_active() const;

        private:

            void destroy();
            void worker_method( const size_t __thread_num );

    };

    template< class InputIt, class UnaryFunction >
    UnaryFunction for_each( pool& thread_pool, InputIt first, InputIt last, UnaryFunction f );
}

#include "thread_pool.inl"

#endif // POOL_THREAD_POOL_HPP_INCLUDED
