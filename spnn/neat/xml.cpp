#include <cstring>
#include <string>

#include "../lib/base64.hpp"

#include "xml.hpp"

namespace neat
{
    namespace b64
    {
        std::string
        Encode_NodeGenotype( const std::vector< NodeDef >& nodes )
        {
            return base64_encode( reinterpret_cast<const unsigned char*>( nodes.data() ), nodes.size() * sizeof( NodeDef ) );
        }

        void
        Decode_NodeGenotype( size_t size, const std::string& encoded_nodes, std::vector< NodeDef >& out )
        {
            out.clear();
            out.resize( size );
            std::string decoded = base64_decode( encoded_nodes );
            size_t len = std::min<size_t>( sizeof( NodeDef ) * size, decoded.size() );
            std::memcpy( out.data(), reinterpret_cast<const void*>( decoded.c_str() ), len );
        }

        std::string
        Encode_ConnectionGenotype( const std::vector< ConnectionDef >& conns )
        {
            return base64_encode( reinterpret_cast<const unsigned char*>( conns.data() ), conns.size() * sizeof( ConnectionDef ) );
        }

        void
        Decode_ConnectionGenotype( size_t size, const std::string& encoded_conns, std::vector< ConnectionDef >& out )
        {
            out.clear();
            out.resize( size );
            std::string decoded = base64_decode( encoded_conns );
            size_t len = std::min<size_t>( sizeof( ConnectionDef ) * size, decoded.size() );
            std::memcpy( out.data(), reinterpret_cast<const void*>( decoded.c_str() ), len );
        }
    }

    namespace xml
    {
        char *
        _stringToXMLStr( const std::string& input, rapidxml::memory_pool<> * mem_pool )
        {
            return mem_pool->allocate_string( input.data(), input.size() );
        }

        rapidxml::xml_node<> *
        _createXMLNode( const char* name, const std::string& data, rapidxml::memory_pool<> * mem_pool )
        {
            return mem_pool->allocate_node( rapidxml::node_element, name, _stringToXMLStr( data, mem_pool ), 0, data.size() );
        }

        rapidxml::xml_attribute<> *
        _createXMLAttribute( const char* name, const std::string& data, rapidxml::memory_pool<> * mem_pool )
        {
            return mem_pool->allocate_attribute( name, _stringToXMLStr( data, mem_pool ), 0, data.size() );
        }

        void
        Encode_NetworkGenotype( const NetworkGenotype& genotype, rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )
        {
            rapidxml::xml_node<> * genotype_node = mem_pool->allocate_node( rapidxml::node_element, "genotype", "" );
            rapidxml::xml_node<> * nodes_node = mem_pool->allocate_node( rapidxml::node_element, "nodes", "" );
            rapidxml::xml_node<> * conns_node = mem_pool->allocate_node( rapidxml::node_element, "connections", "" );

            nodes_node->append_attribute( _createXMLAttribute( "N", std::to_string( genotype.getNumNodes() ), mem_pool ) );
            nodes_node->append_attribute( _createXMLAttribute( "data", b64::Encode_NodeGenotype( genotype.getNodes() ), mem_pool) );
            genotype_node->append_node( nodes_node );

            conns_node->append_attribute( _createXMLAttribute( "N", std::to_string( genotype.getNumConnections() ), mem_pool ) );
            conns_node->append_attribute( _createXMLAttribute( "data", b64::Encode_ConnectionGenotype( genotype.getConnections() ), mem_pool) );
            genotype_node->append_node( conns_node );

            destination->append_node( genotype_node );
        }

        NetworkGenotype
        Decode_NetworkGenotype( rapidxml::xml_node<> * source )
        {
            std::vector< NodeDef > nodes;
            std::vector< ConnectionDef > conns;

            return make_genotype( nodes, conns );
        }
    }
}
