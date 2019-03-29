
#include <map>
#include <set>

#include "splice.hpp"

namespace neat
{
    NetworkGenotype
    SpliceGenotypes( const NetworkGenotype& parent1, const NetworkGenotype& parent2, std::shared_ptr< Rand::RandomFunctor > rand )
    {
        return SpliceGenotypes( { &parent1, &parent2 }, rand );
    }

    NetworkGenotype
    SpliceGenotypes( const std::vector< const NetworkGenotype* >& genotypes, std::shared_ptr< Rand::RandomFunctor > rand )
    {
        // data we need to gather about each genotype given
        struct genotype_data
        {
            const NetworkGenotype *                         genotype_ptr;
            std::map< NodeID, const NodeDef * >             node_map;
            std::map< InnovationID, const ConnectionDef * > innov_map;
            InnovationID                                    max_innov_id;

            // boilerplate for the struct
            genotype_data() : genotype_ptr( nullptr ), node_map(), innov_map(), max_innov_id( 0 ) { }
            genotype_data( const genotype_data& ) = default; // TODO(dot##1/7/2019): should these actually be default?
            genotype_data& operator=( const genotype_data& ) = default; // TODO(dot##1/7/2019): should these actually be default?

            // constructor to extract the information from the given genotype pointer
            genotype_data( const NetworkGenotype * genotype, std::set< NodeID >& all_node_ids, std::set< InnovationID >& all_innov_ids )
                 : genotype_ptr( genotype ), node_map(), innov_map(), max_innov_id( 0 )
            {
                // go through the nodes in the genotype
                for( const auto& node : genotype_ptr->nodeGenotype )
                {
                    // add the node id to the total set of node ids
                    all_node_ids.emplace( node.ID );

                    // map the node id to the actual node def
                    node_map.emplace( node.ID, &node );
                }

                // go through the connections in the genotype
                for( const auto& conn : genotype_ptr->connectionGenotype )
                {
                    // add the innovation id to the  total set of innovation ids
                    all_innov_ids.emplace( conn.innovation );

                    // map the connection to the id in the extra data
                    innov_map.emplace( conn.innovation, &conn );

                    // track the max innovation id for excess finding
                    max_innov_id = std::max( max_innov_id, conn.innovation );
                }
            }
        };

        // make sure we have a functioning random number generator
        if( !rand ) rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

        // the total sets of all the given nodeID's and innovationID's
        std::set< NodeID >       all_node_ids;
        std::set< InnovationID > all_innov_ids;
        InnovationID             all_max_innov_id = 0;

        // list of genotype extracted data
        std::vector< genotype_data > genenotype_extra_data;

        // extract the data for each genotype
        for( const NetworkGenotype * genotype : genotypes )
        {
            // pass the genotype pointer to the data struct, it will extract all the info, and add relevant stuff to all_node_ids and all_innov_ids
            genotype_data data( genotype, all_node_ids, all_innov_ids );

            // get the total max innovation id
            all_max_innov_id = std::max( all_max_innov_id, data.max_innov_id );

            // add the extracted genotype data to the list of extracted genotype data structs
            genenotype_extra_data.push_back( data );
        }

        NetworkGenotype output;

        // construct the genotype
        {
            // add a node for each present NodeID, randomly selected from the input genotypes that have said NodeID
            for( NodeID id : all_node_ids )
            {
                std::vector< const NodeDef * > available_nodes;

                for( const auto& gdata : genenotype_extra_data )
                {
                    auto it = gdata.node_map.find( id );
                    if( it != gdata.node_map.end() && it->second != nullptr )
                    {
                        available_nodes.push_back( it->second );
                    }
                }

                if( available_nodes.empty() )
                {
                    // error
                }

                // add the randomly selected node to the output nodes
                output.nodeGenotype.push_back( *available_nodes[ rand->Int( 0, available_nodes.size()-1 ) ] );
            }

            // repeat for the connections and InnovationID's
            for( InnovationID id : all_innov_ids )
            {
                std::vector< const ConnectionDef * > available_conns;

                for( const auto& gdata : genenotype_extra_data )
                {
                    auto it = gdata.innov_map.find( id );
                    if( it != gdata.innov_map.end() && it->second != nullptr )
                    {
                        available_conns.push_back( it->second );
                    }
                }

                if( available_conns.empty() )
                {
                    // error
                }

                // add the randomly selected connection to the output connections
                output.connectionGenotype.push_back( *available_conns[ rand->Int( 0, available_conns.size()-1 ) ] );
            }
        }

        return output;
    }

}
