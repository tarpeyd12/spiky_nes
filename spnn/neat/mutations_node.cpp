#include "mutations.hpp"

namespace neat
{
    namespace Mutations
    {

        // node value variations

        uint64_t
        Mutation_Node_thresh_min::operator()( NetworkGenotype& genotypeToMutate, InnovationGenerator&, const MutationRates& rates, const MutationLimits& limits, std::shared_ptr< Rand::RandomFunctor > rand ) const
        {
            // make sure we have a functioning random number generator
            if( !rand ) rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

            auto& nodeList = GetNodeList( genotypeToMutate ); // get the node list
            if( nodeList.empty() ) { return 0; } // cant get the node from an empty list
            NodeDef& node = nodeList[ rand->Int( 0, nodeList.size() - 1 ) ]; // select the node to mutate

            // mutate the property
            {
                double rate = rates.thresholdMin;
                node.thresholdMin += Gaussian( rand ) * rate;
                node.thresholdMin = limits.thresholdMin.clamp( node.thresholdMin );
            }

            // everything worked
            return 1;
        }

        uint64_t
        Mutation_Node_thresh_min_new::operator()( NetworkGenotype& genotypeToMutate, InnovationGenerator&, const MutationRates&, const MutationLimits& limits, std::shared_ptr< Rand::RandomFunctor > rand ) const
        {
            // make sure we have a functioning random number generator
            if( !rand ) rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

            auto& nodeList = GetNodeList( genotypeToMutate ); // get the node list
            if( nodeList.empty() ) { return 0; } // cant get the node from an empty list
            NodeDef& node = nodeList[ rand->Int( 0, nodeList.size() - 1 ) ]; // select the node to mutate

            // mutate the property
            {
                node.thresholdMin = rand->Float( limits.thresholdMin.min, limits.thresholdMin.max );
            }

            // everything worked
            return 1;
        }

        uint64_t
        Mutation_Node_thresh_max::operator()( NetworkGenotype& genotypeToMutate, InnovationGenerator&, const MutationRates& rates, const MutationLimits& limits, std::shared_ptr< Rand::RandomFunctor > rand ) const
        {
            // make sure we have a functioning random number generator
            if( !rand ) rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

            auto& nodeList = GetNodeList( genotypeToMutate ); // get the node list
            if( nodeList.empty() ) { return 0; } // cant get the node from an empty list
            NodeDef& node = nodeList[ rand->Int( 0, nodeList.size() - 1 ) ]; // select the node to mutate

            // mutate the property
            {
                double rate = rates.thresholdMax;
                node.thresholdMax += Gaussian( rand ) * rate;
                node.thresholdMax = limits.thresholdMax.clamp( node.thresholdMax );
            }

            // everything worked
            return 1;
        }

        uint64_t
        Mutation_Node_thresh_max_new::operator()( NetworkGenotype& genotypeToMutate, InnovationGenerator&, const MutationRates&, const MutationLimits& limits, std::shared_ptr< Rand::RandomFunctor > rand ) const
        {
            // make sure we have a functioning random number generator
            if( !rand ) rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

            auto& nodeList = GetNodeList( genotypeToMutate ); // get the node list
            if( nodeList.empty() ) { return 0; } // cant get the node from an empty list
            NodeDef& node = nodeList[ rand->Int( 0, nodeList.size() - 1 ) ]; // select the node to mutate

            // mutate the property
            {
                node.thresholdMax = rand->Float( limits.thresholdMax.min, limits.thresholdMax.max );
            }

            // everything worked
            return 1;
        }

        uint64_t
        Mutation_Node_decays_value::operator()( NetworkGenotype& genotypeToMutate, InnovationGenerator&, const MutationRates& rates, const MutationLimits& limits, std::shared_ptr< Rand::RandomFunctor > rand ) const
        {
            // make sure we have a functioning random number generator
            if( !rand ) rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

            auto& nodeList = GetNodeList( genotypeToMutate ); // get the node list
            if( nodeList.empty() ) { return 0; } // cant get the node from an empty list
            NodeDef& node = nodeList[ rand->Int( 0, nodeList.size() - 1 ) ]; // select the node to mutate

            // mutate the property
            {
                double rate = rates.valueDecay;
                node.valueDecay += Gaussian( rand ) * rate;
                node.valueDecay = limits.valueDecay.clamp( node.valueDecay );
            }

            // everything worked
            return 1;
        }

        uint64_t
        Mutation_Node_decays_value_new::operator()( NetworkGenotype& genotypeToMutate, InnovationGenerator&, const MutationRates&, const MutationLimits& limits, std::shared_ptr< Rand::RandomFunctor > rand ) const
        {
            // make sure we have a functioning random number generator
            if( !rand ) rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

            auto& nodeList = GetNodeList( genotypeToMutate ); // get the node list
            if( nodeList.empty() ) { return 0; } // cant get the node from an empty list
            NodeDef& node = nodeList[ rand->Int( 0, nodeList.size() - 1 ) ]; // select the node to mutate

            // mutate the property
            {
                node.valueDecay = rand->Float( limits.valueDecay.min, limits.valueDecay.max );
            }

            // everything worked
            return 1;
        }

