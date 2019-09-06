#include "mutations.hpp"

namespace neat
{
    namespace Mutations
    {
        void
        Mutation_Multi::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool ) const
        {
            auto root_node = xml::Node( "mutation_multi", "", mem_pool );

            for( const auto& mut_chance : mutators )
            {
                if( mut_chance.mutator == nullptr )
                {
                    continue;
                }

                mut_chance.mutator->SaveToXML( root_node, mem_pool );
                {
                    auto added_node = root_node->last_node();

                    added_node->append_attribute( xml::Attribute( "base_chance", xml::to_string( mut_chance.baseChance ), mem_pool ) );
                    added_node->append_attribute( xml::Attribute( "node_chance", xml::to_string( mut_chance.nodeChance ), mem_pool ) );
                    added_node->append_attribute( xml::Attribute( "conn_chance", xml::to_string( mut_chance.connChance ), mem_pool ) );
                }
            }

            // add node to destination
            destination->append_node( root_node );
        }

        void
        Mutation_Multi_one::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool ) const
        {
            auto root_node = xml::Node( "mutation_multi_one", "", mem_pool );

            for( const auto& mutator : mutators )
            {
                if( mutator == nullptr )
                {
                    continue;
                }

                mutator->SaveToXML( root_node, mem_pool );
            }

            // add node to destination
            destination->append_node( root_node );
        }

        void Mutation_Node_thresh_min::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )       const { destination->append_node( xml::Node( "mutation_node_thresh_min", "", mem_pool ) ); }
        void Mutation_Node_thresh_min_new::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )   const { destination->append_node( xml::Node( "mutation_node_thresh_min_new", "", mem_pool ) ); }
        void Mutation_Node_thresh_max::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )       const { destination->append_node( xml::Node( "mutation_node_thresh_max", "", mem_pool ) ); }
        void Mutation_Node_thresh_max_new::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )   const { destination->append_node( xml::Node( "mutation_node_thresh_max_new", "", mem_pool ) ); }
        void Mutation_Node_decays_value::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )     const { destination->append_node( xml::Node( "mutation_node_decays_value", "", mem_pool ) ); }
        void Mutation_Node_decays_value_new::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool ) const { destination->append_node( xml::Node( "mutation_node_decays_value_new", "", mem_pool ) ); }
        void Mutation_Node_decays_activ::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )     const { destination->append_node( xml::Node( "mutation_node_decays_activ", "", mem_pool ) ); }
        void Mutation_Node_decays_activ_new::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool ) const { destination->append_node( xml::Node( "mutation_node_decays_activ_new", "", mem_pool ) ); }
        void Mutation_Node_pulses_fast::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )      const { destination->append_node( xml::Node( "mutation_node_pulses_fast", "", mem_pool ) ); }
        void Mutation_Node_pulses_fast_new::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )  const { destination->append_node( xml::Node( "mutation_node_pulses_fast_new", "", mem_pool ) ); }
        void Mutation_Node_pulses_slow::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )      const { destination->append_node( xml::Node( "mutation_node_pulses_slow", "", mem_pool ) ); }
        void Mutation_Node_pulses_slow_new::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )  const { destination->append_node( xml::Node( "mutation_node_pulses_slow_new", "", mem_pool ) ); }

        void Mutation_Conn_weight::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )           const { destination->append_node( xml::Node( "mutation_conn_weight", "", mem_pool ) ); }
        void Mutation_Conn_weight_new::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )       const { destination->append_node( xml::Node( "mutation_conn_weight_new", "", mem_pool ) ); }
        void Mutation_Conn_length::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )           const { destination->append_node( xml::Node( "mutation_conn_length", "", mem_pool ) ); }
        void Mutation_Conn_length_new::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )       const { destination->append_node( xml::Node( "mutation_conn_length_new", "", mem_pool ) ); }
        void Mutation_Conn_enable::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )           const { destination->append_node( xml::Node( "mutation_conn_enable", "", mem_pool ) ); }

        void Mutation_Add_node::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )              const { destination->append_node( xml::Node( "mutation_add_node", "", mem_pool ) ); }
        void Mutation_Add_conn::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )              const { destination->append_node( xml::Node( "mutation_add_conn", "", mem_pool ) ); }
        void Mutation_Add_conn_unique::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )       const { destination->append_node( xml::Node( "mutation_add_conn_unique", "", mem_pool ) ); }
        void Mutation_Add_conn_dup::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )          const { destination->append_node( xml::Node( "mutation_add_conn_dup", "", mem_pool ) ); }

        void Mutation_Add_conn_multi_in::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )     const { destination->append_node( xml::Node( "mutation_add_conn_multi_in", "", mem_pool ) ); }
        void Mutation_Add_conn_multi_out::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )    const { destination->append_node( xml::Node( "mutation_add_conn_multi_out", "", mem_pool ) ); }


    }
}
