#include "mutations.hpp"

namespace neat
{
    namespace Mutations
    {

        // connection value variations

        uint64_t
        Mutation_Conn_weight::operator()( NetworkGenotype& genotypeToMutate, InnovationGenerator&, const MutationRates& rates, const MutationLimits& limits, std::shared_ptr< Rand::RandomFunctor > rand ) const
        {
            // make sure we have a functioning random number generator
            if( !rand ) rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

            auto& connList = GetConnList( genotypeToMutate ); // get the connection list
            if( connList.empty() ) { return 0; } // cant get the connection from an empty list
            ConnectionDef& conn = connList[ rand->Int( 0, connList.size() - 1 ) ]; // select the connection to mutate

            // mutate the property
            {
                double rate = rates.weight;
                conn.weight += Gaussian( rand ) * rate;
                conn.weight = limits.weight.clamp( conn.weight );
            }

            // everything worked
            return 1;
        }

        uint64_t
        Mutation_Conn_weight_new::operator()( NetworkGenotype& genotypeToMutate, InnovationGenerator&, const MutationRates&, const MutationLimits& limits, std::shared_ptr< Rand::RandomFunctor > rand ) const
        {
            // make sure we have a functioning random number generator
            if( !rand ) rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

            auto& connList = GetConnList( genotypeToMutate ); // get the connection list
            if( connList.empty() ) { return 0; } // cant get the connection from an empty list
            ConnectionDef& conn = connList[ rand->Int( 0, connList.size() - 1 ) ]; // select the connection to mutate

            // mutate the property
            {
                conn.weight = rand->Float( limits.weight.min, limits.weight.max );
            }

            // everything worked
            return 1;
        }

        uint64_t
        Mutation_Conn_length::operator()( NetworkGenotype& genotypeToMutate, InnovationGenerator&, const MutationRates& rates, const MutationLimits& limits, std::shared_ptr< Rand::RandomFunctor > rand ) const
        {
            // make sure we have a functioning random number generator
            if( !rand ) rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

            auto& connList = GetConnList( genotypeToMutate ); // get the connection list
            if( connList.empty() ) { return 0; } // cant get the connection from an empty list
            ConnectionDef& conn = connList[ rand->Int( 0, connList.size() - 1 ) ]; // select the connection to mutate

            // mutate the property
            {
                double rate = rates.length;
                conn.length += ( rand->Int( 0, 1 ) ? rand->Int( 0, rate ) : -rand->Int( 0, rate ) );
                conn.length = limits.length.clamp( conn.length );
            }

            // everything worked
            return 1;
        }

        uint64_t
        Mutation_Conn_length_new::operator()( NetworkGenotype& genotypeToMutate, InnovationGenerator&, const MutationRates&, const MutationLimits& limits, std::shared_ptr< Rand::RandomFunctor > rand ) const
        {
            // make sure we have a functioning random number generator
            if( !rand ) rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

            auto& connList = GetConnList( genotypeToMutate ); // get the connection list
            if( connList.empty() ) { return 0; } // cant get the connection from an empty list
            ConnectionDef& conn = connList[ rand->Int( 0, connList.size() - 1 ) ]; // select the connection to mutate

            // mutate the property
            {
                conn.length = rand->Int( limits.length.min, limits.length.max );
            }

            // everything worked
            return 1;
        }

        uint64_t
        Mutation_Conn_enable::operator()( NetworkGenotype& genotypeToMutate, InnovationGenerator&, const MutationRates&, const MutationLimits&, std::shared_ptr< Rand::RandomFunctor > rand ) const
        {
            // make sure we have a functioning random number generator
            if( !rand ) rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

            auto& connList = GetConnList( genotypeToMutate ); // get the connection list
            if( connList.empty() ) { return 0; } // cant get the connection from an empty list
            ConnectionDef& conn = connList[ rand->Int( 0, connList.size() - 1 ) ]; // select the connection to mutate

            // mutate the property
            {
                conn.enabled = !conn.enabled; // invert the enabled property
            }

            // everything worked
            return 1;
        }

    }
}