        uint64_t
        Mutation_Node_decays_activ::operator()( NetworkGenotype& genotypeToMutate, InnovationGenerator&, const MutationRates& rates, const MutationLimits& limits, std::shared_ptr< Rand::RandomFunctor > rand ) const
        {
            // make sure we have a functioning random number generator
            if( !rand ) rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

            auto& nodeList = GetNodeList( genotypeToMutate ); // get the node list
            if( nodeList.empty() ) { return 0; } // cant get the node from an empty list
            NodeDef& node = nodeList[ rand->Int( 0, nodeList.size() - 1 ) ]; // select the node to mutate

            // mutate the property
            {
                double rate = rates.activDecay;
                node.activDecay += Gaussian( rand ) * rate;
                node.activDecay = limits.activDecay.clamp( node.activDecay );
            }

            // everything worked
            return 1;
        }

        uint64_t
        Mutation_Node_decays_activ_new::operator()( NetworkGenotype& genotypeToMutate, InnovationGenerator&, const MutationRates&, const MutationLimits& limits, std::shared_ptr< Rand::RandomFunctor > rand ) const
        {
            // make sure we have a functioning random number generator
            if( !rand ) rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

            auto& nodeList = GetNodeList( genotypeToMutate ); // get the node list
            if( nodeList.empty() ) { return 0; } // cant get the node from an empty list
            NodeDef& node = nodeList[ rand->Int( 0, nodeList.size() - 1 ) ]; // select the node to mutate

            // mutate the property
            {
                node.activDecay = rand->Float( limits.activDecay.min, limits.activDecay.max );
            }

            // everything worked
            return 1;
        }

        uint64_t
        Mutation_Node_pulses_fast::operator()( NetworkGenotype& genotypeToMutate, InnovationGenerator&, const MutationRates& rates, const MutationLimits& limits, std::shared_ptr< Rand::RandomFunctor > rand ) const
        {
            // make sure we have a functioning random number generator
            if( !rand ) rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

            auto& nodeList = GetNodeList( genotypeToMutate ); // get the node list
            if( nodeList.empty() ) { return 0; } // cant get the node from an empty list
            NodeDef& node = nodeList[ rand->Int( 0, nodeList.size() - 1 ) ]; // select the node to mutate

            // mutate the property
            {
                double rate = rates.pulseFast;
                node.pulseFast += ( rand->Int( 0, 1 ) ? rand->Int( 0, rate ) : -rand->Int( 0, rate ) );
                node.pulseFast = limits.pulseFast.clamp( node.pulseFast );
            }

            // everything worked
            return 1;
        }

        uint64_t
        Mutation_Node_pulses_fast_new::operator()( NetworkGenotype& genotypeToMutate, InnovationGenerator&, const MutationRates&, const MutationLimits& limits, std::shared_ptr< Rand::RandomFunctor > rand ) const
        {
            // make sure we have a functioning random number generator
            if( !rand ) rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

            auto& nodeList = GetNodeList( genotypeToMutate ); // get the node list
            if( nodeList.empty() ) { return 0; } // cant get the node from an empty list
            NodeDef& node = nodeList[ rand->Int( 0, nodeList.size() - 1 ) ]; // select the node to mutate

            // mutate the property
            {
                node.pulseFast = rand->Int( limits.pulseFast.min, limits.pulseFast.max );
            }

            // everything worked
            return 1;
        }

        uint64_t
        Mutation_Node_pulses_slow::operator()( NetworkGenotype& genotypeToMutate, InnovationGenerator&, const MutationRates& rates, const MutationLimits& limits, std::shared_ptr< Rand::RandomFunctor > rand ) const
        {
            // make sure we have a functioning random number generator
            if( !rand ) rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

            auto& nodeList = GetNodeList( genotypeToMutate ); // get the node list
            if( nodeList.empty() ) { return 0; } // cant get the node from an empty list
            NodeDef& node = nodeList[ rand->Int( 0, nodeList.size() - 1 ) ]; // select the node to mutate

            // mutate the property
            {
                double rate = rates.pulseSlow;
                node.pulseSlow += ( rand->Int( 0, 1 ) ? rand->Int( 0, rate ) : -rand->Int( 0, rate ) );
                node.pulseSlow = limits.pulseSlow.clamp( node.pulseSlow );
            }

            // everything worked
            return 1;
        }

        uint64_t
        Mutation_Node_pulses_slow_new::operator()( NetworkGenotype& genotypeToMutate, InnovationGenerator&, const MutationRates&, const MutationLimits& limits, std::shared_ptr< Rand::RandomFunctor > rand ) const
        {
            // make sure we have a functioning random number generator
            if( !rand ) rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

            auto& nodeList = GetNodeList( genotypeToMutate ); // get the node list
            if( nodeList.empty() ) { return 0; } // cant get the node from an empty list
            NodeDef& node = nodeList[ rand->Int( 0, nodeList.size() - 1 ) ]; // select the node to mutate

            // mutate the property
            {
                node.pulseSlow = rand->Int( limits.pulseSlow.min, limits.pulseSlow.max );
            }

            // everything worked
            return 1;
        }

    }
}
