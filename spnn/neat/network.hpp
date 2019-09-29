#ifndef NEAT_NETWORK_HPP_INCLUDED
#define NEAT_NETWORK_HPP_INCLUDED

#include <iostream>
#include <set>
#include <vector>
#include <map>

namespace neat
{
    class NetworkPhenotype;
    class NetworkGenotype;
}

#include "species.hpp"
#include "population.hpp"
#include "mutations.hpp"
#include "neat.hpp"

namespace neat
{
    class NetworkPhenotype : protected spnn::network
    {
        private:

            // data

            std::vector< spnn::neuron >  neuronData;
            std::vector< spnn::neuron* > inputNeurons;
            std::vector< spnn::neuron* > outputNeurons;

        protected:

            // construction structures

            // TODO(dot##11/1/2018): put these variables into a separate structure to cut down on NetworkPhenotype data footprint

            std::map< NodeID, size_t >   neuronIDMap; // for quickly finding a node with given ID in construction of the connections
            std::vector< NodeID >        neuronIDs;
            std::vector< NodeType >      neuronTypes;

        public:

            // constructors

            NetworkPhenotype();

            // destructor

            virtual ~NetworkPhenotype();

            // getters

            size_t numInputs() const;
            size_t numOutputs() const;
            size_t numNeurons() const;

            void printNetworkState( std::ostream& out ) const;

            using spnn::network::Time;
            using spnn::network::DeltaTime;
            using spnn::network::QueueSize;

            using spnn::network::PulsesProcessed;
            using spnn::network::NeuronsProcessed;

            using spnn::network::PulsesProcessedLastTick;
            using spnn::network::NeuronsProcessedLastTick;

        protected:

            // constructors

            NetworkPhenotype( uint64_t dTime );

            // construction functions

            void AddNode( NodeDef nodeDefinition );
            void AddNode( NodeID nodeID, NodeType type, const spnn::neuron& neuron );
            void AddConnection( ConnectionDef connDefinition );
            void AddConnection( NodeID srcID, NodeID dstID, double weight, uint64_t len );

            void Finalize();

            // setters

            bool setInputValues( const std::vector< double >& values );
            bool setOutputCallbacks( const std::vector< NodeCallback >& callbacks );

            size_t QueuePulse( spnn::neuron * neuron, double value, uint64_t arrivalTime = 0 );

            void resetNetworkState();

        private:

            // friends

            friend class NetworkGenotype;
            friend class Population;
            friend class FitnessCalculator;
    };

    class NetworkGenotype
    {
        protected:

            // structure

            std::vector< NodeDef >          nodeGenotype;
            std::vector< ConnectionDef >    connectionGenotype;

            SpeciesID parentSpeciesID;

        public:

            // constructors

            NetworkGenotype();
            NetworkGenotype( const NetworkGenotype& other );

            // destructor

            virtual ~NetworkGenotype() = default;

            // assignment op

            NetworkGenotype& operator=( const NetworkGenotype& other );

            // getters

            size_t getNumNodes() const;
            size_t getNumConnections() const;

            SpeciesID getParentSpeciesID() const;

            const std::vector< NodeDef >& getNodes() const;
            const std::vector< ConnectionDef >& getConnections() const;

            size_t getNumReachableNodes() const;
            size_t gentNumActiveConnections() const;
            void getNumReachableNumActive( size_t& reachable, size_t& active ) const;

            std::shared_ptr< NetworkPhenotype > getNewNetworkPhenotype( uint64_t dTime = 1 ) const;

            void printGenotype( std::ostream& out = std::cout, const std::string& name = "" ) const;

        protected:

            // conformity checks

            bool hasNoDuplicateNodeIDs() const;
            bool hasNoDuplicateConnectionInnovationIDs() const;

            // protected getters

            std::vector< const ConnectionDef * > getOnlyActiveConnections() const;
            void getNodesByType( std::vector< const NodeDef * >& inputNodes, std::vector< const NodeDef * >& outputNodes, std::vector< const NodeDef * >& hiddenNodes ) const;

            std::vector< const NodeDef * > getOnlyReachableNodes() const;
            std::vector< const NodeDef * > getOnlyReachableNodes( const std::vector< const ConnectionDef * >& activeConnections ) const;

            // friends

            friend class Population;
            friend double NetworkGenotypeDistance( const NetworkGenotype&, const NetworkGenotype&, const SpeciesDistanceParameters& );
            friend class SpeciesManager;
            friend class Mutations::Mutation_base;
            friend NetworkGenotype SpliceGenotypes( const std::vector< const NetworkGenotype* >& genotypes, std::shared_ptr< Rand::RandomFunctor > rand );
            friend NetworkGenotype make_genotype( const std::vector< NodeDef >& nodes, const std::vector< ConnectionDef >& conns, const SpeciesID parent_species_id );
    };

    NetworkGenotype make_genotype( const std::vector< NodeDef >& nodes, const std::vector< ConnectionDef >& conns, const SpeciesID parent_species_id = 0 );
}

#endif // NEAT_NETWORK_HPP_INCLUDED
