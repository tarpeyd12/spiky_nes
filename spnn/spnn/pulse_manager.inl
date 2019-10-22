#ifndef PULSE_MANAGER_INL_INCLUDED
#define PULSE_MANAGER_INL_INCLUDED

#include <cassert>

namespace spnn
{
    template < typename Type, typename TimeType >
    pulseManager_base< Type, TimeType >::pulseManager_base()
         : pulseQueue()
    {
        /*  */
    }

    template < typename Type, typename TimeType >
    pulseManager_base< Type, TimeType >::~pulseManager_base()
    {
        /*  */
    }

    template < typename Type, typename TimeType >
    bool
    pulseManager_base< Type, TimeType >::pulse_base_comp::operator()( const pulse_base< Type, TimeType >& l, const pulse_base< Type, TimeType >& r ) const
    {
        if( l.time == r.time )
        {
            return uintptr_t(l.destination) < uintptr_t(r.destination);
        }

        // actually greater than, since we want the pulses with the smallest time-stamp to appear at the top of the queue
        return l.time > r.time;
    }

    template < typename Type, typename TimeType >
    size_t
    pulseManager_base< Type, TimeType >::QueuePulse( const pulse_base< Type, TimeType >& pulse )
    {
        // don't queue malformed pulses/pulses without destinations
        if( pulse.destination != nullptr )
        {
            pulseQueue.push( pulse );

            // return the queue size on success
            return pulseQueue.size();
        }

        // return 0 on malformed pulse
        return 0;
    }

    template < typename Type, typename TimeType >
    std::vector< pulse_base< Type, TimeType > >
    pulseManager_base< Type, TimeType >::GetCurrentTimePulses( const TimeType& time )
    {
        std::vector< pulse_base< Type, TimeType > > out;

        // add the current time pulses to the output
        ProcessCurrentTimePulses( time, [&out]( auto& pulse ){ out.emplace_back( pulse ); } );

        // return the output
        return out;
    }

    template < typename Type, typename TimeType >
    size_t
    pulseManager_base< Type, TimeType >::ProcessCurrentTimePulses( const TimeType& time, std::function<void(const pulse_base< Type, TimeType >&)> func )
    {
        assert( func != nullptr );

        size_t count = 0;

        // get the pulses that are more recent than the given time
        while( !pulseQueue.empty() && pulseQueue.top().time <= time )
        {
            // call func on the current top pulse
            func( pulseQueue.top() );
            pulseQueue.pop();
            ++count;
        }

        return count;
    }

    template < typename Type, typename TimeType >
    void
    pulseManager_base< Type, TimeType >::clear_all_pulses()
    {
        while( !pulseQueue.empty() )
        {
            pulseQueue.pop();
        }
    }

    template < typename Type, typename TimeType >
    size_t
    pulseManager_base< Type, TimeType >::QueueSize() const
    {
        return pulseQueue.size();
    }
}

#endif // PULSE_MANAGER_INL_INCLUDED
