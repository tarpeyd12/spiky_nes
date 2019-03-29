#ifndef NEAT_NEAT_INL_INCLUDED
#define NEAT_NEAT_INL_INCLUDED

#include <cmath>

namespace neat
{
    NodeDef::NodeDef()
         : innovation( 0 ), ID( 0 ), thresholdMin( 0.0 ), thresholdMax( 0.0 ), valueDecay( 0.0 ), activDecay( 0.0 ), pulseFast( 0 ), pulseSlow( 0 ), type( NodeType::Hidden )/*, callback( NodeCallback() )*/
    {
        /*  */
    }

    NodeDef::NodeDef( const NodeDef& o )
         : innovation( o.innovation ), ID( o.ID ), thresholdMin( o.thresholdMin ), thresholdMax( o.thresholdMax ), valueDecay( o.valueDecay ), activDecay( o.activDecay ), pulseFast( o.pulseFast ), pulseSlow( o.pulseSlow ), type( o.type )/*, callback( o.callback )*/
    {
        /*  */
    }

    NodeDef&
    NodeDef::operator=( const NodeDef& other )
    {
        innovation =    other.innovation;

        ID =            other.ID;

        thresholdMin =  other.thresholdMin;
        thresholdMax =  other.thresholdMax;
        valueDecay =    other.valueDecay;
        activDecay =    other.activDecay;
        pulseFast =     other.pulseFast; // small number
        pulseSlow =     other.pulseSlow; // large number

        type =          other.type;

        //callback =      other.callback;

        return *this;
    }

    bool
    NodeDef::is_good() const
    {
        bool good = true;

        good = good && ( std::isnormal( thresholdMin ) || thresholdMin == 0.0 );
        good = good && ( std::isnormal( thresholdMax ) || thresholdMax == 0.0 );
        good = good && ( std::isnormal( valueDecay ) || valueDecay == 0.0 );
        good = good && ( std::isnormal( activDecay ) || activDecay == 0.0 );
        good = good && ( pulseFast );
        good = good && ( pulseSlow );
        good = good && ( type == NodeType::Hidden || type == NodeType::Input || type == NodeType::Output );

        return good;
    }

    ConnectionDef::ConnectionDef()
         : innovation( 0 ), sourceID( 0 ), destinationID( 0 ), weight( 0.0 ), length( 0 ), enabled( false )
    {
        /*  */
    }

    ConnectionDef::ConnectionDef( const ConnectionDef& o )
         : innovation( o.innovation ), sourceID( o.sourceID ), destinationID( o.destinationID ), weight( o.weight ), length( o.length ), enabled( o.enabled )
    {
        /*  */
    }

    ConnectionDef&
    ConnectionDef::operator=( const ConnectionDef& other )
    {
        innovation =    other.innovation;

        sourceID =      other.sourceID;
        destinationID = other.destinationID;

        weight =        other.weight;
        length =        other.length;

        enabled =       other.enabled;

        return *this;
    }

    bool
    ConnectionDef::is_good() const
    {
        bool good = true;

        good = good && ( std::isnormal( weight ) || weight == 0.0 );
        good = good && ( length );

        return good;
    }
}

#endif // NEAT_NEAT_INL_INCLUDED
