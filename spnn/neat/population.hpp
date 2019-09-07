#ifndef NEAT_POPULATION_HPP_INCLUDED
#define NEAT_POPULATION_HPP_INCLUDED

#include <memory>
#include <vector>
#include <deque>
#include <map>

namespace neat
{
    class Population;
}


#include "species.hpp"
#include "network.hpp"
#include "fitness.hpp"
#include "innovation_generator.hpp"
#include "mutations.hpp"
#include "generation.hpp"
#include "neat.hpp"
#include "thread_pool.hpp"
#include "xml.hpp"

namespace neat
{
    class Population
    {
        private:

            std::unique_ptr< InnovationGenerator > innovationCounter;

            size_t numNetworks;

            size_t numInputNodes;
            size_t numOutputNodes;

            uint64_t generationCount;

            std::unique_ptr< NetworkGenotype > initialGenotypeTemplate;

        protected:

            std::vector< NetworkGenotype > populationData;

            std::vector< NodeID > inputNodeIDs;
            std::vector< NodeID > outputNodeIDs;

            std::unique_ptr< SpeciesManager > speciesTracker;

            MutationLimits mutationLimits;
            MutationRates mutationRates;

            std::shared_ptr< Mutations::Mutation_base > mutatorFunctor;

            FitnessFactory& fitnessCalculatorFactory;

            size_t generationDataToKeep;
            std::deque< std::shared_ptr< Generation > > generationLog;

            size_t minSpeciesSize;
            std::map< SpeciesID, size_t > speciesKillDelay;
            size_t killDelayLimit;

            size_t massExtinctionTimer;
            std::deque< long double > pastFitness;
            uint64_t massExtinctionCount;

        public:

            Population( size_t numNets,
                        size_t inNodes,
                        size_t outNodes,
                        const MutationLimits& initLimits,
                        const MutationRates& mutRate,
                        std::shared_ptr< Mutations::Mutation_base > mutator,
                        FitnessFactory& fitFactory,
                        const SpeciesDistanceParameters& speciationParameters,
                        SpeciationMethod specMethod = SpeciationMethod::FirstForward,
                        size_t minSpec = 1,
                        size_t killDelay = 100,
                        size_t massExtinction = 200,
                        size_t gensToKeep = 10 );

            virtual ~Population() = default;

            void Init();
            void Init( const MutationLimits& initLimits );

            std::vector< SpeciesID > getSpeciesIDsForPopulationData( tpl::pool& thread_pool, bool preSpeciate = true );

            size_t getPopulationSize() const;
            size_t getNumTrackedSpecies() const;
            std::map< SpeciesID, size_t > getEndangeredSpecies() const;
            uint64_t getGenerationCount() const;
            const std::shared_ptr< Generation > getLastGenerationData() const;
            uint64_t getNumMassExtinctions() const;

            std::map< SpeciesID, long double > getSpeciesFitness( tpl::pool& thread_pool );
            std::map< SpeciesID, std::pair< long double, std::vector< std::pair< long double, const NetworkGenotype * > > > > getSpeciesAndNetworkFitness( tpl::pool& thread_pool );
            std::map< SpeciesID, std::pair< long double, std::vector< std::pair< long double, const NetworkGenotype * > > > > getSpeciesAndNetworkFitness( tpl::pool& thread_pool, const std::map< SpeciesID, std::vector< NetworkGenotype * > >& speciatedPopulation );

            void printSpeciesArchetypes( std::ostream& out );

            uint64_t mutatePopulation( tpl::pool& thread_pool, std::shared_ptr< Rand::RandomFunctor > rand = nullptr );
            uint64_t mutatePopulation( tpl::pool& thread_pool, neat::Mutations::Mutation_base& custom_mutator, std::shared_ptr< Rand::RandomFunctor > rand = nullptr );
            uint64_t mutatePopulation( tpl::pool& thread_pool, std::vector< NetworkGenotype* > genotypes_to_mutate, std::shared_ptr< Rand::RandomFunctor > rand = nullptr );

            void clearGenerationConnections();

            size_t speciatePopulationAndCount( tpl::pool& thread_pool );

            struct DbgGenerationCallbacks
            {
                std::function<void()> begin;

                std::function<void()> speciate_begin;
                std::function<void()> speciate_end;

                std::function<void()> fitness_begin;
                std::function<void()> fitness_end;

                std::function<void()> archive_begin;
                std::function<void()> archive_end;

                std::function<void()> extinction_begin;
                std::function<void()> extinction_end;
                std::function<void()> extinction_event;

                std::function<void()> matching_begin;
                std::function<void()> matching_end;

                std::function<void()> splicing_begin;
                std::function<void()> splicing_end;

                std::function<void()> mutation_begin;
                std::function<void()> mutation_end;

                std::function<void()> swap_begin;
                std::function<void()> swap_end;

                std::function<void()> end;
            };

            void IterateGeneration( tpl::pool& thread_pool, std::shared_ptr< Rand::RandomFunctor > rand = nullptr, const double attritionRate = 0.5, std::shared_ptr< DbgGenerationCallbacks > dbg_callbacks = nullptr );

            void SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool );

        protected:

            std::map< SpeciesID, std::vector< NetworkGenotype * > > getSpeciatedPopulationData( tpl::pool& thread_pool );

            //std::map< SpeciesID, long double > getSpeciesFitness();

        private:

            NetworkGenotype getDefaultNetworkCopy( const MutationLimits& limits ) const;
    };
}

#endif // NEAT_POPULATION_HPP_INCLUDED
