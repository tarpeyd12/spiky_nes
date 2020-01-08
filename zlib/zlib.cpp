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
    const size_t chunk_size = 1024*1024;
    //const size_t chunk_size = 0x7fff;

    std::string
    compress_string( const std::string& str, int compression )
    {
        z_stream zstrm;
        std::memset( &zstrm, 0, sizeof( zstrm ) );

        if( deflateInit( &zstrm, compression ) != Z_OK )
        {
            throw std::runtime_error( "zlib::compress_string deflateInit() failed." );
        }

        zstrm.next_in  = (unsigned char *)str.data();
        zstrm.avail_in = str.size();

        int code = Z_OK;
        unsigned char buff[ chunk_size ];
        std::string out_string;

        do
        {
            zstrm.next_out  = buff;
            zstrm.avail_out = sizeof( buff );

            code = deflate( &zstrm, Z_FINISH );

            if( out_string.size() < zstrm.total_out )
            {
                out_string.append( reinterpret_cast<char*>(buff), zstrm.total_out - out_string.size() );
            }
        }
        while( code == Z_OK );

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

        zstrm.next_in  = (unsigned char *)str.data();
        zstrm.avail_in = str.size();

        int code = Z_OK;
        unsigned char buff[ chunk_size ];
        std::string out_string;

        do
        {
            zstrm.next_out  = buff;
            zstrm.avail_out = sizeof( buff );

            code = inflate( &zstrm, 0 );

            if( out_string.size() < zstrm.total_out )
            {
                out_string.append( reinterpret_cast<char*>(buff), zstrm.total_out - out_string.size() );
            }
        }
        while( code == Z_OK );

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
