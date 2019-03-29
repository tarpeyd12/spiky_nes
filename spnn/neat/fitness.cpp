#include <iostream>

#include "fitness.hpp"

namespace neat
{
    FitnessCalculator::FitnessCalculator( std::shared_ptr< NetworkPhenotype > net )
         : network( net )
    {
        assert( network );
    }

    FitnessCalculator::~FitnessCalculator()
    {
        network = nullptr;
    }

    size_t
    FitnessCalculator::getNumInputNodes() const
    {
        if( network )
        {
            return network->numInputs();
        }
        return 0;
    }

    size_t
    FitnessCalculator::getNumOutputNodes() const
    {
        if( network )
        {
            return network->numOutputs();
        }
        return 0;
    }

    uint64_t
    FitnessCalculator::Run()
    {
        assert( network );
        assert( network->Time() == 0 );
        assert( network->DeltaTime() );
        assert( getMaxNumTimesteps() );

        // TODO(dot##10/17/2018): Finish the implementation of Fitness::Run

        /*  */

        if( !getInputValueCheckCadence() )
        {
            bool valueOK = network->setInputValues( getInputValues( 0 ) );
            assert( valueOK );
        }

        bool callbackOK = network->setOutputCallbacks( getNodeCallbacks() );
        assert( callbackOK );

        while( !stopTest() && network->Time() < getMaxNumTimesteps() )
        {
            if( getInputValueCheckCadence() && ( network->Time() % getInputValueCheckCadence() == 0 ) )
            {
                bool valueOK = network->setInputValues( getInputValues( network->Time() ) );
                assert( valueOK );
            }

            network->Tick();
            testTick( network->Time() - network->DeltaTime() );

            //std::cout << network->Time() << ": pulses: " << network->PulsesProcessedLastTick() << " neurons: " << network->NeuronsProcessedLastTick() << std::endl;

            //network->printNetworkState( std::cout );
        }

        return network->Time();
    }

    bool
    FitnessCalculator::canRun() const
    {
        return network != nullptr;
    }

    void
    FitnessCalculator::clearNetwork()
    {
        // NOTE(dot##10/26/2018): should FitnessCalculator::clearNetwork delete the network too?
        network = nullptr;
    }

    NodeFitnessCallback::NodeFitnessCallback( FitnessCalculator * f, uint64_t outputNodeID )
         : outputNodeIDNum( outputNodeID ), fitness( f )
    {
        assert( fitness );
    }

    NodeFitnessCallback::NodeFitnessCallback( const NodeFitnessCallback& other )
         : outputNodeIDNum( other.outputNodeIDNum ), fitness( other.fitness )
    {
        assert( fitness );
    }

    NodeFitnessCallback::~NodeFitnessCallback()
    {
        fitness = nullptr;
    }

    NodeFitnessCallback&
    NodeFitnessCallback::operator=( const NodeFitnessCallback& other )
    {
        outputNodeIDNum = other.outputNodeIDNum;
        fitness = other.fitness;

        return *this;
    }

    NodeFitnessCallback_Lambda::NodeFitnessCallback_Lambda( FitnessCalculator * f, NodeID outputNodeID, std::function< void( const spnn::neuron&, FitnessCalculator *, NodeID ) > func )
         : NodeFitnessCallback( f, outputNodeID ), lambdaFunction( func )
    {
        assert( f );
    }

    NodeFitnessCallback_Lambda::NodeFitnessCallback_Lambda( NodeFitnessCallback_Lambda& other )
         : NodeFitnessCallback( other ), lambdaFunction( other.lambdaFunction )
    {
        /*  */
    }
}
