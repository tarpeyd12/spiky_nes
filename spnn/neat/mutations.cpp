#include <cmath>

#include "mutations.hpp"

namespace neat
{
    namespace Mutations
    {
        long double
        Gaussian( std::shared_ptr< Rand::RandomFunctor > rand )
        {
            // https://en.wikipedia.org/wiki/Box%E2%80%93Muller_transform

            assert( rand != nullptr );

            static const long double epsilon = std::numeric_limits<long double>::min();
            static const long double two_pi = 2.0L*3.14159265358979323846L;

            long double u, v;

            // make sure that the number passed to log is NOT 0
            do
            {
                u = rand->Float( epsilon, 1.0 ); // we set epsilon as the min, but we still don't want to be equal to epsilon
            }
            while( u <= epsilon );

            v = rand->Float( 0.0, 1.0 );

            // we use logl as it is specialized for long double
            return sqrt( -2.0 * logl( u ) ) * cos( two_pi * v );
        }

        std::vector< NodeDef >&
        Mutation_base::GetNodeList( NetworkGenotype& genotype )
        {
            return genotype.nodeGenotype;
        }

        std::vector< ConnectionDef >&
        Mutation_base::GetConnList( NetworkGenotype& genotype )
        {
            return genotype.connectionGenotype;
        }

        /*
        bool
        Mutation::operator()(  NetworkGenotype& genotypeToMutate, InnovationGenerator& innovationTracker, const MutationRates& rates, const MutationLimits& limits, std::shared_ptr< Rand::RandomFunctor > rand )
        {
            if( !rand ) rand = std::make_shared< Random::RandomFunctor >( Random::Int() );

            return true;
        }
        */


        Mutation_Multi::Mutation_Multi()
             : mutators()
        {
            /*  */
        }

        size_t
        Mutation_Multi::numMutators() const
        {
            // simply return the number of mutators stored in Mutation_Multi
            return mutators.size();
        }

        size_t
        Mutation_Multi::addMutator( double chance, std::shared_ptr< Mutation_base > mutator )
        {
            return addMutator( chance, 0.0, 0.0, mutator );
        }

        size_t
        Mutation_Multi::addMutator( double baseChance, double nodeChance, double connChance, std::shared_ptr< Mutation_base > mutator )
        {
            // if the chance given is zero or negative(how would that even work?) don't even add the mutator
            if( baseChance <= 0.0 && nodeChance <= 0.0 && connChance <= 0.0 )
            {
                return numMutators();
            }

            // add the mutator with its chance
            mutators.push_back( { baseChance, nodeChance, connChance, mutator } );

            // return successfully
            return numMutators();
        }

        uint64_t
        Mutation_Multi::operator()( NetworkGenotype& genotypeToMutate, InnovationGenerator& innovationTracker, const MutationRates& rates, const MutationLimits& limits, std::shared_ptr< Rand::RandomFunctor > rand ) const
        {
            if( !rand ) rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

            // TODO(dot##11/28/2018): figure out a way to best take the bool returns of the called mutators and express them in this functions bool return.

            if( !numMutators() )
            {
                return 0;
            }

            uint64_t sucessful_mutations = 0;

            // loop through all the pairs of chances and associated mutators
            for( const auto& mutator_chance : mutators )
            {
                // get the chance and the mutator pointer
                //double chance = mutator_chance.baseChance + genotypeToMutate.getNumNodes() * mutator_chance.nodeChance + genotypeToMutate.getNumConnections() * mutator_chance.connChance;
                auto mutator  = mutator_chance.mutator;

                size_t counts[] = { 1, genotypeToMutate.getNumNodes(), genotypeToMutate.getNumConnections() };
                double chances[] = { mutator_chance.baseChance, mutator_chance.nodeChance, mutator_chance.connChance };

                for( size_t i = 0; i < 3; ++i )
                {
                    if( chances[i] <= 0.0 )
                    {
                        continue;
                    }

                    for( size_t count = 0; count < counts[i]; ++count )
                    {
                        double chance = chances[i];

                        // if the chance is greater than one, that means the mutator will be called that many whole number of times, and the remainder will be a probability
                        while( chance >= 1.0 )
                        {
                            // call the mutator
                            sucessful_mutations += (*mutator)( genotypeToMutate, innovationTracker, rates, limits, rand );

                            // decrement
                            chance -= 1.0;
                        }

                        // if the remainder is 0, then there is no chance to randomly add the mutation, so skip that
                        if( chance <= 0.0 )
                        {
                            continue;
                        }

                        //
                        if( rand->Float( 0.0, 1.0 ) <= chance )
                        {
                            // call the mutator
                            sucessful_mutations += (*mutator)( genotypeToMutate, innovationTracker, rates, limits, rand );
                        }
                    }
                }
            }

            // return successfully
            return sucessful_mutations;
        }


        Mutation_Multi_one::Mutation_Multi_one()
             : mutators()
        {
            /*  */
        }

        size_t
        Mutation_Multi_one::numMutators() const
        {
            return mutators.size();
        }

        size_t
        Mutation_Multi_one::addMutator( std::shared_ptr< Mutation_base > mutator )
        {
            if( !mutator )
            {
                return numMutators();
            }

            mutators.push_back( mutator );

            return numMutators();
        }

        uint64_t
        Mutation_Multi_one::operator()( NetworkGenotype& genotypeToMutate, InnovationGenerator& innovationTracker, const MutationRates& rates, const MutationLimits& limits, std::shared_ptr< Rand::RandomFunctor > rand ) const
        {
            if( !rand ) rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

            if( !numMutators() )
            {
                return 0;
            }

            return (*mutators[ rand->Int( 0, numMutators() - 1 ) ])( genotypeToMutate, innovationTracker, rates, limits, rand );
        }

    }
}

