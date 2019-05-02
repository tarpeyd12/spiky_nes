#include "cmd.hpp"

namespace spkn
{
    bool
    Cmd::Arg_base::match( const char * s ) const
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

    Cmd::Arg_void::Arg_void( const std::list< std::string >& a, std::function<void()> func, const std::string& d )
        : Arg_base{ a, d }, _callback( func )
    {
        assert( !arg.empty() && _callback != nullptr );
    }

    void
    Cmd::Arg_void::call( const char * /*s*/ )
    {
        _callback();
    }


    Cmd::Cmd( std::list< Arg_base* > args )
        : argument_list( args ), argsFound()
    {
        /*  */
    }

    Cmd::~Cmd()
    {
        for( auto*& a : argsFound )
        {
            a = nullptr;
        }
        argsFound.clear();

        for( auto* a : argument_list )
        {
            delete a;
        }
    }

    void
    Cmd::add( Arg_base* a )
    {
        argument_list.emplace_back( a );
    }

    void
    Cmd::parse( int argc, char** argv, std::function<void(const std::string&)> defaultCheck )
    {
        argsFound.clear();

        char ** begin = argv;
        char ** end = begin + argc;

        for( auto it = begin; it != end; ++it )
        {
            std::list< Arg_base* > matchedArgs;

            for( auto* arg : argument_list )
            {
                if( arg->match( *it ) )
                {
                    ++it; // increment iterator
                    if( it == end )
                    {
                        // error
                        return;
                    }
                    matchedArgs.emplace_back( arg );
                }
            }

            if( !matchedArgs.empty() )
            {
                argsFound.emplace_back( matchedArgs.front() );
                matchedArgs.front()->call( *it );
            }
            else if( defaultCheck != nullptr )
            {
                defaultCheck( std::string{ *it } );
            }
        }

        //end
    }

    bool
    Cmd::wasArgFound( const std::string& arg ) const
    {
        for( Arg_base* a : argsFound )
        {
            if( a->match( arg.c_str() ) )
            {
                return true;
            }
        }

        return false;
    }
}
