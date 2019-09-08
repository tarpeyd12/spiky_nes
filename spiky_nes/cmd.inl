#ifndef SPKN_CMD_INL_INCLUDED
#define SPKN_CMD_INL_INCLUDED

#include <sstream>
#include <cassert>

namespace spkn
{
    template < typename ArgType >
    Cmd::Arg< ArgType >::Arg( const std::list< std::string >& a, std::function<void(ArgType)> func, const std::string& d )
        : Arg_base{ a, d }, _callback( func )
    {
        assert( !arg.empty() && _callback != nullptr );
    }

    template < typename ArgType >
    void
    Cmd::Arg< ArgType >::call( const char * s )
    {
        if( s != nullptr )
        {
            ArgType v{};

            {
                std::istringstream ss;
                ss.str( std::string{ s } );

                ss >> v;

                //ss.str( std::string{} );
            }

            _callback( v );
        }
    }

    template < typename ArgType >
    void
    Cmd::add( const std::list< std::string >& a, std::function<void(ArgType)> func, const std::string& d )
    {
        add( std::make_shared< Arg< ArgType > >( a, func, d ) );
    }
}

#endif // SPKN_CMD_INL_INCLUDED
