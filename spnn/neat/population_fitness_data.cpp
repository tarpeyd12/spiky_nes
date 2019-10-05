#include "population.hpp"

namespace neat
{
    PopulationSpeciesFitnessData::PopulationSpeciesFitnessData()
     :
        fitness_data(),
        finalized( false )
    {
        /*  */
    }

    bool
    PopulationSpeciesFitnessData::has_species( SpeciesID species ) const
    {
        return fitness_data.find( species ) != fitness_data.end();
    }

    bool
    PopulationSpeciesFitnessData::is_finalized() const
    {
        return finalized;
    }

    size_t
    PopulationSpeciesFitnessData::get_num_species() const
    {
        return fitness_data.size();
    }

    std::vector< SpeciesID >
    PopulationSpeciesFitnessData::get_species_ids() const
    {
        if( !is_finalized() )
        {
            // error
        }

        // make a vector of the keys in fitness_data
        std::vector< SpeciesID > out( get_num_species() );
        std::generate( out.begin(), out.end(), [it = fitness_data.begin()]() mutable { return it++->first; } );
        return out;
    }


    PopulationSpeciesFitnessData::SpeciesFitnessPackage&
    PopulationSpeciesFitnessData::get_species( SpeciesID species )
    {
        if( !is_finalized() )
        {
            // error
        }

        return fitness_data.at( species );
    }

    PopulationSpeciesFitnessData::SpeciesFitnessPackage&
    PopulationSpeciesFitnessData::get_highestFitnessSpecies()
    {
        if( !is_finalized() )
        {
            // error
        }

        SpeciesFitnessPackage& best = fitness_data.begin()->second;
        for( auto& data : fitness_data )
        {
            if( data.second.species_fitness > best.species_fitness )
            {
                best = data.second;
            }
        }
        return best;
    }

    PopulationSpeciesFitnessData::GenotypeFitnessPackage&
    PopulationSpeciesFitnessData::get_highestFitnessIndividual()
    {
        if( !is_finalized() )
        {
            // error
        }

        GenotypeFitnessPackage& best = fitness_data.begin()->second.genotype_fitnesses.front();
        for( auto& species_data : fitness_data )
        {
            for( GenotypeFitnessPackage& data : species_data.second.genotype_fitnesses )
            {
                if( data.genotype_fitness > best.genotype_fitness )
                {
                    best = data;
                }
            }
        }
        return best;
    }

    void
    PopulationSpeciesFitnessData::addFitnessData( const GenotypeFitnessPackage& genotype_pack )
    {
        fitness_data[ genotype_pack.species_id ].genotype_fitnesses.emplace_back( genotype_pack );
        finalized = false;
    }

    void PopulationSpeciesFitnessData::addFitnessData( SpeciesID species, long double fitness, const NetworkGenotype * genotype )
    {
        addFitnessData( { species, fitness, genotype } );
    }

    void PopulationSpeciesFitnessData::Finalize()
    {
        // average the genotype fitnesses for each species
        for( auto& species_data_pair : fitness_data )
        {
            species_data_pair.second.species_fitness = std::accumulate(
                species_data_pair.second.genotype_fitnesses.begin(),
                species_data_pair.second.genotype_fitnesses.end(),
                0.0L,
                [&]( long double a, const GenotypeFitnessPackage& b )
                {
                    return a + b.genotype_fitness;
                }
            );
            species_data_pair.second.species_fitness /= (long double)(species_data_pair.second.genotype_fitnesses.size());
        }

        finalized = true;
    }

    bool
    PopulationSpeciesFitnessData::sort_species_geotypes_by_fitness( SpeciesID species_id )
    {
        if( !is_finalized() || !has_species( species_id ) )
        {
            return false;
        }

        auto& genotype_fitnesses_vec = get_species( species_id ).genotype_fitnesses;
        std::sort( genotype_fitnesses_vec.begin(), genotype_fitnesses_vec.end(), [](const auto& a, const auto& b){ return a.genotype_fitness > b.genotype_fitness; } );
        return true;
    }
}
