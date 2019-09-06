#ifndef SPKN_SETTINGS_HPP_INCLUDED
#define SPKN_SETTINGS_HPP_INCLUDED

#include "../spnn/neat/xml.hpp"

#include "cmd.hpp"

namespace spkn
{
    struct Settings
    {
        // cmd parser
        Cmd cmd;

        // essentially the constructor
        void parse_cmd( int argc, char** argv );

        // for saving to the data file
        void SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool ) const;

        // data

        std::string arg_rom_path;    // designated file cross-checked with hash
        std::string arg_output_path; // not loaded
        std::string arg_input_path;  // not loaded
        int         arg_numThreads;  // not loaded
        float       arg_windowScale;
        int         arg_numColumns;
        int         arg_populationSize;

    };
}

#endif // SPKN_SETTINGS_HPP_INCLUDED
