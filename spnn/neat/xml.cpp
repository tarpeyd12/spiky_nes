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
        void
        Encode_MutationRates( const MutationRates& rates, rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )
        {
            auto mutationRates_node = Node( "mutation_rates", "", mem_pool );

            appendSimpleValueNode( "threshold_min", rates.thresholdMin, mutationRates_node, mem_pool );
            appendSimpleValueNode( "threshold_max", rates.thresholdMax, mutationRates_node, mem_pool );
            appendSimpleValueNode( "value_decay", rates.valueDecay, mutationRates_node, mem_pool );
            appendSimpleValueNode( "activation_decay", rates.activDecay, mutationRates_node, mem_pool );
            appendSimpleValueNode( "pulse_fast", rates.pulseFast, mutationRates_node, mem_pool );
            appendSimpleValueNode( "pulse_slow", rates.pulseSlow, mutationRates_node, mem_pool );
            appendSimpleValueNode( "weight", rates.weight, mutationRates_node, mem_pool );
            appendSimpleValueNode( "length", rates.length, mutationRates_node, mem_pool );

            // append to destination
            destination->append_node( mutationRates_node );
        }

        MutationRates
        Decode_MutationRates( rapidxml::xml_node<> * source )
        {
            if( !source || Name( source ) != "mutation_rates" )
            {
                return MutationRates{};
            }

            MutationRates rates;

            readSimpleValueNode( "threshold_min", rates.thresholdMin, source );
            readSimpleValueNode( "threshold_max", rates.thresholdMax, source );
            readSimpleValueNode( "value_decay", rates.valueDecay, source );
            readSimpleValueNode( "activation_decay", rates.activDecay, source );
            readSimpleValueNode( "pulse_fast", rates.pulseFast, source );
            readSimpleValueNode( "pulse_slow", rates.pulseSlow, source );
            readSimpleValueNode( "weight", rates.weight, source );
            readSimpleValueNode( "length", rates.length, source );

            return rates;
        }

        void
        Encode_MutationLimits( const MutationLimits& limits, rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )
        {
            auto mutationLimits_node = Node( "mutation_limits", "", mem_pool );

            appendMinMaxValueNode( "threshold_min", limits.thresholdMin, mutationLimits_node, mem_pool );
            appendMinMaxValueNode( "threshold_max", limits.thresholdMax, mutationLimits_node, mem_pool );
            appendMinMaxValueNode( "value_decay", limits.valueDecay, mutationLimits_node, mem_pool );
            appendMinMaxValueNode( "activation_decay", limits.activDecay, mutationLimits_node, mem_pool );
            appendMinMaxValueNode( "pulse_fast", limits.pulseFast, mutationLimits_node, mem_pool );
            appendMinMaxValueNode( "pulse_slow", limits.pulseSlow, mutationLimits_node, mem_pool );
            appendMinMaxValueNode( "weight", limits.weight, mutationLimits_node, mem_pool );
            appendMinMaxValueNode( "length", limits.length, mutationLimits_node, mem_pool );

            // append to destination
            destination->append_node( mutationLimits_node );
        }

        MutationLimits
        Decode_MutationLimits( rapidxml::xml_node<> * source )
        {
            if( !source || Name( source ) != "mutation_limits" )
            {
                return MutationLimits{};
            }

            MutationLimits limits;

            readMinMaxValueNode( "threshold_min", limits.thresholdMin, source );
            readMinMaxValueNode( "threshold_max", limits.thresholdMax, source );
            readMinMaxValueNode( "value_decay", limits.valueDecay, source );
            readMinMaxValueNode( "activation_decay", limits.activDecay, source );
            readMinMaxValueNode( "pulse_fast", limits.pulseFast, source );
            readMinMaxValueNode( "pulse_slow", limits.pulseSlow, source );
            readMinMaxValueNode( "weight", limits.weight, source );
            readMinMaxValueNode( "length", limits.length, source );

            return limits;
        }

        void
        Encode_SpeciesDistanceParameters( const SpeciesDistanceParameters& params, rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )
        {
            auto node = Node( "species_distance_parameters", "", mem_pool );

            appendSimpleValueNode( "excess", params.excess, node, mem_pool );
            appendSimpleValueNode( "disjoint", params.disjoint, node, mem_pool );
            appendSimpleValueNode( "weights", params.weights, node, mem_pool );
            appendSimpleValueNode( "lengths", params.lengths, node, mem_pool );
            appendSimpleValueNode( "activations", params.activations, node, mem_pool );
            appendSimpleValueNode( "decays", params.decays, node, mem_pool );
            appendSimpleValueNode( "pulses", params.pulses, node, mem_pool );
            appendSimpleValueNode( "nodes", params.nodes, node, mem_pool );
            appendSimpleValueNode( "threshold", params.threshold, node, mem_pool );

            // append to destination
            destination->append_node( node );
        }

        SpeciesDistanceParameters
        Decode_SpeciesDistanceParameters( rapidxml::xml_node<> * source )
        {
            if( !source || Name( source ) != "species_distance_parameters" )
            {
                return SpeciesDistanceParameters{};
            }

            SpeciesDistanceParameters params;

            readSimpleValueNode( "excess", params.excess, source );
            readSimpleValueNode( "disjoint", params.disjoint, source );
            readSimpleValueNode( "weights", params.weights, source );
            readSimpleValueNode( "lengths", params.lengths, source );
            readSimpleValueNode( "activations", params.activations, source );
            readSimpleValueNode( "decays", params.decays, source );
            readSimpleValueNode( "pulses", params.pulses, source );
            readSimpleValueNode( "nodes", params.nodes, source );
            readSimpleValueNode( "threshold", params.threshold, source );

            return params;
        }


        void
        Encode_NetworkGenotype( const NetworkGenotype& genotype, rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )
        {
            auto genotype_node = Node( "genotype", "", mem_pool );
            auto nodes_node    = Node( "nodes", "", mem_pool );
            auto conns_node    = Node( "connections", "", mem_pool );

            nodes_node->append_attribute( Attribute( "N", std::to_string( genotype.getNumNodes() ), mem_pool ) );
            nodes_node->append_attribute( Attribute( "data", b64::Encode_NodeGenotype( genotype.getNodes() ), mem_pool ) );
            genotype_node->append_node( nodes_node );

            conns_node->append_attribute( Attribute( "N", std::to_string( genotype.getNumConnections() ), mem_pool ) );
            conns_node->append_attribute( Attribute( "data", b64::Encode_ConnectionGenotype( genotype.getConnections() ), mem_pool ) );
            genotype_node->append_node( conns_node );

            // append to destination
            destination->append_node( genotype_node );
        }

        NetworkGenotype
        Decode_NetworkGenotype( rapidxml::xml_node<> * source )
        {
            if( !source || Name( source ) != "genotype" )
            {
                // error, wrong xml node type. return empty genotype
                return make_genotype( {}, {} );
            }

            std::vector< NodeDef > nodes;
            size_t num_nodes = 0;
            auto nodes_node = FindNode( "nodes", source );
            num_nodes = from_string<size_t>( Value( FindAttribute( "N", nodes_node ) ) );
            b64::Decode_NodeGenotype( num_nodes, Value( FindAttribute( "data", nodes_node ) ), nodes );

            std::vector< ConnectionDef > conns;
            size_t num_conns = 0;
            auto conns_node = FindNode( "connections", source );
            num_conns = from_string<size_t>( Value( FindAttribute( "N", conns_node ) ) );
            b64::Decode_ConnectionGenotype( num_conns, Value( FindAttribute( "data", conns_node ) ), conns );

            return make_genotype( nodes, conns );
        }
    }
}
