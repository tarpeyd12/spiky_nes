#ifndef SYNAPSE_INL_INCLUDED
#define SYNAPSE_INL_INCLUDED

namespace spnn
{

    // pulse_base

    // constructors

    template < typename Type, typename TimeType >
    pulse_base< Type, TimeType >::pulse_base()
        : time( 0 ), value( 0 ), destination( nullptr ), source( nullptr )
    {
        /*  */
    }

    template < typename Type, typename TimeType >
    pulse_base< Type, TimeType >::pulse_base( neuron_base< Type, TimeType > * dest, const TimeType& t, const Type& v )
        : time( t ), value( v ), destination( dest ), source( nullptr )
    {
        /*  */
    }

    template < typename Type, typename TimeType >
    pulse_base< Type, TimeType >::pulse_base( neuron_base< Type, TimeType > * dest, neuron_base< Type, TimeType > * src, const TimeType& t, const Type& v )
        : time( t ), value( v ), destination( dest ), source( src )
    {
        /*  */
    }


    // synapse_base

    // constructors

    template < typename Type, typename TimeType >
    synapse_base< Type, TimeType >::synapse_base()
        : length( 0 ), value( 0 ), destination( nullptr ), source( nullptr )
    {
        /*  */
    }

    template < typename Type, typename TimeType >
    synapse_base< Type, TimeType >::synapse_base( neuron_base< Type, TimeType > * dest, const TimeType& l, const Type& v )
        : length( l ), value( v ), destination( dest ), source( nullptr )
    {
        /*  */
    }

    // methods

    template < typename Type, typename TimeType >
    pulse_base< Type, TimeType >
    synapse_base< Type, TimeType >::generatePulse( const TimeType& time )
    {
        return pulse_base< Type, TimeType >( destination, source, time + length, value );
    }
}

#endif // SYNAPSE_INL_INCLUDED
