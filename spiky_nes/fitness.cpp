#include <iostream>
#include <cassert>

#include "spikey_nes.hpp"
#include "fitness.hpp"

namespace spkn
{
    FitnessCalculator::FitnessCalculator( std::shared_ptr< neat::NetworkPhenotype > net, const std::string& rom_path, uint64_t stepsPerFrame, size_t colorRings, double maxActivationWeight, size_t downscaleRatio )
         : neat::FitnessCalculator( net ),
        networkStepsPerFrame( stepsPerFrame ),
        emulator(),
        controllerState(),
        controllerSet( false ),
        numVBlanksWithoutButtonpress( 0 ),
        gameStateExtractor( emulator ),
        maxScreenPosPerLevel(),
        highestWorldLevel(0),
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
        while( networkOutputCallbacks.size() < getNumOutputNodes() )
        {
            size_t i = networkOutputCallbacks.size();
            auto func = [&,i](const spnn::neuron&){ controllerSet = controllerState[i] = true; };
            networkOutputCallbacks.push_back( func );
            if( i == size_t(sn::Controller::Start) || i == size_t(sn::Controller::Select) )
                networkOutputCallbacks.back() = nullptr;
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
        return size_t(sn::Controller::TotalButtons);
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
        long double fitness = 0.0;

        fitness += (long double)( gameStateExtractor.Score_High() );
        fitness += (long double)( gameStateExtractor.Lives() - 2 ) * 1000.0;
        fitness += highestWorldLevel;

        for( const auto& p : maxScreenPosPerLevel )
        {
            fitness += p.second;
        }

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
        if( numVBlanksWithoutButtonpress >= 2 * 60 )
        {
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
            emulator.stepFrame();
            resetControllerState();

            uint16_t worldLevel = gameStateExtractor.WorldLevel();
            highestWorldLevel = std::max( highestWorldLevel, worldLevel );
            maxScreenPosPerLevel[ worldLevel ] = std::max( maxScreenPosPerLevel[ worldLevel ], gameStateExtractor.ScreenPosition() );
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
        screenInput.back() = activationMaxValue;

        auto emuScreen = emulator.getScreenData();

        size_t downsizeSize = NESpixelsPerNetworkPixel;
        double avgscl = downsizeSize*downsizeSize;

        size_t scaled_width = sn::NESVideoWidth / downsizeSize;
        size_t scaled_height = sn::NESVideoHeight / downsizeSize;

        for( size_t y = 0; y < scaled_height; ++y )
        {
            for( size_t x = 0; x < scaled_width; ++x )
            {
                auto scaled_index = y * scaled_width + x;

                double r = 0.0, g = 0.0, b = 0.0;

                for( size_t i = 0; i < downsizeSize*downsizeSize; ++i )
                {
                    size_t _x = x * downsizeSize + ( i % downsizeSize );
                    size_t _y = y * downsizeSize + ( i / downsizeSize );

                    auto _index = _y * sn::NESVideoWidth + _x;

                    auto color = emuScreen->getPixel( _x, _y );

                    r += color.r;
                    g += color.g;
                    b += color.b;
                }

                auto hsl = ConvertRGBtoHSL( { uint8_t(r/avgscl), uint8_t(g/avgscl), uint8_t(b/avgscl), 255 } );
                screenInput[ scaled_index ] = ConvertHSLtoSingle( hsl, spiralRings ) * activationMaxValue;
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
            controllerState[i] = false;
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

    FitnessFactory::FitnessFactory( const std::string& mario_rom, std::shared_ptr<PreviewWindow> window, double maxWeightForActivation, uint64_t steps_per_frame, size_t color_rings, size_t downscaleRatio )
         :
        rom_path( mario_rom ),
        stepsPerFrame( steps_per_frame ),
        colorRings( color_rings ),
        preview_window( window ),
        avtivationMaxValue( maxWeightForActivation ),
        NESpixelsPerNetworkPixel( downscaleRatio )
    {
        /*  */
    }

    FitnessFactory::~FitnessFactory()
    {
        /*  */
    }

    std::shared_ptr< neat::FitnessCalculator >
    FitnessFactory::getNewFitnessCalculator( std::shared_ptr< neat::NetworkPhenotype > net, size_t testNum ) const
    {
        std::shared_ptr< spkn::FitnessCalculator > calc;
        calc = std::make_shared<spkn::FitnessCalculator>( net, rom_path, stepsPerFrame, colorRings, avtivationMaxValue, NESpixelsPerNetworkPixel );

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
        return size_t(sn::Controller::TotalButtons);
    }
}
