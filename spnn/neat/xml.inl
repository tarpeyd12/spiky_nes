#ifndef NEAT_XML_INL_INCLUDED
#define NEAT_XML_INL_INCLUDED

#include <cstring>

#include "../lib/base64.hpp"

namespace neat
{
    namespace b64
    {
        template < typename T >
        std::string
        Encode_DataStruct( const T& data )
        {
            return base64_encode( reinterpret_cast<const unsigned char*>( *data ), sizeof( T ) );
        }

        template < typename T >
        T
        Decode_DataStruct( const std::string& data )
        {
            std::string decoded = base64_decode( data );
            T out;
            size_t size = std::min<size_t>( sizeof( T ), decoded.size() );
            std::memcpy( &out, reinterpret_cast<void*>( decoded.c_str() ), size );
            return out;
        }
    }
}

#endif // NEAT_XML_INL_INCLUDED
