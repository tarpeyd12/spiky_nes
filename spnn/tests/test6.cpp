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
    namespace t6
    {
        class TestFitnessCalculator;

        class TestNodeFitnessCallback : public neat::NodeFitnessCallback
        {
            public:

                TestNodeFitnessCallback( neat::FitnessCalculator * f, neat::NodeID outputNodeID )
                     : neat::NodeFitnessCallback( f, outputNodeID )
                {
                    /*  */
                }

                //virtual ~TestNodeFitnessCallback() { }

                void operator()( const spnn::neuron& n ) override;
        };

        class TestFitnessCalculator : public neat::FitnessCalculator
        {
            private:

                long double fitness;
                bool stop;

                std::vector< neat::NodeCallback > callbacks;
                std::vector< double > inputValues;

                uint8_t data;

            public:

                TestFitnessCalculator( std::shared_ptr< neat::NetworkPhenotype > net, uint8_t _data )
                     : neat::FitnessCalculator( net ), fitness( 0.0 ), stop( false ), callbacks(), inputValues(), data( _data )
                {
                    {
                        inputValues.reserve( getNumInputNodes() );

                        inputValues.push_back( 1000.0 ); // constant
                        inputValues.push_back( ( data & 1 ) ? 1000.0 : 0.0 );
                        inputValues.push_back( ( data & 2 ) ? 1000.0 : 0.0 );

                        while( inputValues.size() < getNumInputNodes() )
                        {
                            inputValues.push_back( 0.0 );
                        }
                    }

                    {
                        callbacks.reserve( getNumOutputNodes() );

                        while( callbacks.size() < getNumOutputNodes() )
                        {
                            //callbacks.push_back( TestNodeFitnessCallback( this, callbacks.size() ) );
                            auto s = callbacks.size();
                            auto t = this;
                            callbacks.push_back( [=]( const spnn::neuron& ){ t->nodeActivationCallback( s ); } );
                        }
                    }

                }

                virtual ~TestFitnessCalculator() = default;

                long double
                getFitnessScore() const override
                {
                    return fitness;
                }

                uint64_t
                getMaxNumTimesteps() const override
                {
                    return 100;
                }

                bool
                stopTest() const override
                {
                    return stop;
                }

                void
                testTick( uint64_t ) override
                {
                    /*  */
                }

                uint64_t
                getInputValueCheckCadence() const override
                {
                    // 0 means only once on init, 1 is every tick 2, every other
                    return 1;
                }

                std::vector< double >
                getInputValues( uint64_t /*time*/ ) override
                {
                    return inputValues;
                }

                std::vector< neat::NodeCallback >
                getNodeCallbacks() override
                {
                    return callbacks;
                }

                void
                nodeActivationCallback( neat::NodeID nodeID )
                {
                    if( nodeID > 1 )
                    {
                        return;
                    }

                    bool a = ( data & 1 );
                    bool b = ( data & 2 );
                    bool correct = ( a != b );

                    bool res = bool( nodeID );

                    if( correct == res )
                    {
                        fitness += 1000.0;
                        stop = true;
                    }
                }
        };

        void
        TestNodeFitnessCallback::operator()( const spnn::neuron& /*n*/ )
        {
            //std::cout << targetNodeID() << " ACTIVATION!!!" << std::endl;
            dynamic_cast<TestFitnessCalculator*>(fitnessCalculatorData())->nodeActivationCallback( targetNodeID() );
        }

        class TestFitnessFactory : public neat::FitnessFactory
        {
            public:

                TestFitnessFactory() = default;
                virtual ~TestFitnessFactory() = default;

                std::shared_ptr< neat::FitnessCalculator >
                getNewFitnessCalculator( std::shared_ptr< neat::NetworkPhenotype > net, size_t count ) const override
                {
                    return std::make_shared< TestFitnessCalculator >( net, uint8_t(count) );
                }

                size_t
                numTimesToTest() const override
                {
                    return 4;
                }
        };
    }

    void
    Test6()
    {

        //Rand::Seed( 0 );

        //auto random = std::make_shared< Rand::Random_Safe >();
        auto random = std::make_shared< Rand::Random_Unsafe >( );

        neat::MutationLimits limits;
        neat::MutationRates rates;
        neat::SpeciesDistanceParameters speciationParams;

        t6::TestFitnessFactory fitnessFactory;

        limits.thresholdMin.min =     2.0;
        limits.thresholdMin.max =   900.0;

        limits.thresholdMax.min =    10.0;
        limits.thresholdMax.max =  1000.0;

        limits.valueDecay.min =       0.01;
        limits.valueDecay.max =      10.0;

        limits.activDecay.min =       0.001;
        limits.activDecay.max =       1.0;

        limits.pulseFast.min =        1;
        limits.pulseFast.max =      100;

        limits.pulseSlow.min =       10;
        limits.pulseSlow.max =     1000;

        limits.weight.min =       -1000.0;
        limits.weight.max =        1000.0;

        limits.length.min =           1;
        limits.length.max =         500;

        rates.thresholdMin =          1.0;
        rates.thresholdMax =          1.0;

        rates.valueDecay =            0.1;
        rates.activDecay =            0.01;

        rates.pulseFast =             1;
        rates.pulseSlow =             5;

        rates.weight =              100.0;
        rates.length =              100;

        speciationParams.excess =      1.0;
        speciationParams.disjoint =    1.0;
        speciationParams.weights =     0.0005;
        speciationParams.lengths =     0.002;

        speciationParams.activations = 0.0001;
        speciationParams.decays =      0.01;
        speciationParams.pulses =      0.0001;
        speciationParams.nodes =       1.0;

        speciationParams.threshold =   3.0;

        auto mutator = std::make_shared< neat::Mutations::Mutation_Multi >();

        const double nodeMutation = 0.001;
        const double connMutation = 0.01;
        const double ntwkMutation = 0.00001;

        {
            mutator->addMutator< neat::Mutations::Mutation_Node_thresh_min   >( nodeMutation );
            mutator->addMutator< neat::Mutations::Mutation_Node_thresh_max   >( nodeMutation );
            mutator->addMutator< neat::Mutations::Mutation_Node_decays_value >( nodeMutation );
            mutator->addMutator< neat::Mutations::Mutation_Node_decays_activ >( nodeMutation );
            mutator->addMutator< neat::Mutations::Mutation_Node_pulses_fast  >( nodeMutation );
            mutator->addMutator< neat::Mutations::Mutation_Node_pulses_slow  >( nodeMutation );

            mutator->addMutator< neat::Mutations::Mutation_Conn_weight       >( connMutation );
            mutator->addMutator< neat::Mutations::Mutation_Conn_length       >( connMutation );
            mutator->addMutator< neat::Mutations::Mutation_Conn_enable       >( connMutation );

            mutator->addMutator< neat::Mutations::Mutation_Add_node          >( ntwkMutation );
            mutator->addMutator< neat::Mutations::Mutation_Add_conn          >( ntwkMutation );
            mutator->addMutator< neat::Mutations::Mutation_Add_conn_unique   >( ntwkMutation );
        }


        std::cout << "Population construct call" << std::endl;

        //neat::Population population( 500, 256*240/(16*16), 6, limits, rates, mutator, fitnessFactory, speciationParams, neat::SpeciationMethod::FirstForward, 2 );
        neat::Population population( 120, 3, 2, limits, rates, mutator, fitnessFactory, speciationParams, neat::SpeciationMethod::FirstForward, 1, 100, 200, 10 );

        tpl::pool thread_pool( 1 );

        std::cout << "Population Init call" << std::endl;

        population.Init();

        {
            std::ofstream file( "out.csv", std::ofstream::trunc );
            file.close();
        }

        std::cout << "Population Evolution ..." << std::endl;

        //while( population.getGenerationCount() < 200000 )
        while( true )
        {


            std::cout << "\n\nGeneration " << population.getGenerationCount() << ":\n";

            std::cout << "\n\tCalculating ...\n";

            population.IterateGeneration( thread_pool, random, 0.95 );


            std::cout << "\tDone.\n\n";

            auto genData = population.getLastGenerationData();

            if( !genData )
            {
                continue;
            }

            auto fitVec = genData->getFitnessVector();

            size_t over = 0;
            for( auto fitness : fitVec )
            {
                if( fitness > 900.0 )
                {
                    over++;
                }
            }

            double ratioHighPreformance = double(over)/double(fitVec.size());

            auto speciesPresent = genData->getSpeciesPresent();

            double avgGenotypesPerSpecies = 0.0;

            for( auto c : speciesPresent )
            {
                avgGenotypesPerSpecies += c.second;
            }
            avgGenotypesPerSpecies /= double( speciesPresent.size() );

            std::cout << "\tFitness:\n";
            std::cout << "\t\tMax: " << genData->getMaxFitness() << "\n";
            std::cout << "\t\tMin: " << genData->getMinFitness() << "\n";
            std::cout << "\t\tAvg: " << genData->getAvgFitness() << "\n";
            std::cout << "\n";

            {
                std::ofstream file( "out.csv", std::ofstream::app );

                file << population.getGenerationCount() - 1 << "," << genData->getMinFitness() << "," << genData->getMaxFitness() << "," << genData->getAvgFitness() << "\n";

                file.close();
            }

            auto specFitMap = genData->getSpeciesFitness();
            long double maxSpeciesFitness = specFitMap.begin()->second;
            neat::SpeciesID maxFitSpeciesID = specFitMap.begin()->first;
            for( auto f : specFitMap )
            {
                if( f.second > maxSpeciesFitness )
                {
                    maxSpeciesFitness = f.second;
                    maxFitSpeciesID = f.first;
                }
            }

            std::cout << "\tSpecies:\n";
            std::cout << "\t\tTracked: " << genData->getSpeciesManager().getNumTrackedSpecies() << "\n";
            std::cout << "\t\tPresent: " << speciesPresent.size() << "\n";
            std::cout << "\t\tExtinct: " << genData->getSpeciesManager().getNumTrackedSpecies() - speciesPresent.size() << "\n";
            std::cout << "\t\t#/Spec:  " << avgGenotypesPerSpecies << "\n";
            std::cout << "\t\tmaxFit:  " << maxSpeciesFitness << "(" << maxFitSpeciesID << ")\n";
            std::cout << "\n";

            auto genotypePtrs = genData->getGenotypesVector();
            if( !genotypePtrs.empty() )
            {
                auto front = genotypePtrs.front().lock();
                neat::MinMax<size_t> nodes( front->getNumNodes() );
                neat::MinMax<size_t> conns( front->getNumConnections() );

                long double avgNodes = 0.0;
                long double avgConns = 0.0;

                for( auto wp : genotypePtrs )
                {
                    auto p = wp.lock();
                    size_t numNodes = p->getNumNodes();
                    size_t numConns = p->getNumConnections();

                    avgNodes += numNodes;
                    avgConns += numConns;

                    nodes.expand( numNodes );
                    conns.expand( numConns );
                }

                avgNodes /= (long double)(genotypePtrs.size());
                avgConns /= (long double)(genotypePtrs.size());

                std::cout << "\tGenotypes:\n";
                std::cout << "\t\tminNodes:" << nodes.min << "\n";
                std::cout << "\t\tmaxNodes:" << nodes.max << "\n";
                std::cout << "\t\tavgNodes:" << avgNodes << "\n";
                std::cout << "\t\tminConns:" << conns.min << "\n";
                std::cout << "\t\tmaxConns:" << conns.max << "\n";
                std::cout << "\t\tavgConns:" << avgConns << "\n";
                std::cout << "\n";
            }

            std::cout << "\t% Over 900.0 fitness: " << ratioHighPreformance*100.0 << "%\n\n";

            std::cout << "\t";
            size_t i = 0;
            for( auto s : speciesPresent )
            {
                ++i;
                std::cout << "" << std::setfill('0') << std::setw(3) << s.first;
                std::cout << ": " << std::setfill(' ') << std::setw(5) << s.second << ",  ";
                if( i >= 10 )
                {
                    i = 0;
                    std::cout << "\n\t";
                }
            }

            std::cout << "\n\n\t";
            auto endangered = population.getEndangeredSpecies();
            for( auto s : endangered )
            {
                std::cout << std::setfill('0') << std::setw(3) << s.first << ":" << 100 - s.second << ", ";
            }

            std::cout << std::endl;

            //std::cout << std::flush;

            std::this_thread::sleep_for( std::chrono::duration<double>( 0.05 ) );

            //if( genData->getAvgFitness() >= 900.0 )
            if( ratioHighPreformance >= 0.99 )
            {
                break;
            }

        }

        std::cout << "\n\a";
        //std::ofstream file( "out.txt" );
        //population.printSpeciesArchetypes( std::cout );
        //file.close();

    }
}
