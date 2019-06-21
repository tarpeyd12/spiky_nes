#include <iostream>

#include "population.hpp"

#include "thread_pool.hpp"
#include "random.hpp"

namespace neat
{
    Population::Population( size_t numNets, size_t inNodes, size_t outNodes, const MutationLimits& initLimits, const MutationRates& mutRate, Mutations::Mutation_base& mutator, FitnessFactory& fitFactory, const SpeciesDistanceParameters& speciationParameters, SpeciationMethod specMethod, size_t minSpec, size_t killDelay, size_t massExtinction, size_t gensToKeep )
         : innovationCounter( new InnovationGenerator() ), numNetworks( numNets ), numInputNodes( inNodes ), numOutputNodes( outNodes ), generationCount( 0 ), initialGenotypeTemplate( new NetworkGenotype() ), populationData(), inputNodeIDs(), outputNodeIDs(), speciesTracker( new SpeciesManager( speciationParameters, specMethod ) ), mutationLimits( initLimits ), mutationRates( mutRate ), mutatorFunctor( mutator ), fitnessCalculatorFactory( fitFactory ), generationDataToKeep( gensToKeep ), generationLog(), minSpeciesSize( minSpec ), speciesKillDelay(), killDelayLimit( killDelay ), massExtinctionTimer( massExtinction ), pastFitness(), massExtinctionCount( 0 )
    {
        assert( massExtinctionTimer > killDelayLimit );
        //assert( generationDataToKeep > 0 );
        generationDataToKeep = std::max<size_t>( generationDataToKeep, 1 ); // must be min 1 for iterate generation to work correctly

        // make sure that we have appropriate parameters for the networks, and the population as a whole
        assert( numNetworks > 0 );
        assert( numInputNodes > 0 );
        assert( numOutputNodes > 0 );

        // make sure that we have reasonable minimum species size
        assert( minSpeciesSize > 0 && minSpeciesSize <= numNetworks/2 );

        // define temporary default input and output nodes for the template network
        std::vector< NodeID > inNodesIDMade;
        std::vector< NodeID > outNodesIDMade;

        // reserve the space for them
        inNodesIDMade.reserve( numInputNodes );
        outNodesIDMade.reserve( numOutputNodes );

        // TODO(dot##6/17/2019): move the construction of the template genotype to Init functions

        // make our input and output nodes and add them into the default genotype, at the same time
        for( size_t i = 0; i < numInputNodes + numOutputNodes; ++i )
        {
            // split the number of nodes made so that we have both input and output types
            bool isInput = ( i < numInputNodes );

            // generate the node, 0's are for data we don't care about
            NodeDef node = innovationCounter->GetNextNode( 0.0, 0.0, 0.0, 0.0, 0, 0, isInput ? NodeType::Input : NodeType::Output );

            // add the node to the template genotype, and add the nodes ID into the node ID lists for later use
            initialGenotypeTemplate->nodeGenotype.push_back( node );
            ( isInput ? inNodesIDMade : outNodesIDMade ).push_back( node.ID );
        }

        // make our initial connections and add them into the default genotype
        // connecting each input node to all output nodes
        for( auto inNodeID : inNodesIDMade )
        {
            for( auto outNodeID : outNodesIDMade )
            {
                // each connection for the default template goes from the input to the output nodes, and is enabled
                ConnectionDef connDef = innovationCounter->GetNextConnection( inNodeID, outNodeID, 0.0, 0.0 );

                // and add the connection to the default template
                initialGenotypeTemplate->connectionGenotype.push_back( connDef );
            }
        }

        // make sure we aren't wasting space in the default template (since it sticks around for a while)
        initialGenotypeTemplate->nodeGenotype.shrink_to_fit();
        initialGenotypeTemplate->connectionGenotype.shrink_to_fit();
    }

    uint64_t
    Population::getGenerationCount() const
    {
        return generationCount;
    }

    const std::shared_ptr< Generation >
    Population::getLastGenerationData() const
    {
        if( generationLog.empty() )
        {
            return std::shared_ptr< Generation >( nullptr );
        }
        return generationLog.back();
    }

    uint64_t
    Population::getNumMassExtinctions() const
    {
        return massExtinctionCount;
    }

    uint64_t
    Population::mutatePopulation( tpl::pool& thread_pool, std::shared_ptr< Rand::RandomFunctor > rand )
    {
        // make sure we have a functioning random number generator
        if( !rand ) rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

        std::atomic< uint64_t > num_muts{ 0 };

        // mutate each genotype present in the population
        tpl::for_each( thread_pool, populationData.begin(), populationData.end(), [&]( auto& genotype )
        {
            auto _rand = std::make_shared< Rand::Random_Unsafe >( rand->Int() );
            num_muts += mutatorFunctor( genotype, *innovationCounter, mutationRates, mutationLimits, _rand );
        } );

        return num_muts;
    }

