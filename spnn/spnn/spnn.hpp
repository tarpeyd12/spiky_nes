#ifndef SPNN_SPNN_HPP_INCLUDED
#define SPNN_SPNN_HPP_INCLUDED

#include "synapse.hpp"
#include "neuron.hpp"
#include "pulse_manager.hpp"
#include "network.hpp"

namespace spnn
{
    using synapse      = synapse_base< float, uint64_t >;
    using pulse        = pulse_base< float, uint64_t >;
    using neuron       = neuron_base< float, uint64_t >;
    using pulseManager = pulseManager_base< float, uint64_t >;
    using network      = network_base< float, uint64_t >;
}


#endif // SPNN_SPNN_HPP_INCLUDED
