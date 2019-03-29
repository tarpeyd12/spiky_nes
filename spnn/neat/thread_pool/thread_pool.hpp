#ifndef POOL_THREAD_POOL_HPP_INCLUDED
#define POOL_THREAD_POOL_HPP_INCLUDED

#include <atomic>
#include <vector>
#include <thread>

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

        private:

            void destroy();
            void worker_method();

    };

    template< class InputIt, class UnaryFunction >
    UnaryFunction for_each( pool& thread_pool, InputIt first, InputIt last, UnaryFunction f );
}

#include "thread_pool.inl"

#endif // POOL_THREAD_POOL_HPP_INCLUDED
