#ifndef SPNN_NETWORK_HPP_INCLUDED
#define SPNN_NETWORK_HPP_INCLUDED

namespace spnn
{
    template < typename Type, typename TimeType >
    class network_base
    {
        private:

            // tracking

            uint64_t pulsesProcessedLastTick;
            uint64_t neuronsProcessedLastTick;

            // behavior variables

            TimeType deltaTime;

        protected:

            // network data

            std::vector< neuron_base< Type, TimeType > * > neurons;

            // state

            pulseManager_base< Type, TimeType > pulses;

            TimeType currentTime;

        public:

            // constructors

            network_base( const TimeType& dTime );

            // destructor

            virtual ~network_base();

            // methods

            void AddNeuron( neuron_base< Type, TimeType > * neuron );

            void Tick();

            size_t QueuePulse( const pulse_base< Type, TimeType >& pulse );

            bool Verify( const std::set< neuron_base< Type, TimeType > * >& acceptable_neurons ) const;

            void clear_network_state();

            // properties

            TimeType Time() const;
            TimeType DeltaTime() const;
            size_t QueueSize() const;

            uint64_t PulsesProcessedLastTick() const;
            uint64_t NeuronsProcessedLastTick() const;

    };
}

#include "network.inl"

#endif // SPNN_NETWORK_HPP_INCLUDED
