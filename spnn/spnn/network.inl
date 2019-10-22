#ifndef NETWORK_INL_INCLUDED
#define NETWORK_INL_INCLUDED

#include <iostream>
#include <string>

namespace spnn
{
    template < typename Type, typename TimeType >
    network_base< Type, TimeType >::network_base( const TimeType& dTime )
        : pulsesProcessed( 0 ), neuronsProcessed( 0 ), pulsesProcessedLastTick( 0 ), neuronsProcessedLastTick( 0 ), deltaTime( dTime ), neurons(), pulses(), currentTime( 0 )
    {
        assert( DeltaTime() > 0 );
    }

    template < typename Type, typename TimeType >
    network_base< Type, TimeType >::~network_base()
    {
        /*  */
    }

    template < typename Type, typename TimeType >
    void
    network_base< Type, TimeType >::AddNeuron( neuron_base< Type, TimeType > * neuron )
    {
        neurons.push_back( neuron );
    }

    template < typename Type, typename TimeType >
    void
    network_base< Type, TimeType >::Tick()
    {
        pulsesProcessedLastTick = 0;
        neuronsProcessedLastTick = 0;

        // transmit the pulses to their destinations
        {
            pulses.ProcessCurrentTimePulses( currentTime, [this]( const auto& pulse )
            {
                if( pulse.destination && pulse.destination->AcceptPulse( pulse ) )
                {
                    ++pulsesProcessedLastTick;
                    //std::cout << "\tNeuron #" << pulse.destination->getID() << " received pulse with value " << pulse.value << " at time " << pulse.time << " from neuron #" << ( pulse.source ? pulse.source->getID() : -1 ) <<  "\n";
                }
            } );
        }

        // process the neurons
        {
            for( auto neuron : neurons )
            {
                if( neuron->Tick( pulses, currentTime, deltaTime ) )
                {
                    ++neuronsProcessedLastTick;
                }
            }
        }

        pulsesProcessed += pulsesProcessedLastTick;
        neuronsProcessed += neuronsProcessedLastTick;

        // increment the time
        currentTime += deltaTime;

    }

    template < typename Type, typename TimeType >
    size_t
    network_base< Type, TimeType >::QueuePulse( const pulse_base< Type, TimeType >& pulse )
    {
        return pulses.QueuePulse( pulse );
    }

    template < typename Type, typename TimeType >
    bool
    network_base< Type, TimeType >::Verify( const std::set< neuron_base< Type, TimeType > * >& acceptable_neurons ) const
    {
        // all neurons must be non-null, present in the given set, and verified with the given set of neurons
        for( auto p_neuron : neurons )
        {
            bool verifiyed = false;

            if( p_neuron == nullptr || acceptable_neurons.count( p_neuron ) == 0 || !( verifiyed = p_neuron->Verify( acceptable_neurons ) ) )
            {
                std::cout << "network_base< Type, TimeType >::Verify Failed\n";
                std::cout << "\tp_neuron =   " << this << "\n";
                std::cout << "\tpnr in set = " << acceptable_neurons.count( p_neuron ) << "\n";
                std::cout << "\tVerifiyed =  " << verifiyed << "\n" << std::endl;

                return false;
            }
        }

        return true;
    }

    template < typename Type, typename TimeType >
    void
    network_base< Type, TimeType >::clear_network_state()
    {
        pulsesProcessed = 0;
        neuronsProcessed = 0;

        pulsesProcessedLastTick = 0;
        neuronsProcessedLastTick = 0;

        currentTime = 0;

        pulses.clear_all_pulses();

        for( auto neuron : neurons )
        {
            neuron->reset_neuron_state();
        }
    }


    template < typename Type, typename TimeType >
    TimeType
    network_base< Type, TimeType >::Time() const
    {
        return currentTime;
    }

    template < typename Type, typename TimeType >
    TimeType
    network_base< Type, TimeType >::DeltaTime() const
    {
        return deltaTime;
    }

    template < typename Type, typename TimeType >
    size_t
    network_base< Type, TimeType >::QueueSize() const
    {
        return pulses.QueueSize();
    }

    template < typename Type, typename TimeType >
    uint64_t
    network_base< Type, TimeType >::PulsesProcessed() const
    {
        return pulsesProcessed;
    }

    template < typename Type, typename TimeType >
    uint64_t
    network_base< Type, TimeType >::NeuronsProcessed() const
    {
        return neuronsProcessed;
    }

    template < typename Type, typename TimeType >
    uint64_t
    network_base< Type, TimeType >::PulsesProcessedLastTick() const
    {
        return pulsesProcessedLastTick;
    }

    template < typename Type, typename TimeType >
    uint64_t
    network_base< Type, TimeType >::NeuronsProcessedLastTick() const
    {
        return neuronsProcessedLastTick;
    }

}

#endif // NETWORK_INL_INCLUDED
