#ifndef NEURON_INL_INCLUDED
#define NEURON_INL_INCLUDED

#include <cassert>

namespace spnn
{

    // constructors

    template < typename Type, typename TimeType >
    neuron_base< Type, TimeType >::neuron_base()
        : ID( totalNeurons++ ), numActivations( 0 ), threshold_min( 0 ), threshold_max( 0 ), refractoryTime_high( 0 ), refractoryTime_low( 0 ), valueDecay( 0 ), activationDecay( 0 ), synapses(), value( 0 ), refractoryCount( 0 ), refractoryCountForLastActivation( 0 )
    {
        /*  */
    }

    template < typename Type, typename TimeType >
    neuron_base< Type, TimeType >::neuron_base( const Type& t_min, const Type& t_max, const TimeType& rt_high, const TimeType& rt_low, const Type& vdec, const Type& adec, const std::vector< synapse_base< Type, TimeType > >& syns )
        : ID( totalNeurons++ ), numActivations( 0 ), threshold_min( t_min ), threshold_max( t_max ), refractoryTime_high( rt_high ), refractoryTime_low( rt_low ), valueDecay( vdec ), activationDecay( adec ), synapses( syns.size() ), onActivationFunc(), value( 0 ), refractoryCount( 0 ), refractoryCountForLastActivation( 0 )
    {
        assert( threshold_max >= threshold_min && "Neuron cannot Tick with reverse threshold min/max." );
        assert( refractoryTime_low >= refractoryTime_high && "Neuron cannot Tick with reverse refractory times min/max." );

        for( auto syn : syns )
        {
            // null safety, synapse length cannot be 0 or less
            if( syn.destination == nullptr || syn.length <= 0 )
            {
                continue;
            }

            synapse_base< Type, TimeType > syn2( syn );

            syn2.source = this;

            synapses.push_back( syn2 );
        }
    }

    template < typename Type, typename TimeType >
    neuron_base< Type, TimeType >::neuron_base( const Type& t_min, const Type& t_max, const TimeType& rt_high, const TimeType& rt_low, const Type& vdec, const Type& adec, const std::function< void( const neuron_base< Type, TimeType >& ) >& func )
        : ID( totalNeurons++ ), numActivations( 0 ), threshold_min( t_min ), threshold_max( t_max ), refractoryTime_high( rt_high ), refractoryTime_low( rt_low ), valueDecay( vdec ), activationDecay( adec ), synapses(), onActivationFunc( func ), value( 0 ), refractoryCount( 0 ), refractoryCountForLastActivation( 0 )
    {
        assert( threshold_max >= threshold_min && "Neuron cannot Tick with reverse threshold min/max." );
        assert( refractoryTime_low >= refractoryTime_high && "Neuron cannot Tick with reverse refractory times min/max." );

        /*  */
    }

    template < typename Type, typename TimeType >
    neuron_base< Type, TimeType >::neuron_base( const Type& t_min, const Type& t_max, const TimeType& rt_high, const TimeType& rt_low, const Type& vdec, const Type& adec, const std::vector< synapse_base< Type, TimeType > >& syns, const std::function< void( const neuron_base< Type, TimeType >& ) >& func )
        : ID( totalNeurons++ ), numActivations( 0 ), threshold_min( t_min ), threshold_max( t_max ), refractoryTime_high( rt_high ), refractoryTime_low( rt_low ), valueDecay( vdec ), activationDecay( adec ), synapses( syns.size() ), onActivationFunc( func ), value( 0 ), refractoryCount( 0 ), refractoryCountForLastActivation( 0 )
    {
        assert( threshold_max >= threshold_min && "Neuron cannot Tick with reverse threshold min/max." );
        assert( refractoryTime_low >= refractoryTime_high && "Neuron cannot Tick with reverse refractory times min/max." );

        for( auto syn : syns )
        {
            // null safety, synapse length cannot be 0 or less
            if( syn.destination == nullptr || syn.length <= 0 )
            {
                continue;
            }

            synapse_base< Type, TimeType > syn2( syn );

            syn2.source = this;

            synapses.push_back( syn2 );
        }
    }

    // destructor

    template < typename Type, typename TimeType >
    neuron_base< Type, TimeType >::~neuron_base()
    {
        /*  */
    }

}

#endif // NEURON_INL_INCLUDED
