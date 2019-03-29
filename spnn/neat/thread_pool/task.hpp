#ifndef POOL_TASK_HPP_INCLUDED
#define POOL_TASK_HPP_INCLUDED

#include <utility>

namespace tpl
{
    class task_base
    {
        public:
            task_base() = default;
            virtual ~task_base() = default;

            // no copy
            task_base( const task_base& ) = delete;
            task_base& operator=( const task_base& ) = delete;

            // move OK
            task_base( task_base&& ) = default;
            task_base& operator=( task_base&& ) = default;

            virtual void execute() = 0;
    };

    template < typename FuncType >
    class thread_task : public task_base
    {
        private:

            FuncType func;

        public:

            thread_task( FuncType&& f ) : func( std::move( f ) ) { }

            ~thread_task() override = default;

            // no copy
            thread_task( const thread_task& ) = delete;
            thread_task& operator=( const thread_task& ) = delete;

            // move OK
            thread_task( thread_task&& ) = default;
            thread_task& operator=( thread_task&& ) = default;

            void execute() override { func(); };
    };
}

#endif // POOL_TASK_HPP_INCLUDED
