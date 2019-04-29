#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <future>

#include "../spnn/spnn.hpp"

#include "spikey_nes.hpp"


int
main( int argc, char** argv )
{
    std::ios_base::sync_with_stdio( false );

    spkn::SetProcessPriority_lowest();

    spkn::InitEmulatorLogs();

    std::string rom_path;

    for( int i = 1; i < argc; ++i )
    {
        std::string arg( argv[i] );
        if( argv[i][0] != '-' )
        {
            rom_path = argv[i];
        }
        else
        {
            std::cerr << "Unrecognized argument: " << argv[i] << std::endl;
        }
    }

    if( rom_path.empty() )
    {
        std::cout << "Argument required: ROM path" << std::endl;
        return 1;
    }

    neat::MutationLimits limits;
    neat::MutationRates rates;
    neat::SpeciesDistanceParameters speciationParams;

    limits.thresholdMin = {  2.0,  900.0 };
    limits.thresholdMax = { 10.0, 1000.0 };

    limits.valueDecay =   { 0.01,   900.0 };
    limits.activDecay =   { 0.001, 1000.0 };

    limits.pulseFast =    {  1,   90 };
    limits.pulseSlow =    { 10, 1000 };
    //limits.pulseFast =    { 1,  10 };
    //limits.pulseSlow =    { 5, 100 };

    limits.weight =       { -1000.0, 1000.0 };
    limits.length =       { 1, 100*60*10 };



    const double simpleMutationRate_node = 0.0001;
    const double simpleMutationRate_conn = 0.0001;

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
    speciationParams.weights =     0.25 / limits.weight.range();
    speciationParams.lengths =     0.25 / limits.length.range();

    speciationParams.activations = 0.25 / ( limits.thresholdMax.range() + limits.thresholdMin.range() );
    speciationParams.decays =      0.25 / ( limits.valueDecay.range() + limits.activDecay.range() );
    speciationParams.pulses =      0.25 / ( limits.pulseFast.range() + limits.pulseSlow.range() );
    speciationParams.nodes =       0.0;

    speciationParams.threshold =   3.5*2.0;

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

        mutator.addMutator( 0.0,  0.001,  0.0,    nodeMutator );
        mutator.addMutator( 0.0,  0.0008, 0.0,    nodeMutator_new );
        mutator.addMutator( 0.0,  0.0,    0.001,  connMutator );
        mutator.addMutator( 0.0,  0.0,    0.0008, connMutator_new );
        mutator.addMutator( 0.05, 0.0,    0.0,    nwtkMutator );
        mutator.addMutator( 0.0,  0.0,    0.0001, connMutator_enable );
    }

    auto random = std::make_shared< Rand::Random_Safe >(  );

    float pixelMultiplier = 2.0f;

    tpl::pool thread_pool{ 4 };

    auto previewWindow = std::make_shared<spkn::PreviewWindow>( "SpikeyNES", thread_pool.num_threads(), pixelMultiplier );

    spkn::FitnessFactory fitnessFactory(
        rom_path,
        previewWindow,
        limits.thresholdMax.max, // maximum activation value, used to scale input values
        3.0 * 60.0, // APM allowed
        100, // network steps per NES frame
        5, // color winding value
        16  // ratio of NES pixels (squared) to network inputs, powers of 2 are a best bet here
    );

    std::cout << "Population construct call ... " << std::flush;

    neat::Population population(
        150,
        fitnessFactory.numInputs(),
        fitnessFactory.numOutputs(),
        limits,
        rates,
        mutator,
        fitnessFactory,
        speciationParams,
        neat::SpeciationMethod::Closest,
        5, // num generations to buffer before species goes extinct
        25, // min generations between mass extinctions
        1 // num generation data to keep
    );

    std::cout << "Done." << std::endl;

    std::cout << "Population Init call ... " << std::flush;

    population.Init();

    std::cout << "Done." << std::endl;

    std::cout << "Population First Mutation ... " << std::flush;

    if( false ) do
    {
        population.mutatePopulation( thread_pool, random );
        std::cout << " ... " << std::flush;
    }
    while( population.speciatePopulationAndCount( thread_pool ) < 2 );


    std::cout << "Done." << std::endl;

    std::ofstream logfile( "out.csv", std::ofstream::trunc );
    std::ofstream popfile( "pop.csv", std::ofstream::trunc );
    {
        //std::ofstream logfile( "out.csv", std::ofstream::trunc );

        logfile << "Generation,";
        logfile << "Min,Max,Avg,";
        logfile << "MinNodesTotal,MaxNodesTotal,AvgNodesTotal,MinConnTotal,MaxConnTotal,AvgConnTotal,";
        logfile << "MinNodesActive,MaxNodesActive,AvgNodesActive,MinConnActive,MaxConnActive,AvgConnActive,";
        logfile << "numSpecies,numExtinct,numMassExtinct,calculation time,attrition rate\n";
        //logfile.close();

        popfile << "Generation,Fitness,Species\n";
    }

    std::cout << "Population Evolution ..." << std::endl;

    auto evolution_start_time = std::chrono::high_resolution_clock::now();

    //while( population.getGenerationCount() < 100 )
    while( true )
    {
        auto generation_start_time = std::chrono::high_resolution_clock::now();

        std::cout << "\n\nGeneration " << population.getGenerationCount() << ":\n";

        std::cout << "\n\tCalculating ...\n" << std::flush;

        double base_attrition  = 0.5;
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
        fitnessFactory.incrementGeneration();

        std::cout << "\tDone. (~" << round( 1000.0*std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - generation_start_time).count())/1000.0 << "s)\n\n";

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

        {
            auto genSpeciesIDs = genData->getSpeciesIDVector();
            auto genFitness = genData->getFitnessVector();

            size_t len = std::min( genSpeciesIDs.size(), genFitness.size() );

            for( size_t i = 0; i < len; ++i )
            {
                popfile << population.getGenerationCount() - 1 << ",";
                popfile << genFitness[i] << ",";
                popfile << genSpeciesIDs[i] << "\n";
                popfile << std::flush;
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

            //std::ofstream logfile( "out.csv", std::ofstream::app );

            logfile << population.getGenerationCount() - 1 << ",";
            logfile << genData->getMinFitness() << ",";
            logfile << genData->getMaxFitness() << ",";
            logfile << genData->getAvgFitness() << ",";
            logfile << nodes.min << ",";
            logfile << nodes.max << ",";
            logfile << avgNodes << ",";
            logfile << conns.min << ",";
            logfile << conns.max << ",";
            logfile << avgConns << ",";
            logfile << nodes_active.min << ",";
            logfile << nodes_active.max << ",";
            logfile << avgNodes_active << ",";
            logfile << conns_active.min << ",";
            logfile << conns_active.max << ",";
            logfile << avgConns_active << ",";
            logfile << speciesPresent.size() << ",";
            logfile << genData->getSpeciesManager().getNumTrackedSpecies() - speciesPresent.size() << ",";
            logfile << population.getNumMassExtinctions() << ",";
            logfile << dur.count() << ",";
            logfile << attritionRate << ",";
            logfile << "\n" << std::flush;

            //logfile.close();
        }

        //std::this_thread::sleep_for( std::chrono::duration<double>( 0.05 ) );

        //if( genData->getAvgFitness() >= 900.0 )
        /*if( ratioHighPreformance >= 0.95 || genData->getMaxFitness() >= 1000.0 )
        {
            break;
        }*/

    }

    logfile.close();
    popfile.close();

    std::cout << "\n\a";
    std::ofstream success_file( "out.txt" );
    //population.printSpeciesArchetypes( std::cout );
    population.printSpeciesArchetypes( success_file );
    success_file.close();

    previewWindow->close();

    return 0;
}









