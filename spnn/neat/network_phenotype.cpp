#include <iostream>
#include <string>

#include <cassert>

#include "network.hpp"

namespace neat
{

    NetworkPhenotype::NetworkPhenotype()
         : spnn::network( 1 ), neuronData(), inputNeurons(), outputNeurons(), neuronIDMap(), neuronIDs(), neuronTypes()
    {
        /*  */
    }

    NetworkPhenotype::NetworkPhenotype( uint64_t dTime )
         : spnn::network( dTime ), neuronData(), inputNeurons(), outputNeurons(), neuronIDMap(), neuronIDs(), neuronTypes()
    {
        /*  */
    }

    NetworkPhenotype::~NetworkPhenotype()
    {
        /*  */
    }

    size_t
    NetworkPhenotype::numInputs() const
    {
        return inputNeurons.size();
    }

    size_t
    NetworkPhenotype::numOutputs() const
    {
        return outputNeurons.size();
    }

    size_t
    NetworkPhenotype::numNeurons() const
    {
        return neuronData.size();
    }

    void
    NetworkPhenotype::printNetworkState( std::ostream& out ) const
    {
        out << "Network:\n{\n";

        out << "\ttime = " << Time() << ", dtime = " << DeltaTime() << ", pulses_queued = " << QueueSize() << "\n";
        out << "\tLastTick: { pulses = " << PulsesProcessedLastTick() << ", neurons = " << NeuronsProcessedLastTick() << "}\n";
        out << "\tRawNeurons:\n\t{\n";

        for( auto& neuron : neuronData )
        {
            out << "\t\tNeuron " << neuron.getID() << ": { ";
            out << "value = " << neuron.getValue() << ", ";
            out << "rcount = " << neuron.getRefractoryCount() << ", ";
            out << "active = " << neuron.getIsActive() << ", ";
            out << "numactivations = " << neuron.getNumActivations() << ", ";
            out << "numsynapses = " << neuron.getNumSynapses() << ", ";
            out << "activationratio = " << neuron.getCurrentActivationPercent() << " ";
            out << "}\n";
        }

        out << "\t}\n}\n";
    }

    void
    NetworkPhenotype::AddNode( NodeDef nodeDefinition )
    {
        assert( nodeDefinition.is_good() );

        // make sure the values are sane and mutation did not switch min/max
        double   tMin =   std::min( nodeDefinition.thresholdMin, nodeDefinition.thresholdMax );
        double   tMax =   std::max( nodeDefinition.thresholdMin, nodeDefinition.thresholdMax );
        uint64_t pFast =  std::min( nodeDefinition.pulseFast, nodeDefinition.pulseSlow );
        uint64_t pSlow =  std::max( nodeDefinition.pulseFast, nodeDefinition.pulseSlow );
        double   vDecay = nodeDefinition.valueDecay;
        double   aDecay = nodeDefinition.activDecay;

        // make the Neuron!
        spnn::neuron neuron( tMin, tMax, pFast, pSlow, vDecay, aDecay /*, std::vector< spnn::synapse >{} */ );

        /*if( nodeDefinition.type == NodeType::Output )
        {
            neuron.setCallbackFunction( nodeDefinition.callback );
        }*/

        // add it with the ID and type
        AddNode( nodeDefinition.ID, nodeDefinition.type, neuron );
    }

    void
    NetworkPhenotype::AddNode( NodeID nodeID, NodeType type, const spnn::neuron& neuron )
    {
        // make sure we haven't finalized yet
        assert( neuronIDs.size() == neuronData.size() );

        // add the neuron id to the index of the next neuron in neuronData
        neuronIDMap.emplace( nodeID, neuronData.size() );

        // add neuron as the next neuron in neuronData
        neuronData.push_back( neuron );

        // for later reference add the nodeID to the end of neuronIDs to correspond with neuronData
        neuronIDs.push_back( nodeID );

        // add the node type to the node type vector for finalization
        neuronTypes.push_back( type );
    }

