#include <iostream>
#include <string>

#include <cassert>

#include "network.hpp"

namespace neat
{

    NetworkGenotype::NetworkGenotype()
         : nodeGenotype(), connectionGenotype()
    {
        /*  */
    }

    NetworkGenotype::NetworkGenotype( const NetworkGenotype& other )
         : nodeGenotype( other.nodeGenotype ), connectionGenotype( other.connectionGenotype )
    {
        /*  */
    }

    NetworkGenotype&
    NetworkGenotype::operator=( const NetworkGenotype& other )
    {
        // assign all variables in this to the values in other
        nodeGenotype = other.nodeGenotype;
        connectionGenotype = other.connectionGenotype;

        return *this;
    }

    size_t
    NetworkGenotype::getNumNodes() const
    {
        return nodeGenotype.size();
    }

    size_t
    NetworkGenotype::getNumConnections() const
    {
        return connectionGenotype.size();
    }

    size_t
    NetworkGenotype::getNumReachableNodes() const
    {
        return getOnlyReachableNodes().size();
    }


    size_t
    NetworkGenotype::gentNumActiveConnections() const
    {
        size_t count = 0;
        for( const ConnectionDef& connection : connectionGenotype )
        {
            if( connection.enabled )
            {
                ++count;
            }
        }

        return count;
    }

    void
    NetworkGenotype::getNumReachableNumActive( size_t& reachable, size_t& active ) const
    {
        auto active_conns = getOnlyActiveConnections();
        active = active_conns.size();
        reachable = getOnlyReachableNodes( active_conns ).size();
    }

    std::shared_ptr< NetworkPhenotype >
    NetworkGenotype::getNewNetworkPhenotype( uint64_t dTime ) const
    {
        std::shared_ptr< NetworkPhenotype > phenotype = std::shared_ptr< NetworkPhenotype >( new NetworkPhenotype( dTime ) );

        //assert( hasNoDuplicateNodeIDs() && "Genotype must not have duplicate multiple nodes with the same NodeID." );
        //assert( hasNoDuplicateConnectionInnovationIDs() && "Genotype must not have multiple connections with the same InnovationID." );

        // the simple way of adding everything
        /*{
            for( const NodeDef& node : nodeGenotype ) { phenotype->AddNode( node ); }

            std::vector< const ConnectionDef * > activeConnections = getOnlyActiveConnections();

            for( const ConnectionDef * conn : activeConnections )
            {
                phenotype->AddConnection( *conn );
            }
        }*/

        // the more complex way that gives a "faster" network. This way eliminates dead ends and islands in the network.
        {
            std::vector< const ConnectionDef * > activeConnections = getOnlyActiveConnections();

            std::vector< const NodeDef * > reachableNodes = getOnlyReachableNodes( activeConnections );

            std::set< NodeID > reachableNodeIDs;

            for( const NodeDef * node : reachableNodes )
            {
                phenotype->AddNode( *node );

                reachableNodeIDs.emplace( node->ID );
            }

            for( const ConnectionDef * conn : activeConnections )
            {
                if( reachableNodeIDs.count( conn->sourceID ) != 0 && reachableNodeIDs.count( conn->destinationID ) != 0 )
                {
                    phenotype->AddConnection( *conn );
                }
            }
        }

        // tell the network that we are done building it, so it can clean up its stuff
        phenotype->Finalize();

        return phenotype;
    }

