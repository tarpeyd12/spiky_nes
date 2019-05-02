#include <sstream>
#include <cassert>

#include "cmd.hpp"

namespace spkn
{

    Cmd::Arg::Arg( const std::list< std::string >& a, std::function<void(int)> func, const std::string& d )
        : _int( func ), _float( nullptr ), _string( nullptr ), param( Type::Int ), arg( a ), description( d )
    {
        assert( !arg.empty() && func != nullptr );
    }

    Cmd::Arg::Arg( const std::list< std::string >& a, std::function<void(float)> func, const std::string& d )
        : _int( nullptr ), _float( func ), _string( nullptr ), param( Type::Float ), arg( a ), description( d )
    {
        assert( !arg.empty() && func != nullptr );
    }

    Cmd::Arg::Arg( const std::list< std::string >& a, std::function<void(const std::string&)> func, const std::string& d )
        : _int( nullptr ), _float( nullptr ), _string( func ), param( Type::String ), arg( a ), description( d )
    {
        assert( !arg.empty() && func != nullptr );
    }


    void
    Cmd::Arg::call( const char * s )
    {
        std::stringstream ss;
        ss.str( std::string{ s } );

        switch( param )
        {
            case Type::Int:
            {
                int i = 0;
                ss >> i;
                _int( i );
                break;
            }

            case Type::Float:
            {
                float f = 0.0f;
                ss >> f;
                _float( f );
                break;
            }

            case Type::String:
            {
                std::string s = "";
                ss >> s;
                _string( s );
                break;
            }

            default: break; // should never get here
        }
    }

    bool
    Cmd::Arg::match( const char * s ) const
    {
        std::string check_arg( s );

        for( auto a : arg )
        {
            if( a == check_arg )
            {
                return true;
            }
        }

        return false;
    }

    Cmd::Cmd( std::list< Arg > args )
        : argument_list( args )
    {
        /*  */
    }

    void
    Cmd::add( Arg a )
    {
        argument_list.emplace_back( a );
    }

    void
    Cmd::parse( int argc, char** argv, std::function<void(const std::string&)> defaultCheck )
    {
        char ** begin = argv;
        char ** end = begin + argc;

        for( auto it = begin; it != end; ++it )
        {
            std::list< Arg* > matchedArgs;

            for( Arg& arg : argument_list )
            {
                if( arg.match( *it ) )
                {
                    ++it; // increment iterator
                    if( it == end )
                    {
                        // error
                        return;
                    }
                    matchedArgs.emplace_back( &arg );
                }
            }

            if( !matchedArgs.empty() )
            {
                matchedArgs.front()->call( *it );
            }
            else if( defaultCheck != nullptr )
            {
                defaultCheck( std::string{ *it } );
            }
        }

        //end
    }
}
