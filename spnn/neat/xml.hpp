#ifndef NEAT_XML_HPP_INCLUDED
#define NEAT_XML_HPP_INCLUDED

#include "../lib/rapidxml.hpp"

#include "neat.hpp"

namespace neat
{
    namespace b64
    {
        template < typename T >
        std::string Encode_DataStruct( const T& data );

        template < typename T >
        T Decode_DataStruct( const std::string& data );

        std::string Encode_NodeGenotype( const std::vector< NodeDef >& nodes );
        void Decode_NodeGenotype( size_t size, const std::string& encoded_nodes, std::vector< NodeDef >& out );

        std::string Encode_ConnectionGenotype( const std::vector< ConnectionDef >& conns );
        void Decode_ConnectionGenotype( size_t size, const std::string& encoded_conns, std::vector< ConnectionDef >& out );
    }

    namespace xml
    {
        void Encode_NetworkGenotype( const NetworkGenotype& genotype, rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool );
        NetworkGenotype Decode_NetworkGenotype( rapidxml::xml_node<> * source );
    }
}

#include "xml.inl"

#endif // NEAT_XML_HPP_INCLUDED
