#include "settings.hpp"

#include "cmd.hpp"

namespace spkn
{
    void
    Settings::parse_cmd( int argc, char** argv )
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

        cmd.add( new spkn::Cmd::Arg<std::string>{ { "--rom", "rom_path" },       [&]( const std::string& s ){ arg_rom_path = s; },                     "Path to rom file" } );
        cmd.add( new spkn::Cmd::Arg<std::string>{ { "-o", "output" },            [&]( const std::string& s ){ arg_output_path = s; },                  "Path to file to save output" } );
        cmd.add( new spkn::Cmd::Arg<std::string>{ { "-i", "input" },             [&]( const std::string& s ){ arg_input_path = s; },                   "Path to file to load at startup" } );
        cmd.add( new spkn::Cmd::Arg<std::string>{ { "-f", "file" },              [&]( const std::string& s ){ arg_input_path = arg_output_path = s; }, "Path to file to load at startup and to save output to" } );
        cmd.add( new spkn::Cmd::Arg<int>{         { "-t", "threads" },           [&]( int i ){ arg_numThreads = i; },                                  "Number of threads" } );
        cmd.add( new spkn::Cmd::Arg<float>{       { "-s", "scale" },             [&]( float f ){ arg_windowScale = f; },                               "Scale of NES preview windows" } );
        cmd.add( new spkn::Cmd::Arg<int>{         { "-w", "columns" },           [&]( int i ){ arg_numColumns = i; },                                  "Number of NES preview Windows per row" } );
        cmd.add( new spkn::Cmd::Arg<int>{         { "-n", "population" },        [&]( int i ){ arg_populationSize = i; },                              "Number of networks" } );
        cmd.add( new spkn::Cmd::Arg_void{         { "?", "-h", "help" },         help_func,                                                            "Prints help" } );
        cmd.add( new spkn::Cmd::Arg_void{         { "-v", "version" },           version_func,                                                         "Prints version" } );
        cmd.add( new spkn::Cmd::Arg_void{         { "--file-sync", "filesync" }, []{},                                                                 "Flag to save file on main thread" } );
        cmd.add( new spkn::Cmd::Arg_void{         { "--headless", "headless" },  []{},                                                                 "Flag to disable preview window" } );

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

            size_t hash_value = 0;

            std::ifstream rom_file( arg_rom_path.c_str(), std::ifstream::in | std::ifstream::binary);

            if( rom_file.is_open() )
            {
                rom_file.seekg( 0, rom_file.end );
                size_t rom_file_size = rom_file.tellg();
                rom_file.seekg( 0, rom_file.beg );

                //char * rom_data = new char[ rom_file_size ];

                std::string rom_data( rom_file_size, '\0' );

                rom_file.read( &rom_data[0], rom_file_size );

                rom_file.close();

                hash_value = std::hash<std::string>{}( rom_data );

                //delete [] rom_data;
            }

            std::stringstream ss;
            ss << std::hex << hash_value;
            neat::xml::FindNode( "arg_rom_path", settings_node )->append_attribute( neat::xml::Attribute( "hash", ss.str(), mem_pool ) );
        }
        neat::xml::appendSimpleValueNode( "arg_output_path", arg_output_path, settings_node, mem_pool );
        neat::xml::appendSimpleValueNode( "arg_input_path", arg_input_path, settings_node, mem_pool );
        neat::xml::appendSimpleValueNode( "arg_num_threads", arg_numThreads, settings_node, mem_pool );
        neat::xml::appendSimpleValueNode( "arg_window_scale", arg_windowScale, settings_node, mem_pool );
        neat::xml::appendSimpleValueNode( "arg_num_columns", arg_numColumns, settings_node, mem_pool );
        neat::xml::appendSimpleValueNode( "arg_population_size", arg_populationSize, settings_node, mem_pool );

        // add node to destination
        destination->append_node( settings_node );
    }
}
