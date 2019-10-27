#include <iostream>

#include "population.hpp"

#include "thread_pool.hpp"
#include "random.hpp"

namespace neat
{
    Population::DbgGenerationCallbacks::DbgGenerationCallbacks()
     :
        begin( nullptr ),
        speciate_begin( nullptr ),
        speciate_end( nullptr ),
        fitness_begin( nullptr ),
        fitness_end( nullptr ),
        archive_begin( nullptr ),
        archive_end( nullptr ),
        extinction_begin( nullptr ),
        extinction_end( nullptr ),
        extinction_event( nullptr ),
        matching_begin( nullptr ),
        matching_end( nullptr ),
        splicing_begin( nullptr ),
        splicing_end( nullptr ),
        mutation_begin( nullptr ),
        mutation_end( nullptr ),
        swap_begin( nullptr ),
        swap_end( nullptr ),
        end( nullptr )
    {
        /*  */
    }

    Population::Population(
        size_t numNets,
        size_t inNodes,
        size_t outNodes,
        const MutationLimits& initLimits,
        const MutationRates& mutRate,
        std::shared_ptr< Mutations::Mutation_base > mutator,
        std::shared_ptr< FitnessFactory > fitFactory,
        const SpeciesDistanceParameters& speciationParameters,
        SpeciationMethod specMethod,
        size_t minSpec,
        size_t killDelay,
        size_t massExtinction,
        size_t gensToKeep
        )
     :
        innovationCounter( std::make_unique< InnovationGenerator >() ),
        numNetworks( numNets ),
        numInputNodes( inNodes ),
        numOutputNodes( outNodes ),
        generationCount( 0 ),
        initialGenotypeTemplate( std::make_unique< NetworkGenotype >() ),
        populationData(),
        //inputNodeIDs(),
        //outputNodeIDs(),
        speciesTracker( std::make_unique< SpeciesManager >( speciationParameters, specMethod ) ),
        mutationLimits( initLimits ),
        mutationRates( mutRate ),
        mutatorFunctor( mutator ),
        fitnessCalculatorFactory( fitFactory ),
        generationDataToKeep( gensToKeep ),
        generationLog(),
        minSpeciesSize( minSpec ),
        speciesKillDelay(),
        killDelayLimit( killDelay ),
        massExtinctionTimer( massExtinction ),
        pastFitness(),
        massExtinctionCount( 0 )
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

        // make sure we have a mutator
        assert( mutatorFunctor != nullptr );

