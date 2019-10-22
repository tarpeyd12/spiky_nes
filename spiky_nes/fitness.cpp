#include <iostream>
#include <cassert>

#include "spikey_nes.hpp"
#include "fitness.hpp"

namespace spkn
{
    FitnessCalculator::FitnessCalculator( std::shared_ptr< neat::NetworkPhenotype > net, const std::string& rom_path, uint64_t stepsPerFrame, size_t colorRings, double maxActivationWeight, size_t downscaleRatio, double APM, std::shared_ptr<Rand::RandomFunctor> _rand )
         : neat::FitnessCalculator( net ),
        networkStepsPerFrame( stepsPerFrame ),
        emulator(),
        controllerState(),
        previousControllerState(),
        controllerSet( false ),
        actionsAvailable( 3.0 ),
        actionsPerMinute( APM ),
        numVBlanksWithoutButtonpress( 0 ),
        lastKnownScreenPosition( 0.0 ),
        numVBlanksWithoutMoving( 0 ),
        gameStateExtractor( emulator ),
        random( _rand ),
        maxScreenPosPerLevel(),
        highestWorldLevel( 0 ),
        controllStopped( false ),
        networkOutputCallbacks(),
        screenInput(),
        spiralRings( colorRings ),
        activationMaxValue( maxActivationWeight ),
        NESpixelsPerNetworkPixel( downscaleRatio ),
        parentFactory( nullptr )
    {
        networkStepsPerFrame = std::max<size_t>( 1, networkStepsPerFrame );

        screenInput.resize( numInputs(), 0.0 );

        // init the emulator with the rom
        emulator.init( rom_path );
        gameStateExtractor.InitGameToRunning();

        // hook network output to the controller state
        /*while( networkOutputCallbacks.size() < getNumOutputNodes() )
        {
            size_t i = networkOutputCallbacks.size();
            auto func = [&,i](const spnn::neuron&){ controllerSet = controllerState[i] = true; };
            networkOutputCallbacks.push_back( func );
            if( i == size_t(sn::Controller::Start) || i == size_t(sn::Controller::Select) )
                networkOutputCallbacks.back() = nullptr;
        }*/

        networkOutputCallbacks.emplace_back( [&](const spnn::neuron&){ activateButton( size_t( sn::Controller::A ) ); } );
        networkOutputCallbacks.emplace_back( [&](const spnn::neuron&){ activateButton( size_t( sn::Controller::B ) ); } );
        networkOutputCallbacks.emplace_back( [&](const spnn::neuron&){ activateButton( size_t( sn::Controller::Up ) ); } );
        networkOutputCallbacks.emplace_back( [&](const spnn::neuron&){ activateButton( size_t( sn::Controller::Down ) ); } );
        networkOutputCallbacks.emplace_back( [&](const spnn::neuron&){ activateButton( size_t( sn::Controller::Left ) ); } );
        networkOutputCallbacks.emplace_back( [&](const spnn::neuron&){ activateButton( size_t( sn::Controller::Right ) ); } );

        while( networkOutputCallbacks.size() < getNumOutputNodes() )
        {
            networkOutputCallbacks.emplace_back( nullptr );
        }

        // hook the emulator input to the controller state
        std::vector<std::function<bool(void)>> controllerCallbacks;
        while( controllerCallbacks.size() < size_t(sn::Controller::TotalButtons) )
        {
            size_t i = controllerCallbacks.size();
            controllerCallbacks.push_back( [&,i]{ return controllerState[i]; } );
        }

        // hook the callbacks to the emulator, and disable the start and select buttons
        emulator.setControllerCallbacks( controllerCallbacks, {} );
        emulator.setControllerCallbackMap( { { sn::Controller::Start, []{return false;} }, { sn::Controller::Select, []{return false;} }, }, {} );
    }

    FitnessCalculator::~FitnessCalculator()
    {
        // add the vblanks gone through to the factory
        if( parentFactory != nullptr )
        {
            parentFactory->addToTotalVBlanks( getNumVBlank() );
            parentFactory->unregesterScreenData( getScreenData() );
        }
    }

