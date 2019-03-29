#ifndef NEAT_INNOVATION_GENERATOR_INL_INCLUDED
#define NEAT_INNOVATION_GENERATOR_INL_INCLUDED

namespace neat
{
    NodeDef
    InnovationGenerator::GetNextNode( double tMin, double tMax, double vDec, double aDec, uint64_t pF, uint64_t pS, NodeType type )
    {
        NodeDef out;

        // copy the given data into the output node
        out.thresholdMax = tMin;
        out.thresholdMin = tMax;
        out.valueDecay   = vDec;
        out.activDecay   = aDec;
        out.pulseFast    = pF;
        out.pulseSlow    = pS;
        out.type         = type;

        // set the innovation number, since it is a node, it does not really need a innovation id ( i think ).
        out.innovation = 0;

        // set the nodes ID, so that we can know which node is which
        out.ID = getNextNodeIDAndIncrement();

        return out;
    }

    ConnectionDef
    InnovationGenerator::GetNextConnection( NodeID src, NodeID dst, double weight, uint64_t length )
    {
        std::lock_guard< std::mutex > lock( generationConnections_mutex );

        ConnectionDef out;

        // copy the data into the output connection
        out.sourceID = src;
        out.destinationID = dst;
        out.weight = weight;
        out.length = length;

        out.enabled = true;

        // check if this generation already has this specific connection
        auto it = generationConnections.find( { out.sourceID, out.destinationID } );

        // if not
        if( it == generationConnections.end() )
        {
            // get the next innovation number, so that we can differentiate the innovations
            out.innovation = getNextInnovationIDAndIncrement();

            // add the connections specific innovation to the connection innovation tracker for later reference
            generationConnections[ { out.sourceID, out.destinationID } ] = out.innovation;
        }
        else
        {
            // we already have that connection and its innovation number!
            out.innovation = it->second;
        }

        return out;
    }

    void
    InnovationGenerator::clearGenerationConnections()
    {
        std::lock_guard< std::mutex > lock( generationConnections_mutex );
        generationConnections.clear();
    }

    InnovationID
    InnovationGenerator::getNextInnovationIDAndIncrement()
    {
        std::lock_guard< std::mutex > lock( innovationCounter_mutex );
        // return the current innovationID then increment
        return innovationCounter++;
    }

    InnovationID
    InnovationGenerator::getLastInnovationID() const
    {
        std::lock_guard< std::mutex > lock( innovationCounter_mutex );
        // return the current innovationID
        return innovationCounter;
    }

    NodeID
    InnovationGenerator::getNextNodeIDAndIncrement()
    {
        std::lock_guard< std::mutex > lock( nodeCounter_mutex );
        // return the current NodeID then increment it
        return nodeCounter++;
    }

    NodeID
    InnovationGenerator::getLastNodeID() const
    {
        std::lock_guard< std::mutex > lock( nodeCounter_mutex );
        // return the current NodeID
        return nodeCounter;
    }
}

#endif // NEAT_INNOVATION_GENERATOR_INL_INCLUDED
