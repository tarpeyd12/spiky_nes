#ifndef SPKN_CMD_HPP_INCLUDED
#define SPKN_CMD_HPP_INCLUDED

#include <functional>
#include <list>

namespace spkn
{
    class Cmd
    {
        private:

            struct Arg_base
            {
                public:

                    std::list< std::string > arg; // list for aliases. eg: ? -h --help
                    std::string description;

                    Arg_base( const std::list< std::string >& a, const std::string& s ) : arg( a ), description( s ) {  }
                    virtual ~Arg_base() = default;

                protected:

                    virtual void call( const char * s ) {};
                    bool match( const char * s ) const;

                    friend class Cmd;
            };

        public:

            template < typename ArgType >
            struct Arg : public Arg_base
            {
                private:

                    std::function<void(ArgType)> _callback;

                public:

                    Arg( const std::list< std::string >& a, std::function<void(ArgType)> func, const std::string& d = "" );

                protected:

                    void call( const char * s ) override;

                    friend class Cmd;
            };

            struct Arg_void : public Arg_base
            {
                private:

                    std::function<void()> _callback;

                public:

                    Arg_void( const std::list< std::string >& a, std::function<void()> func, const std::string& d = "" );

                protected:

                    void call( const char * s ) override;

                    friend class Cmd;
            };

        private:

            std::list< Arg_base* > argument_list;
            std::list< Arg_base* > argsFound;

        public:

            Cmd( std::list< Arg_base* > args = {} );
            ~Cmd();

            void add( Arg_base* a );
            void parse( int argc, char** argv, std::function<void(const std::string&)> defaultCheck = nullptr );

            bool wasArgFound( const std::string& arg ) const;

    };

}

#include "cmd.inl"

#endif // SPKN_CMD_HPP_INCLUDED
