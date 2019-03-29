#ifndef NEURON_PROPERTIES_INL_INCLUDED
#define NEURON_PROPERTIES_INL_INCLUDED

namespace spnn
{

    // setters

    template < typename Type, typename TimeType >
    void
    neuron_base< Type, TimeType >::setCallbackFunction( std::function< void( const neuron_base< Type, TimeType >& ) > func )
    {
        onActivationFunc = func;
    }


    // getters

    template < typename Type, typename TimeType >
    uint64_t
    neuron_base< Type, TimeType >::getID() const
    {
        return ID;
    }

    template < typename Type, typename TimeType >
    Type
    neuron_base< Type, TimeType >::getValue() const
    {
        return value;
    }

    template < typename Type, typename TimeType >
    TimeType
    neuron_base< Type, TimeType >::getRefractoryCount() const
    {
        return refractoryCount;
    }

    template < typename Type, typename TimeType >
    bool
    neuron_base< Type, TimeType >::getIsActive() const
    {
        return ( value >= threshold_min );
    }

    template < typename Type, typename TimeType >
    uint64_t
    neuron_base< Type, TimeType >::getNumActivations() const
    {
        return numActivations;
    }

    template < typename Type, typename TimeType >
    size_t
    neuron_base< Type, TimeType >::getNumSynapses() const
    {
        return synapses.size();
    }

    template < typename Type, typename TimeType >
    long double
    neuron_base< Type, TimeType >::getCurrentActivationPercent() const
    {
        long double rv = ( (long double)value - (long double)threshold_min ) / ( (long double)threshold_max - (long double)threshold_min );
        return std::min( std::max( rv, 0.0L ), 1.0L );
    }

    template < typename Type, typename TimeType >
    TimeType
    neuron_base< Type, TimeType >::currentRefractoryIfActivated() const
    {
        long double rel = getCurrentActivationPercent();
        return ( refractoryTime_low * ( 1.0 - rel ) ) + ( refractoryTime_high * rel );
    }

}

#endif // NEURON_PROPERTIES_INL_INCLUDED
