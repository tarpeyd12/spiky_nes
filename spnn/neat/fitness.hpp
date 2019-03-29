#ifndef NEAT_FITNESS_HPP_INCLUDED
#define NEAT_FITNESS_HPP_INCLUDED

#include <functional>
#include <vector>

namespace neat
{
    class FitnessCalculator;
    class FitnessFactory;
    class NodeFitnessCallback;
}

#include "network.hpp"
#include "population.hpp"
#include "neat.hpp"

namespace neat
{
    class FitnessCalculator
    {
        private:

            std::shared_ptr< NetworkPhenotype > network;

        public:

            FitnessCalculator( std::shared_ptr< NetworkPhenotype > net );
            virtual ~FitnessCalculator();

            virtual long double getFitnessScore() const = 0;
            virtual uint64_t getMaxNumTimesteps() const = 0;

            virtual bool stopTest() const = 0;
            virtual void testTick( uint64_t time ) = 0;

            virtual uint64_t getInputValueCheckCadence() const = 0; // 0 means only once on init, 1 is every tick 2, every other etc.
            virtual std::vector< double > getInputValues( uint64_t time ) = 0;

            virtual std::vector< NodeCallback > getNodeCallbacks() = 0;

        protected:

            size_t getNumInputNodes() const;
            size_t getNumOutputNodes() const;

        private:

            uint64_t Run();
            bool canRun() const;
            void clearNetwork();

            friend class Population;

            FitnessCalculator( FitnessCalculator& ) = delete;
            FitnessCalculator& operator=( FitnessCalculator& ) = delete;
    };

    class FitnessFactory
    {
        public:

            FitnessFactory() = default;
            virtual ~FitnessFactory() = default;

            virtual std::shared_ptr< FitnessCalculator > getNewFitnessCalculator( std::shared_ptr< NetworkPhenotype > net, size_t testNum ) const = 0;
            virtual size_t numTimesToTest() const { return 1; }
    };

    class NodeFitnessCallback
    {
        private:

            NodeID outputNodeIDNum;
            FitnessCalculator * fitness;

        public:

            NodeFitnessCallback( FitnessCalculator * f, NodeID outputNodeID );
            NodeFitnessCallback( const NodeFitnessCallback& other );
            virtual ~NodeFitnessCallback();

            virtual void operator()( const spnn::neuron& n ) = 0;

            NodeFitnessCallback& operator=( const NodeFitnessCallback& other );

        protected:

            inline NodeID targetNodeID() const;
            inline FitnessCalculator * fitnessCalculatorData() const;
    };

    class NodeFitnessCallback_Lambda : public NodeFitnessCallback
    {
        protected:

            std::function< void( const spnn::neuron&, FitnessCalculator *, NodeID ) > lambdaFunction;

        public:

            NodeFitnessCallback_Lambda( FitnessCalculator * f, NodeID outputNodeID, std::function< void( const spnn::neuron&, FitnessCalculator *, NodeID ) > func );
            NodeFitnessCallback_Lambda( NodeFitnessCallback_Lambda& other );
            virtual ~NodeFitnessCallback_Lambda() = default;

            inline void operator()( const spnn::neuron& n ) final;
    };
}

#include "fitness.inl"

#endif // NEAT_FITNESS_HPP_INCLUDED
