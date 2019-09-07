#include "population.hpp"

namespace neat
{
    void
    Population::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )
    {
        auto population_node = xml::Node( "population", "", mem_pool );

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

            mutatorFunctor->SaveToXML( mutator_functor_node, mem_pool );

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
                xml::Encode_NetworkGenotype( genotype, pop_data, mem_pool );
            }

            // add to population node
            population_node->append_node( pop_data );
        }

        // speciesTracker
        //population_node->append_node( xml::Node( "species_tracker", "TODO", mem_pool ) );
        speciesTracker->SaveToXML( population_node, mem_pool );

        // add node to destination
        destination->append_node( population_node );
    }
}
