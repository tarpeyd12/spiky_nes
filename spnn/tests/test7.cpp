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
    namespace t7
    {
        inline
        std::string
        ToMorseCode( const std::string& input )
        {
            std::string output;

            for( auto c : input )
            {
                switch( c )
                {
                    case 'A': case 'a': output += ".- ";   break;
                    case 'B': case 'b': output += "-... "; break;
                    case 'C': case 'c': output += "-.-. "; break;
                    case 'D': case 'd': output += "-.. ";  break;
                    case 'E': case 'e': output += ". ";    break;
                    case 'F': case 'f': output += "..-. "; break;
                    case 'G': case 'g': output += "--. ";  break;
                    case 'H': case 'h': output += ".... "; break;
                    case 'I': case 'i': output += ".. ";   break;
                    case 'J': case 'j': output += ".--- "; break;
                    case 'K': case 'k': output += "-.- ";  break;
                    case 'L': case 'l': output += ".-.. "; break;
                    case 'M': case 'm': output += "-- ";   break;
                    case 'N': case 'n': output += "-. ";   break;
                    case 'O': case 'o': output += "--- ";  break;
                    case 'P': case 'p': output += ".--. "; break;
                    case 'Q': case 'q': output += "--.- "; break;
                    case 'R': case 'r': output += ".-. ";  break;
                    case 'S': case 's': output += "... ";  break;
                    case 'T': case 't': output += "- ";    break;
                    case 'U': case 'u': output += "..- ";  break;
                    case 'V': case 'v': output += "...- "; break;
                    case 'W': case 'w': output += ".-- ";  break;
                    case 'X': case 'x': output += "-..- "; break;
                    case 'Y': case 'y': output += "-.-- "; break;
                    case 'Z': case 'z': output += "--.. "; break;

                    case '0': output += "----- "; break;
                    case '1': output += ".---- "; break;
                    case '2': output += "..--- "; break;
                    case '3': output += "...-- "; break;
                    case '4': output += "....- "; break;
                    case '5': output += "..... "; break;
                    case '6': output += "-.... "; break;
                    case '7': output += "--... "; break;
                    case '8': output += "---.. "; break;
                    case '9': output += "----. "; break;

                    default: output += "   "; break;
                }
            }

            return output;
        }


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

                std::vector< neat::NodeCallback > callbacks;
                std::map< char, size_t > inputMap;

                std::string dataString;
                std::string codeString;
                std::string resultString;

                uint64_t stepsPerCharacter;

                uint64_t currentTimestep;
                uint64_t lastSignalTimestep;

            public:

                TestFitnessCalculator( std::shared_ptr< neat::NetworkPhenotype > net, const std::string& d, const std::string& inputOrder, uint64_t stepsPerChar, std::shared_ptr< Rand::RandomFunctor > rand )
                     : neat::FitnessCalculator( net ), callbacks(), inputMap(), dataString( d ), codeString(), resultString(), stepsPerCharacter( stepsPerChar ), currentTimestep( 0 ), lastSignalTimestep( 0 )
                {
                    if( !rand ) rand = std::make_shared< Rand::Random_Safe >( Rand::Int() );

                    {
                        if( dataString.empty() )
                        {
                            dataString = "abcdefghijklmnopqrstuvwxyz0123456789";
                        }

                        if( dataString.size() > 1 )
                        {
                            for( size_t i = 0; i < dataString.size(); ++i )
                            {
                                std::swap( dataString[ i ], dataString[ rand->Int( 0, dataString.size() - 1 ) ] );
                            }
                        }

                        codeString = ToMorseCode( dataString );
                        //std::cout << dataString << " -> " << codeString << std::endl;

                        for( size_t i = 0; i < inputOrder.size(); ++i )
                        {
                            inputMap[ tolower( inputOrder[ i ] ) ] = i;
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
                    //std::cout << "'" << codeString << "'\n'" << resultString << "'\n\n";

                    size_t numCorrect = 0;

                    for( size_t i = 0; i < std::min( codeString.size(), resultString.size() ); ++i )
                    {
                        if( codeString[i] == resultString[i] )
                        {
                            numCorrect++;
                        }
                    }

                    return (long double)( numCorrect ) / (long double)( std::max<size_t>( codeString.size(), resultString.size() ) ) * 1000.0;
                }

                uint64_t
                getMaxNumTimesteps() const override
                {
                    return stepsPerCharacter * dataString.size() * 1;
                }

                bool
                stopTest() const override
                {
                    return bool( currentTimestep - lastSignalTimestep > stepsPerCharacter * 1.5 ) || ( int(resultString.size()) - int(codeString.size()) >= 6 );
                }

                void
                testTick( uint64_t time ) override
                {
                    currentTimestep = time;
                }

                uint64_t
                getInputValueCheckCadence() const override
                {
                    // 0 means only once on init, 1 is every tick, 2 every other
                    return stepsPerCharacter;
                }

                std::vector< double >
                getInputValues( uint64_t time ) override
                {
                    const double activeValue = 1000.0;

                    std::vector< double > inputValues( getNumInputNodes() + 1, 0.0 );

                    uint64_t character = time / stepsPerCharacter;

                    // set the constant
                    inputValues.back() = activeValue;

                    if( character < getNumInputNodes() )
                    {
                        char c = tolower( dataString[ character ] );

                        size_t i = getNumInputNodes() + 1;

                        auto it = inputMap.find( c );

                        if( it != inputMap.end() )
                        {
                            i = it->second;
                        }

                        if( i < getNumInputNodes() )
                        {
                            inputValues[ i ] = activeValue;
                        }
                    }

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
                    if( nodeID > 2 )
                    {
                        return;
                    }

                    lastSignalTimestep = currentTimestep;

                    {
                        char c;
                        switch( nodeID )
                        {
                            default:
                            case 0: c = ' '; break;
                            case 1: c = '.'; break;
                            case 2: c = '-'; break;
                        }

                        resultString += c;
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
            private:

                std::shared_ptr< Rand::RandomFunctor > rand;
                size_t count;
                std::string data;

                uint64_t stepsPerCharacter;

            public:

                TestFitnessFactory( uint64_t stepsPerChar, const std::string& test_data, std::shared_ptr< Rand::RandomFunctor > _rand )
                     : neat::FitnessFactory(), rand( _rand ), count( 0 ), data( test_data ), stepsPerCharacter( stepsPerChar )
                {
                    /*  */
                }

                virtual ~TestFitnessFactory() = default;

                std::shared_ptr< neat::FitnessCalculator >
                getNewFitnessCalculator( std::shared_ptr< neat::NetworkPhenotype > net, size_t count ) const override
                {
                    std::string randomized_data = "abc123";

                    if( count < data.size() )
                    {
                        randomized_data = std::string( 7, data[ count ] );

                        for( size_t i = 0; i < 1; ++i )
                        {
                            randomized_data += data[ rand->Int( 0, data.size() - 1 ) ];
                        }
                    }
                    else
                    {
                        randomized_data = data;

                        while( randomized_data.size() < data.size() * 5.0 )
                        {
                            randomized_data += data[ rand->Int( 0, data.size() - 1 ) ];
                        }
                    }

                    return std::make_shared< TestFitnessCalculator >( net, randomized_data, data, stepsPerCharacter, rand );
                }

                size_t
                numTimesToTest() const override
                {
                    return data.size() * 1 + 1;
                }

                uint64_t
                numInputNodes() const
                {
                    return data.size() + 1;
                }

                uint64_t
                numOutputNodes() const
                {
                    return 3;
                }
        };
    }

    void
    Test7()
    {
        //SetProcessPriority_low();
        SetProcessPriority_lowest();

        //unsigned seed = 1000;

        //Rand::Seed( seed );

        auto random = std::make_shared< Rand::Random_Safe >(  );
        //auto random = std::make_shared< Rand::Random_Unsafe >( );

        neat::MutationLimits limits;
        neat::MutationRates rates;
        neat::SpeciesDistanceParameters speciationParams;

        std::string data_string = "abcdefghijklmnopqrstuvwxyz0123456789";
        {
            std::string possible_data = "abcdefghijklmnopqrstuvwxyz0123456789";

            std::set<char> possible_data_set;

            for( auto c : possible_data )
            {
                possible_data_set.emplace( c );
            }

            double percent_of_data = 1.0;

            size_t num_chars = neat::MinMax<size_t>{ 1, possible_data.size() }.clamp( percent_of_data * possible_data.size() );

            data_string = "";

            while( !possible_data_set.empty() && data_string.size() < num_chars )
            {
                auto it = possible_data_set.begin();

                std::advance( it, random->Int( 0, possible_data_set.size() - 1 ) );
                data_string += *it;

                possible_data_set.erase( *it );
            }

            std::sort( data_string.begin(), data_string.end() );

            std::cout << "Training in characters: \"" << data_string << "\"\n\n";
        }

        t7::TestFitnessFactory fitnessFactory( 200, data_string, random );

        limits.thresholdMin = {  2.0,  900.0 };
        limits.thresholdMax = { 10.0, 1000.0 };

        limits.valueDecay =   { 0.01,   900.0 };
        limits.activDecay =   { 0.001, 1000.0 };

        limits.pulseFast =    {  1,  90 };
        limits.pulseSlow =    { 10, 100 };
        //limits.pulseFast =    { 1,  10 };
        //limits.pulseSlow =    { 5, 100 };

        limits.weight =       { -1000.0, 1000.0 };
        limits.length =       { 1, 100 };

        /*limits.thresholdMin = {  150.0 };
        limits.thresholdMax = { 1000.0 };

        limits.valueDecay =   { 0.01 };
        limits.activDecay =   { 0.001 };

        limits.pulseFast =    {   1 };
        limits.pulseSlow =    { 100 };

        limits.weight =       { -1000.0, 1000.0 };
        limits.length =       { 1 };*/

        const double simpleMutationRate_node = 0.01;
        const double simpleMutationRate_conn = 0.01;

        rates.thresholdMin =         simpleMutationRate_node * limits.thresholdMin.range();
        rates.thresholdMax =         simpleMutationRate_node * limits.thresholdMax.range();

        rates.valueDecay =           simpleMutationRate_node * limits.valueDecay.range();
        rates.activDecay =           simpleMutationRate_node * limits.activDecay.range();

        rates.pulseFast =            std::max<uint64_t>( 1, simpleMutationRate_node * limits.pulseFast.range() );
        rates.pulseSlow =            std::max<uint64_t>( 1, simpleMutationRate_node * limits.pulseSlow.range() );

        rates.weight =               simpleMutationRate_conn * limits.weight.range();
        rates.length =               std::max<uint64_t>( 1, simpleMutationRate_conn * limits.length.range() );

        speciationParams.excess =      1.0;
        speciationParams.disjoint =    1.0;
        speciationParams.weights =     1.5 / limits.weight.range();
        speciationParams.lengths =     1.5 / limits.length.range();

        speciationParams.activations = 1.5 / ( limits.thresholdMax.range() + limits.thresholdMin.range() );
        speciationParams.decays =      1.5 / ( limits.valueDecay.range() + limits.activDecay.range() );
        speciationParams.pulses =      1.5 / ( limits.pulseFast.range() + limits.pulseSlow.range() );
        speciationParams.nodes =       1.0;

        speciationParams.threshold =   3.5*1.0;

        neat::Mutations::Mutation_Multi mutator;

        {
            auto nodeMutator = std::make_shared< neat::Mutations::Mutation_Multi_one >();
            auto nodeMutator_new = std::make_shared< neat::Mutations::Mutation_Multi_one >();
            auto connMutator = std::make_shared< neat::Mutations::Mutation_Multi_one >();
            auto connMutator_new = std::make_shared< neat::Mutations::Mutation_Multi_one >();
            auto nwtkMutator = std::make_shared< neat::Mutations::Mutation_Multi_one >();

            auto connMutator_enable = std::make_shared< neat::Mutations::Mutation_Conn_enable >();

            nodeMutator->addMutator< neat::Mutations::Mutation_Node_thresh_min   >();
            nodeMutator->addMutator< neat::Mutations::Mutation_Node_thresh_max   >();
            nodeMutator->addMutator< neat::Mutations::Mutation_Node_decays_activ >();
            nodeMutator->addMutator< neat::Mutations::Mutation_Node_decays_value >();
            nodeMutator->addMutator< neat::Mutations::Mutation_Node_pulses_fast  >();
            nodeMutator->addMutator< neat::Mutations::Mutation_Node_pulses_slow  >();

            nodeMutator_new->addMutator< neat::Mutations::Mutation_Node_thresh_min_new   >();
            nodeMutator_new->addMutator< neat::Mutations::Mutation_Node_thresh_max_new   >();
            nodeMutator_new->addMutator< neat::Mutations::Mutation_Node_decays_activ_new >();
            nodeMutator_new->addMutator< neat::Mutations::Mutation_Node_decays_value_new >();
            nodeMutator_new->addMutator< neat::Mutations::Mutation_Node_pulses_fast_new  >();
            nodeMutator_new->addMutator< neat::Mutations::Mutation_Node_pulses_slow_new  >();

            connMutator->addMutator< neat::Mutations::Mutation_Conn_weight >();
            connMutator->addMutator< neat::Mutations::Mutation_Conn_length >();
            //connMutator->addMutator< neat::Mutations::Mutation_Conn_enable >();

            connMutator_new->addMutator< neat::Mutations::Mutation_Conn_weight_new >();
            connMutator_new->addMutator< neat::Mutations::Mutation_Conn_length_new >();
            //connMutator_new->addMutator< neat::Mutations::Mutation_Conn_enable >();

            nwtkMutator->addMutator< neat::Mutations::Mutation_Add_node        >();
            nwtkMutator->addMutator< neat::Mutations::Mutation_Add_conn        >();
            nwtkMutator->addMutator< neat::Mutations::Mutation_Add_conn_unique >();
            nwtkMutator->addMutator< neat::Mutations::Mutation_Add_conn_dup    >();

            mutator.addMutator( 0.0,   0.01,  0.0,   nodeMutator );
            mutator.addMutator( 0.0,   0.008, 0.0,   nodeMutator_new );
            mutator.addMutator( 0.0,   0.0,   0.01,  connMutator );
            mutator.addMutator( 0.0,   0.0,   0.008, connMutator_new );
            mutator.addMutator( 0.002, 0.0,   0.0,   nwtkMutator );
            mutator.addMutator( 0.0,   0.0,   0.0002, connMutator_enable );
        }

        std::cout << "Population construct call" << std::endl;

        uint64_t numInNodes = fitnessFactory.numInputNodes();
        uint64_t numOutNodes = fitnessFactory.numOutputNodes();

        neat::Population population( 1000, numInNodes, numOutNodes, limits, rates, mutator, fitnessFactory, speciationParams, neat::SpeciationMethod::Closest, 3, 500, 1 );

        tpl::pool thread_pool{ 4 };

        std::cout << "Population Init call" << std::endl;

        population.Init();

        {
            std::ofstream file( "out.csv", std::ofstream::trunc );

            file << "Generation,";
            file << "Min,Max,Avg,";
            file << "MinNodesTotal,MaxNodesTotal,AvgNodesTotal,MinConnTotal,MaxConnTotal,AvgConnTotal,";
            file << "MinNodesActive,MaxNodesActive,AvgNodesActive,MinConnActive,MaxConnActive,AvgConnActive,";
            file << "numSpecies,numExtinct,numMassExtinct,calculation time,attrition rate\n";
            file.close();
        }

        std::cout << "Population Evolution ..." << std::endl;

        auto evolution_start_time = std::chrono::high_resolution_clock::now();

        //while( population.getGenerationCount() < 100 )
        while( true )
        {
            auto generation_start_time = std::chrono::high_resolution_clock::now();

            std::cout << "\n\nGeneration " << population.getGenerationCount() << ":\n";

            std::cout << "\n\tCalculating ...\n";

            double base_attrition  = 0.75;
            double attrition_range = 0.0125;
            neat::MinMax<double> attritionRange( base_attrition - attrition_range, base_attrition + attrition_range );
            double attritionRate = 0.5;
            {
                double x = double( population.getGenerationCount() ) / 100.0;
                attritionRate = ( ( -cos( x * 3.14159 ) + 1.0 ) / 2.0 ) * attritionRange.range() + attritionRange.min;
                //if( population.getLastGenerationData() ) { attritionRate = neat::MinMax<double>(0.05,0.95).clamp( population.getLastGenerationData()->getMaxFitness()/1000.0 ); }
            }

            // this is the only line in this loop that really matters *******************************************************
            population.IterateGeneration( thread_pool, random, attritionRate );

            std::cout << "\tDone. (~" << round( 1000.0*std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - generation_start_time).count())/1000.0 << "s)\n\n";


            std::cout << "\tData str = \"" << data_string << "\"\n";
            std::cout << "\tCode str = \"" << t7::ToMorseCode( data_string ) << "\"\n";
            std::cout << "\tattrRate = " << attritionRate << "\n";

            auto genData = population.getLastGenerationData();

            if( !genData )
            {
                continue;
            }

            auto fitVec = genData->getFitnessVector();

            size_t over = 0;
            for( auto fitness : fitVec )
            {
                if( fitness > 990.0 )
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

            auto specFitMap = genData->getSpeciesFitness();
            long double maxSpeciesFitness = specFitMap.begin()->second;
            long double minSpeciesFitness = specFitMap.begin()->second;
            neat::SpeciesID maxFitSpeciesID = specFitMap.begin()->first;
            neat::SpeciesID minFitSpeciesID = specFitMap.begin()->first;
            for( auto f : specFitMap )
            {
                if( f.second > maxSpeciesFitness )
                {
                    maxSpeciesFitness = f.second;
                    maxFitSpeciesID = f.first;
                }
                if( f.second < minSpeciesFitness )
                {
                    minSpeciesFitness = f.second;
                    minFitSpeciesID = f.first;
                }
            }

            std::cout << "\tSpecies:\n";
            std::cout << "\t\tTracked: " << genData->getSpeciesManager().getNumTrackedSpecies() << "\n";
            std::cout << "\t\tPresent: " << speciesPresent.size() << "\n";
            std::cout << "\t\tExtinct: " << genData->getSpeciesManager().getNumTrackedSpecies() - speciesPresent.size() << "(" << population.getNumMassExtinctions() << ")\n";
            std::cout << "\t\t#/Spec:  " << round( 100.0*avgGenotypesPerSpecies )/100.0 << "(" << floor( double( avgGenotypesPerSpecies )*(1.0-attritionRate) ) << ")\n";
            std::cout << "\t\tmaxFit:  " << maxSpeciesFitness << "(" << maxFitSpeciesID << ")\n";
            std::cout << "\t\tminFit:  " << minSpeciesFitness << "(" << minFitSpeciesID << ")\n";
            std::cout << "\n";

            neat::MinMax<size_t> nodes;
            neat::MinMax<size_t> conns;

            neat::MinMax<size_t> nodes_active;
            neat::MinMax<size_t> conns_active;

            long double avgNodes = 0.0;
            long double avgConns = 0.0;

            long double avgNodes_active = 0.0;
            long double avgConns_active = 0.0;

            auto genotypePtrs = genData->getGenotypesVector();
            if( !genotypePtrs.empty() )
            {
                auto front = genotypePtrs.front().lock();
                nodes = neat::MinMax<size_t>( front->getNumNodes() );
                conns = neat::MinMax<size_t>( front->getNumConnections() );

                size_t numNodes;
                size_t numConns;

                front->getNumReachableNumActive( numNodes, numConns );

                nodes_active = neat::MinMax<size_t>( numNodes );
                conns_active = neat::MinMax<size_t>( numConns );

                for( auto wp : genotypePtrs )
                {
                    auto p = wp.lock();
                    numNodes = p->getNumNodes();
                    numConns = p->getNumConnections();

                    avgNodes += numNodes;
                    avgConns += numConns;

                    nodes.expand( numNodes );
                    conns.expand( numConns );

                    p->getNumReachableNumActive( numNodes, numConns );

                    avgNodes_active += numNodes;
                    avgConns_active += numConns;

                    nodes_active.expand( numNodes );
                    conns_active.expand( numConns );
                }

                avgNodes /= (long double)(genotypePtrs.size());
                avgConns /= (long double)(genotypePtrs.size());

                avgNodes_active /= (long double)(genotypePtrs.size());
                avgConns_active /= (long double)(genotypePtrs.size());

                std::cout << "\tGenotypes:\n";
                std::cout << "\t\tminNodesT:" << nodes.min << "\n";
                std::cout << "\t\tmaxNodesT:" << nodes.max << "\n";
                std::cout << "\t\tavgNodesT:" << avgNodes << "\n";
                std::cout << "\t\tminConnsT:" << conns.min << "\n";
                std::cout << "\t\tmaxConnsT:" << conns.max << "\n";
                std::cout << "\t\tavgConnsT:" << avgConns << "\n";
                std::cout << "\n";
                std::cout << "\t\tminNodesA:" << nodes_active.min << "\n";
                std::cout << "\t\tmaxNodesA:" << nodes_active.max << "\n";
                std::cout << "\t\tavgNodesA:" << avgNodes_active << "\n";
                std::cout << "\t\tminConnsA:" << conns_active.min << "\n";
                std::cout << "\t\tmaxConnsA:" << conns_active.max << "\n";
                std::cout << "\t\tavgConnsA:" << avgConns_active << "\n";
                std::cout << "\n";
            }

            {
                double runtime_total_seconds = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - evolution_start_time).count();

                double runtime_seconds = fmod( runtime_total_seconds, 60.0 );
                int64_t runtime_minutes = fmod( runtime_total_seconds/60.0, 60.0 );
                int64_t runtime_hours = runtime_total_seconds / ( 60.0 * 60.0 );

                std::cout << "\tRuntime: (" << runtime_total_seconds << "s)\n\n";
                if( runtime_hours ) std::cout <<   "\t\tHours:  " << std::setfill(' ') << std::setw(3) << runtime_hours << "\n";
                if( runtime_minutes ) std::cout << "\t\tMinutes: " << std::setfill(' ') << std::setw(2) << runtime_minutes << "\n";

                {
                    // HOLY HELL could C++ be more verbose in formatting a single fucking float

                    auto cout_fmt_flags = std::cout.flags(); // get flags before
                    auto precision = std::cout.precision(); // get precision before

                    std::cout << "\t\tSeconds: " << std::fixed << std::setfill( ' ' ) << std::setprecision( 1 ) << std::setw( 4 ) << runtime_seconds << "\n";

                    std::cout << std::setprecision( precision ); // reset precision
                    std::cout.flags( cout_fmt_flags ); // go back to old flags
                }

                std::cout << "\n";
            }


            std::cout << "\t% Over 990.0 fitness: " << ratioHighPreformance*100.0 << "%\n\n";

            std::cout << "\t";
            size_t i = 0;
            for( auto s : speciesPresent )
            {
                ++i;
                std::cout << "[" << std::setfill('0') << std::setw(4) << s.first;
                std::cout << ":" << std::setfill(' ') << std::setw(4) << s.second << "], ";
                if( i >= 10 )
                {
                    i = 0;
                    std::cout << "\n\t";
                }
            }

            std::cout << "\n\n\t";
            auto endangered = population.getEndangeredSpecies();
            std::vector< std::pair< neat::SpeciesID, size_t > > endangered_sorted;
            endangered_sorted.reserve( endangered.size() );
            for( auto s : endangered )
            {
                //std::cout << std::setfill('0') << std::setw(3) << s.first << ":" << s.second << ", ";
                endangered_sorted.push_back( s );
            }
            std::stable_sort( endangered_sorted.begin(), endangered_sorted.end(), []( const auto& a, const auto& b ){ return std::less<size_t>()( a.second, b.second ); } );
            i = 0;
            for( auto s : endangered_sorted )
            {
                ++i;
                std::cout << std::setfill('0') << std::setw(4) << s.first << ":" << std::setfill( ' ' ) << std::setw( 3 ) << s.second << ", ";
                if( i >= 10 )
                {
                    i = 0;
                    std::cout << "\n\t";
                }
            }

            std::cout << std::endl;

            //std::cout << std::flush;

            {
                auto generation_end_time = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> dur = generation_end_time - generation_start_time;

                std::ofstream file( "out.csv", std::ofstream::app );

                file << population.getGenerationCount() - 1 << ",";
                file << genData->getMinFitness() << ",";
                file << genData->getMaxFitness() << ",";
                file << genData->getAvgFitness() << ",";
                file << nodes.min << ",";
                file << nodes.max << ",";
                file << avgNodes << ",";
                file << conns.min << ",";
                file << conns.max << ",";
                file << avgConns << ",";
                file << nodes_active.min << ",";
                file << nodes_active.max << ",";
                file << avgNodes_active << ",";
                file << conns_active.min << ",";
                file << conns_active.max << ",";
                file << avgConns_active << ",";
                file << speciesPresent.size() << ",";
                file << genData->getSpeciesManager().getNumTrackedSpecies() - speciesPresent.size() << ",";
                file << population.getNumMassExtinctions() << ",";
                file << dur.count() << ",";
                file << attritionRate << ",";
                file << "\n";

                file.close();
            }

            //std::this_thread::sleep_for( std::chrono::duration<double>( 0.05 ) );

            //if( genData->getAvgFitness() >= 900.0 )
            if( ratioHighPreformance >= 0.95 || genData->getMaxFitness() >= 1000.0 )
            {
                break;
            }

        }

        std::cout << "\n\a";
        std::ofstream success_file( "out.txt" );
        //population.printSpeciesArchetypes( std::cout );
        population.printSpeciesArchetypes( success_file );
        success_file.close();

    }
}