/*
auto _up =    [](unsigned int j){ return sf::Joystick::hasAxis(j,sf::Joystick::Axis::Y) && sf::Joystick::getAxisPosition(j,sf::Joystick::Axis::Y) < -80.0; };
auto _down =  [](unsigned int j){ return sf::Joystick::hasAxis(j,sf::Joystick::Axis::Y) && sf::Joystick::getAxisPosition(j,sf::Joystick::Axis::Y) > 80.0; };
auto _left =  [](unsigned int j){ return sf::Joystick::hasAxis(j,sf::Joystick::Axis::X) && sf::Joystick::getAxisPosition(j,sf::Joystick::Axis::X) < -80.0; };
auto _right = [](unsigned int j){ return sf::Joystick::hasAxis(j,sf::Joystick::Axis::X) && sf::Joystick::getAxisPosition(j,sf::Joystick::Axis::X) > 80.0; };

std::map<sn::Controller::Buttons,std::function<bool(void)>> controller_map = {
    { sn::Controller::A,      [&]{ return sf::Joystick::isButtonPressed(0, 1) || sf::Joystick::isButtonPressed(0, 2) || sf::Keyboard::isKeyPressed(sf::Keyboard::J); } },
    { sn::Controller::B,      [&]{ return sf::Joystick::isButtonPressed(0, 0) || sf::Joystick::isButtonPressed(0, 3) || sf::Keyboard::isKeyPressed(sf::Keyboard::K); } },
    { sn::Controller::Select, [&]{ return sf::Joystick::isButtonPressed(0, 6) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift); } },
    { sn::Controller::Start,  [&]{ return sf::Joystick::isButtonPressed(0, 7) || sf::Keyboard::isKeyPressed(sf::Keyboard::Return); } },
    { sn::Controller::Up,     [&]{ return _up(0)    || sf::Keyboard::isKeyPressed(sf::Keyboard::W); } },
    { sn::Controller::Down,   [&]{ return _down(0)  || sf::Keyboard::isKeyPressed(sf::Keyboard::S); } },
    { sn::Controller::Left,   [&]{ return _left(0)  || sf::Keyboard::isKeyPressed(sf::Keyboard::A); } },
    { sn::Controller::Right,  [&]{ return _right(0) || sf::Keyboard::isKeyPressed(sf::Keyboard::D); } },
};
*/
