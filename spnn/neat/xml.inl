#ifndef NEAT_XML_INL_INCLUDED
#define NEAT_XML_INL_INCLUDED

#include <cstring>
#include <iomanip>
#include <sstream>
#include <limits>

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

        template < typename T >
        std::string
        Encode_VectorData( const std::vector<T>& data )
        {
            return base64_encode( reinterpret_cast<const unsigned char*>( data.data() ), data.size() * sizeof( T ) );
        }

        template < typename T >
        void
        Decode_VectorData( const std::string& encoded, size_t size, std::vector<T>& out )
        {
            out.clear();
            out.resize( size );
            std::string decoded = base64_decode( encoded );
            size_t len = std::min<size_t>( sizeof( T ) * size, decoded.size() );
            std::memcpy( out.data(), reinterpret_cast<const void*>( decoded.c_str() ), len );
        }
    }

    namespace xml
    {
        template < typename T >
        T from_string( const std::string& s )
        {
            std::istringstream ss;
            ss.str( s );

            T v{};
            ss >> v;

            return v;
        }

        template < >
        inline
        std::string from_string< std::string >( const std::string& s )
        {
            return s;
        }

        template < typename T >
        std::string to_string( const T& v )
        {
            std::ostringstream ss;
            ss << v;
            std::string result = ss.str();
            if( std::is_floating_point<T>::value )
            {
                std::ostringstream ss2;
                ss2 << std::setprecision( std::numeric_limits<T>::max_digits10 + 2 );
                ss2 << v;
                std::string result2 = ss2.str();
                if( result2.find('.') == std::string::npos ) { result2 += ".0"; }
                if( fabs( v - from_string<T>( result2 ) ) < fabs( v - from_string<T>( result ) ) )
                {
                    return result2;
                }
                if( result.find('.') == std::string::npos ) { result += ".0"; }
            }
            return result;
        }

        template < >
        inline
        std::string to_string< std::string >( const std::string& v ) // specialized for strings
        {
            return v;
        }

        std::string
        Name( const rapidxml::xml_base<> * base )
        {
            if( base == nullptr ) { return std::string(); }
            return std::string( base->name(), base->name_size() );
        }

        std::string
        Value( const rapidxml::xml_base<> * base )
        {
            if( base == nullptr ) { return std::string(); }
            return std::string( base->value(), base->value_size() );
        }

        char *
        Str( const std::string& input, rapidxml::memory_pool<> * mem_pool )
        {
            return mem_pool->allocate_string( input.data(), input.size() );
        }

        rapidxml::xml_node<> *
        Node( const char * name, const std::string& data, rapidxml::memory_pool<> * mem_pool, rapidxml::node_type type )
        {
            if( data.size() )
            {
                return mem_pool->allocate_node( type, name, Str( data, mem_pool ), 0, data.size() );
            }
            return mem_pool->allocate_node( type, name, nullptr, 0, 0 );
        }

        rapidxml::xml_attribute<> *
        Attribute( const char * name, const std::string& data, rapidxml::memory_pool<> * mem_pool )
        {
            if( data.size() )
            {
                return mem_pool->allocate_attribute( name, Str( data, mem_pool ), 0, data.size() );
            }
            return mem_pool->allocate_attribute( name, nullptr, 0, 0 );
        }

        rapidxml::xml_node<> *
        FindNode( const std::string& name, const rapidxml::xml_node<> * node )
        {
            if( node == nullptr )
            {
                return nullptr;
            }
            if( name.empty() )
            {
                return node->first_node();
            }
            return node->first_node( name.c_str(), name.size() );
        }

        rapidxml::xml_attribute<> *
        FindAttribute( const std::string& name, const rapidxml::xml_node<> * node )
        {
            if( node == nullptr )
            {
                return nullptr;
            }
            if( name.empty() )
            {
                return node->first_attribute();
            }
            return node->first_attribute( name.c_str(), name.size() );
        }

        template < typename T >
        void
        appendSimpleValueNode( const char * name, const T& value, rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )
        {
            auto node = Node( name, "", mem_pool );
            node->append_attribute( Attribute( "value", xml::to_string( value ), mem_pool ) );
            destination->append_node( node );
        }

        template < typename T >
        void
        readSimpleValueNode( const char * name, T& value, const rapidxml::xml_node<> * source )
        {
            auto node = FindNode( name, source );
            if( node )
            {
                value = from_string<T>( Value( FindAttribute( "value", node ) ) );
            }
        }

        template < typename T >
        void
        appendMinMaxValueNode( const char * name, const MinMax<T>& value, rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )
        {
            auto node = Node( name, "", mem_pool );
            node->append_attribute( Attribute( "min", xml::to_string( value.min ), mem_pool ) );
            node->append_attribute( Attribute( "max", xml::to_string( value.max ), mem_pool ) );
            destination->append_node( node );
        }

        template < typename T >
        void
        readMinMaxValueNode( const char * name, MinMax<T>& value, rapidxml::xml_node<> * source )
        {
            auto node = FindNode( name, source );
            if( node )
            {
                value.min = from_string<T>( Value( FindAttribute( "min", node ) ) );
                value.max = from_string<T>( Value( FindAttribute( "max", node ) ) );
            }
        }

        template < typename T >
        void
        appendVectorData( const char * name, const std::vector<T>& data, rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )
        {
            auto node = Node( name, "", mem_pool );
            node->append_attribute( Attribute( "N", xml::to_string( data.size() ), mem_pool ) );
            node->append_attribute( Attribute( "data", b64::Encode_VectorData( data ), mem_pool ) );
            destination->append_node( node );
        }

        template < typename T >
        void
        readVectorData( const char * name, std::vector<T>& data, rapidxml::xml_node<> * source )
        {
            size_t size = 0;
            auto node = FindNode( name, source );
            size = from_string<size_t>( Value( FindAttribute( "N", node ) ) );
            b64::Decode_VectorData( Value( FindAttribute( "data", node ) ), size, data );
        }
    }
}

#endif // NEAT_XML_INL_INCLUDED