        // make sure we have a fitness factory
        assert( fitnessCalculatorFactory != nullptr );

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
            num_muts += (*mutatorFunctor)( genotype, *innovationCounter, mutationRates, mutationLimits, _rand );
        } );

        return num_muts;
    }

    uint64_t
    Population::mutatePopulation( tpl::pool& thread_pool, neat::Mutations::Mutation_base& custom_mutator, std::shared_ptr< Rand::RandomFunctor > rand )
    {
        // make sure we have a functioning random number generator
        if( !rand ) rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

        std::atomic< uint64_t > num_muts{ 0 };

        // mutate each genotype present in the population
        tpl::for_each( thread_pool, populationData.begin(), populationData.end(), [&]( auto& genotype )
        {
            auto _rand = std::make_shared< Rand::Random_Unsafe >( rand->Int() );
            num_muts += custom_mutator( genotype, *innovationCounter, mutationRates, mutationLimits, _rand );
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
            num_muts += (*mutatorFunctor)( *genotype, *innovationCounter, mutationRates, mutationLimits, _rand );
        } );

        return num_muts;
    }

    void
    Population::clearGenerationConnections()
    {
        // make sure the mutations are classified as new
        innovationCounter->clearGenerationConnections();
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
    Population::IterateGeneration( tpl::pool& thread_pool, std::shared_ptr< Rand::RandomFunctor > rand, const double attritionRate, std::shared_ptr< DbgGenerationCallbacks > dbg_callbacks )
    {
        // TODO(dot##1/22/2019): rewrite the comments in this function

        assert( attritionRate > 0.0 && attritionRate < 1.0 );

        const bool dbg = bool( dbg_callbacks != nullptr );

        // callback for starting calculations
        if( dbg && dbg_callbacks->begin ) dbg_callbacks->begin();


        const size_t populationSize = populationData.size();

        // make sure we have a functioning random number generator
        if( !rand ) rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

        if( dbg && dbg_callbacks->speciate_begin ) dbg_callbacks->speciate_begin();

        auto speciatedPopulation = getSpeciatedPopulationData( thread_pool );

        if( dbg && dbg_callbacks->speciate_end ) dbg_callbacks->speciate_end();


        if( dbg && dbg_callbacks->fitness_begin ) dbg_callbacks->fitness_begin();

        PopulationFitness fitnessMap = getSpeciesAndNetworkFitness( thread_pool, speciatedPopulation );

        // clean up the map since we cant wrap this part in brackets :/
        speciatedPopulation.clear();

        if( dbg && dbg_callbacks->fitness_end ) dbg_callbacks->fitness_end();


        // archive the generation data
        {
            if( dbg && dbg_callbacks->archive_begin ) dbg_callbacks->archive_begin();

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

            if( dbg && dbg_callbacks->archive_end ) dbg_callbacks->archive_end();
        }


        // mass extinction stuff
        {
            if( dbg && dbg_callbacks->extinction_begin ) dbg_callbacks->extinction_begin();

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
                    if( dbg && dbg_callbacks->extinction_event ) dbg_callbacks->extinction_event();

                    // ok, we are going extinct now!
                    ++massExtinctionCount;

                    std::vector< std::pair< SpeciesID, long double > > speciesFitnesses;
                    speciesFitnesses.reserve( fitnessMap.get_num_species() );

                    fitnessMap.for_each_species( [&,this]( PopulationFitness::Species& species_data )
                    {
                        speciesFitnesses.emplace_back( species_data.species_id, species_data.species_fitness );
                    } );

                    std::sort( speciesFitnesses.begin(), speciesFitnesses.end(), []( const auto& a, const auto& b ){ return a.second > b.second; } );
                    //std::stable_sort( speciesFitnesses.begin(), speciesFitnesses.end(), []( const auto& a, const auto& b ){ return a.second > b.second; } );

                    PopulationFitness newFitnessMap;

                    newFitnessMap.addFitnessData( fitnessMap.get_highestFitnessIndividual() );

                    // TODO(dot##10/5/2019): make sure that the highest fitness individual is not added twice? (might not need to do this)

                    if( speciesFitnesses.size() >= 1 )
                    {
                        newFitnessMap.addFitnessData( fitnessMap.get_species( speciesFitnesses[0].first ) );
                    }
                    if( speciesFitnesses.size() >= 2 )
                    {
                        newFitnessMap.addFitnessData( fitnessMap.get_species( speciesFitnesses[1].first ) );
                    }

                    newFitnessMap.Finalize();

                    fitnessMap = newFitnessMap;

                    while( pastFitness.size() > 1 )
                    {
                        pastFitness.pop_front();
                    }
                } // end actual extinction
            }

            if( dbg && dbg_callbacks->extinction_end ) dbg_callbacks->extinction_end();
        } // end mass extinction code


        // fitness pair sorting function lambda
        //auto sortFunc = []( const std::pair< long double, const NetworkGenotype * >& a, const std::pair< long double, const NetworkGenotype * >& b ){ return bool( a.first > b.first ); };

        // calculate the sizes of the next generations species
        std::map< SpeciesID, long double > speciesFitnessRatio;
        std::map< SpeciesID, size_t > speciesNextGenCount;
        {
            if( dbg && dbg_callbacks->matching_begin ) dbg_callbacks->matching_begin();

            MinMax< long double > species_fitness_bounds( fitnessMap.get_species( fitnessMap.get_species_ids()[0] ).species_fitness );

            std::mutex archetype_mutex;
            std::list< tpl::future< void > > species_sort_futures;

            fitnessMap.for_each_species( [&,this]( PopulationFitness::Species& species_fitness )
            {
                species_fitness_bounds.expand( species_fitness.species_fitness );

                speciesFitnessRatio.emplace( species_fitness.species_id, species_fitness.species_fitness );

                // std::vector< std::pair<long double, const NetworkGenotype * > >
                auto _rand =  std::make_shared< Rand::Random_Safe >( rand->Int() );
                species_sort_futures.emplace_back( thread_pool.submit(
                [&archetype_mutex,this,_rand]( auto& species_data )
                {
                    species_data.sort_species_geotypes_by_fitness();

                    {
                        std::lock_guard<std::mutex> lock( archetype_mutex );
                        speciesTracker->updateSpeciesArchtype( species_data.species_id, *species_data.genotype_fitnesses[0].genotype );
                        //speciesTracker->updateSpeciesArchtype( species_data.species_id, *species_data.get_highest_fitness_individual().genotype );
                    }

                },  std::ref( species_fitness ) ) );
            } );

            long double fitnessRatiosSum = 0.0;

            for( auto& f : speciesFitnessRatio )
            {
                f.second -= species_fitness_bounds.min;
                //f.second += 1.0;
                f.second += ( (long double)populationSize / (long double)fitnessMap.get_num_species() );
                fitnessRatiosSum += f.second;
            }

            auto& highest_fitness_individual = fitnessMap.get_highestFitnessIndividual();

            for( auto& f : speciesFitnessRatio )
            {
                f.second /= fitnessRatiosSum;

                size_t speciesNextPopSize = f.second * populationSize;

                if( speciesNextPopSize < std::max<size_t>( 1, minSpeciesSize / 2 ) && minSpeciesSize > 1 )
                {
                    speciesKillDelay[ f.first ] += 2;
                }
                else if( speciesNextPopSize < minSpeciesSize )
                {
                    speciesKillDelay[ f.first ] += 1;
                }

                if( f.first == highest_fitness_individual.species_id )
                {
                    speciesKillDelay[ f.first ] = 0;
                }

                if( speciesKillDelay[ f.first ] < killDelayLimit )
                {
                    speciesNextPopSize = std::max<size_t>( speciesNextPopSize, minSpeciesSize );
                }
                else if( speciesKillDelay[ f.first ] < killDelayLimit && speciesNextPopSize >= minSpeciesSize && speciesKillDelay[ f.first ] > 0 )
                {
                    speciesKillDelay[ f.first ] -= 1;
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

                if( fitnessMap.has_species( nextCount.first ) )
                {
                    // average next species size with previous species size to smooth out next species size
                    nextCount.second = std::max<size_t>( 1, size_t( double( nextCount.second ) * 0.5 + double( fitnessMap.get_species( nextCount.first ).genotype_fitnesses.size() ) * 0.5 ) );
                }
                countSum += nextCount.second;
            }

            // account for too few
            while( countSum < populationSize )
            {
                /*SpeciesID minID = speciesNextGenCount.begin()->first;
                for( auto c : speciesNextGenCount ) { if( speciesNextGenCount[ minID ] > c.second ) { minID = c.first; } }*/

                std::vector< SpeciesID > minIDs = { speciesNextGenCount.begin()->first };
                size_t minCount = speciesNextGenCount.begin()->second;
                for( auto c : speciesNextGenCount )
                {
                    if( minCount > c.second ) { minIDs.clear(); minIDs.emplace_back( c.first ); minCount = c.second; }
                    else if( minCount == c.second ) { minIDs.emplace_back( c.first ); }
                }
                SpeciesID minID = minIDs[ rand->Int( 0, minIDs.size() - 1 ) ];

                speciesNextGenCount[ minID ]++;
                countSum++;
            }

            // account for too many
            while( countSum > populationSize )
            {
                /*SpeciesID maxID = speciesNextGenCount.begin()->first;
                for( auto c : speciesNextGenCount ) { if( speciesNextGenCount[ maxID ] < c.second ) { maxID = c.first; } }*/

                std::vector< SpeciesID > maxIDs = { speciesNextGenCount.begin()->first };
                size_t maxCount = speciesNextGenCount.begin()->second;
                for( auto c : speciesNextGenCount )
                {
                    if( maxCount < c.second ) { maxIDs.clear(); maxIDs.emplace_back( c.first ); maxCount = c.second; }
                    else if( maxCount == c.second ) { maxIDs.emplace_back( c.first ); }
                }
                SpeciesID maxID = maxIDs[ rand->Int( 0, maxIDs.size() - 1 ) ];

                if( speciesNextGenCount[ maxID ] > 0 )
                {
                    speciesNextGenCount[ maxID ]--;
                    countSum--;
                }
            }

            // wait for all sorts to be done
            while( !species_sort_futures.empty() )
            {
                species_sort_futures.pop_front();
            }

            if( dbg && dbg_callbacks->matching_end ) dbg_callbacks->matching_end();
        }

        // splice the current population of genotypes to create the next generation of genotypes. and identify which ones to mutate
        std::vector< NetworkGenotype > nextPopulation;
        std::vector< NetworkGenotype * > nextPopulation_to_mutate;
        {
            if( dbg && dbg_callbacks->splicing_begin ) dbg_callbacks->splicing_begin();

            struct future_package
            {
                tpl::future< NetworkGenotype > future;
                bool do_mutate;
                SpeciesID species;
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

                if( !fitnessMap.has_species( species ) )
                {
                    continue;
                }

                const auto& oldGenotypesVec = fitnessMap.get_species( species ).genotype_fitnesses; // should already be sorted, high to low

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

                while( matingPairs.size() > newGenotypesToMake )
                {
                    matingPairs.pop_back();
                }

                for( auto p : matingPairs )
                {
                    // if the pair is the same, then just return one of them instead of splicing
                    if( p.first == p.second )
                    {
                        genotype_futures.push_back( { thread_pool.submit( [&,p]{ return *oldGenotypesVec[ p.first ].genotype; } ), p.do_mutate, species } );
                        continue;
                    }

                    //nextPopulation.push_back( SpliceGenotypes( *oldGenotypesVec[ p.first ].genotype, *oldGenotypesVec[ p.second ].genotype, rand ) );
                    auto _rand = std::make_shared< Rand::Random_Unsafe >( rand->Int() );
                    using overload_type = NetworkGenotype( const NetworkGenotype&, const NetworkGenotype&, std::shared_ptr< Rand::RandomFunctor > );
                    genotype_futures.push_back( { thread_pool.submit< overload_type >( SpliceGenotypes, std::cref( *oldGenotypesVec[ p.first ].genotype ), std::cref( *oldGenotypesVec[ p.second ].genotype ), _rand ), p.do_mutate, species } );
                }
            }

            assert( genotype_futures.size() == populationSize );

            std::set< size_t > no_mutate_indexes;
            while( !genotype_futures.empty() )
            {
                nextPopulation.emplace_back( genotype_futures.front().future.get() );
                nextPopulation.back().parentSpeciesID = genotype_futures.front().species;

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

            if( dbg && dbg_callbacks->splicing_end ) dbg_callbacks->splicing_end();
        }

        // mutate the population
        {
            if( dbg && dbg_callbacks->mutation_begin ) dbg_callbacks->mutation_begin();

            // make sure the mutations are classified as new
            //innovationCounter->clearGenerationConnections();
            clearGenerationConnections();

            // mutate the population
            mutatePopulation( thread_pool, nextPopulation_to_mutate, rand );

            if( dbg && dbg_callbacks->mutation_end ) dbg_callbacks->mutation_end();
        }

        // swap out the old population for the new population
        {
            if( dbg && dbg_callbacks->swap_begin ) dbg_callbacks->swap_begin();

            // if we royally screwed up the population sizing and got too big, randomly reduce as a last resort
            while( nextPopulation.size() > populationData.size() )
            {
                nextPopulation.erase( nextPopulation.begin() + rand->Int( 0, nextPopulation.size() - 1 ) );
            }

            // double check we didn't screw up
            assert( nextPopulation.size() == populationData.size() );

            // we now assign the population data to the new population that we just created
            populationData = nextPopulation;

            if( dbg && dbg_callbacks->swap_end ) dbg_callbacks->swap_end();
        }

        // callback for ending calculations
        if( dbg && dbg_callbacks->end ) dbg_callbacks->end();

        // and done
    }

}
