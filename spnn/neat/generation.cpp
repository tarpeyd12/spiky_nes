
#include "generation.hpp"

namespace neat
{
    Generation::Generation()
         : keepCopyOfGenotypes( false ), generationNumber( ~1 ), speciesData( nullptr ), populationData(), speciesFitness(), minFitness( 0.0 ), maxFitness( 0.0 ), avgFitness( 0.0 )
    {
        /*  */
    }

    Generation::Generation( const Generation& other )
         : keepCopyOfGenotypes( other.keepCopyOfGenotypes ), generationNumber( other.generationNumber ), speciesData( new SpeciesManager( *other.speciesData ) ), populationData( other.populationData ), speciesFitness( other.speciesFitness ), minFitness( other.minFitness ), maxFitness( other.maxFitness ), avgFitness( other.avgFitness )
    {
        /*  */
    }

    Generation::Generation( uint64_t genNum, bool pointers, const SpeciesManager& speciesTracker, const std::map< SpeciesID, std::pair< long double, std::vector< std::pair< long double, const NetworkGenotype * > > > >& fitnessData )
         : keepCopyOfGenotypes( pointers ), generationNumber( genNum ), speciesData( new SpeciesManager( speciesTracker ) ), populationData(), speciesFitness(), minFitness( 0.0 ), maxFitness( 0.0 ), avgFitness( 0.0 )
    {
        if( !fitnessData.empty() && !fitnessData.begin()->second.second.empty() )
        {
            minFitness = maxFitness = fitnessData.begin()->second.second[0].first;
        }


        for( auto map_it : fitnessData )
        {
            SpeciesID speciesID = map_it.first;
            long double speciesFit = map_it.second.first;

            speciesFitness.emplace( speciesID, speciesFit );

            const auto& speciesGenotypesFitnesPairs = map_it.second.second;
            for( auto p : speciesGenotypesFitnesPairs )
            {
                long double fitness = p.first;
                std::shared_ptr< NetworkGenotype > genotype_copy = nullptr;

                if( keepsGenotypeCopies() )
                {
                    genotype_copy = std::make_shared< NetworkGenotype >( *p.second );
                }

                populationData.push_back( std::make_tuple( speciesID, fitness, genotype_copy ) );

                {
                    minFitness = std::min( minFitness, fitness );
                    maxFitness = std::max( maxFitness, fitness );
                    avgFitness += fitness;
                }
            }
        }
        avgFitness /= (long double)( populationData.size() );
    }

    bool
    Generation::keepsGenotypeCopies() const
    {
        return keepCopyOfGenotypes;
    }

    uint64_t
    Generation::getGenerationNumber() const
    {
        return generationNumber;
    }

    size_t
    Generation::getNumGenotypes() const
    {
        return populationData.size();
    }

    SpeciesManager
    Generation::getSpeciesManager() const
    {
        return *speciesData;
    }

    std::vector< SpeciesID >
    Generation::getSpeciesIDVector() const
    {
        std::vector< SpeciesID > out;

        for( auto t : populationData )
        {
            out.push_back( std::get<0>( t ) );
        }

        return out;
    }

    std::vector< long double >
    Generation::getFitnessVector() const
    {
        std::vector< long double > out;

        for( auto t : populationData )
        {
            out.push_back( std::get<1>( t ) );
        }

        return out;
    }

    std::vector< std::weak_ptr< NetworkGenotype > >
    Generation::getGenotypesVector() const
    {
        std::vector< std::weak_ptr< NetworkGenotype > > out;

        if( !keepsGenotypeCopies() )
        {
            return out;
        }

        for( auto t : populationData )
        {
            out.push_back( std::get<2>( t ) );
        }

        return out;
    }

    std::map< SpeciesID, size_t >
    Generation::getSpeciesPresent() const
    {
        std::map< SpeciesID, size_t > out;

        for( auto t : populationData )
        {
            out[ std::get<0>( t ) ]++;
        }

        return out;
    }

    std::map< SpeciesID, long double >
    Generation::getSpeciesFitness() const
    {
        return speciesFitness;
    }

    long double
    Generation::getMinFitness() const
    {
        return minFitness;
    }

    long double
    Generation::getMaxFitness() const
    {
        return maxFitness;
    }

    long double
    Generation::getAvgFitness() const
    {
        return avgFitness;
    }

    void
    Generation::clearStoredGenotypes()
    {
        for( auto t : populationData )
        {
            std::get<2>( t ) = nullptr;
        }
    }
}
