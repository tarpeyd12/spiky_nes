#include "settings.hpp"

#include "cmd.hpp"

#include "helpers.hpp"

namespace spkn
{
    Settings::Settings()
     : cmd(), var(),
        arg_rom_path( "" ),
        arg_output_path( "" ),
        arg_input_path( "" ),
        arg_numThreads( 1 ),
        arg_windowScale( 2.0f ),
        arg_numColumns( 2 ),
        arg_populationSize( 150 ),
        arg_file_sync( false ),
        arg_headless( false )
    {
        auto version_func = []() -> void
        {
            std::cout << "spiky_nes " << __DATE__ << ", " << __TIME__ << std::endl;
            exit( 0 );
        };

        auto help_func = [&]() -> void
        {
            std::cout << "spiky_nes " << __DATE__ << ", " << __TIME__ << "\n" << std::endl;
            std::cout << "\t-h            Help\n";
            std::cout << "\t-v            Version\n";
            //std::cout << "\t-i            data xml file to load from\n";
            std::cout << "\t-o            data xml file to save to\n";
            std::cout << "\t-t            num_threads\n";
            std::cout << "\t-s            pixel_scale\n";
            std::cout << "\t-w            num_columns\n";
            std::cout << "\t-n            size_population\n\n";
            std::cout << "\t--rom         rom_path\n";
            std::cout << "\t--file-sync   save file on main thread\n";
            std::cout << "\t--headless    disable preview window\n";
            exit( 0 );
        };

        auto assign_var = [&]( const std::string& var_data_str ) -> void
        {
            const std::string delim_str = "=";
            auto delim_pos = var_data_str.find_first_of( delim_str );
            if( delim_pos != var_data_str.npos )
            {
                std::string var_str = var_data_str.substr( 0, delim_pos );
                std::string data_str = var_data_str.substr( delim_pos + delim_str.length() );

                if( !var_str.empty() && !data_str.empty() )
                {
                    var[ var_str ] = data_str;
                }
            }
        };

        cmd.add( new spkn::Cmd::Arg_void{         { "?", "-h", "help" },         help_func,                                                            "Prints help" } );
        cmd.add( new spkn::Cmd::Arg_void{         { "-v", "version" },           version_func,                                                         "Prints version" } );

        cmd.add( new spkn::Cmd::Arg<std::string>{ { "--rom", "rom_path" },       [&]( const std::string& s ){ arg_rom_path = s; },                     "Path to rom file" } );
        cmd.add( new spkn::Cmd::Arg<std::string>{ { "-o", "output" },            [&]( const std::string& s ){ arg_output_path = s; },                  "Path to file to save output" } );
        cmd.add( new spkn::Cmd::Arg<std::string>{ { "-i", "input" },             [&]( const std::string& s ){ arg_input_path = s; },                   "Path to file to load at startup" } );
        cmd.add( new spkn::Cmd::Arg<std::string>{ { "-f", "file" },              [&]( const std::string& s ){ arg_input_path = arg_output_path = s; }, "Path to file to load at startup and to save output to" } );
        cmd.add( new spkn::Cmd::Arg<size_t>{      { "-t", "threads" },           [&]( size_t i ){ arg_numThreads = i; },                               "Number of threads" } );
        cmd.add( new spkn::Cmd::Arg<float>{       { "-s", "scale" },             [&]( float f ){ arg_windowScale = f; },                               "Scale of NES preview windows" } );
        cmd.add( new spkn::Cmd::Arg<size_t>{      { "-w", "columns" },           [&]( size_t i ){ arg_numColumns = i; },                               "Number of NES preview Windows per row" } );
        cmd.add( new spkn::Cmd::Arg<size_t>{      { "-n", "population" },        [&]( size_t i ){ arg_populationSize = i; },                           "Number of networks" } );

        cmd.add( new spkn::Cmd::Arg_void{         { "--file-sync", "filesync" }, [&]{ arg_file_sync = true },                                          "Flag to save file on main thread" } );
        cmd.add( new spkn::Cmd::Arg_void{         { "--headless", "headless" },  [&]{ arg_headless = true },                                           "Flag to disable preview window" } );
        cmd.add( new spkn::Cmd::Arg<std::string>{ { "--set", "_var" },           assign_var,                                                           "Set misc variables" } );
    }

    void
    Settings::parse_cmd( int argc, char** argv )
    {
        // parse the cmd, and have the fall-back arg be considered the rom path
        cmd.parse( argc, argv, [&]( const std::string& s ){ arg_rom_path = s; } );
    }

    void
    Settings::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool ) const
    {
        auto settings_node = neat::xml::Node( "settings", "", mem_pool );

        neat::xml::appendSimpleValueNode( "arg_rom_path", arg_rom_path, settings_node, mem_pool );
        {
            // append hash of ROM file for future cross-checking
            neat::xml::FindNode( "arg_rom_path", settings_node )->append_attribute( neat::xml::Attribute( "hash", spkn::GetROMFileHashString( arg_rom_path ), mem_pool ) );
        }
        neat::xml::appendSimpleValueNode( "arg_output_path", arg_output_path, settings_node, mem_pool );
        neat::xml::appendSimpleValueNode( "arg_input_path", arg_input_path, settings_node, mem_pool );
        neat::xml::appendSimpleValueNode( "arg_num_threads", arg_numThreads, settings_node, mem_pool );
        neat::xml::appendSimpleValueNode( "arg_window_scale", arg_windowScale, settings_node, mem_pool );
        neat::xml::appendSimpleValueNode( "arg_num_columns", arg_numColumns, settings_node, mem_pool );
        neat::xml::appendSimpleValueNode( "arg_population_size", arg_populationSize, settings_node, mem_pool );
        neat::xml::appendSimpleValueNode( "arg_file_sync", arg_file_sync, settings_node, mem_pool );
        neat::xml::appendSimpleValueNode( "arg_headless", arg_headless, settings_node, mem_pool );

        var.SaveToXML( settings_node, mem_pool );

        // add node to destination
        destination->append_node( settings_node );
    }

    void
    Variables::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool ) const
    {
        auto variables_node = neat::xml::Node( "var", "", mem_pool );

        for( const auto& var : vars )
        {
            neat::xml::appendSimpleValueNode( var.first.c_str(), var.second, variables_node, mem_pool );
        }

        destination->append_node( variables_node );
    }

}
