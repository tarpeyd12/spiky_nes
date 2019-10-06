#ifndef NEAT_GENERATION_HPP_INCLUDED
#define NEAT_GENERATION_HPP_INCLUDED

#include <memory>
#include <map>
#include <vector>
#include <tuple>

// TODO(dot##11/20/2018): remember what i was doing with this!

namespace neat
{
    class Generation;
}

#include "neat.hpp"
#include "population.hpp"

namespace neat
{
    class Generation
    {
        private:

            bool keepCopyOfGenotypes;

        protected:

            uint64_t generationNumber;

            std::unique_ptr< SpeciesManager > speciesData;
            std::vector< std::tuple< SpeciesID, long double, std::shared_ptr< NetworkGenotype > > > populationData;
            std::map< SpeciesID, long double > speciesFitness;

            long double minFitness;
            long double maxFitness;
            long double avgFitness;

        public:

            Generation();
            Generation( const Generation& other );
            virtual ~Generation() = default;

            bool keepsGenotypeCopies() const;

            uint64_t getGenerationNumber() const;
            size_t   getNumGenotypes() const;

            SpeciesManager                                  getSpeciesManager() const;
            std::vector< SpeciesID >                        getSpeciesIDVector() const;
            std::vector< long double >                      getFitnessVector() const;
            std::vector< std::weak_ptr< NetworkGenotype > > getGenotypesVector() const; // will return empty vector when keepsGenotypeCopies() is false
            std::map< SpeciesID, size_t >                   getSpeciesPresent() const;
            std::map< SpeciesID, long double >              getSpeciesFitness() const;

            long double getMinFitness() const;
            long double getMaxFitness() const;
            long double getAvgFitness() const;

            Generation& operator=( const Generation& other );

        protected:

            Generation( uint64_t genNum, bool pointers, const SpeciesManager& speciesTracker, PopulationSpeciesFitnessData& fitnessData );

            void clearStoredGenotypes();

            friend class Population;
    };
}

#endif // NEAT_GENERATION_HPP_INCLUDED