    void
    NetworkGenotype::printGenotype( std::ostream& out, const std::string& name ) const
    {
        if( name.size() )
        {
            out << name << ":\n";
        }

        out << "{\n";
        out << "\tNodes\n\t{\n";
        {
            for( auto node : nodeGenotype )
            {
                out << "\t\t";
                out << "Node " << node.ID << ": { ";

                out << "innov = " << node.innovation << ", ";

                out << "type = ";
                switch( node.type )
                {
                    case NodeType::Hidden: out << "Hidden"; break;
                    case NodeType::Input: out << "Input"; break;
                    case NodeType::Output: out << "Output"; break;
                }

                out << ", tMin = " << node.thresholdMin << ", ";
                out << "tMax = " << node.thresholdMax << ", ";

                out << "vDec = " << node.valueDecay << ", ";
                out << "aDec = " << node.activDecay << ", ";

                out << "pFst = " << node.pulseFast << ", ";
                out << "pSlw = " << node.pulseSlow << ", ";

                out << " }\n";
            }
        }
        out << "\t}\n\tConnections\n\t{\n";
        {
            for( auto connection : connectionGenotype )
            {
                out << "\t\t";
                out << "Connection: { ";

                out << "innov = " << connection.innovation << ", ";

                out << "src = " << connection.sourceID << ", ";
                out << "dst = " << connection.destinationID << ", ";

                out << "weight = " << connection.weight << ", ";
                out << "length = " << connection.length << ", ";

                out << "enabled = " << connection.enabled;

                out << " }\n";
            }
        }
        out << "\t}\n";
        out << "}\n\n";
    }


    bool
    NetworkGenotype::hasNoDuplicateNodeIDs() const
    {
        std::map< NodeID, uint8_t > idCount; // a byte because we don't want to use too much memory

        for( const NodeDef& node : nodeGenotype )
        {
            if( ++idCount[ node.ID ] > 1 )
            {
                return false;
            }
        }

        return true;
    }

    bool
    NetworkGenotype::hasNoDuplicateConnectionInnovationIDs() const
    {
        std::map< InnovationID, uint8_t > idCount; // a byte because we don't want to use too much memory

        for( const ConnectionDef& conn : connectionGenotype )
        {
            if( ++idCount[ conn.innovation ] > 1 )
            {
                return false;
            }
        }

        return true;
    }

    std::vector< const ConnectionDef * >
    NetworkGenotype::getOnlyActiveConnections() const
    {
        std::vector< const ConnectionDef * > output;
        output.reserve( connectionGenotype.size() );

        for( const ConnectionDef& connection : connectionGenotype )
        {
            if( connection.enabled )
            {
                output.emplace_back( &connection );
            }
        }

        return output;
    }


    void
    NetworkGenotype::getNodesByType( std::vector< const NodeDef * >& inputNodes, std::vector< const NodeDef * >& outputNodes, std::vector< const NodeDef * >& hiddenNodes ) const
    {
        inputNodes.clear();
        outputNodes.clear();
        hiddenNodes.clear();

        for( const NodeDef& node : nodeGenotype )
        {
            switch( node.type )
            {
                case NodeType::Input:   inputNodes.emplace_back( &node ); break;
                case NodeType::Output: outputNodes.emplace_back( &node ); break;
                case NodeType::Hidden: hiddenNodes.emplace_back( &node ); break;
                default: break;
            }
        }

        inputNodes.shrink_to_fit();
        outputNodes.shrink_to_fit();
        hiddenNodes.shrink_to_fit();
    }

    std::vector< const NodeDef * >
    NetworkGenotype::getOnlyReachableNodes() const
    {
        return getOnlyReachableNodes( getOnlyActiveConnections() );
    }

