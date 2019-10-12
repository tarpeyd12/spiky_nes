#ifndef ZLIB_ZLIB_HPP_INCLUDED
#define ZLIB_ZLIB_HPP_INCLUDED

#include <string>

namespace zlib
{
    std::string compress_string( const std::string& str, int compression = 9 ); // 9 == Z_BEST_COMPRESSION
    std::string decompress_string( const std::string& str );
}

#endif // ZLIB_ZLIB_HPP_INCLUDED
