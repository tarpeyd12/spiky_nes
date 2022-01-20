#include "population.hpp"

namespace neat
{
    PopulationFitness::Species::Species()
     :
        species_id( 0 ),
        species_fitness( 0.0L ),
        fitness_bounds(),
        genotype_fitnesses()
    {
        /*  */
    }

    PopulationFitness::Genotype&
    PopulationFitness::Species::get_highest_fitness_individual()
    {
        if( genotype_fitnesses.empty() )
        {
            // error
        }

        auto& best = genotype_fitnesses.front();
        for_each_genotype( [&]( Genotype& data )
        {
            if( data.genotype_fitness > best.genotype_fitness )
            {
                best = data;
            }
        } );

        return best;
    }

    void
    PopulationFitness::Species::for_each_genotype( std::function< void(Genotype&) > func )
    {
        for( auto& g : genotype_fitnesses )
        {
            func( g );
        }
    }

    void
    PopulationFitness::Species::for_each_genotype( std::function< void(const Genotype&) > func ) const
    {
        for( const auto& g : genotype_fitnesses )
        {
            func( g );
        }
    }

    bool
    PopulationFitness::Species::sort_species_geotypes_by_fitness()
    {
        std::sort( genotype_fitnesses.begin(), genotype_fitnesses.end(), [](const auto& a, const auto& b){ return a.genotype_fitness > b.genotype_fitness; } );
        return true;
    }

    PopulationFitness::PopulationFitness()
     :
        fitness_data(),
        fitness_bounds(),
        finalized( false )
    {
        /*  */
    }

    PopulationFitness::PopulationFitness( const PopulationFitness& other )
     : PopulationFitness()
    {
        other.for_each_genotype( [&]( const Genotype& g ){ addFitnessData( g ); } );

        if( other.is_finalized() )
        {
            Finalize();
        }
    }

    bool
    PopulationFitness::has_species( SpeciesID species ) const
    {
        return fitness_data.find( species ) != fitness_data.end();
    }

    bool
    PopulationFitness::is_finalized() const
    {
        return finalized;
    }

    size_t
    PopulationFitness::get_num_species() const
    {
        return fitness_data.size();
    }

    std::vector< SpeciesID >
    PopulationFitness::get_species_ids() const
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


    PopulationFitness::Species&
    PopulationFitness::get_species( SpeciesID species )
    {
        if( !is_finalized() )
        {
            // error
        }

        return fitness_data.at( species );
    }

    MinMax< long double >
    PopulationFitness::get_fitness_bounds() const
    {
        return fitness_bounds;
    }

    PopulationFitness::Species&
    PopulationFitness::get_highestFitnessSpecies()
    {
        if( !is_finalized() )
        {
            // error
        }

        Species& best = fitness_data.begin()->second;
        for_each_species( [&]( Species& data )
        {
            if( data.species_fitness > best.species_fitness )
            {
                best = data;
            }
        } );

        return best;
    }

    PopulationFitness::Genotype&
    PopulationFitness::get_highestFitnessIndividual()
    {
        if( !is_finalized() )
        {
            // error
        }

        Genotype& best = fitness_data.begin()->second.genotype_fitnesses.front();
        for_each_genotype( [&]( Genotype& data )
        {
            if( data.genotype_fitness > best.genotype_fitness )
            {
                best = data;
            }
        } );

        return best;
    }

    void
    PopulationFitness::for_each_species( std::function< void(Species&) > func )
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
    PopulationFitness::for_each_species( std::function< void(const Species&) > func ) const
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
    PopulationFitness::for_each_genotype( std::function< void(Genotype&) > func )
    {
        if( !func )
        {
            return;
        }

        for_each_species( [&]( Species& data ){ data.for_each_genotype( func ); } );
    }

    void
    PopulationFitness::for_each_genotype( std::function< void(const Genotype&) > func ) const
    {
        if( !func )
        {
            return;
        }

        for_each_species( [&]( const Species& data ){ data.for_each_genotype( func ); } );
    }

    void
    PopulationFitness::addFitnessData( const Species& species_pack )
    {
        for( auto& genotype_data : species_pack.genotype_fitnesses )
        {
            addFitnessData( genotype_data );
        }
    }

    void
    PopulationFitness::addFitnessData( const Genotype& genotype_pack )
    {
        // add and/or get reff to specified species_id fitness data
        auto& species_data = fitness_data[ genotype_pack.species_id ];

        species_data.species_id = genotype_pack.species_id;
        species_data.genotype_fitnesses.emplace_back( genotype_pack );

        // flag that things need to be re-finalized
        finalized = false;
    }

    void PopulationFitness::addFitnessData( SpeciesID species, long double fitness, const NetworkGenotype * genotype )
    {
        addFitnessData( { species, fitness, genotype } );
    }

    void PopulationFitness::Finalize()
    {
        fitness_bounds = MinMax< long double >( fitness_data.begin()->second.genotype_fitnesses.front().genotype_fitness );

        for_each_species( [&]( Species& species_data )
        {
            species_data.species_id = species_data.genotype_fitnesses.front().species_id;
            species_data.fitness_bounds = MinMax< long double >( species_data.genotype_fitnesses.front().genotype_fitness );

            // average the genotype fitnesses for each species
            /*
            {
                species_data.species_fitness = std::accumulate(
                    species_data.genotype_fitnesses.begin(),
                    species_data.genotype_fitnesses.end(),
                    0.0L,
                    [&species_data]( long double a, const Genotype& b )
                    {
                        species_data.fitness_bounds.expand( b.genotype_fitness );
                        return a + b.genotype_fitness;
                    }
                );
                species_data.species_fitness /= (long double)(species_data.genotype_fitnesses.size());
            }
            */

            // median
            {
                species_data.sort_species_geotypes_by_fitness();

                species_data.fitness_bounds.expand( species_data.genotype_fitnesses.front().genotype_fitness );
                species_data.fitness_bounds.expand( species_data.genotype_fitnesses.back().genotype_fitness );

                size_t midpoint = species_data.genotype_fitnesses.size() / 2;

                species_data.species_fitness = species_data.genotype_fitnesses[ midpoint ].genotype_fitness;

                // median for even-length sets is averaged between the two center-most indexes
                if( species_data.genotype_fitnesses.size() % 2 == 0 && species_data.genotype_fitnesses.size() >= 2 )
                {
                    long double fit1 = species_data.species_fitness; // we already got the first of the two fitnesses
                    long double fit2 = species_data.genotype_fitnesses[ midpoint + 1 ].genotype_fitness;

                    species_data.species_fitness = ( fit1 + fit2 ) / 2.0L;

                }
            }

            fitness_bounds.expand( species_data.fitness_bounds );
        } );

        finalized = true;
    }

    bool
    PopulationFitness::sort_species_geotypes_by_fitness( SpeciesID species_id )
    {
        if( !is_finalized() || !has_species( species_id ) )
        {
            return false;
        }

        return get_species( species_id ).sort_species_geotypes_by_fitness();
    }
}
