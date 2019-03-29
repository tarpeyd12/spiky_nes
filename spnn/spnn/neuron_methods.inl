#ifndef NEURON_METHODS_INL_INCLUDED
#define NEURON_METHODS_INL_INCLUDED

#include <cassert>

namespace spnn
{

    // Tick Begin

    template < typename Type, typename TimeType >
    bool
    neuron_base< Type, TimeType >::Tick( pulseManager_base< Type, TimeType >& pulse_manager, const TimeType& time, const TimeType& dTime )
    {
        // sanity checks

        assert( dTime > 0 && "Neuron cannot Tick with deltaTime of 0." );
        assert( threshold_max >= threshold_min && "Neuron cannot Tick with reverse threshold min/max." );
        assert( refractoryTime_low >= refractoryTime_high && "Neuron cannot Tick with reverse refractory times min/max." );

        // clamp so we have a sane value ( do not clamp max, it breaks things )
        if( value < 0 )
        {
            value = 0;
        }

        // are we a sleepy neuron? if yes, then we don't need to continue
        if( !( value > 0 ) && !( refractoryCount > 0 ) )
        {
            return false;
        }

        // if we have recently been activated;
        if( refractoryCount > 0 )
        {
            // decrement time
            refractoryCount -= dTime;

            // ensure sanity
            if( refractoryCount < 0 )
            {
                refractoryCount = 0;
            }
        }

        TimeType rct = currentRefractoryIfActivated();

        // for activation:
        //   we must be above the minimum threshold
        //   and
        //      refractoryCount must be zero ie. there has not been a recent pulse
        //      or
        //      if the time since the last pulse is greater than the current time a pulse would take ie. if the activation value has gone up, cut short the refractoryCount and pulse again
        if( value >= threshold_min && ( !( refractoryCount > 0 ) || ( refractoryCountForLastActivation - refractoryCount > rct ) ) )
        {
            // WOO we activated!

            // set refractory counts
            refractoryCount = refractoryCountForLastActivation = rct;

            // activation stuff!
            {
                // keep track
                ++numActivations;

                // subtract the activation decay, so that activation itself can slow down the pulse train
                value -= activationDecay;

                // tell other neurons what happened
                for( auto synapse : synapses )
                {
                    pulse_manager.QueuePulse( synapse.generatePulse( time ) );
                }

                // call the callback
                if( onActivationFunc )
                {
                    onActivationFunc( *this );
                }
            }
            // activation stuff end

        }

        // decrement the value by the decay amount
        value -= valueDecay;


        // make sure we have a sane value, clamp value between 0 and threshold_max
        //value = std::min<Type>( std::max<Type>( value, 0 ), threshold_max );
        if( value < 0 )
        {
            value = 0;
        }
        else if( value > threshold_max )
        {
            value = threshold_max;
        }

        // make sure that the refractory count is upper bounded
        if( refractoryCount >= refractoryTime_low )
        {
            refractoryCount = refractoryTime_low;
        }

        return true;
    }

    // Tick End


    template < typename Type, typename TimeType >
    bool
    neuron_base< Type, TimeType >::AcceptPulse( const pulse_base< Type, TimeType >& pulse )
    {
        // if we are the target
        if( this == pulse.destination )
        {
            value += pulse.value;
            return true;
        }
        return false;
    }

    template < typename Type, typename TimeType >
    size_t
    neuron_base< Type, TimeType >::AddSynapse( const synapse_base< Type, TimeType >& synapse )
    {
        // copy the synapse
        synapse_base< Type, TimeType > syn( synapse );

        syn.source = this;

        synapses.push_back( syn );

        return synapses.size();
    }

    template < typename Type, typename TimeType >
    bool
    neuron_base< Type, TimeType >::Verify( const std::set< neuron_base< Type, TimeType > * >& acceptable_neurons ) const
    {
        // all sources and destinations of synapses contained in the neuron must be non-null and be present in the given set
        for( auto& synapse : synapses )
        {
            if( synapse.destination == nullptr || synapse.source == nullptr || synapse.source != this || acceptable_neurons.count( synapse.destination ) == 0 || acceptable_neurons.count( synapse.source ) == 0 )
            {
                /*std::cout << "neuron_base< Type, TimeType >::Verify Failed\n";
                std::cout << "\tthis =        " << this << "\n";
                std::cout << "\tsource =      " << synapse.source << "\n";
                std::cout << "\tdestination = " << synapse.destination << "\n";
                std::cout << "\tSrc in set =  " << acceptable_neurons.count( synapse.source ) << "\n";
                std::cout << "\tDst in set =  " << acceptable_neurons.count( synapse.destination ) << "\n" << std::endl;*/

                return false;
            }
        }

        return true;
    }


    template < typename Type, typename TimeType >
    void
    neuron_base< Type, TimeType >::reset_neuron_state()
    {
        numActivations = 0;

        value = 0;
        refractoryCount = 0;
        refractoryCountForLastActivation = 0;
    }
}

#endif // NEURON_METHODS_INL_INCLUDED
