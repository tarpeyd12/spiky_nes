#ifndef SPKN_CMD_HPP_INCLUDED
#define SPKN_CMD_HPP_INCLUDED

#include <functional>
#include <list>

namespace spkn
{
    class Cmd
    {
        public:

            struct Arg
            {
                public:

                    enum Type : uint8_t { Int, Float, String };

                private:

                    std::function<void(int)>                _int;
                    std::function<void(float)>              _float;
                    std::function<void(const std::string&)> _string;

                    Type param; // for the type expected

                public:

                    std::list< std::string > arg; // list for aliases. eg: ? -h --help
                    std::string description;

                    Arg( const std::list< std::string >& a, std::function<void(int)> func, const std::string& d = "" );
                    Arg( const std::list< std::string >& a, std::function<void(float)> func, const std::string& d = "" );
                    Arg( const std::list< std::string >& a, std::function<void(const std::string&)> func, const std::string& d = "" );

                protected:

                    void call( const char * s );
                    bool match( const char * s ) const;

                    friend class Cmd;
            };

        private:

            std::list< Arg > argument_list;

        public:

            Cmd( std::list< Arg > args = {} );
            ~Cmd() = default;

            void add( Arg a );
            void parse( int argc, char** argv, std::function<void(const std::string&)> defaultCheck = nullptr );

    };
}

#endif // SPKN_CMD_HPP_INCLUDED
