#ifndef SPNN_SYNAPSE_HPP_INCLUDED
#define SPNN_SYNAPSE_HPP_INCLUDED

// TODO(dot##9/17/2018): rename synapse to axon to be more accurate.

namespace spnn
{
    template < typename Type, typename TimeType > struct pulse_base;
    template < typename Type, typename TimeType > struct synapse_base;
}

#include "neuron.hpp"

namespace spnn
{

    template < typename Type, typename TimeType >
    struct pulse_base
    {
        // data

        TimeType time;
        Type value;
        neuron_base< Type, TimeType > * destination;
        neuron_base< Type, TimeType > * source;

        // constructors

        pulse_base();
        pulse_base( neuron_base< Type, TimeType > * dest, const TimeType& t, const Type& v );
        pulse_base( neuron_base< Type, TimeType > * dest, neuron_base< Type, TimeType > * src, const TimeType& t, const Type& v );
    };

    template <  typename Type, typename TimeType  >
    struct synapse_base
    {
        // data

        TimeType length;
        Type value;
        neuron_base< Type, TimeType > * destination;
        neuron_base< Type, TimeType > * source;

        // constructors

        synapse_base();
        synapse_base( neuron_base< Type, TimeType > * dest, const TimeType& l, const Type& v );

        // methods

        pulse_base< Type, TimeType > generatePulse( const TimeType& time );
    };

}

#include "synapse.inl"

#endif // SPNN_SYNAPSE_HPP_INCLUDED
