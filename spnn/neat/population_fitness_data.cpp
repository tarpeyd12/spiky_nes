#include "population.hpp"

namespace neat
{
    PopulationSpeciesFitnessData::SpeciesFitnessPackage::SpeciesFitnessPackage()
     :
        species_id( 0 ),
        species_fitness( 0.0L ),
        fitness_bounds(),
        genotype_fitnesses()
    {
        /*  */
    }

    PopulationSpeciesFitnessData::GenotypeFitnessPackage&
    PopulationSpeciesFitnessData::SpeciesFitnessPackage::get_highest_fitness_individual()
    {
        if( genotype_fitnesses.empty() )
        {
            // error
        }

        GenotypeFitnessPackage& best = genotype_fitnesses.front();
        for( auto& genotype_data : genotype_fitnesses )
        {
            if( genotype_data.genotype_fitness > best.genotype_fitness )
            {
                best = genotype_data;
            }
        }

        return best;
    }

    void
    PopulationSpeciesFitnessData::SpeciesFitnessPackage::for_each_genotype( std::function< void(GenotypeFitnessPackage&) > func )
    {
        for( auto& g : genotype_fitnesses )
        {
            func( g );
        }
    }

    void
    PopulationSpeciesFitnessData::SpeciesFitnessPackage::for_each_genotype( std::function< void(const GenotypeFitnessPackage&) > func ) const
    {
        for( const auto& g : genotype_fitnesses )
        {
            func( g );
        }
    }

    bool
    PopulationSpeciesFitnessData::SpeciesFitnessPackage::sort_species_geotypes_by_fitness()
    {
        std::sort( genotype_fitnesses.begin(), genotype_fitnesses.end(), [](const auto& a, const auto& b){ return a.genotype_fitness > b.genotype_fitness; } );
        return true;
    }

    PopulationSpeciesFitnessData::PopulationSpeciesFitnessData()
     :
        fitness_data(),
        fitness_bounds(),
        finalized( false )
    {
        /*  */
    }

    PopulationSpeciesFitnessData::PopulationSpeciesFitnessData( const PopulationSpeciesFitnessData& other )
     : PopulationSpeciesFitnessData()
    {
        other.for_each_genotype( [&]( const auto& g ){ addFitnessData( g ); } );

        if( other.is_finalized() )
        {
            Finalize();
        }
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

    MinMax< long double >
    PopulationSpeciesFitnessData::get_fitness_bounds() const
    {
        return fitness_bounds;
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
    PopulationSpeciesFitnessData::for_each_species( std::function< void(SpeciesFitnessPackage&) > func )
    {
        if( !func )
        {
            return;
        }

        for( auto& species_data : fitness_data )
        {
            func( species_data.second );
        }
    }

    void
    PopulationSpeciesFitnessData::for_each_species( std::function< void(const SpeciesFitnessPackage&) > func ) const
    {
        if( !func )
        {
            return;
        }

        for( const auto& species_data : fitness_data )
        {
            func( species_data.second );
        }
    }

    void
    PopulationSpeciesFitnessData::for_each_genotype( std::function< void(GenotypeFitnessPackage&) > func )
    {
        if( !func )
        {
            return;
        }

        for( auto& species_data : fitness_data )
        {
            species_data.second.for_each_genotype( func );
        }
    }

    void
    PopulationSpeciesFitnessData::for_each_genotype( std::function< void(const GenotypeFitnessPackage&) > func ) const
    {
        if( !func )
        {
            return;
        }

        for( auto& species_data : fitness_data )
        {
            species_data.second.for_each_genotype( func );
        }
    }

    void
    PopulationSpeciesFitnessData::addFitnessData( const SpeciesFitnessPackage& species_pack )
    {
        for( auto& genotype_data : species_pack.genotype_fitnesses )
        {
            addFitnessData( genotype_data );
        }
    }

    void
    PopulationSpeciesFitnessData::addFitnessData( const GenotypeFitnessPackage& genotype_pack )
    {
        auto& species_data = fitness_data[ genotype_pack.species_id ];
        species_data.species_id = genotype_pack.species_id;
        species_data.genotype_fitnesses.emplace_back( genotype_pack );
        finalized = false;
    }

    void PopulationSpeciesFitnessData::addFitnessData( SpeciesID species, long double fitness, const NetworkGenotype * genotype )
    {
        addFitnessData( { species, fitness, genotype } );
    }

    void PopulationSpeciesFitnessData::Finalize()
    {
        fitness_bounds = MinMax< long double >( fitness_data.begin()->second.genotype_fitnesses.front().genotype_fitness );

        for( auto& species_data_pair : fitness_data )
        {
            auto& species_data = species_data_pair.second;
            species_data.species_id = species_data.genotype_fitnesses.front().species_id;
            species_data.fitness_bounds = MinMax< long double >( species_data.genotype_fitnesses.front().genotype_fitness );

            // average the genotype fitnesses for each species
            species_data.species_fitness = std::accumulate(
                species_data.genotype_fitnesses.begin(),
                species_data.genotype_fitnesses.end(),
                0.0L,
                [&species_data]( long double a, const GenotypeFitnessPackage& b )
                {
                    species_data.fitness_bounds.expand( b.genotype_fitness );
                    return a + b.genotype_fitness;
                }
            );
            species_data.species_fitness /= (long double)(species_data.genotype_fitnesses.size());

            fitness_bounds.expand( species_data.fitness_bounds );
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

        return get_species( species_id ).sort_species_geotypes_by_fitness();
    }
}
