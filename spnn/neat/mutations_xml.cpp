#include "mutations.hpp"

namespace neat
{
    namespace Mutations
    {
        MutationsFileLoadFactory::MutationsFileLoadFactory()
             : functionMap()
        {
            add_factory( "mutation_node_thresh_min",       { [](const auto*){ return std::make_shared<Mutation_Node_thresh_min>(); } } );
            add_factory( "mutation_node_thresh_min_new",   { [](const auto*){ return std::make_shared<Mutation_Node_thresh_min_new>(); } } );
            add_factory( "mutation_node_thresh_max",       { [](const auto*){ return std::make_shared<Mutation_Node_thresh_max>(); } } );
            add_factory( "mutation_node_thresh_max_new",   { [](const auto*){ return std::make_shared<Mutation_Node_thresh_max_new>(); } } );
            add_factory( "mutation_node_decays_value",     { [](const auto*){ return std::make_shared<Mutation_Node_decays_value>(); } } );
            add_factory( "mutation_node_decays_value_new", { [](const auto*){ return std::make_shared<Mutation_Node_decays_value_new>(); } } );
            add_factory( "mutation_node_decays_activ",     { [](const auto*){ return std::make_shared<Mutation_Node_decays_activ>(); } } );
            add_factory( "mutation_node_decays_activ_new", { [](const auto*){ return std::make_shared<Mutation_Node_decays_activ_new>(); } } );
            add_factory( "mutation_node_pulses_fast",      { [](const auto*){ return std::make_shared<Mutation_Node_pulses_fast>(); } } );
            add_factory( "mutation_node_pulses_fast_new",  { [](const auto*){ return std::make_shared<Mutation_Node_pulses_fast_new>(); } } );
            add_factory( "mutation_node_pulses_slow",      { [](const auto*){ return std::make_shared<Mutation_Node_pulses_slow>(); } } );
            add_factory( "mutation_node_pulses_slow_new",  { [](const auto*){ return std::make_shared<Mutation_Node_pulses_slow_new>(); } } );

            add_factory( "mutation_conn_weight",           { [](const auto*){ return std::make_shared<Mutation_Conn_weight>(); } } );
            add_factory( "mutation_conn_weight_new",       { [](const auto*){ return std::make_shared<Mutation_Conn_weight_new>(); } } );
            add_factory( "mutation_conn_length",           { [](const auto*){ return std::make_shared<Mutation_Conn_length>(); } } );
            add_factory( "mutation_conn_length_new",       { [](const auto*){ return std::make_shared<Mutation_Conn_length_new>(); } } );
            add_factory( "mutation_conn_enable",           { [](const auto*){ return std::make_shared<Mutation_Conn_enable>(); } } );

            add_factory( "mutation_add_node",              { [](const auto*){ return std::make_shared<Mutation_Add_node>(); } } );
            add_factory( "mutation_add_conn",              { [](const auto*){ return std::make_shared<Mutation_Add_conn>(); } } );
            add_factory( "mutation_add_conn_unique",       { [](const auto*){ return std::make_shared<Mutation_Add_conn_unique>(); } } );
            add_factory( "mutation_add_conn_dup",          { [](const auto*){ return std::make_shared<Mutation_Add_conn_dup>(); } } );

            add_factory( "mutation_add_conn_multi_in",     { [](const auto*){ return std::make_shared<Mutation_Add_conn_multi_in>(); } } );
            add_factory( "mutation_add_conn_multi_out",    { [](const auto*){ return std::make_shared<Mutation_Add_conn_multi_out>(); } } );

            add_factory( "mutation_multi_one", {
                [this](const rapidxml::xml_node<> * source_node )
                {
                    auto out = std::make_shared<Mutation_Multi_one>();

                    auto node = source_node->first_node();

                    while( node != nullptr )
                    {
                        auto mutator = this->operator()( node );

                        if( mutator != nullptr )
                        {
                            out->addMutator( mutator );
                        }

                        node = node->next_sibling();
                    }

                    return out;
                }
            } );

            add_factory( "mutation_multi", {
                [this](const rapidxml::xml_node<> * source_node )
                {
                    auto out = std::make_shared<Mutation_Multi>();

                    auto node = source_node->first_node();

                    while( node != nullptr )
                    {
                        double baseChance = xml::GetAttributeValue<double>( "base_chance", node );
                        double nodeChance = xml::GetAttributeValue<double>( "node_chance", node );
                        double connChance = xml::GetAttributeValue<double>( "conn_chance", node );

                        auto mutator = this->operator()( node );

                        if( mutator != nullptr )
                        {
                            out->addMutator( baseChance, nodeChance, connChance, mutator );
                        }

                        node = node->next_sibling();
                    }

                    return out;
                }
            } );

        }

        std::shared_ptr< Mutation_base >
        MutationsFileLoadFactory::operator()( const rapidxml::xml_node<> * source_node ) const
        {
            assert( source_node != nullptr );

            std::string name = xml::Name( source_node );

            auto search = functionMap.find( name );

            if( search != functionMap.end() )
            {
                return search->second.constructor( source_node );
            }

            return nullptr;
        }

        void
        MutationsFileLoadFactory::add_factory( const std::string& name_tag, const ConstructionSet& init_funcs )
        {
            functionMap.emplace( name_tag, init_funcs );
        }


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