    uint64_t
    Population::mutatePopulation( tpl::pool& thread_pool, std::vector< NetworkGenotype* > genotypes_to_mutate, std::shared_ptr< Rand::RandomFunctor > rand )
    {
        // make sure we have a functioning random number generator
        if( !rand ) rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

        std::atomic< uint64_t > num_muts{ 0 };

        // mutate each genotype present in the population
        tpl::for_each( thread_pool, genotypes_to_mutate.begin(), genotypes_to_mutate.end(), [&]( auto * genotype )
        {
            auto _rand = std::make_shared< Rand::Random_Unsafe >( rand->Int() );
            num_muts += mutatorFunctor( *genotype, *innovationCounter, mutationRates, mutationLimits, _rand );
        } );

        return num_muts;
    }

    inline
    std::vector< size_t >
    RandomIndexes( size_t num, std::shared_ptr< Rand::RandomFunctor > rand )
    {
        // make sure we have a functioning random number generator
        if( !rand ) rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

        // init out to be numbers from 0 to num-1
        std::vector< size_t > out( num );
        size_t i = 0;
        std::generate( out.begin(), out.end(), [&]{ return i++; } );

        // scramble
        for( i = 0; i < out.size(); ++i )
        {
            std::swap( out[ i ], out[ rand->Int( 0, out.size() - 1 ) ] );
        }

        return out;
    }

    size_t
    Population::speciatePopulationAndCount( tpl::pool& thread_pool )
    {
        return getSpeciatedPopulationData( thread_pool ).size();
    }