    std::vector< const NodeDef * >
    NetworkGenotype::getOnlyReachableNodes( const std::vector< const ConnectionDef * >& activeConnections ) const
    {

        assert( hasNoDuplicateNodeIDs() && "Genotype must not have duplicate multiple nodes with the same NodeID." );
        assert( hasNoDuplicateConnectionInnovationIDs() && "Genotype must not have multiple connections with the same InnovationID." );

        std::vector< const NodeDef * > inputNodes, outputNodes, hiddenNodes;
        getNodesByType( inputNodes, outputNodes, hiddenNodes );

        std::map< NodeID, const NodeDef * > nodeIDMap;
        {
            for( const NodeDef * node :  inputNodes ) { nodeIDMap[ node->ID ] = node; }
            for( const NodeDef * node : outputNodes ) { nodeIDMap[ node->ID ] = node; }
            for( const NodeDef * node : hiddenNodes ) { nodeIDMap[ node->ID ] = node; }
        }

        std::map< NodeID, std::list< const ConnectionDef * > > connectionsToNodeID;
        std::map< NodeID, std::list< const ConnectionDef * > > connectionsFromNodeID;

        for( const ConnectionDef * conn : activeConnections )
        {
            // make sure that the pointers given to this function are at the least not null.
            assert( conn != nullptr && "Pointer to ConnectionDef given to getOnlyReachableNodes must not be null." );

            connectionsFromNodeID[ conn->sourceID ].emplace_back( conn );
            connectionsToNodeID[ conn->destinationID ].emplace_back( conn );
        }

        // the set of nodes that are touched when traversing the connections from the input nodes to the output nodes
        std::set< NodeID > nodeIDsGoingForward;
        {
            std::queue< const ConnectionDef * > connectionsToProcess;
            {
                // add the input node connections to the queue
                for( const NodeDef * node : inputNodes )
                {
                    for( const ConnectionDef * conn : connectionsFromNodeID[ node->ID ] )
                    {
                        connectionsToProcess.push( conn );
                    }
                }

                // process all the connections in the queue, adding the NodeID of the connections destination, and that nodes outgoing connections to the queue
                while( !connectionsToProcess.empty() )
                {
                    const ConnectionDef * connection = connectionsToProcess.front();
                    connectionsToProcess.pop();

                    NodeID destinationID = connection->destinationID;

                    // if the id is not in the set of ID's, add it, and its connections
                    if( nodeIDsGoingForward.count( destinationID ) == 0 )
                    {
                        // add the connections destination to the set of nodes.
                        nodeIDsGoingForward.emplace( destinationID );

                        // add the destinations list of connections to the queue of connections
                        for( const ConnectionDef * conn : connectionsFromNodeID[ destinationID ] )
                        {
                            connectionsToProcess.push( conn );
                        }
                    }
                }
            }
        }

        // the set of nodes that are touched when traversing the connections from the output nodes to the input nodes
        std::set< NodeID > nodeIDsGoingBackward;
        {
            std::queue< const ConnectionDef * > connectionsToProcess;
            {
                // add the output node connections to the queue
                for( const NodeDef * node : outputNodes )
                {
                    for( const ConnectionDef * conn : connectionsToNodeID[ node->ID ] )
                    {
                        connectionsToProcess.push( conn );
                    }
                }

                // process all the connections in the queue, adding the NodeID of the connections source, and that nodes outgoing connections to the queue
                while( !connectionsToProcess.empty() )
                {
                    const ConnectionDef * connection = connectionsToProcess.front();
                    connectionsToProcess.pop();

                    NodeID sourceID = connection->sourceID;

                    // if the id is not in the set of ID's, add it, and its connections
                    if( nodeIDsGoingBackward.count( sourceID ) == 0 )
                    {
                        // add the connections source to the set of nodes.
                        nodeIDsGoingBackward.emplace( sourceID );

                        // add the sources list of connections to the queue of connections
                        for( const ConnectionDef * conn : connectionsToNodeID[ sourceID ] )
                        {
                            connectionsToProcess.push( conn );
                        }
                    }
                }
            }
        }

        std::set< NodeID > commonNodeIDs;

        // find the common nodes to both sets of NodeIDs with a set intersection.
        std::set_intersection( nodeIDsGoingForward.begin(), nodeIDsGoingForward.end(), nodeIDsGoingBackward.begin(), nodeIDsGoingBackward.end(), std::inserter( commonNodeIDs, commonNodeIDs.begin() ) );

        for( const NodeDef * node :  inputNodes ) { commonNodeIDs.emplace( node->ID ); }
        for( const NodeDef * node : outputNodes ) { commonNodeIDs.emplace( node->ID ); }

        // finally add the nodes with the correct NodeIDs to the returned vector
        std::vector< const NodeDef * > reachableNodes;

        for( const NodeID id : commonNodeIDs )
        {
            reachableNodes.emplace_back( nodeIDMap[ id ] );
        }

        return reachableNodes;
    }
}










