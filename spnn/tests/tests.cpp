
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <iomanip>

#include "tests.hpp"
#include "../spnn.hpp"
#include "../neat.hpp"

namespace _tests
{
    inline
    std::vector<size_t>
    RandomIndexes( size_t num )
    {
        // init out to be numbers from 0 to num-1
        std::vector< size_t > out( num );
        size_t i = 0;
        std::generate( out.begin(), out.end(), [&]{ return i++; } );

        // scramble
        for( i = 0; i < out.size(); ++i )
        {
            std::swap( out[ i ], out[ ( size_t(rand()) + (size_t(rand())<<16) /*+ (size_t(rand())<<32) + (size_t(rand())<<48)*/ ) % out.size() ] );
        }

        return out;
    }

    inline
    spnn::neuron
    GenerateNeuron( const std::vector<spnn::synapse>& synapses = std::vector<spnn::synapse>{} )
    {
        return spnn::neuron(
                            15.0,                           // 15 mV minimum activation
                            100.0,                          // 100 mV maximum activation
                            1,                              // 1 tick fastest refractory
                            100,                            // 100 ticks slowest refractory
                            0.1,                            // 0.1 mV per tick loss
                            0.01,                           // 0.01 mV per activation loss
                            synapses                        // synapses
        );
    }

    void
    Test()
    {
        std::ios_base::sync_with_stdio( false );

        auto echo = [&](const spnn::neuron&nr){ std::cout << "\t************ \tNeuron #" << nr.getID() << " Has just activated!\n"; };
        auto bell = [&](const spnn::neuron&nr){ std::cout << "\t************ \tNeuron #" << nr.getID() << " Has just activated!\n\a"; };

        spnn::network network( 1 );

        spnn::neuron n0( 15.0, 100.0, 1, 100, 0.1, 0.01, std::vector<spnn::synapse>{}, bell );
        spnn::neuron n1( 15.0, 100.0, 1, 100, 0.1, 0.01, std::vector<spnn::synapse>{ spnn::synapse( &n0, 5, 1.0 ) }, echo );

        network.AddNeuron( &n0 );
        network.AddNeuron( &n1 );

        network.QueuePulse( spnn::pulse( &n1, 1, 100.0 ) );

        while( network.Time() <= 1000 )
        {
            std::cout << "\nTime = " << network.Time() << "\n";

            network.Tick();

            std::cout << "\tn0->value = " << n0.getValue() << " active: " << n0.getIsActive() << " activations: " << n0.getNumActivations() << "\n";
            std::cout << "\tn1->value = " << n1.getValue() << " active: " << n1.getIsActive() << " activations: " << n1.getNumActivations() << "\n";

            std::cout << " actR: " << (long double)n0.getNumActivations() / (long double)n1.getNumActivations() << "\n";

            //std::this_thread::sleep_for( std::chrono::duration<double>(0.1) );
        }
    }

    void
    Test2()
    {
        std::vector< spnn::neuron > neurons;

        spnn::neuron inputNeuron = GenerateNeuron();
        spnn::neuron outputNeuron = GenerateNeuron();

        inputNeuron.setCallbackFunction( [&]( const spnn::neuron& /*nr*/ ){ std::cout << "Ping" << "  "; } );
        outputNeuron.setCallbackFunction( [&]( const spnn::neuron& /*nr*/ ){ std::cout << "OUTPUT!" << "  "; } );

        std::cout << "Generating Neurons ... " << std::endl;

        for( size_t i = 0; i < 10000; ++i )
        {
            neurons.push_back( GenerateNeuron() );
        }

        auto random_indexes = RandomIndexes( neurons.size() );

        std::cout << "Generating input synapses ... " << std::endl;

        for( size_t i = 0; i < 1000; ++i )
        {
            inputNeuron.AddSynapse( spnn::synapse( &neurons[ random_indexes.back() ], 1 + rand()%10, float(rand()%2001)/2000.0 * 2.0 - 1.0 ) );
            //inputNeuron.AddSynapse( spnn::synapse( &neurons[ random_indexes.back() ], 1 + rand()%100, float(rand()%1001)/1000.0 ) );
            random_indexes.pop_back();
        }

        std::cout << "Generating connectivity synapses ... " << std::endl;

        for( size_t i = 0; i < neurons.size(); ++i )
        {
            if(i%1000==0) std::cout << "for neuron #" << i << "\r" << std::flush;

            //auto rand_ind = RandomIndexes( neurons.size() );

            for( size_t n = 0; n < 250; ++n )
            {
                //size_t index = rand_ind[n];
                size_t index = ( size_t(rand()) + (size_t(rand())<<16) /*+ (size_t(rand())<<32) + (size_t(rand())<<48)*/ ) % neurons.size();

                if( index == i )
                {
                    continue;
                }

                neurons[i].AddSynapse( spnn::synapse( &neurons[ index ], 1 + rand()%10, float(rand()%2001)/2000.0 * 2.0 - 1.0 ) );
            }
        }

        std::cout << "\nGenerating output synapses ... " << std::endl;

        for( size_t i = 0; i < 1000; ++i )
        {
            neurons[ random_indexes.back() ].AddSynapse( spnn::synapse( &outputNeuron, 1 + rand()%10, float(rand()%2001)/2000.0 * 2.0 - 1.0 ) );
            random_indexes.pop_back();
        }

        spnn::network network( 1 );

        network.AddNeuron( &inputNeuron );
        network.AddNeuron( &outputNeuron );

        std::cout << "Adding neurons to the network ... " << std::endl;

        for( size_t i = 0; i < neurons.size(); ++i )
        {
            network.AddNeuron( &neurons[i] );
        }

        std::ofstream csv( "out.csv" );

        network.QueuePulse( spnn::pulse( &inputNeuron, 1, 100.0 ) );
        network.QueuePulse( spnn::pulse( &inputNeuron, 1+5000, 100.0 ) );
        for( size_t i = 100; i <= 1000; i+= 100 )
        {
            network.QueuePulse( spnn::pulse( &inputNeuron, i, 10.0 ) );
            network.QueuePulse( spnn::pulse( &inputNeuron, i+5000, 10.0 ) );
        }
        //network.QueuePulse( spnn::pulse( &inputNeuron, 700, -100.0 ) );

        std::cout << "Starting ... " << std::endl;

        while( network.Time() <= 1000 )
        {
            std::cout << "Time = "  << network.Time() << " neurons processed = " << network.NeuronsProcessedLastTick() << " pulses processed = " << network.PulsesProcessedLastTick() << " pings waiting: " << network.QueueSize() << " current input value = " << inputNeuron.getValue() << " current output value = " << outputNeuron.getValue() << " output activations = " << outputNeuron.getNumActivations() << "  ";

            csv << network.Time() << "," << network.QueueSize() << "," << network.PulsesProcessedLastTick() << "," << network.NeuronsProcessedLastTick() << "\n";

            network.Tick();

            /*if( network.Time() <= 100 && inputNeuron.getValue() < 15.0 && !outputNeuron.getNumActivations() )
            {
                network.QueuePulse( spnn::pulse( &inputNeuron, network.Time(), 20.0 - inputNeuron.getValue() ) );
            }*/

            std::cout << std::endl;
        }

        csv.close();
    }


