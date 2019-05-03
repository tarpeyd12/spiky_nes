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

        template < typename T >
        std::string Encode_VectorData( const std::vector<T>& data );
        template < typename T >
        void Decode_VectorData( const std::string& encoded, size_t size, std::vector<T>& out );

        std::string Encode_NodeGenotype( const std::vector< NodeDef >& nodes );
        void Decode_NodeGenotype( size_t size, const std::string& encoded_nodes, std::vector< NodeDef >& out );

        std::string Encode_ConnectionGenotype( const std::vector< ConnectionDef >& conns );
        void Decode_ConnectionGenotype( size_t size, const std::string& encoded_conns, std::vector< ConnectionDef >& out );
    }

    namespace xml
    {
        template < typename T >
        T from_string( const std::string& s );

        inline std::string Name( const rapidxml::xml_base<> * base );
        inline std::string Value( const rapidxml::xml_base<> * base );
        inline char * Str( const std::string& input, rapidxml::memory_pool<> * mem_pool );
        inline rapidxml::xml_node<> * Node( const char * name, const std::string& data, rapidxml::memory_pool<> * mem_pool, rapidxml::node_type type = rapidxml::node_element );
        inline rapidxml::xml_attribute<> * Attribute( const char * name, const std::string& data, rapidxml::memory_pool<> * mem_pool );

        inline rapidxml::xml_node<> * FindNode( const std::string& name, rapidxml::xml_node<> * node );
        inline rapidxml::xml_attribute<> * FindAttribute( const std::string& name, rapidxml::xml_node<> * node );


        template < typename T >
        inline void appendSimpleValueNode( const char * name, const T& value, rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool );
        template < typename T >
        inline void readSimpleValueNode( const char * name, T& value, rapidxml::xml_node<> * source );

        template < typename T >
        inline void appendMinMaxValueNode( const char * name, const MinMax<T>& value, rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool );
        template < typename T >
        inline void readMinMaxValueNode( const char * name, MinMax<T>& value, rapidxml::xml_node<> * source );

        template < typename T >
        void appendVectorData( const char * name, const std::vector<T>& data, rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool );
        template < typename T >
        void readVectorData( const char * name, std::vector<T>& data, rapidxml::xml_node<> * source );

        void Encode_MutationRates( const MutationRates& rates, rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool );
        MutationRates Decode_MutationRates( rapidxml::xml_node<> * source );

        void Encode_MutationLimits( const MutationLimits& limits, rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool );
        MutationLimits Decode_MutationLimits( rapidxml::xml_node<> * source );

        void Encode_SpeciesDistanceParameters( const SpeciesDistanceParameters& params, rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool );
        SpeciesDistanceParameters Decode_SpeciesDistanceParameters( rapidxml::xml_node<> * source );

        void Encode_NetworkGenotype( const NetworkGenotype& genotype, rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool );
        NetworkGenotype Decode_NetworkGenotype( rapidxml::xml_node<> * source );
    }
}

#include "xml.inl"

#endif // NEAT_XML_HPP_INCLUDED