    size_t
    FitnessCalculator::numInputs()
    {
        return sn::NESVideoHeight * sn::NESVideoWidth / ( NESpixelsPerNetworkPixel * NESpixelsPerNetworkPixel ) + 1;
    }

    size_t
    FitnessCalculator::numOutputs()
    {
        return size_t(sn::Controller::TotalButtons) - 2; // exclude start and select
    }

    std::shared_ptr<sf::Image>
    FitnessCalculator::getScreenData() const
    {
        return emulator.getScreenData();
    }

    uint64_t
    FitnessCalculator::getNumVBlank() const
    {
        return emulator.getNumVBlank();
    }

    long double
    FitnessCalculator::getFitnessScore() const
    {
        const long double screen_points = 1.0L;

        long double fitness = 0.0L;

        long double network_activity = (long double)( Network()->PulsesProcessed() ) / (long double)( Network()->numNeurons() ) / (long double)( Network()->Time() ) * (long double)( networkStepsPerFrame );
        long double minutes_played   = (long double)( getNumVBlank() ) / 3600.0L;
        long double game_score       = (long double)( gameStateExtractor.Score_High() );
        long double world_score      = (long double)( highestWorldLevel - 11 ); // BCD ex. World 3-2 would be 32
        long double level_count      = (long double)( std::max<size_t>( 1, maxScreenPosPerLevel.size() ) );
        long double lives_score      = ( !controllStopped ? (long double)( gameStateExtractor.Lives() - 2 ) : 0.0L );
        long double traversal_score  = std::accumulate( maxScreenPosPerLevel.begin(), maxScreenPosPerLevel.end(), 0.0L, []( const auto& a, const auto& b ){ return a + b.second; } );

        fitness += game_score      * screen_points * 0.0001L;
        fitness += traversal_score * screen_points * 1.0L;
        fitness += world_score     * screen_points * 1.0L;
        fitness += level_count     * screen_points * 1.0L;
        fitness += lives_score     * screen_points * 1.0L;

        fitness *= ( controllStopped ? 1.0L - ( 0.1L ) : 1.0L ); // -10% to score if the network stopped giving inputs

        fitness -= ( pow( minutes_played   + 1.0L, 1.0L / 2.0L ) - 1.0L ) * screen_points;
        fitness -= ( pow( network_activity + 1.0L, 1.0L / 3.0L ) - 1.0L ) * screen_points;

        return fitness;
    }

    uint64_t
    FitnessCalculator::getMaxNumTimesteps() const
    {
        return ~0;
    }

    bool
    FitnessCalculator::stopTest() const
    {
        if( numVBlanksWithoutButtonpress / 60.0 >= 15.0 )
        {
            controllStopped = true;
            return true;
        }
        if( numVBlanksWithoutMoving / 60.0 >= 30.0 )
        {
            controllStopped = true;
            return true;
        }
        if( gameStateExtractor.Lives() > 2 )
        {
            return false;
        }
        return true;
    }

    void
    FitnessCalculator::testTick( uint64_t time )
    {
        if( time % networkStepsPerFrame == 0 )
        {
            actionsAvailable += actionsPerMinute / 3600.0L; // accumulate apm

            actionsAvailable = std::min( 12.0L, actionsAvailable );

            emulator.stepFrame();
            resetControllerState();

            uint16_t worldLevel = gameStateExtractor.WorldLevel();
            highestWorldLevel = std::max( highestWorldLevel, worldLevel );

            double current_screen_position = gameStateExtractor.ScreenPosition();
            maxScreenPosPerLevel[ worldLevel ] = std::max( maxScreenPosPerLevel[ worldLevel ], current_screen_position );

            if( lastKnownScreenPosition != current_screen_position )
            {
                numVBlanksWithoutMoving = 0;
            }
            else
            {
                ++numVBlanksWithoutMoving;
            }
            lastKnownScreenPosition = current_screen_position;
        }
    }


    uint64_t
    FitnessCalculator::getInputValueCheckCadence() const
    {
        // 0 means only once on init, 1 is every tick 2, every other etc.
        return networkStepsPerFrame;
    }

