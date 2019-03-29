#ifndef NEAT_FITNESS_INL_INCLUDED
#define NEAT_FITNESS_INL_INCLUDED

namespace neat
{
    NodeID
    NodeFitnessCallback::targetNodeID() const
    {
        return outputNodeIDNum;
    }

    FitnessCalculator *
    NodeFitnessCallback::fitnessCalculatorData() const
    {
        return fitness;
    }

    void
    NodeFitnessCallback_Lambda::operator()( const spnn::neuron& n )
    {
        lambdaFunction( n, fitnessCalculatorData(), targetNodeID() );
    }
}

#endif // NEAT_FITNESS_INL_INCLUDED
