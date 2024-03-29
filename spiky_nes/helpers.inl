#ifndef HELPERS_INL_INCLUDED
#define HELPERS_INL_INCLUDED

#include <fstream>
#include <sstream>

namespace spkn
{
    void
    SecondsToHMS( long double seconds, uint64_t &h, uint8_t &m, long double &s )
    {
        h = uint64_t( seconds / 3600l );
        m = uint8_t( uint64_t( seconds / 60l ) % 60l );
        s = fmodl( seconds, 60.0L );
    }

    std::string
    SecondsToHMS( long double seconds, uint8_t precision, bool prepad_zeros )
    {
        std::ostringstream ss;

        uint64_t    h;
        uint8_t     m;
        long double s;

        SecondsToHMS( seconds, h, m, s );

        if( seconds >= 3600.0L )
        {
            ss << h << "h";

        }

        bool display_minutes = seconds >= 60.0L;

        if( prepad_zeros && display_minutes )
        {
            ss << std::setw( 2 ) << std::setfill( '0' ) << uint16_t( m ) << "m";
        }
        else if( display_minutes )
        {
            ss << uint16_t( m ) << "m";
        }

        if( prepad_zeros )
        {
            ss << std::fixed << std::setprecision( precision ) << std::setw( 2 + precision + ( precision > 0 ? 1 : 0 ) ) << std::setfill( '0' ) << s << "s";
        }
        else
        {
            ss << std::fixed << std::setprecision( precision ) << s << "s";
        }

        return ss.str();
    }

    std::string
    GetFileAsString( const std::string& file_path )
    {
        std::ifstream file( file_path.c_str(), std::ifstream::in | std::ifstream::binary);

        if( file.is_open() )
        {
            file.seekg( 0, file.end );
            size_t file_size = file.tellg();
            file.seekg( 0, file.beg );

            std::string data( file_size + 1, '\0' );

            file.read( &data[0], file_size );

            file.close();

            return data;
        }

        return std::string();
    }

    std::string
    GetFileAsString_untill( const std::string& file_path, const std::string& stop_sequence )
    {
        const size_t chunk_size = 0xffff;

        std::ifstream file( file_path.c_str(), std::ifstream::in | std::ifstream::binary);

        if( file.is_open() )
        {
            std::string output;

            std::string previous_chunk_end;

            bool end_found = false;

            while( file.good() && !end_found )
            {
                std::string chunk( chunk_size, '\0' );

                file.read( const_cast<char *>( chunk.data() ), chunk_size );

                if( ( previous_chunk_end + chunk ).find( stop_sequence ) != std::string::npos )
                {
                    end_found = true;
                }

                previous_chunk_end = chunk.substr( chunk.size() - stop_sequence.size() );

                output += chunk;
            }

            file.close();

            return output.substr( 0, output.find( stop_sequence ) + stop_sequence.size() );
        }

        return std::string();
    }

    std::string
    GetROMFileHashString( const std::string& rom_file_path )
    {
        std::stringstream ss;
        ss << std::hex << std::hash<std::string>{}( GetFileAsString( rom_file_path ) );

        return ss.str();
    }
}

#endif // HELPERS_INL_INCLUDED
