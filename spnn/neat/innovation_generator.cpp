#include "innovation_generator.hpp"

namespace neat
{
    void
    InnovationGenerator::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )
    {
        std::unique_lock< std::mutex > node_lock( nodeCounter_mutex, std::defer_lock );
        std::unique_lock< std::mutex > innov_lock( innovationCounter_mutex, std::defer_lock );
        std::unique_lock< std::mutex > gen_lock( generationConnections_mutex, std::defer_lock );
        std::lock( node_lock, innov_lock, gen_lock );

        auto node = xml::Node( "innovation_generator", "", mem_pool );

        node->append_attribute( xml::Attribute( "innovation_counter", xml::to_string( innovationCounter ), mem_pool ) );
        node->append_attribute( xml::Attribute( "node_counter", xml::to_string( nodeCounter ), mem_pool ) );

        destination->append_node( node );
    }
}
