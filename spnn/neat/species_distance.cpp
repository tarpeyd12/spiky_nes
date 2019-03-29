#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>

#include "species.hpp"

namespace neat
{
    double
    NetworkGenotypeDistance( const NetworkGenotype& networkA, const NetworkGenotype& networkB, const SpeciesDistanceParameters& params )
    {
        // the parameters must allow some sort of score to exist, so at leas one must be non-zero
        assert( params.excess != 0.0 || params.disjoint != 0.0 || params.weights != 0.0 || params.lengths != 0.0 || params.activations != 0.0 || params.decays != 0.0 || params.pulses != 0.0 || params.nodes != 0.0 );

        // normalization threshold, to stop smaller networks from having odd ratios
        //const size_t N_threshold = 20; // add to GenotypeDistanceParameters?

        double nodesDistance = 0.0;
        double connectionsDistance = 0.0;

        // connections
        if( params.excess != 0.0 || params.disjoint != 0.0 || params.weights != 0.0 || params.lengths != 0.0 ) // if params ignore connections, then don't calculate them
        {
            // the normalization value is set to the larger of the two genotypes sizes, so that they will be compared equally
            size_t N = std::max( networkA.connectionGenotype.size(), networkB.connectionGenotype.size() );

            // make sure that there are even connections to compare
            assert( N );

            // if the normalization factor is below the threshold, clamp to 1
            //if( N < N_threshold )
            {
                N = 1;
            }

            // maps to keep track of which connections have which innovationID, and a set to keep track of which innovationID's are present in either genotypes
            std::unordered_map< InnovationID, ConnectionDef * > netAconnections;
            std::unordered_map< InnovationID, ConnectionDef * > netBconnections;
            std::unordered_set< InnovationID > innovations;

            // keep track of the maximum innovationID, so that we may know which connections are excess connections instead of just disjoint connections
            InnovationID innovMaxA = 0;
            InnovationID innovMaxB = 0;

            // run through the connections in both networks, and associate the InnovationID's to a pointer to the connection itself
            {
                // for networkA
                for( const ConnectionDef& connection : networkA.connectionGenotype )
                {
                    // some foolery to get the pointer to the actual connectionDef
                    ConnectionDef * connDefPtr = const_cast< ConnectionDef* >( &connection );

                    // add the connection and its ID to the map
                    netAconnections.emplace( connection.innovation, connDefPtr );

                    // add the innovationID to the set
                    innovations.emplace( connection.innovation );

                    // make sure that we keep track of the max innovation number for networkA
                    innovMaxA = std::max( innovMaxA, connection.innovation );
                }

                // for networkB
                for( const ConnectionDef& connection : networkB.connectionGenotype )
                {
                    // some foolery to get the pointer to the actual connectionDef
                    ConnectionDef * connDefPtr = const_cast< ConnectionDef* >( &connection );

                    // add the connection and its ID to the map
                    netBconnections.emplace( connection.innovation, connDefPtr );

                    // add the innovationID to the set
                    innovations.emplace( connection.innovation );

                    // make sure that we keep track of the max innovation number for networkB
                    innovMaxB = std::max( innovMaxB, connection.innovation );
                }
            }

            double W = 0.0; // average difference of weight of connections shared between networks
            double L = 0.0; // average difference of length of connections shared between networks
            size_t E = 0;   // number of excess connections
            size_t D = 0;   // number of disjoint connections

            // process to find W, L, E, and D
            {
                // number of connections that are shared between both networks
                size_t numSame = 0;

                auto it = innovations.begin();

                // iterate through the set of all used innovationID's
                while( it != innovations.end() )
                {
                    // the current innovationID
                    InnovationID current = *it;

                    // ask if each network has that connections innovID in it
                    auto it_connA = netAconnections.find( current );
                    auto it_connB = netBconnections.find( current );

                    // if the network does have that innovationID, get the pointer to the connection it represents, otherwise null
                    ConnectionDef * connA = it_connA != netAconnections.end() ? it_connA->second : nullptr;
                    ConnectionDef * connB = it_connB != netBconnections.end() ? it_connB->second : nullptr;

                    // if innovation is in both genotypes
                    if( connA && connB )
                    {
                        W += fabs( connA->weight - connB->weight ); // sum the differences of weights
                        L += std::max( connA->length, connB->length ) - std::min( connA->length, connB->length ); // sum the differences of length
                        ++numSame; // count for averaging
                    }
                    // if current innovationID is in only one of the networks, and is greater than the max of the lesser of the two max innovationID's, then it is excess
                    else if( current > innovMaxA || current > innovMaxB )
                    {
                        // excess
                        ++E;
                    }
                    // if current innovationID is in only one network, and is less than the lesser of the maximum innovationID's in the two networks, then its disjoint
                    else
                    {
                        // disjoint
                        ++D;
                    }

                    // increment the iterator for the set;
                    ++it;
                }

                // average the sums
                W /= double( numSame );
                L /= double( numSame );
            }

            // use the parameter weights to give different importance to each factor in the "distance" between two networks
            connectionsDistance = ( params.excess * double( E ) ) / double( N ) + ( params.disjoint * double( D ) ) / double( N ) + params.weights * W + params.lengths * L;
        }

        // nodes
        if( params.activations != 0.0 || params.decays != 0.0 || params.pulses != 0.0 || params.nodes != 0.0 ) // if params ignores nodes, then don't calculate them
        {
            // NOTE(dot##10/22/2018): here we aren't considering excess or disjoint nodes, only similar nodes

            // the normalization value is set to the larger of the two genotypes sizes, so that they will be compared equally
            size_t N = std::max( networkA.nodeGenotype.size(), networkB.nodeGenotype.size() );

            // make sure that there are even connections to compare
            assert( N );

            // if the normalization factor is below the threshold, clamp to 1
            //if( N < N_threshold )
            {
                N = 1;
            }

            // node maps, note that the set contains the nodes present in both networks
            std::unordered_map< NodeID, NodeDef * > netAnodes;
            std::unordered_map< NodeID, NodeDef * > netBnodes;
            std::unordered_set< NodeID > commonNodes;

            double T = 0.0; // the average difference of the activation thresholds of the common nodes
            double D = 0.0; // the average difference of the decay rates of the common nodes
            double P = 0.0; // the average difference of the pulse rates of the common nodes
            size_t E = 0;   // the number of nodes that are different between both networks

            // set up the node maps
            {
                // set of the node ID's in network A
                std::unordered_set< NodeID > nodesInNetA;

                // iterate through the nodes in network A
                for( const NodeDef& node : networkA.nodeGenotype )
                {
                    // some foolery to get the pointer to the actual NodeDef
                    NodeDef * nodeDefPtr = const_cast< NodeDef* >( &node );

                    // associate the NodeID to the pointer to the node in the genotype
                    netAnodes.emplace( node.ID, nodeDefPtr );

                    // add the NodeID number to the set for network A
                    nodesInNetA.emplace( node.ID );
                }

                // iterate through the nodes in network B
                for( const NodeDef& node : networkB.nodeGenotype )
                {
                    // some foolery to get the pointer to the actual NodeDef
                    NodeDef * nodeDefPtr = const_cast< NodeDef* >( &node );

                    // associate the NodeID to the pointer to the node in the genotype
                    netBnodes.emplace( node.ID, nodeDefPtr );

                    // if the current nodeID of a node in network B, is also in network A
                    if( nodesInNetA.count( node.ID ) )
                    {
                        // then add it to the commonNodes set
                        commonNodes.emplace( node.ID );
                    }
                }

                // calculate the number of nodes that are not common to both networks
                E = ( networkA.nodeGenotype.size() + networkB.nodeGenotype.size() - ( 2 * commonNodes.size() ) );
            }

            // process to find T, D and P
            if( params.activations != 0.0 || params.decays != 0.0 || params.pulses != 0.0 ) // if we don't need to calculate, then don't!
            {
                // for each common node ID
                for( NodeID id : commonNodes )
                {
                    // get iterators to the node def pointers in the node maps
                    auto it_nodeA = netAnodes.find( id );
                    auto it_nodeB = netBnodes.find( id );

                    // make sure that they are actually in the map, if yes, then get that pointer!, otherwise nullptr
                    NodeDef * nodeA = it_nodeA != netAnodes.end() ? it_nodeA->second : nullptr;
                    NodeDef * nodeB = it_nodeB != netBnodes.end() ? it_nodeB->second : nullptr;

                    // if they were not in both node maps: oops
                    if( !nodeA || !nodeB )
                    {
                        // error, we should not be able to get here, but just in case, we can mostly safely skip this NodeID
                        continue;
                    }

                    // the difference of thresholdMin
                    T += fabs( std::min( nodeA->thresholdMax, nodeA->thresholdMin ) - std::min( nodeB->thresholdMax, nodeB->thresholdMin ) );
                    // the difference of thresholdMax
                    T += fabs( std::max( nodeA->thresholdMax, nodeA->thresholdMin ) - std::max( nodeB->thresholdMax, nodeB->thresholdMin ) );

                    // difference in value decay
                    D += fabs( nodeA->valueDecay - nodeB->valueDecay );
                    // difference in activation decay
                    D += fabs( nodeA->activDecay - nodeB->activDecay );

                    // difference in fast pulse frequency
                    P += std::max( nodeA->pulseFast, nodeB->pulseFast ) - std::min( nodeA->pulseFast, nodeB->pulseFast );
                    // difference in slow pulse frequency
                    P += std::max( nodeA->pulseSlow, nodeB->pulseSlow ) - std::min( nodeA->pulseSlow, nodeB->pulseSlow );
                }

                // average the sums
                T /= double( commonNodes.size() );
                D /= double( commonNodes.size() );
                P /= double( commonNodes.size() );
            }

            // use the parameter weights to give different importance to each factor in the "distance" between two networks
            nodesDistance = T * params.activations + D * params.decays + P * params.pulses + ( params.nodes * double( E ) ) / double( N );
        }

        // the returned distance between the networks, the sum of the node distance and the connection distance
        return connectionsDistance + nodesDistance;
    }
}
