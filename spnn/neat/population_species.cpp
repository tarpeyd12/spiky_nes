#include <iostream>

#include "population.hpp"

#include "random.hpp"

namespace neat
{
    std::vector< SpeciesID >
    Population::getSpeciesIDsForPopulationData( tpl::pool& thread_pool, bool preSpeciate )
    {
        return speciesTracker->getSpeciesIDsOfGenotypes( thread_pool, populationData, preSpeciate );
    }

    size_t
    Population::getPopulationSize() const
    {
        return populationData.size();
    }

    size_t
    Population::getNumTrackedSpecies() const
    {
        return speciesTracker->getNumTrackedSpecies();
    }

    std::map< SpeciesID, size_t >
    Population::getEndangeredSpecies() const
    {
        std::map< SpeciesID, size_t > out;

        auto lastGenData = getLastGenerationData();

        if( lastGenData )
        {
            auto lastGenSpeciesCount = lastGenData->getSpeciesPresent();
            for( auto p : speciesKillDelay )
            {
                if( p.second > 0 && lastGenSpeciesCount[ p.first ] )
                {
                    out.emplace( p.first, killDelayLimit - p.second );
                }
            }
        }
        else
        {
            for( auto p : speciesKillDelay )
            {
                if( p.second > 0 )
                {
                    out.emplace( p.first, killDelayLimit - p.second );
                }
            }
        }


        return out;
    }

    std::map< SpeciesID, std::vector< NetworkGenotype * > >
    Population::getSpeciatedPopulationData( tpl::pool& thread_pool )
    {
        std::map< SpeciesID, std::vector< NetworkGenotype * > > out;

        std::vector< SpeciesID > ids = getSpeciesIDsForPopulationData( thread_pool );

        for( size_t i = 0; i < populationData.size(); ++i )
        {
            out[ ids[ i ] ].push_back( &populationData[ i ] );
        }

        return out;
    }

    std::map< SpeciesID, long double >
    Population::getSpeciesFitness( tpl::pool& thread_pool )
    {
        std::map< SpeciesID, long double > speciesFitness;

        PopulationFitness pf = getSpeciesAndNetworkFitness( thread_pool );

        pf.for_each_species( [&]( PopulationFitness::Species& species_data )
        {
            speciesFitness[ species_data.species_id ] = species_data.species_fitness;
        } );

        return speciesFitness;
    }

    PopulationFitness
    Population::getSpeciesAndNetworkFitness( tpl::pool& thread_pool )
    {
        auto speciatedPopulation = getSpeciatedPopulationData( thread_pool );
        return getSpeciesAndNetworkFitness( thread_pool, speciatedPopulation );
    }

    PopulationFitness
    Population::getSpeciesAndNetworkFitness( tpl::pool& thread_pool, const std::map< SpeciesID, std::vector< NetworkGenotype * > >& speciatedPopulation, std::shared_ptr< Rand::RandomFunctor > rand )
    {
        struct fitness_package
        {
            SpeciesID species;
            long double fitness;
            const NetworkGenotype * genotype;
        };

        auto fitness_lambda = []( SpeciesID species, const NetworkGenotype * genotype, std::shared_ptr< FitnessFactory > fitness_factory ) -> fitness_package
        {
            long double fitScore = 0.0;
            size_t count = fitness_factory->numTimesToTest();

            auto network_phenotype = genotype->getNewNetworkPhenotype();

            while( count-- )
            {
                auto fitnessCalculator = fitness_factory->getNewFitnessCalculator( network_phenotype, count );

                fitnessCalculator->Run();

                fitScore += fitnessCalculator->getFitnessScore();

                fitnessCalculator->clearNetwork();
                network_phenotype->resetNetworkState();
            }

            fitScore /= (long double)( fitness_factory->numTimesToTest() );

            return { species, fitScore, genotype };
        };

        std::queue< tpl::future< fitness_package > > fitness_futures;

        struct pre_fitness_package
        {
            SpeciesID species;
            const NetworkGenotype * genotype;
        };

        std::vector< pre_fitness_package > flatenedSpeciatedPopulation;

        for( auto it = speciatedPopulation.begin(); it != speciatedPopulation.end(); ++it )
        {
            SpeciesID species = it->first;
            const std::vector< NetworkGenotype * >& genotypes = it->second;

            for( const NetworkGenotype * genotype : genotypes )
            {
                flatenedSpeciatedPopulation.push_back( { species, genotype } );
            }
        }

        // shuffle the order in which the speciated population pairs are ordered, to even out long vs short fitness times throughout the fitness evaluations
        if( rand != nullptr )
        {
            std::shuffle( flatenedSpeciatedPopulation.begin(), flatenedSpeciatedPopulation.end(), std::mt19937_64( rand->Int() ) );
        }

        // prep fitness evaluation futures
        for( auto pfp : flatenedSpeciatedPopulation )
        {
            auto fitness_package_future = thread_pool.submit( fitness_lambda, pfp.species, pfp.genotype, fitnessCalculatorFactory );
            fitness_futures.push( std::move( fitness_package_future ) );
        }

        PopulationFitness speciesFitness;

        // retrieve the fitness from the evaluated futures
        while( !fitness_futures.empty() )
        {
            auto fitness_pack = fitness_futures.front().get();
            fitness_futures.pop();

            speciesFitness.addFitnessData( fitness_pack.species, fitness_pack.fitness, fitness_pack.genotype );
        }

        // finalize the fitnesses
        speciesFitness.Finalize();

        return speciesFitness;
    }

    void
    Population::printSpeciesArchetypes( std::ostream& out )
    {
        speciesTracker->printSpeciesArchetypes( out );
    }
}
