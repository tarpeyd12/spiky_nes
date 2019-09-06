#ifndef SPKN_SETTINGS_HPP_INCLUDED
#define SPKN_SETTINGS_HPP_INCLUDED

#include "../spnn/neat/xml.hpp"

#include "cmd.hpp"

namespace spkn
{
    class Variables
    {
        private:

            std::map< std::string, std::string > vars;

        public:

            Variables() = default;
            ~Variables() = default;

            inline std::string& operator[]( const std::string& key ) { return vars[ key ]; }

            template < typename T >
            inline
            T get( const std::string& key, const T& _default ) const
            {
                auto it = vars.find( key );
                if( it != vars.end() )
                {
                    return neat::xml::from_string<T>( it->second );
                }
                return _default;
            }

            template < typename T >
            inline
            T operator()( const std::string& key, const T& _default ) const
            {
                return get<T>( key, _default );
            }

            void SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool ) const;
    };

    struct Settings
    {
        Settings();

        // command line parser
        Cmd cmd;

        // storage of other variables
        Variables var;

        // get info from the command line
        void parse_cmd( int argc, char** argv );

        // for saving to the data file
        void SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool ) const;

        // data

        std::string arg_rom_path;    // designated file cross-checked with hash
        std::string arg_output_path; // not loaded
        std::string arg_input_path;  // not loaded
        size_t      arg_numThreads;  // not loaded
        float       arg_windowScale;
        size_t      arg_numColumns;
        size_t      arg_populationSize;
        bool        arg_file_sync;
        bool        arg_headless;

    };
}

#endif // SPKN_SETTINGS_HPP_INCLUDED