    std::vector< double >
    FitnessCalculator::getInputValues( uint64_t time )
    {
        if( time % networkStepsPerFrame == 0 )
        {
            screenInput.back() = activationMaxValue;

            auto emuScreen = emulator.getScreenData();

            size_t downsizeSize = NESpixelsPerNetworkPixel;

            size_t scaled_width = sn::NESVideoWidth / downsizeSize;
            size_t scaled_height = sn::NESVideoHeight / downsizeSize;

            /*ImageToSingle( LaplacianEdgeDetection( ResizeImage( *emuScreen, scaled_width, scaled_height ) ), spiralRings, true, screenInput, activationMaxValue, 0 );*/
            //ImageToSingle( ResizeImage( *emuScreen, scaled_width, scaled_height ), spiralRings, true, screenInput, activationMaxValue, 0 );

            // fast and good enough
            // downscale then edge-find
            ImageSobelEdgeDetectionToLightness( ResizeImage( *emuScreen, scaled_width, scaled_height ), screenInput, activationMaxValue, 0 );

            // slower but less aliasing
            //ImageSobelEdgeDetectionToLightness( DownsizeImage_Multiple( *emuScreen, downsizeSize ), screenInput, activationMaxValue, 0 );

            // a much better way, but waaaay slower, and somehow not as effective
            /*std::vector<double> tmp( emuScreen->getSize().x * emuScreen->getSize().y, 0.0 );
            ImageSobelEdgeDetectionToLightness( *emuScreen, tmp, activationMaxValue, 0 );
            DownsizeImageVec_Multiple( tmp, emuScreen->getSize().x, emuScreen->getSize().y, screenInput, downsizeSize, 0 );
            // double the values, but clamp to max value, this is because the sobel edge image is predominantly dark, thus the downscale
            // darkens the pixels, and thus the network will only be operating on a partial value space per input pixel.
            for( double& v : screenInput ) { v = std::min<double>( v * 2.0, activationMaxValue ); }*/

            /*ImageVecToImage( screenInput, scaled_width, scaled_height, activationMaxValue ).saveToFile( "tmp_scaled.png" );
            //ImageVecToImage( tmp, sn::NESVideoWidth, sn::NESVideoHeight, activationMaxValue ).saveToFile( "tmp.png" );
            tmp.saveToFile( "tmp.png" );
            exit(0);*/


            /*std::vector<double> tmp( emuScreen->getSize().x * emuScreen->getSize().y, 0.0 );
            ImageLaplacianEdgeDetectionToLightness( *emuScreen, tmp, activationMaxValue, 0 );
            ResizeImageVec( tmp, emuScreen->getSize().x, emuScreen->getSize().y, screenInput, scaled_width, scaled_height, 0 );*/

            /*ImageVecToImage( tmp, emuScreen->getSize().x, emuScreen->getSize().y, activationMaxValue ).saveToFile( "tmp_edge.png" );
            SobelEdgeDetection( ImageToGreyscale( *emuScreen ) ).saveToFile( "tmp_edge2.png" );
            ImageVecToImage( screenInput, scaled_width, scaled_height, activationMaxValue ).saveToFile( "tmp_scaled.png" );
            exit(0);*/

            /*sf::Image qimage = QuarterImage( *emuScreen );
            std::vector<double> tmp( qimage.getSize().x * qimage.getSize().y, 0.0 );
            ImageSobelEdgeDetectionToLightness( qimage, tmp, activationMaxValue, 0 );
            ResizeImageVec( tmp, qimage.getSize().x, qimage.getSize().y, screenInput, scaled_width, scaled_height, 0 );*/

            if( random != nullptr )
            {
                const double peturb = activationMaxValue / 256.0;

                neat::MinMax<double> bounds( 0.0, activationMaxValue );

                for( size_t i = 0; i < screenInput.size() - 1; ++i )
                {
                    screenInput[ i ] = bounds.clamp( screenInput[ i ] + neat::Mutations::Gaussian( random ) * peturb / 2.0L );
                }
            }
        }

        return screenInput;
    }

    std::vector< neat::NodeCallback >
    FitnessCalculator::getNodeCallbacks()
    {
        return networkOutputCallbacks;
    }

