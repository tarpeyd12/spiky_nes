#include <iostream>

#include "population.hpp"

#include "random.hpp"

namespace neat
{
    void
    Population::Init()
    {
        // call Init() but with the inbuilt mutationLimits
        Init( mutationLimits );
    }

    void
    Population::Init( const MutationLimits& initLimits )
    {
        // since this may be called after a few generations, and there might be some population already present, clear it.
        populationData.clear();

        // reserve the space for the networks
        populationData.reserve( numNetworks );

        // TODO(dot##10/17/2018): finish implementation of Population::Init

        /*  */

        // add networks to the population
        while( populationData.size() < numNetworks )
        {
            populationData.push_back( getDefaultNetworkCopy( initLimits ) );
        }

        innovationCounter.clearGenerationConnections();
    }

    NetworkGenotype
    Population::getDefaultNetworkCopy( const MutationLimits& limits ) const
    {
        // TODO(dot##1/15/2019): make this use a given random functor rather than the global one.

        NetworkGenotype out;
        out.nodeGenotype.reserve( initialGenotypeTemplate->nodeGenotype.size() );
        out.connectionGenotype.reserve( initialGenotypeTemplate->connectionGenotype.size() );

        // add the nodes from the template to the new genotype
        for( NodeDef node : initialGenotypeTemplate->nodeGenotype )
        {
            // assign random values to the parameters that the template node does not have using the limits given
            node.thresholdMin = Rand::Float( limits.thresholdMin.min, limits.thresholdMin.max );
            node.thresholdMax = Rand::Float( limits.thresholdMax.min, limits.thresholdMax.max );
            node.valueDecay =   Rand::Float( limits.valueDecay.min, limits.valueDecay.max );
            node.activDecay =   Rand::Float( limits.activDecay.min, limits.activDecay.max );
            node.pulseFast =    Rand::Int( limits.pulseFast.min, limits.pulseFast.max );
            node.pulseSlow =    Rand::Int( limits.pulseSlow.min, limits.pulseSlow.max );

            // make sure that min is less than max
            if( node.thresholdMin > node.thresholdMax )
            {
                std::swap( node.thresholdMin, node.thresholdMax );
            }

            // again, make sure that the fast frequency is less time than the slow frequency
            if( node.pulseFast > node.pulseSlow )
            {
                std::swap( node.pulseFast, node.pulseSlow );
            }

            // add the node to the output genotype
            out.nodeGenotype.push_back( node );
        }

        // add the connections from the template to the new genotype
        for( ConnectionDef connDef : initialGenotypeTemplate->connectionGenotype )
        {
            // assign a random values to the parts of the template connection that the template does not have
            connDef.weight = Rand::Float( limits.weight.min, limits.weight.max );
            connDef.length = Rand::Int( limits.length.min, limits.length.max );

            // make sure that the length is at least 1
            if( connDef.length < 1 )
            {
                connDef.length = 1;
            }

            // add the connection to the output genotype
            out.connectionGenotype.push_back( connDef );
        }

        // and we are done, return the output genotype
        return out;
    }
}
