#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <iostream>

#include "mutations.hpp"

namespace neat
{
    namespace Mutations
    {

        // FIXME(dot##11/25/2018): if the same structural mutation occurs two times, it will be counted as two separate innovations. this i think is incorrect behavior

        uint64_t
        Mutation_Add_node::operator()(  NetworkGenotype& genotypeToMutate, InnovationGenerator& innovationTracker, const MutationRates&, const MutationLimits& limits, std::shared_ptr< Rand::RandomFunctor > rand ) const
        {
            if( !rand ) rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

            // get references for the genotypes nodes and connections
            auto& nodeList = GetNodeList( genotypeToMutate );
            auto& connList = GetConnList( genotypeToMutate );

            // need at least 1 connection to work
            if( connList.empty() )
            {
                return 0;
            }

            // add the node
            {
                // randomly get a connection
                ConnectionDef& originalConnection = connList[ rand->Int( 0, connList.size() - 1 ) ];

                // define the values within the prescribed limits to give the new node
                double tMin = rand->Float( limits.thresholdMin.min, limits.thresholdMin.max );
                double tMax = rand->Float( limits.thresholdMax.min, limits.thresholdMax.max );
                double vDec = rand->Float( limits.valueDecay.min, limits.valueDecay.max );
                double aDec = rand->Float( limits.activDecay.min, limits.activDecay.max );
                uint64_t pF = rand->Int( limits.pulseFast.min, limits.pulseFast.max );
                uint64_t pS = rand->Int( limits.pulseSlow.min, limits.pulseSlow.max );

                // add the node and get a reference to it
                nodeList.push_back( innovationTracker.GetNextNode( tMin, tMax, vDec, aDec, pF, pS, NodeType::Hidden ) );
                const NodeDef& nodeAdded = nodeList.back();

                // connect the node
                {
                    // disable the connection to force the new node to be used
                    originalConnection.enabled = false;

                    // from the original connections source to the new node
                    auto c1 = innovationTracker.GetNextConnection( originalConnection.sourceID, nodeAdded.ID, limits.weight.max, limits.length.clamp( originalConnection.length/2 ) );

                    // from the new node to the original connections destination
                    auto c2 = innovationTracker.GetNextConnection( nodeAdded.ID, originalConnection.destinationID, originalConnection.weight, limits.length.clamp( originalConnection.length/2 ) );

                    // now we actually add the connections
                    connList.push_back( c1 );
                    connList.push_back( c2 );
                }
            }

            return 1;
        }

        uint64_t
        Mutation_Add_conn::operator()(  NetworkGenotype& genotypeToMutate, InnovationGenerator& innovationTracker, const MutationRates&, const MutationLimits& limits, std::shared_ptr< Rand::RandomFunctor > rand ) const
        {
            if( !rand ) rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

            auto& nodeList = GetNodeList( genotypeToMutate );

            // can't add a connection to a genotype with no nodes
            if( nodeList.empty() )
            {
                return 0;
            }

            // setup and add the connection
            {

                // input nodes can only be connection sources, output nodes can only be connection destinations, hidden nodes can be either
                std::vector< NodeID > possibleSrcIDs;
                std::vector< NodeID > possibleDstIDs;
                possibleSrcIDs.reserve( nodeList.size() );
                possibleDstIDs.reserve( nodeList.size() );

                for( const auto& node : nodeList )
                {
                    if( node.type == NodeType::Hidden )
                    {
                        possibleSrcIDs.emplace_back( node.ID );
                        possibleDstIDs.emplace_back( node.ID );
                        continue;
                    }

                    if( node.type == NodeType::Input )
                    {
                        possibleSrcIDs.emplace_back( node.ID );
                        continue;
                    }

                    if( node.type == NodeType::Output )
                    {
                        possibleDstIDs.emplace_back( node.ID );
                        continue;
                    }
                    // error
                }

                // must be able to create connection
                if( possibleSrcIDs.empty() || possibleDstIDs.empty() )
                {
                    return 0;
                }

                NodeID srcID    = possibleSrcIDs[ rand->Int( 0, possibleSrcIDs.size() - 1 ) ];
                NodeID dstID    = possibleDstIDs[ rand->Int( 0, possibleDstIDs.size() - 1 ) ];
                double weight   = rand->Float( limits.weight.min, limits.weight.max );
                uint64_t length = rand->Int( limits.length.min, limits.length.max );

                GetConnList( genotypeToMutate ).push_back( innovationTracker.GetNextConnection( srcID, dstID, weight, length ) );
            }

            return 1;
        }