    void
    NetworkPhenotype::AddConnection( ConnectionDef connDefinition )
    {
        assert( connDefinition.is_good() );

        if( !connDefinition.enabled )
        {
            return;
        }

        AddConnection( connDefinition.sourceID, connDefinition.destinationID, connDefinition.weight, connDefinition.length );
    }

    void
    NetworkPhenotype::AddConnection( NodeID srcID, NodeID dstID, double weight, uint64_t len )
    {
        // get the iterators of the IDs of the nodes we are connecting
        auto end_it = neuronIDMap.end();
        auto sourceID_it = neuronIDMap.find( srcID );
        auto destinationID_it = neuronIDMap.find( dstID );

        // both must exist!
        if( sourceID_it != end_it && destinationID_it != end_it )
        {
            // finally the pointers to the source and destination neurons
            spnn::neuron * source      = &neuronData[ sourceID_it->second ];
            spnn::neuron * destination = &neuronData[ destinationID_it->second ];

            // add the synapse to the source neuron pointing to the destination neuron
            source->AddSynapse( spnn::synapse( destination, len, weight ) );
        }
    }

    void
    NetworkPhenotype::Finalize()
    {
        assert( neuronData.size() == neuronTypes.size() );

        std::set< spnn::neuron * > verification_neurons;

        for( size_t i = 0; i < neuronData.size(); ++i )
        {
            spnn::neuron * neuron = &neuronData[ i ];

            AddNeuron( neuron );
            verification_neurons.emplace( neuron );

            switch( neuronTypes[i] )
            {
                case NodeType::Input:   inputNeurons.push_back( neuron ); break;
                case NodeType::Output: outputNeurons.push_back( neuron ); break;
                case NodeType::Hidden: break;
                default:               break;
            }

            //neuronData[i].setCallbackFunction( []( const spnn::neuron& n ){ std::cout << "spnn::neuron ID" << n.getID() << " activated" << std::endl; } );
        }

        assert( Verify( verification_neurons ) && "the network must only contain pointers to neurons that are in the network." );

        inputNeurons.shrink_to_fit();
        outputNeurons.shrink_to_fit();

        neuronTypes.clear();
        neuronTypes.shrink_to_fit();

        // empty neuronIDs as its not needed anymore
        neuronIDs.clear();
        neuronIDs.shrink_to_fit();

        // empty neuronIDMap as its not needed anymore
        neuronIDMap.clear();
    }

    bool
    NetworkPhenotype::setInputValues( const std::vector< double >& values )
    {
        // make sure we are the right size!
        if( !values.size() || !inputNeurons.size() || values.size() < inputNeurons.size() )
        {
            // say something went wrong
            return false;
        }

         // only go for as many callbacks as nodes exist
        size_t numValues = std::min( values.size(), inputNeurons.size() );

        // set the output values to those in the given list
        for( size_t i = 0; i < numValues; ++i )
        {
            if( !QueuePulse( inputNeurons[i], values[i] - inputNeurons[i]->getValue(), 0 ) )
            {
                return false;
            }
        }

        // everything went right
        return true;
    }

    bool
    NetworkPhenotype::setOutputCallbacks( const std::vector< NodeCallback >& callbacks )
    {
        // make sure we are the right size!
        if( !callbacks.size() || !outputNeurons.size() || callbacks.size() < outputNeurons.size() )
        {
            // say something went wrong
            return false;
        }

        // only go for as many callbacks as nodes exist
        size_t numCallbacks = std::min( callbacks.size(), outputNeurons.size() );

        // set the output callbacks to those in the given list
        for( size_t i = 0; i < numCallbacks; ++i )
        {
            outputNeurons[i]->setCallbackFunction( callbacks[i] );
        }

        // everything went right
        return true;
    }

    size_t
    NetworkPhenotype::QueuePulse( spnn::neuron * neuron, double value, uint64_t arrivalTime )
    {
        return spnn::network::QueuePulse( spnn::pulse( neuron, arrivalTime, value ) );
    }

    void
    NetworkPhenotype::resetNetworkState()
    {
        spnn::network::clear_network_state();
    }
}