    void
    Population::IterateGeneration( tpl::pool& thread_pool, std::shared_ptr< Rand::RandomFunctor > rand, const double attritionRate )
    {
        // TODO(dot##1/22/2019): rewrite the comments in this function

        //const double attritionRate = 0.95; // portion to eliminate from each species

        assert( attritionRate > 0.0 && attritionRate < 1.0 );

        const size_t populationSize = populationData.size();

        // make sure we have a functioning random number generator
        if( !rand ) rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

        // std::map< SpeciesID, std::pair<long double, std::vector< std::pair<long double, const NetworkGenotype * > > > >
        // std::map< SpeciesID, std::pair< avgFitness, std::vector< std::pair< fitness, const NetworkGenotype * > > > >
        auto fitnessMap = getSpeciesAndNetworkFitness( thread_pool );

        // log the generation and iterate the generation count
        //if( generationDataToKeep )
        {
            generationLog.push_back( std::shared_ptr< Generation >( new Generation( generationCount++, true, *speciesTracker, fitnessMap ) ) );
            while( generationLog.size() > generationDataToKeep )
            {
                generationLog.pop_front();
            }
        }

        speciesTracker->archiveSpeciesArchetypes( generationCount-1 );

        // mass extinction stuff
        {
            bool check = false;
            auto gen = getLastGenerationData();
            long double currentFitness = gen->getAvgFitness();
            pastFitness.push_back( currentFitness );

            while( pastFitness.size() > massExtinctionTimer )
            {
                pastFitness.pop_front();
                check = true;
            }

            if( check )
            {
                long double avgOld = 0.0;
                long double avgNew = 0.0;
                {
                    for( size_t i = 0; i < pastFitness.size(); ++i )
                    {
                        long double f = pastFitness[ i ];
                        if( i < pastFitness.size()/2 )
                        {
                            avgOld += f;
                        }
                        else
                        {
                            avgNew += f;
                        }
                    }
                    avgOld /= (long double)( pastFitness.size()/2 );
                    avgNew /= (long double)( pastFitness.size()/2 );
                }

                if( avgNew - avgOld < 0.1 )
                {
                    // ok, we are going extinct now!
                    ++massExtinctionCount;

                    std::vector< std::pair< SpeciesID, long double > > speciesFitnesses;
                    speciesFitnesses.reserve( fitnessMap.size() );

                    for( auto f : fitnessMap )
                    {
                        speciesFitnesses.emplace_back( f.first, f.second.first );
                    }

                    std::sort( speciesFitnesses.begin(), speciesFitnesses.end(), []( const auto& a, const auto& b ){ return a.second > b.second; } );
                    //std::stable_sort( speciesFitnesses.begin(), speciesFitnesses.end(), []( const auto& a, const auto& b ){ return a.second > b.second; } );

                    std::map< SpeciesID, std::pair<long double, std::vector< std::pair<long double, const NetworkGenotype * > > > > newFitnessMap;

                    if( speciesFitnesses.size() >= 1 )
                    {
                        newFitnessMap.emplace( speciesFitnesses[0].first, fitnessMap[ speciesFitnesses[0].first ] );
                    }
                    if( speciesFitnesses.size() >= 2 )
                    {
                        newFitnessMap.emplace( speciesFitnesses[1].first, fitnessMap[ speciesFitnesses[1].first ] );
                    }

                    for( auto& f : newFitnessMap )
                    {
                        auto& genotype_fitness_vector = f.second.second;
                        while( genotype_fitness_vector.size() > 1 )
                        {
                            genotype_fitness_vector.pop_back();
                        }
                        genotype_fitness_vector.shrink_to_fit();
                    }

                    fitnessMap = newFitnessMap;

                    while( pastFitness.size() > 1 )
                    {
                        pastFitness.pop_front();
                    }
                } // end actual extinction
            }
        } // end mass extinction code


        // fitness pair sorting function lambda
        //auto sortFunc = []( const std::pair< long double, const NetworkGenotype * >& a, const std::pair< long double, const NetworkGenotype * >& b ){ return bool( a.first > b.first ); };

        // calculate the sizes of the next generations species
        std::map< SpeciesID, long double > speciesFitnessRatio;
        std::map< SpeciesID, size_t > speciesNextGenCount;
        {
            MinMax< long double > fitnessBounds( fitnessMap.begin()->second.first );

            std::mutex archetype_mutex;
            std::list< tpl::future< void > > species_sort_futures;

            for( auto& f : fitnessMap )
            {
                fitnessBounds.expand( f.second.first );

                speciesFitnessRatio.emplace( f.first, f.second.first );

                // std::vector< std::pair<long double, const NetworkGenotype * > >
                auto _rand =  std::make_shared< Rand::Random_Safe >( rand->Int() );
                species_sort_futures.emplace_back( thread_pool.submit(
                [&archetype_mutex,this,_rand]( SpeciesID species, auto& species_vec )
                {
                    std::sort( species_vec.begin(), species_vec.end(), [](const auto& a, const auto& b){ return a.first > b.first; } );
                    //std::stable_sort( species_vec.begin(), species_vec.end(), [](const auto& a, const auto& b){ return a.first > b.first; } );
                    {
                        std::lock_guard<std::mutex> lock( archetype_mutex );
                        speciesTracker->updateSpeciesArchtype( species, *species_vec.front().second );
                        //speciesTracker->updateSpeciesArchtype( species, *species_vec[ _rand->Int( 0, species_vec.size() - 1 ) ].second );
                    }

                }, f.first, std::ref( f.second.second ) ) );
            }

            long double fitnessRatiosSum = 0.0;

            for( auto& f : speciesFitnessRatio )
            {
                f.second -= fitnessBounds.min;
                f.second += 1.0;
                fitnessRatiosSum += f.second;
            }


            for( auto& f : speciesFitnessRatio )
            {
                f.second /= fitnessRatiosSum;

                size_t speciesNextPopSize = f.second * populationSize;

                if( speciesNextPopSize < 1 && minSpeciesSize > 1 )
                {
                    speciesKillDelay[ f.first ] += 2;
                }
                else if( speciesNextPopSize < minSpeciesSize )
                {
                    speciesKillDelay[ f.first ] += 1;
                }

                if( speciesKillDelay[ f.first ] < killDelayLimit )
                {
                    speciesNextPopSize = std::max<size_t>( speciesNextPopSize, minSpeciesSize );
                }
                else
                {
                    speciesKillDelay[ f.first ] = 0;
                    speciesNextPopSize = 0;
                }

                speciesNextGenCount.emplace( f.first, speciesNextPopSize );
            }

            // smoothing
            size_t countSum = 0;
            for( auto& nextCount : speciesNextGenCount )
            {
                if( nextCount.second < 1 )
                {
                    countSum += nextCount.second; // paranoid
                    continue;
                }

                auto it = fitnessMap.find( nextCount.first );
                if( it != fitnessMap.end() )
                {
                    nextCount.second = std::max<size_t>( 1, size_t( double( nextCount.second ) * 0.5 + double( it->second.second.size() ) * 0.5 ) );
                    countSum += nextCount.second;
                }
            }

            // account for too few
            while( countSum < populationSize )
            {
                SpeciesID minID = speciesNextGenCount.begin()->first;
                for( auto c : speciesNextGenCount ) { if( speciesNextGenCount[ minID ] < c.second ) { minID = c.first; } }

                speciesNextGenCount[ minID ]++;
                countSum++;
            }

            // account for too many
            while( countSum > populationSize )
            {
                SpeciesID maxID = speciesNextGenCount.begin()->first;
                for( auto c : speciesNextGenCount ) { if( speciesNextGenCount[ maxID ] > c.second ) { maxID = c.first; } }

                if( speciesNextGenCount[ maxID ] <= 1 )
                {
                    // ERREOR
                }
                else
                {
                    speciesNextGenCount[ maxID ]--;
                }
                countSum--;
            }

            while( !species_sort_futures.empty() )
            {
                species_sort_futures.pop_front();
            }
        }

        // splice the current population of genotypes to create the next generation of genotypes. and identify which ones to mutate
        std::vector< NetworkGenotype > nextPopulation;
        std::vector< NetworkGenotype * > nextPopulation_to_mutate;
        {
            struct future_package
            {
                tpl::future< NetworkGenotype > future;
                bool do_mutate;
            };

            std::list< future_package > genotype_futures;

            for( const auto s : speciesNextGenCount )
            {
                const SpeciesID species = s.first;
                const size_t newGenotypesToMake = s.second;

                // don't bother with making 0 new genotypes
                if( newGenotypesToMake == 0 )
                {
                    continue;
                }

                const auto& oldGenotypesVec = fitnessMap[ species ].second; // should already be sorted, high to low

                // we cant make the new genotypes without the old genotypes, on to the next ones!
                if( oldGenotypesVec.empty() )
                {
                    continue;
                }

                struct mating_pair_data
                {
                    size_t first;
                    size_t second;
                    bool do_mutate;
                };

                std::vector< mating_pair_data > matingPairs;
                matingPairs.reserve( newGenotypesToMake );

                matingPairs.push_back( { 0, 0, false } ); // elitism

                size_t numIndexesToTake = MinMax<size_t>( 1, oldGenotypesVec.size() ).clamp( oldGenotypesVec.size() * ( 1.0 - attritionRate ) );

                // set up the randomized mating pairs
                while( matingPairs.size() < newGenotypesToMake )
                {
                    std::vector< size_t > fitRandomIndexes1 = RandomIndexes( numIndexesToTake, rand );
                    std::vector< size_t > fitRandomIndexes2 = RandomIndexes( numIndexesToTake, rand );

                    while( !fitRandomIndexes1.empty() && !fitRandomIndexes1.empty() && matingPairs.size() < newGenotypesToMake )
                    {
                        matingPairs.push_back( { fitRandomIndexes1.back(), fitRandomIndexes2.back(), true } );
                        fitRandomIndexes1.pop_back();
                        fitRandomIndexes2.pop_back();
                    }
                }

                for( auto p : matingPairs )
                {
                    // if the pair is the same, then just return one of them instead of splicing
                    if( p.first == p.second )
                    {
                        genotype_futures.push_back( { thread_pool.submit( [&,p]{ return *oldGenotypesVec[ p.first ].second; } ), p.do_mutate } );
                        continue;
                    }

                    //nextPopulation.push_back( SpliceGenotypes( *oldGenotypesVec[ p.first ].second, *oldGenotypesVec[ p.second ].second, rand ) );
                    auto _rand = std::make_shared< Rand::Random_Unsafe >( rand->Int() );
                    using overload_type = NetworkGenotype( const NetworkGenotype&, const NetworkGenotype&, std::shared_ptr< Rand::RandomFunctor > );
                    genotype_futures.push_back( { thread_pool.submit< overload_type >( SpliceGenotypes, std::cref( *oldGenotypesVec[ p.first ].second ), std::cref( *oldGenotypesVec[ p.second ].second ), _rand ), p.do_mutate } );
                }
            }

            std::set< size_t > no_mutate_indexes;
            while( !genotype_futures.empty() )
            {
                nextPopulation.emplace_back( genotype_futures.front().future.get() );

                // elitism
                if( !genotype_futures.front().do_mutate )
                {
                    // since we add the new genotype to nextPopulation before taking the size for the index we must subtract one
                    no_mutate_indexes.emplace( nextPopulation.size() - 1 );
                }

                genotype_futures.pop_front();
            }

            nextPopulation_to_mutate.reserve( nextPopulation.size() );
            for( size_t i = 0; i < nextPopulation.size(); ++i )
            {
                // elitism
                if( no_mutate_indexes.count( i ) )
                {
                    continue;
                }

                nextPopulation_to_mutate.emplace_back( &nextPopulation[ i ] );
            }
        }

        // mutate the population
        {
            // make sure the mutations are classified as new
            innovationCounter->clearGenerationConnections();

            // mutate the population
            mutatePopulation( thread_pool, nextPopulation_to_mutate, rand );
        }

        // if we royally screwed up the population sizing and got too big, randomly reduce
        while( nextPopulation.size() > populationData.size() )
        {
            nextPopulation.erase( nextPopulation.begin() + rand->Int( 0, nextPopulation.size() - 1 ) );
        }

        // double check we didn't screw up
        assert( nextPopulation.size() == populationData.size() );

        // we now assign the population data to the new population that we just created
        populationData = nextPopulation;

        // and done
    }

}
