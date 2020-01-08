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
        arg_numGenerations( ~0L ),
        arg_file_sync( false ),
        arg_headless( false )
    {
        // threads logic
        auto auto_threads = [&]( float ratio, Settings& settings ) -> void
        {
            settings.arg_numThreads = std::max<size_t>( 1, std::thread::hardware_concurrency() * ratio );
            settings.arg_numColumns = std::max<size_t>( 1, settings.arg_numThreads > 4 ? settings.arg_numThreads / 2 : settings.arg_numThreads );
            settings.arg_windowScale = ( settings.arg_numColumns > 4 || settings.arg_numThreads > 8 ) ? 1.0f : 2.0f;
        };

        // setup default threads
        auto_threads( 0.5f, *this );

        auto version_func = []() -> void
        {
            std::cout << "spiky_nes " << __DATE__ << ", " << __TIME__ << std::endl;
            exit( 0 );
        };

        auto help_func = [&]() -> void
        {
            std::cout << "spiky_nes " << __DATE__ << ", " << __TIME__ << "\n" << std::endl;
            std::cout << "\t-h              Help\n";
            std::cout << "\t-v              Version\n";
            std::cout << "\t-f              data xml file to load from and save to\n";
            std::cout << "\t-i              data xml file to load from\n";
            std::cout << "\t-o              data xml file to save to\n";
            std::cout << "\t-t              num_threads\n";
            std::cout << "\t-s              pixel_scale\n";
            std::cout << "\t-w              num_columns\n";
            std::cout << "\t-p              size_population\n\n";
            std::cout << "\t-n              num_generations\n\n";
            std::cout << "\t--rom           rom_path\n";
            std::cout << "\t--hash-rom      rom_path_to_hash\n";
            std::cout << "\t--file-sync     save file on main thread\n";
            std::cout << "\t--file-async    save file on worker thread\n";
            std::cout << "\t--headless      disable preview window\n";
            std::cout << "\t--auto-threads  automatically choose max num threads\n";
            exit( 0 );
        };

        auto rom_hash_func = [&]( const std::string& rom_to_hash_path ) -> void
        {
            std::cout << spkn::GetROMFileHashString( rom_to_hash_path );
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

        auto load_settings_file = [&]( const std::string& file_path, Settings& settings ) -> void
        {
            rapidxml::xml_document<> doc;

            try
            {
                std::string file_data = GetFileAsString( file_path );

                if( file_data.empty() )
                {
                    return;
                }

                doc.parse<rapidxml::parse_default>( &file_data[0] );

                settings.__LoadFromXML( neat::xml::FindNode( "settings", &doc ) );

                file_data.clear();
            }
            catch( const rapidxml::parse_error& e )
            {
                std::cout << "\nParsing exception caught during file settings load" << std::endl;
                std::cout << e.what() << std::endl;
                exit( 1 );
            }
            catch( const std::exception& e )
            {
                std::cout << "\nException caught during file settings load" << std::endl;
                std::cout << e.what() << std::endl;
                exit( 1 );
            }
            catch( ... )
            {
                std::cout << "Unknown exception during file settings load" << std::endl;
                exit( 1 );
            }
        };

        cmd.add_void(          { "?", "-h", "help" },               help_func,                                                                                         "Prints help"  );
        cmd.add_void(          { "-v", "version" },                 version_func,                                                                                      "Prints version"  );

        cmd.add<std::string>(  { "--hash-rom", "hash_rom" },        rom_hash_func,                                                                                     "Path to rom file to hash"  );

        cmd.add<std::string>(  { "--set", "_var" },                 assign_var,                                                                                        "Set misc variables"  );

        cmd.add<std::string>(  { "--rom", "rom_path" },             [&]( const std::string& s ){ arg_rom_path = s; },                                                  "Path to rom file"  );
        cmd.add<std::string>(  { "-o", "output" },                  [&]( const std::string& s ){ arg_output_path = s; },                                               "Path to file to save output"  );
        cmd.add<std::string>(  { "-i", "input" },                   [&]( const std::string& s ){ load_settings_file(s,*this); arg_input_path = s; },                   "Path to file to load at startup"  );
        cmd.add<std::string>(  { "-f", "file" },                    [&]( const std::string& s ){ load_settings_file(s,*this); arg_input_path = arg_output_path = s; }, "Path to file to load at startup and to save output to"  );
        cmd.add<size_t>(       { "-t", "threads" },                 [&]( size_t i ){ arg_numThreads = i; },                                                            "Number of threads"  );
        cmd.add<float>(        { "-s", "scale" },                   [&]( float f ){ arg_windowScale = f; },                                                            "Scale of NES preview windows"  );
        cmd.add<size_t>(       { "-w", "columns" },                 [&]( size_t i ){ arg_numColumns = i; },                                                            "Number of NES preview Windows per row"  );
        cmd.add<size_t>(       { "-p", "population" },              [&]( size_t i ){ arg_populationSize = i; },                                                        "Number of networks"  );
        cmd.add<size_t>(       { "-n", "generations" },             [&]( size_t i ){ arg_numGenerations = i; },                                                        "Number of generations to calculate"  );

        cmd.add_void(          { "--file-sync", "filesync" },       [&](){ arg_file_sync = true; },                                                                    "Flag to save file on main thread"  );
        cmd.add_void(          { "--file-async", "fileasync" },     [&](){ arg_file_sync = false; },                                                                   "Flag to save file on worker thread"  );
        cmd.add_void(          { "--headless", "headless" },        [&](){ arg_headless = true; },                                                                     "Flag to disable preview window"  );
        cmd.add_void(          { "--auto-threads", "autothreads" }, [&](){ auto_threads( 1.0f, *this ); },                                                             "Automatically set threads to max allowed and window to accommodate"  );
    }

    Settings::Settings( const rapidxml::xml_node<> * settings_node )
     : Settings()
    {
        __LoadFromXML( settings_node );
    }

    void
    Settings::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool ) const
    {
        auto settings_node = neat::xml::Node( "settings", "", mem_pool );

        neat::xml::appendSimpleValueNode( "arg_rom_path", arg_rom_path, settings_node, mem_pool );
        {
            // append hash of ROM file for future cross-checking
            neat::xml::FindNode( "arg_rom_path", settings_node )->append_attribute( neat::xml::Attribute( "rom_hash", spkn::GetROMFileHashString( arg_rom_path ), mem_pool ) );
        }
        neat::xml::appendSimpleValueNode( "arg_output_path", arg_output_path, settings_node, mem_pool );
        neat::xml::appendSimpleValueNode( "arg_input_path", arg_input_path, settings_node, mem_pool );
        neat::xml::appendSimpleValueNode( "arg_num_threads", arg_numThreads, settings_node, mem_pool );
        neat::xml::appendSimpleValueNode( "arg_window_scale", arg_windowScale, settings_node, mem_pool );
        neat::xml::appendSimpleValueNode( "arg_num_columns", arg_numColumns, settings_node, mem_pool );
        neat::xml::appendSimpleValueNode( "arg_population_size", arg_populationSize, settings_node, mem_pool );
        neat::xml::appendSimpleValueNode( "arg_num_generations", arg_numGenerations, settings_node, mem_pool );
        neat::xml::appendSimpleValueNode( "arg_file_sync", arg_file_sync, settings_node, mem_pool );
        neat::xml::appendSimpleValueNode( "arg_headless", arg_headless, settings_node, mem_pool );

        var.SaveToXML( settings_node, mem_pool );

        // add node to destination
        destination->append_node( settings_node );
    }

    void
    Settings::__LoadFromXML( const rapidxml::xml_node<> * settings_node )
    {
        assert( settings_node && neat::xml::Name( settings_node ) == "settings" );

        neat::xml::readSimpleValueNode<std::string>( "arg_rom_path", arg_rom_path, settings_node );
        {
            std::string saved_hash = neat::xml::Value( neat::xml::FindAttribute( "rom_hash", neat::xml::FindNode( "arg_rom_path", settings_node ) ) );
            assert( saved_hash == spkn::GetROMFileHashString( arg_rom_path ) );
        }

        neat::xml::readSimpleValueNode( "arg_output_path", arg_output_path, settings_node );
        neat::xml::readSimpleValueNode( "arg_input_path", arg_input_path, settings_node );
        neat::xml::readSimpleValueNode( "arg_num_threads", arg_numThreads, settings_node );
        neat::xml::readSimpleValueNode( "arg_window_scale", arg_windowScale, settings_node );
        neat::xml::readSimpleValueNode( "arg_num_columns", arg_numColumns, settings_node );
        neat::xml::readSimpleValueNode( "arg_population_size", arg_populationSize, settings_node );
        neat::xml::readSimpleValueNode( "arg_num_generations", arg_numGenerations, settings_node );
        neat::xml::readSimpleValueNode( "arg_file_sync", arg_file_sync, settings_node );
        neat::xml::readSimpleValueNode( "arg_headless", arg_headless, settings_node );

        var = Variables( neat::xml::FindNode( "var", settings_node ) );
    }

    void
    Settings::parse_cmd( int argc, char** argv )
    {
        // parse the cmd, and have the fall-back arg be considered the rom path
        cmd.parse( argc, argv, [&]( const std::string& s ){ arg_rom_path = s; } );
    }

    Variables::Variables( const rapidxml::xml_node<> * variables_node )
     : vars()
    {
        assert( variables_node && neat::xml::Name( variables_node ) == "var" );

        auto var_node = variables_node->first_node();

        while( var_node != nullptr )
        {
            auto value = neat::xml::FindAttribute( "value", var_node );
            if( value != nullptr )
            {
                vars[ neat::xml::Name( var_node ) ] = neat::xml::Value( value );
            }

            var_node = var_node->next_sibling();
        }
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
