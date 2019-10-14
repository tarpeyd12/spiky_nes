#include "population.hpp"

namespace neat
{
    void
    Population::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool, std::shared_ptr<xml::DataBlob> data_blob, bool use_datablob_fallback  )
    {
        auto population_node = xml::Node( "population", "", mem_pool );

        bool internal_data_blob = false;
        if( data_blob == nullptr && use_datablob_fallback )
        {
            data_blob = std::make_shared< xml::DataBlob >();
            internal_data_blob = true;
        }

        {
            auto settings = xml::Node( "settings", "", mem_pool );

            xml::Encode_MutationRates( mutationRates, settings, mem_pool );
            xml::Encode_MutationLimits( mutationLimits, settings, mem_pool );

            xml::Encode_SpeciesDistanceParameters( speciesTracker->getClassificationParameters(), settings, mem_pool );

            xml::appendSimpleValueNode( "generation_data_to_keep", generationDataToKeep, settings, mem_pool );

            // add to population node
            population_node->append_node( settings );
        }

        // mutatorFunctor
        {
            auto mutator_functor_node = xml::Node( "mutator_functor", "", mem_pool );

            if( mutatorFunctor != nullptr )
            {
                mutatorFunctor->SaveToXML( mutator_functor_node, mem_pool );
            }

            population_node->append_node( mutator_functor_node );
        }


        {
            auto state = xml::Node( "state", "", mem_pool );

            xml::appendSimpleValueNode( "num_networks", numNetworks, state, mem_pool );
            xml::appendSimpleValueNode( "num_input_nodes", numInputNodes, state, mem_pool );
            xml::appendSimpleValueNode( "num_output_nodes", numOutputNodes, state, mem_pool );
            xml::appendSimpleValueNode( "generation_count", generationCount, state, mem_pool );

            // innovationCounter
            innovationCounter->SaveToXML( state, mem_pool );

            // minSpeciesSize
            xml::appendSimpleValueNode( "min_species_size", minSpeciesSize, state, mem_pool );
            // killDelayLimit
            xml::appendSimpleValueNode( "kill_delay_limit", killDelayLimit, state, mem_pool );
            // speciesKillDelay
            {
                auto kill_delay = xml::Node( "species_kill_delay", "", mem_pool );
                //kill_delay->append_attribute( xml::Attribute( "N", xml::to_string( speciesKillDelay.size() ), mem_pool ) );
                for( auto kd : speciesKillDelay )
                {
                    if( !kd.second )
                    {
                        continue;
                    }
                    auto kn_node = xml::Node( "species", "", mem_pool );
                    kn_node->append_attribute( xml::Attribute( "ID", xml::to_string( kd.first ), mem_pool ) );
                    kn_node->append_attribute( xml::Attribute( "delay", xml::to_string( kd.second ), mem_pool ) );
                    kill_delay->append_node( kn_node );
                }
                state->append_node( kill_delay );
            }

            // massExtinctionTimer
            xml::appendSimpleValueNode( "mass_extinction_timer", massExtinctionTimer, state, mem_pool );
            // massExtinctionCount
            xml::appendSimpleValueNode( "mass_extinction_count", massExtinctionCount, state, mem_pool );
            // pastFitness
            {
                auto past_fitness = xml::Node( "past_fitness", "", mem_pool );
                past_fitness->append_attribute( xml::Attribute( "N", xml::to_string( pastFitness.size() ), mem_pool ) );
                for( auto fitness : pastFitness )
                {
                    xml::appendSimpleValueNode( "fitness", fitness, past_fitness, mem_pool );
                }
                state->append_node( past_fitness );
            }

            // add to population node
            population_node->append_node( state );
        }

        // populationData
        {
            auto pop_data = xml::Node( "population_data", "", mem_pool );
            pop_data->append_attribute( xml::Attribute( "N", xml::to_string( populationData.size() ), mem_pool ) );
            for( const auto& genotype : populationData )
            {
                xml::Encode_NetworkGenotype( genotype, pop_data, mem_pool, data_blob );
            }

            // add to population node
            population_node->append_node( pop_data );
        }

        // speciesTracker
        speciesTracker->SaveToXML( population_node, mem_pool, data_blob );

        // save the data blob
        if( data_blob != nullptr && internal_data_blob )
        {
            data_blob->SaveToXML( population_node, mem_pool, { "zlib" } );
        }

        // add node to destination
        destination->append_node( population_node );
    }

    Population::Population( std::shared_ptr< FitnessFactory > fitFactory, std::shared_ptr< Mutations::MutationsFileLoadFactory > mutations_factory, const rapidxml::xml_node<> * population_node, std::shared_ptr<xml::DataBlob> data_blob )
        :
        innovationCounter( nullptr ),
        numNetworks( ~0L ),
        numInputNodes( ~0L ),
        numOutputNodes( ~0L ),
        generationCount( ~0L ),
        initialGenotypeTemplate( nullptr ),
        populationData(),
        //inputNodeIDs(),
        //outputNodeIDs(),
        speciesTracker( nullptr ),
        mutationLimits(),
        mutationRates(),
        mutatorFunctor( nullptr ),
        fitnessCalculatorFactory( fitFactory ),
        generationDataToKeep( ~0L ),
        generationLog(),
        minSpeciesSize( ~0L ),
        speciesKillDelay(),
        killDelayLimit( ~0L ),
        massExtinctionTimer( ~0L ),
        pastFitness(),
        massExtinctionCount( ~0L )
    {
        assert( population_node && neat::xml::Name( population_node ) == "population" );

        SpeciesDistanceParameters species_distance_params;

        // find/init datablob
        if( data_blob == nullptr )
        {
            auto data_blob_node = xml::FindNode( "data_blob", population_node );
            if( data_blob_node != nullptr )
            {
                data_blob = std::make_shared<xml::DataBlob>( data_blob_node );
            }
        }

        {
            auto settings_node = xml::FindNode( "settings", population_node );

            mutationRates = xml::Decode_MutationRates( xml::FindNode( "mutation_rates", settings_node ) );
            mutationLimits = xml::Decode_MutationLimits( xml::FindNode( "mutation_limits", settings_node ) );
            species_distance_params = xml::Decode_SpeciesDistanceParameters( xml::FindNode( "species_distance_parameters", settings_node ) );

            xml::readSimpleValueNode( "generation_data_to_keep", generationDataToKeep, settings_node );
        }

        {
            auto mutator_functor_node = xml::FindNode( "mutator_functor", population_node );

            mutatorFunctor = (*mutations_factory)( mutator_functor_node->first_node() );
        }

        {
            auto state_node = xml::FindNode( "state", population_node );

            xml::readSimpleValueNode( "num_networks", numNetworks, state_node );
            xml::readSimpleValueNode( "num_input_nodes", numInputNodes, state_node );
            xml::readSimpleValueNode( "num_output_nodes", numOutputNodes, state_node );
            xml::readSimpleValueNode( "generation_count", generationCount, state_node );

            {
                auto innovation_counter_node = xml::FindNode( "innovation_generator", state_node );

                innovationCounter = std::make_unique< InnovationGenerator >( innovation_counter_node );
            }

            xml::readSimpleValueNode( "min_species_size", minSpeciesSize, state_node );
            xml::readSimpleValueNode( "kill_delay_limit", killDelayLimit, state_node );

            {
                auto species_kill_delay_node = xml::FindNode( "species_kill_delay", state_node );

                auto species_delay_node = species_kill_delay_node->first_node();

                while( species_delay_node != nullptr )
                {
                    if( xml::Name( species_delay_node ) != "species" )
                    {
                        species_delay_node = species_delay_node->next_sibling();
                        continue;
                    }
                    speciesKillDelay[ xml::GetAttributeValue< SpeciesID >( "ID", species_delay_node ) ] = xml::GetAttributeValue< size_t >( "delay", species_delay_node );

                    species_delay_node = species_delay_node->next_sibling();
                }
            }

            xml::readSimpleValueNode( "mass_extinction_timer", massExtinctionTimer, state_node );
            xml::readSimpleValueNode( "mass_extinction_count", massExtinctionCount, state_node );

            {
                auto past_fitness_node = xml::FindNode( "past_fitness", state_node );

                size_t _num_past_fitnesses_expected = xml::GetAttributeValue<size_t>( "N", past_fitness_node );

                auto fitness_node = past_fitness_node->first_node();

                while( fitness_node != nullptr )
                {
                    if( xml::Name( fitness_node ) != "fitness" )
                    {
                        fitness_node = fitness_node->next_sibling();
                        continue;
                    }

                    pastFitness.emplace_back( xml::GetAttributeValue< long double >( "value", fitness_node ) );

                    fitness_node = fitness_node->next_sibling();
                }

                assert( pastFitness.size() == _num_past_fitnesses_expected );
            }
        }

        {
            auto population_data_node = xml::FindNode( "population_data", population_node );

            size_t _num_genotypes_expected = xml::GetAttributeValue<size_t>( "N", population_data_node );

            populationData.reserve( _num_genotypes_expected );

            auto genotype_node = population_data_node->first_node();

            while( genotype_node != nullptr )
            {
                populationData.emplace_back( xml::Decode_NetworkGenotype( genotype_node, data_blob ) );
                genotype_node = genotype_node->next_sibling();
            }

            assert( populationData.size() == _num_genotypes_expected );
        }

        {
            auto species_tracker_node = xml::FindNode( "species_tracker", population_node );

            speciesTracker = std::make_unique< SpeciesManager >( species_tracker_node, species_distance_params, data_blob );
        }
    }
}
