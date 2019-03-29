#ifndef SPNN_NEURON_HPP_INCLUDED
#define SPNN_NEURON_HPP_INCLUDED

#include <vector>
#include <functional>
#include <set>

namespace spnn
{
    template < typename Type, typename TimeType > class neuron_base;
}

#include "synapse.hpp"
#include "pulse_manager.hpp"

namespace spnn
{
    template < typename Type, typename TimeType >
    class neuron_base
    {
        private:

            // tracking

            static uint64_t totalNeurons;
            uint64_t ID;
            uint64_t numActivations;

            // behavior variables

            Type threshold_min;
            Type threshold_max;
            TimeType refractoryTime_high; // high frequency ( smaller number )
            TimeType refractoryTime_low;  // low frequency ( larger number )
            Type valueDecay;
            Type activationDecay;

        protected:

            // connections

            std::vector< synapse_base< Type, TimeType > > synapses; // contains pointers!
            std::function< void( const neuron_base< Type, TimeType >& ) > onActivationFunc;

            // state

            Type value;
            TimeType refractoryCount;
            TimeType refractoryCountForLastActivation;

        public:

            // constructors

            neuron_base();
            neuron_base( const Type& t_min, const Type& t_max, const TimeType& rt_high, const TimeType& rt_low, const Type& vdec, const Type& adec, const std::vector< synapse_base< Type, TimeType > >& syns = std::vector< synapse_base< Type, TimeType > >{} );
            neuron_base( const Type& t_min, const Type& t_max, const TimeType& rt_high, const TimeType& rt_low, const Type& vdec, const Type& adec, const std::function< void( const neuron_base< Type, TimeType >& ) >& func );
            neuron_base( const Type& t_min, const Type& t_max, const TimeType& rt_high, const TimeType& rt_low, const Type& vdec, const Type& adec, const std::vector< synapse_base< Type, TimeType > >& syns, const std::function< void( const neuron_base< Type, TimeType >& ) >& func );

            // destructor

            virtual ~neuron_base();

            // methods

            bool Tick( pulseManager_base< Type, TimeType >& pulse_manager, const TimeType& time, const TimeType& dTime );
            bool AcceptPulse( const pulse_base< Type, TimeType >& pulse );

            size_t AddSynapse( const synapse_base< Type, TimeType >& synapse );

            bool Verify( const std::set< neuron_base< Type, TimeType > * >& acceptable_neurons ) const;

            void reset_neuron_state();

            // setters

            void setCallbackFunction( std::function< void( const neuron_base< Type, TimeType >& ) > func = nullptr );

            // getters

            uint64_t     getID() const;
            Type         getValue() const;
            TimeType     getRefractoryCount() const;
            bool         getIsActive() const;
            uint64_t     getNumActivations() const;
            size_t       getNumSynapses() const;
            long double  getCurrentActivationPercent() const;

        protected:

            // getters

            TimeType currentRefractoryIfActivated() const;
    };

    template < typename Type, typename TimeType > uint64_t neuron_base< Type, TimeType >::totalNeurons = 0;

}

#include "neuron.inl"
#include "neuron_methods.inl"
#include "neuron_properties.inl"

#endif // SPNN_NEURON_HPP_INCLUDED
