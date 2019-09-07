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
    namespace t4
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

                uint64_t lastKnownTime;
                uint64_t lastSignalTime;
                long double fitness;

                bool stopCondition;

                std::vector< neat::NodeCallback > callbacks;

                uint8_t data;

            public:

                TestFitnessCalculator( std::shared_ptr< neat::NetworkPhenotype > net )
                     : neat::FitnessCalculator( net ), lastKnownTime( 0 ), lastSignalTime( 0 ), fitness( 0.0 ), stopCondition( false ), callbacks(), data() /*  */
                {
                    while( callbacks.size() < getNumOutputNodes() )
                    {
                        callbacks.push_back( TestNodeFitnessCallback( this, callbacks.size() ) );
                        /*auto s = callbacks.size();
                        auto t = this;
                        callbacks.push_back( [=]( const spnn::neuron& ){ t->nodeActivationCallback( s ); } );*/
                    }

                    data = Rand::Int( 0, 3 );
                }

                virtual ~TestFitnessCalculator() = default;

                long double
                getFitnessScore() const override
                {
                    if( !stopCondition )
                    {
                        return fitness - double( lastKnownTime - lastSignalTime ) / 1000.0;
                    }

                    return fitness;
                }

                uint64_t
                getMaxNumTimesteps() const override
                {
                    return 1000000;
                }

                bool
                stopTest() const override
                {
                    if( stopCondition || lastKnownTime - lastSignalTime > 100 )
                    {
                        return true;
                    }

                    return false;
                }

                void
                testTick( uint64_t time ) override
                {
                    lastKnownTime = time;

                    /*  */
                }

                uint64_t
                getInputValueCheckCadence() const override
                {
                    // 0 means only once on init, 1 is every tick 2, every other
                    return 0;
                }

                std::vector< double >
                getInputValues( uint64_t /*time*/ ) override
                {
                    std::vector< double > out;

                    out.push_back( 1000.0 ); // constant
                    out.push_back( ( data & 1 ) ? 1000.0 : 0.0 );
                    out.push_back( ( data & 2 ) ? 1000.0 : 0.0 );

                    while( out.size() < getNumInputNodes() )
                    {
                        out.push_back( 0.0 );
                    }

                    return out;
                }

                std::vector< neat::NodeCallback >
                getNodeCallbacks() override
                {
                    return callbacks;
                }

                void
                nodeActivationCallback( neat::NodeID nodeID )
                {
                    uint64_t currentTime = lastKnownTime;

                    {
                        bool a = ( data & 1 );
                        bool b = ( data & 2 );
                        bool correct = ( a != b );

                        bool res = bool( nodeID );

                        if( correct == res )
                        {
                            fitness += 1000.0;
                        }
                        else
                        {
                            fitness -= 1000.0;
                        }

                        stopCondition = true;
                    }

                    lastSignalTime = currentTime;
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
                getNewFitnessCalculator( std::shared_ptr< neat::NetworkPhenotype > net, size_t ) const override
                {
                    return std::make_shared< TestFitnessCalculator >( net );
                }
        };
    }

    void
    Test4()
    {


        {
            //Rand::Seed( 0 );

            neat::MutationLimits limits;
            neat::MutationRates rates;
            neat::SpeciesDistanceParameters speciationParams;

            t4::TestFitnessFactory fitnessFactory;

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

            rates.weight =                1.0;
            rates.length =               10;

            speciationParams.excess =      3.0;
            speciationParams.disjoint =    2.5;
            speciationParams.weights =     0.0005;
            speciationParams.lengths =     0.002;

            speciationParams.activations = 0.0001;
            speciationParams.decays =      0.01;
            speciationParams.pulses =      0.0001;
            speciationParams.nodes =       1.0;

            speciationParams.threshold =   2.0;

            auto mutator = std::make_shared< neat::Mutations::Mutation_Multi >();

            mutator->addMutator< neat::Mutations::Mutation_Node_thresh_min   >( 0.05000 );
            mutator->addMutator< neat::Mutations::Mutation_Node_thresh_max   >( 0.05000 );
            mutator->addMutator< neat::Mutations::Mutation_Node_decays_value >( 0.05000 );
            mutator->addMutator< neat::Mutations::Mutation_Node_decays_activ >( 0.05000 );
            mutator->addMutator< neat::Mutations::Mutation_Node_pulses_fast  >( 0.05000 );
            mutator->addMutator< neat::Mutations::Mutation_Node_pulses_slow  >( 0.05000 );

            mutator->addMutator< neat::Mutations::Mutation_Conn_weight       >( 0.05000 );
            mutator->addMutator< neat::Mutations::Mutation_Conn_length       >( 0.05000 );
            mutator->addMutator< neat::Mutations::Mutation_Conn_enable       >( 0.01000 );

            mutator->addMutator< neat::Mutations::Mutation_Add_node          >( 0.00100 );
            mutator->addMutator< neat::Mutations::Mutation_Add_conn          >( 0.00010 );
            mutator->addMutator< neat::Mutations::Mutation_Add_conn_unique   >( 0.00500 );

            std::cout << "Population construct call" << std::endl;

            //neat::Population population( 150, 256*240/(16*16), 6, limits, rates, mutator, fitnessFactory, speciationParams, neat::SpeciationMethod::FirstForward );
            neat::Population population( 150, 3, 2, limits, rates, mutator, fitnessFactory, speciationParams );

            tpl::pool thread_pool( 1 );

            std::cout << "Population Init call" << std::endl;

            population.Init();

            {
                std::cout << "Speciating the population" << std::endl;

                auto speciesIDs = population.getSpeciesIDsForPopulationData( thread_pool, true );

                std::cout << "Printing the Species of each genotype" << std::endl;

                std::map< neat::SpeciesID, uint64_t > speciesCount;

                {
                    size_t i = 0;
                    for( auto id : speciesIDs )
                    {
                        ++speciesCount[ id ];

                        std::cout << "sid: " << id;
                        if( ++i % 15 == 0 ) { std::cout << "\n"; }
                        else { std::cout << ", "; }
                    }

                    if( i % 15 != 0 ) { std::cout << "\n"; }
                }

                for( auto it : speciesCount )
                {
                    std::cout << "Species " << it.first << ": " << it.second << std::endl;
                }

                std::cout << "There are " << population.getNumTrackedSpecies() << " species classified." << std::endl;
                std::cout << "There are " << speciesCount.size() << " species represented in the list." << std::endl;
            }


            {
                std::cout << "Mutating the population" << std::endl;

                auto _rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

                for( size_t i = 0; i < 10; ++i )
                {
                    std::cout << "\t" << i << "\r";
                    if( i%7==0 ) { std::cout << std::flush; }
                    population.mutatePopulation( thread_pool, _rand );
                    auto speciesIDs = population.getSpeciesIDsForPopulationData( thread_pool, !true );
                }

                /*{
                    neat::Mutations::Mutation_Add_conn_unique tmp;
                    population.mutatePopulation( tmp, _rand );
                }*/


                std::cout << "Speciating the population" << std::endl;

                auto speciesIDs = population.getSpeciesIDsForPopulationData( thread_pool, !true );

                std::cout << "Printing the Species of each genotype" << std::endl;

                std::map< neat::SpeciesID, uint64_t > speciesCount;

                {
                    size_t i = 0;
                    for( auto id : speciesIDs )
                    {
                        ++speciesCount[ id ];

                        std::cout << "sid: " << id;
                        if( ++i % 15 == 0 ) { std::cout << "\n"; }
                        else { std::cout << ", "; }
                    }

                    if( i % 15 != 0 ) { std::cout << "\n"; }
                }

                for( auto it : speciesCount )
                {
                    std::cout << "Species " << it.first << ": " << it.second << std::endl;
                }

                std::cout << "There are " << population.getNumTrackedSpecies() << " species classified." << std::endl;
                std::cout << "There are " << speciesCount.size() << " species represented in the list." << std::endl;
            }


            std::cout << "Calculating species fitnesses" << std::endl;

            for( size_t i = 0; i < 10; ++i )
            {
                //std::this_thread::sleep_for( std::chrono::duration<double>( 1 ) );

                std::cout << "\nTest " << i << std::endl;
                auto speciesFitness = population.getSpeciesFitness( thread_pool );

                for( auto it : speciesFitness )
                {
                    std::cout << "\tSpecies " << it.first << " has fitness " << it.second << std::endl;
                }
            }

            std::cout << "\n";
            //std::ofstream file( "out.txt" );
            population.printSpeciesArchetypes( std::cout );
            //file.close();


        }
    }
}
