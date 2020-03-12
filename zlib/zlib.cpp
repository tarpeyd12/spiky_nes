#include <cstring>
#include <exception>
#include <stdexcept>
#include <sstream>

#include <iostream> // for some debugging

#include "zlib-1.2.11/zlib.h"
#include "zlib-1.2.11/zutil.h"
#include "zlib-1.2.11/zconf.h"

#include "zlib.hpp"

// https://panthema.net/2007/0328-ZLibString.html

namespace zlib
{
    //const size_t chunk_size = 1024*1024;
    const size_t chunk_size = 0x7fffff;

    std::string
    compress_string( const std::string& str, int compression )
    {
        z_stream zstrm;
        std::memset( &zstrm, 0, sizeof( zstrm ) );

        if( deflateInit( &zstrm, compression ) != Z_OK )
        {
            throw std::runtime_error( "zlib::compress_string deflateInit() failed." );
        }

        int code = Z_OK;
        unsigned char * buff = new unsigned char[ chunk_size ];
        std::string out_string;

        size_t input_processed = 0;

        do
        {
            zstrm.next_in  = (unsigned char *)str.data() + input_processed;
            zstrm.avail_in = std::min<uInt>( chunk_size, str.size() - input_processed );
            input_processed += chunk_size;

            do
            {
                zstrm.next_out  = buff;
                zstrm.avail_out = sizeof( unsigned char ) * chunk_size;

                code = deflate( &zstrm, input_processed >= str.size() ? Z_FINISH : Z_NO_FLUSH );

                out_string.append( reinterpret_cast<char*>(buff), chunk_size - zstrm.avail_out );
            }
            while( zstrm.avail_out == 0 );
        }
        while( input_processed < str.size() );

        delete[] buff;

        deflateEnd( &zstrm );

        if( code != Z_STREAM_END )
        {
            std::ostringstream ss;
            ss << "zlib::compress_string deflate() failed with code " << code << ", message: '" << zstrm.msg << "'";
            throw std::runtime_error( ss.str() );
        }

        return out_string;
    }

    std::string
    decompress_string( const std::string& str )
    {
        z_stream zstrm;
        std::memset( &zstrm, 0, sizeof( zstrm ) );

        if( inflateInit( &zstrm ) != Z_OK )
        {
            throw std::runtime_error( "zlib::decompress_string inflateInit() failed." );
        }

        int code = Z_OK;
        unsigned char * buff = new unsigned char[ chunk_size ];
        std::string out_string;

        size_t input_processed = 0;

        do
        {
            zstrm.next_in  = (unsigned char *)str.data() + input_processed;
            zstrm.avail_in = std::min<uInt>( chunk_size, str.size() - input_processed );
            input_processed += chunk_size;

            if( zstrm.avail_in == 0 )
            {
                break;
            }

            do
            {
                zstrm.next_out  = buff;
                zstrm.avail_out = sizeof( unsigned char ) * chunk_size;

                code = inflate( &zstrm, Z_NO_FLUSH );

                if( code == Z_STREAM_ERROR )
                {
                    throw std::runtime_error( "zlib::decompress_string inflate() failed Z_STREAM_ERROR." );
                }

                switch( code )
                {
                    default: break;
                    case Z_NEED_DICT:  throw std::runtime_error( "zlib::decompress_string inflate() failed Z_NEED_DICT." ); break;
                    case Z_DATA_ERROR: throw std::runtime_error( "zlib::decompress_string inflate() failed Z_DATA_ERROR." ); break;
                    case Z_MEM_ERROR:  throw std::runtime_error( "zlib::decompress_string inflate() failed Z_MEM_ERROR." ); break;
                }

                out_string.append( reinterpret_cast<char*>(buff), chunk_size - zstrm.avail_out );
            }
            while( zstrm.avail_out == 0 );
        }
        while( code != Z_STREAM_END );

        delete[] buff;

        inflateEnd( &zstrm );

        if( code != Z_STREAM_END )
        {
            std::ostringstream ss;
            ss << "zlib::decompress_string deflate() failed with code " << code << ", message: '" << zstrm.msg << "'";
            throw std::runtime_error( ss.str() );
        }

        return out_string;
    }
}
