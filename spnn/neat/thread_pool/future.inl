#ifndef POOL_FUTURE_INL_INCLUDED
#define POOL_FUTURE_INL_INCLUDED

#include <utility>

namespace tpl
{
    template < typename T >
    future< T >::future( std::future< T >&& f )
         :  _future( std::move( f ) ), block_on_destruction( true )
    {
        /*  */
    }

    template < typename T >
    future< T >::~future()
    {
        if( block_on_destruction && _future.valid() )
        {
            _future.get();
        }
    }

    template < typename T >
    T
    future< T >::get()
    {
        return _future.get();
    }

    template < typename T >
    void
    future< T >::release()
    {
        block_on_destruction = false;
    }

    template < typename T >
    bool
    future< T >::valid() const
    {
        return block_on_destruction && _future.valid();
    }
}

#endif // POOL_FUTURE_INL_INCLUDED