    void
    FitnessCalculator::resetControllerState()
    {
        if( !controllerSet )
        {
            ++numVBlanksWithoutButtonpress;
        }
        else
        {
            numVBlanksWithoutButtonpress = 0;
        }
        controllerSet = false;
        for( size_t i = 0; i < controllerState.size(); ++i )
        {
            previousControllerState[ i ] = controllerState[ i ];
            controllerState[ i ] = false;
        }
    }

    void
    FitnessCalculator::activateButton( size_t button )
    {
        if( actionsAvailable >= 1.0 || previousControllerState[ button ] )
        {
            if( !previousControllerState[ button ] )
            {
                actionsAvailable -= 1.0;
                controllerSet = true;
            }

            controllerState[ button ] = true;
        }
    }

    void
    FitnessCalculator::setParentFactory( const FitnessFactory * factory )
    {
        parentFactory = const_cast<FitnessFactory*>(factory);
        if( parentFactory )
        {
            parentFactory->regesterScreenData( getScreenData() );
        }
    }


    // fitness factory

    FitnessFactory::FitnessFactory( const std::string& mario_rom, std::shared_ptr<PreviewWindow> window, double maxWeightForActivation, double APM, std::shared_ptr<Rand::RandomFunctor> _rand, uint64_t steps_per_frame, size_t color_rings, size_t downscaleRatio )
         :
        rom_path( mario_rom ),
        stepsPerFrame( steps_per_frame ),
        colorRings( color_rings ),
        random( _rand ),
        totalVBlanks( 0 ),
        individualsProcessed( 0 ),
        generationsProcessed( 0 ),
        preview_window( window ),
        avtivationMaxValue( maxWeightForActivation ),
        actionsPerMinute( APM ),
        NESpixelsPerNetworkPixel( downscaleRatio )
    {
        /*  */
    }

    FitnessFactory::~FitnessFactory()
    {
        /*  */
    }

    uint64_t
    FitnessFactory::getTotalVBlanks() const
    {
        return totalVBlanks;
    }

    size_t
    FitnessFactory::numInputs()
    {
        //return FitnessCalculator::numInputs();
        return sn::NESVideoHeight * sn::NESVideoWidth / ( NESpixelsPerNetworkPixel * NESpixelsPerNetworkPixel ) + 1;
    }

    size_t
    FitnessFactory::numOutputs()
    {
        //return FitnessCalculator::numOutputs();
        return size_t(sn::Controller::TotalButtons) - 2; // exclude start and select
    }

    void
    FitnessFactory::incrementGeneration()
    {
        ++generationsProcessed;
        if( preview_window )
        {
            preview_window->setNumGenerations( generationsProcessed );
        }
    }

    std::shared_ptr< neat::FitnessCalculator >
    FitnessFactory::getNewFitnessCalculator( std::shared_ptr< neat::NetworkPhenotype > net, size_t /*testNum*/ ) const
    {
        std::shared_ptr< spkn::FitnessCalculator > calc;
        calc = std::make_shared<spkn::FitnessCalculator>( net, rom_path, stepsPerFrame, colorRings, avtivationMaxValue, NESpixelsPerNetworkPixel, actionsPerMinute, ( random != nullptr ? std::make_shared<Rand::Random_Unsafe>( random->Int() ) : nullptr ) );

        calc->setParentFactory( this );

        return calc;
    }

    size_t
    FitnessFactory::numTimesToTest() const
    {
        return 1;
    }

    void
    FitnessFactory::addToTotalVBlanks( uint64_t num_vblanks )
    {
        totalVBlanks += num_vblanks;
        ++individualsProcessed;
        if( preview_window )
        {
            preview_window->setNumVBlanks( totalVBlanks );
            preview_window->setNumProcessed( individualsProcessed );
        }
    }

    void
    FitnessFactory::regesterScreenData( std::shared_ptr<sf::Image> data )
    {
        if( preview_window )
        {
            preview_window->addScreenData( data );
        }
    }

    void
    FitnessFactory::unregesterScreenData( std::shared_ptr<sf::Image> data )
    {
        if( preview_window )
        {
            preview_window->removeScreenData( data );
        }
    }
}