        uint64_t
        Mutation_Add_conn_unique::operator()(  NetworkGenotype& genotypeToMutate, InnovationGenerator& innovationTracker, const MutationRates&, const MutationLimits& limits, std::shared_ptr< Rand::RandomFunctor > rand ) const
        {
            // TODO(dot##1/17/2019): Fix the comments, they currently do not make sense

            if( !rand ) rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

            // get references for the genotypes nodes and connections
            auto& nodeList = GetNodeList( genotypeToMutate );
            auto& connList = GetConnList( genotypeToMutate );

            // can't add a connection to a genotype with no nodes
            if( nodeList.empty() )
            {
                return 0;
            }

            // setup and add the connection
            {
                // the map contains the sets of all available destination ID's given the key ID as a source
                std::unordered_map< NodeID, std::set< NodeID > > unavailableDestSetMap;

                // this set contains the source node ID's that have available destinations
                std::set< NodeID > sourceIDsWithAvailableDests;

                // get the set of all node ID's, as these will represent all the possible sources and destinations
                std::set<NodeID> allNodeIDs;
                std::vector<NodeID> inputNodeIDs;
                std::vector<NodeID> outputNodeIDs;
                for( const auto& node : nodeList )
                {
                    allNodeIDs.emplace( node.ID );
                    if( node.type == NodeType::Input )
                    {
                        inputNodeIDs.emplace_back( node.ID );
                    }
                    else if( node.type == NodeType::Output )
                    {
                        outputNodeIDs.emplace_back( node.ID );
                    }
                }

                // set up unavailableDestSetMap and sourceIDsWithAvailableDests
                {
                    for( const auto& conn : connList )
                    {
                        unavailableDestSetMap[ conn.sourceID ].emplace( conn.destinationID );
                    }

                    // TODO(dot##2/22/2019): figure out a way to not duplicate the unavailability of the input node id's, as this will use a lot of memory for large numbers of input nodes
                    // add all the input node ids to the set of unavailible destination ID's
                    for( auto& udsm : unavailableDestSetMap )
                    {
                        for( NodeID id : inputNodeIDs )
                        {
                            udsm.second.emplace( id );
                        }
                    }

                    // get the ID's of the possible source ID's whose sets of possible destination ID's are not empty
                    for( const auto it : unavailableDestSetMap )
                    {
                        if( allNodeIDs.size() - it.second.size() > 1 ) // 1 account for removal of source id
                        {
                            sourceIDsWithAvailableDests.emplace( it.first );
                        }
                    }

                    // remove the output nodes from the set of available destination ids
                    for( NodeID id : outputNodeIDs )
                    {
                        if( sourceIDsWithAvailableDests.count( id ) > 0 )
                        {
                            sourceIDsWithAvailableDests.erase( id );
                        }
                    }

                    // if the set of source ID's whose destination ID sets are not empty, is empty, then the genotype represents a fully connected network and there are no new unique connections to add
                    if( sourceIDsWithAvailableDests.empty() )
                    {
                        return 0;
                    }
                }

                // randomly select a source ID from the available ID's
                NodeID srcID    = *std::next( sourceIDsWithAvailableDests.begin(), rand->Int( 0, sourceIDsWithAvailableDests.size() - 1 ) );

                // get the set of unavailable destination ids for the selected source id
                auto invDestinationSet = unavailableDestSetMap[ srcID ];
                unavailableDestSetMap.clear();

                auto destinationSet = std::move( allNodeIDs );

                {
                    //destinationSet.

                    // remove the source id
                    //auto it = destinationSet.find( srcID );
                    if( auto it = destinationSet.find( srcID ) != destinationSet.end() )
                    {
                        destinationSet.erase( it );
                    }

                    for( auto id : invDestinationSet )
                    {
                        if( auto it = destinationSet.find( id ) != destinationSet.end() )
                        {
                            destinationSet.erase( it );
                        }
                    }
                }

                // just in case we screwed up and there is actually no available destination ids, then return false to signal an error
                if( destinationSet.empty() )
                {
                    return 0;
                }


                // randomly select a destination ID from the available ID's
                NodeID dstID    = *std::next( destinationSet.begin(), rand->Int( 0, destinationSet.size() - 1 ) );

                // set the random connection parameters within the prescribed limits
                double weight   = rand->Float( limits.weight.min, limits.weight.max );
                uint64_t length = rand->Int( limits.length.min, limits.length.max );

                // finally add the unique connection
                connList.push_back( innovationTracker.GetNextConnection( srcID, dstID, weight, length ) );

            }

            // Yay we succeeded!
            return 1;
        }

        uint64_t
        Mutation_Add_conn_dup::operator()(  NetworkGenotype& genotypeToMutate, InnovationGenerator& innovationTracker, const MutationRates&, const MutationLimits& limits, std::shared_ptr< Rand::RandomFunctor > rand ) const
        {
            if( !rand ) rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

            auto& connList = GetConnList( genotypeToMutate );

            // can't duplicate a connection to a genotype with no connections
            // we should also not duplicate a connection if there aren't enough different lengths
            if( connList.empty() || limits.length.range() < 1 )
            {
                return 0;
            }

            // setup and add the connection
            {
                const auto& original_connection = connList[ rand->Int( 0, connList.size() - 1 ) ];

                NodeID srcID    = original_connection.sourceID;
                NodeID dstID    = original_connection.destinationID;
                double weight   = rand->Float( limits.weight.min, limits.weight.max );
                uint64_t length;

                do
                {
                    length = rand->Int( limits.length.min, limits.length.max );
                }
                while( length == original_connection.length && limits.length.range() > 0 );

                auto next_connextion = innovationTracker.GetNextConnection( srcID, dstID, weight, length );

                //next_connextion.enabled = original_connection.enabled;

                connList.push_back( next_connextion );
            }

            return 1;
        }

    }
}
