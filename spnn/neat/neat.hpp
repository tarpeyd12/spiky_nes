#ifndef NEAT_NEAT_HPP_INCLUDED
#define NEAT_NEAT_HPP_INCLUDED

#include <type_traits>

#include "../spnn/spnn.hpp"

#include "random.hpp"

namespace neat
{

    // Types

    typedef uint64_t NodeID;
    typedef uint64_t InnovationID;
    typedef uint64_t SpeciesID;

    typedef std::function< void( const spnn::neuron& ) > NodeCallback;

    enum class NodeType : uint8_t { Hidden, Input, Output };

    // Data structures

    struct NodeDef
    {
        InnovationID    innovation;

        NodeID          ID;

        double          thresholdMin;
        double          thresholdMax;
        double          valueDecay;
        double          activDecay;
        uint64_t        pulseFast; // small number
        uint64_t        pulseSlow; // large number

        NodeType        type;

        // copy constructor & operator

        inline NodeDef();
        inline NodeDef( const NodeDef& other );
        inline NodeDef& operator=( const NodeDef& other );

        inline bool is_good() const;
    };

    struct ConnectionDef
    {
        InnovationID    innovation;

        NodeID          sourceID;
        NodeID          destinationID;

        double          weight;
        uint64_t        length;

        bool            enabled;

        // copy constructor & operator

        inline ConnectionDef();
        inline ConnectionDef( const ConnectionDef& other );
        inline ConnectionDef& operator=( const ConnectionDef& other );

        inline bool is_good() const;
    };

    template < typename Type >
    struct MinMax
    {
        Type min;
        Type max;

        MinMax() : min(), max() {  }
        MinMax( const Type& _min, const Type& _mix ) : min( _min ), max( _mix ) { assert( min < max || !( max < min ) ); }
        MinMax( const Type& _v ) : min( _v ), max( _v ) { assert( min < max || !( max < min ) ); }

        template< typename std::enable_if< std::is_arithmetic< Type >::value, int >::type = 0 >
        Type clamp( const Type& value ) const { return std::min<Type>( std::max<Type>( value, min ), max ); }

        template< typename std::enable_if< std::is_arithmetic< Type >::value, int >::type = 0 >
        Type range() const { return max - min; }

        template< typename std::enable_if< std::is_arithmetic< Type >::value, int >::type = 0 >
        MinMax< Type >& expand( const Type& value ) { min = std::min<Type>( min, value ); max = std::max<Type>( max, value ); return *this; }

        template< typename std::enable_if< std::is_arithmetic< Type >::value, int >::type = 0 >
        MinMax< Type >& expand( const MinMax< Type >& other ) { expand( other.min ); expand( other.max ); return *this; }
    };

    struct MutationLimits
    {
        MinMax< double >    thresholdMin;
        MinMax< double >    thresholdMax;
        MinMax< double >    valueDecay;
        MinMax< double >    activDecay;
        MinMax< uint64_t >  pulseFast; // small number
        MinMax< uint64_t >  pulseSlow; // large number

        MinMax< double >    weight;
        MinMax< uint64_t >  length;

        MutationLimits() : thresholdMin(), thresholdMax(), valueDecay(), activDecay(), pulseFast(), pulseSlow(), weight(), length() { /*  */ }
    };

    struct MutationRates
    {
        double          thresholdMin;
        double          thresholdMax;
        double          valueDecay;
        double          activDecay;
        uint64_t        pulseFast; // small number
        uint64_t        pulseSlow; // large number

        double          weight;
        uint64_t        length;
    };

}

#include "neat.inl"

#include "population.hpp"
#include "network.hpp"
#include "species.hpp"
#include "fitness.hpp"
#include "innovation_generator.hpp"
#include "mutations.hpp"
#include "splice.hpp"

#endif // NEAT_NEAT_HPP_INCLUDED
