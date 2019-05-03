#ifndef NEAT_INNOVATION_GENERATOR_HPP_INCLUDED
#define NEAT_INNOVATION_GENERATOR_HPP_INCLUDED

#include <mutex>
#include <map>

namespace neat
{
    class InnovationGenerator;
}

#include "neat.hpp"
#include "xml.hpp"

namespace neat
{
    class InnovationGenerator
    {
        private:

            mutable std::mutex innovationCounter_mutex;
            InnovationID innovationCounter;

            mutable std::mutex nodeCounter_mutex;
            NodeID nodeCounter;

            std::mutex generationConnections_mutex;
            std::map< std::pair< NodeID, NodeID >, InnovationID > generationConnections;

        public:

            InnovationGenerator() : innovationCounter_mutex(), innovationCounter( 0 ), nodeCounter_mutex(), nodeCounter( 0 ), generationConnections_mutex(), generationConnections() { /*  */ }

            inline NodeDef GetNextNode( double tMin, double tMax, double vDec, double aDec, uint64_t pF, uint64_t pS, NodeType type );
            inline ConnectionDef GetNextConnection( NodeID src, NodeID dst, double weight, uint64_t length );

            inline void clearGenerationConnections();

            void SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool );

        protected:

            inline InnovationID getNextInnovationIDAndIncrement();
            inline InnovationID getLastInnovationID() const;

            inline NodeID getNextNodeIDAndIncrement();
            inline NodeID getLastNodeID() const;
    };
}

#include "innovation_generator.inl"

#endif // NEAT_INNOVATION_GENERATOR_HPP_INCLUDED