    void
    Test3()
    {
        std::ios_base::sync_with_stdio( false );

        std::cout << std::fixed << std::setw( 6 ) << std::setprecision( 2 ) << std::setfill( ' ' );

        auto echo = [&](const spnn::neuron&nr){ std::cout << "\t************ \tNeuron #" << nr.getID() << " Has just activated!"; };

        spnn::network network( 1 );

        double perTickDecay = 0.1;

        spnn::neuron n( 15.0, 100.0, 1, 100, perTickDecay, 0.0, echo );

        network.AddNeuron( &n );

        uint64_t pingNeuron = 10;

        //network.QueuePulse( spnn::pulse( &n, 0, 100.0 ) );

        while( n.getValue() > 0.0 || network.Time() == 0 )
        {
            if( true && pingNeuron )
            {
                network.QueuePulse( spnn::pulse( &n, 0, perTickDecay*2.0 ) );

                if( n.getValue() >= 100.0-perTickDecay )
                {
                    pingNeuron--;
                }
            }



            std::cout << "Time = " << network.Time() << "";

            std::cout << "\tn->value = " << std::setw( 6 ) << n.getValue() << " active: " << n.getIsActive() << " activations: " << n.getNumActivations() << " rcount = " << std::setw( 3 ) << n.getRefractoryCount() << ", ";;

            network.Tick();

            std::cout  << "\n" << std::flush;

            //std::this_thread::sleep_for( std::chrono::duration<double>(0.01) );
        }
    }

    void
    Test5()
    {

        std::vector< neat::NodeDef >       node_list;
        std::vector< neat::ConnectionDef > conn_list;

        neat::NodeDef node;

        // node 0 input

        node.ID = 0;
        node.innovation = 0;
        node.thresholdMin = 15.0;
        node.thresholdMax = 100.0;
        node.valueDecay = 0.1;
        node.activDecay = 0.01;
        node.pulseFast = 1;
        node.pulseSlow = 100;
        node.type = neat::NodeType::Input;

        node_list.push_back( node );

        // node 1 output

        node.ID = 1;
        node.type = neat::NodeType::Output;

        node_list.push_back( node );



        neat::ConnectionDef conn;

        // connection 0: from node 0 to node 1

        conn.innovation = 0;
        conn.sourceID = 0;
        conn.destinationID = 1;
        conn.weight = 1.0;
        conn.length = 10;
        conn.enabled = true;

        conn_list.push_back( conn );


        neat::NetworkGenotype network0 = neat::make_genotype( node_list, conn_list );

        network0.printGenotype(std::cout,"Network #0");


        // node 2 hidden

        node.ID = 2;
        node.type = neat::NodeType::Hidden;

        node_list.push_back( node );

        // node 3 hidden

        node.ID = 3;
        node.type = neat::NodeType::Hidden;

        node_list.push_back( node );


        // connection 1: from node 0 to node 2

        conn.innovation = 1;
        conn.sourceID = 0;
        conn.destinationID = 2;

        conn_list.push_back( conn );

        // connection 2: from node 2 to node 3

        conn.innovation = 2;
        conn.sourceID = 2;
        conn.destinationID = 3;

        conn_list.push_back( conn );

        // connection 3: from node 3 to node 1

        conn.innovation = 3;
        conn.sourceID = 3;
        conn.destinationID = 1;

        conn_list.push_back( conn );


        // connection 4: from node 0 to node 3

        conn.innovation = 4;
        conn.sourceID = 0;
        conn.destinationID = 3;

        conn_list.push_back( conn );


        neat::NetworkGenotype network1 = neat::make_genotype( node_list, conn_list );

        network1.printGenotype( std::cout,"Network #1" );

        conn_list.pop_back();


        // connection 5: from node 2 to node 1

        conn.innovation = 5;
        conn.sourceID = 2;
        conn.destinationID = 1;

        conn_list.push_back( conn );


        neat::NetworkGenotype network2 = neat::make_genotype( node_list, conn_list );

        network2.printGenotype( std::cout,"Network #2" );

        neat::SpliceGenotypes( network1, network2 ).printGenotype( std::cout,"Network #3" );
    }
}
