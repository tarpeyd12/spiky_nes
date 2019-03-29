#ifndef SPNN_PULSE_MANAGER_HPP_INCLUDED
#define SPNN_PULSE_MANAGER_HPP_INCLUDED

#include <vector>
#include <queue>

namespace spnn
{
    template < typename Type, typename TimeType > class pulseManager_base;
}

#include "neuron.hpp"

namespace spnn
{


    template < typename Type, typename TimeType >
    class pulseManager_base
    {
        private:

            // comparison functor of two pulse_base< Type, TimeType >

            struct pulse_base_comp
            {
                bool operator()( const pulse_base< Type, TimeType >& l, const pulse_base< Type, TimeType >& r ) const;
            };

        protected:

            // state

            std::priority_queue< pulse_base< Type, TimeType >, std::vector< pulse_base< Type, TimeType > >, pulse_base_comp > pulseQueue;

        public:

            // constructor

            pulseManager_base();

            // destructor

            virtual ~pulseManager_base();

            // mutators

            size_t QueuePulse( const pulse_base< Type, TimeType >& pulse );

            std::vector< pulse_base< Type, TimeType > > GetCurrentTimePulses( const TimeType& time );

            void clear_all_pulses();

            // accessors

            size_t QueueSize() const;
    };

}

#include "pulse_manager.inl"

#endif // SPNN_PULSE_MANAGER_HPP_INCLUDED
