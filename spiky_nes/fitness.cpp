#include <iostream>
#include <cassert>

#include "spikey_nes.hpp"
#include "fitness.hpp"

namespace spkn
{
    FitnessCalculator::FitnessCalculator( std::shared_ptr< neat::NetworkPhenotype > net, const std::string& rom_path, uint64_t stepsPerFrame, size_t colorRings )
         : neat::FitnessCalculator( net ),
        networkStepsPerFrame( stepsPerFrame ),
        emulator(),
        controllerState(),
        gameStateExtractor( emulator ),
        maxScreenPosPerLevel(),
        highestWorldLevel(0),
        networkOutputCallbacks(),
        spiralRings( colorRings ),
        parentFactory( nullptr )
    {
        networkStepsPerFrame = std::max<size_t>( 1, networkStepsPerFrame );

        // init the emulator with the rom
        emulator.init( rom_path );
        gameStateExtractor.InitGameToRunning();

        // hook network output to the controller state
        while( networkOutputCallbacks.size() < getNumOutputNodes() )
        {
            size_t i = networkOutputCallbacks.size();
            networkOutputCallbacks.push_back( [&,i](const spnn::neuron&){ controllerState[i] = true; } );
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
        }
    }

    size_t
    FitnessCalculator::numInputs()
    {
        return sn::NESVideoHeight * sn::NESVideoWidth / 4;
    }

    size_t
    FitnessCalculator::numOutputs()
    {
        return size_t(sn::Controller::TotalButtons);
    }

    std::shared_ptr<std::vector<sf::Color>>
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
        //fitness += (long double)( gameStateExtractor.Coins_BCD() );
        fitness += (long double)( gameStateExtractor.Lives() ) * 1000.0;
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
        if( gameStateExtractor.Lives() == 0 )
        {
            return true;
        }
        return false;
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
        std::vector< double > output( getNumInputNodes(), 0.0 );

        auto emuScreen = emulator.getScreenData();

        size_t scaled_width = sn::NESVideoWidth / 2;
        size_t scaled_height = sn::NESVideoHeight / 2;

        for( size_t y = 0; y < scaled_height; ++y )
        {
            for( size_t x = 0; x < scaled_width; ++x )
            {
                auto scaled_index = y * scaled_width + x;

                double r = 0.0, g = 0.0, b = 0.0;

                for( size_t i = 0; i < 4; ++i )
                {
                    size_t _x = x * 2 + ( (i & 1) ? 1 : 0 );
                    size_t _y = y * 2 + ( (i & 2) ? 1 : 0 );

                    auto _index = _y * sn::NESVideoWidth + _x;

                    auto color = (*emuScreen)[ _index ];

                    r += color.r;
                    g += color.g;
                    b += color.b;
                }

                auto hsl = ConvertRGBtoHSL( { uint8_t(r/4.0), uint8_t(g/4.0), uint8_t(b/4.0), 255 } );
                output[ scaled_index ] = ConvertHSLtoSingle( hsl, spiralRings );
            }
        }

        return output;
    }

    std::vector< neat::NodeCallback >
    FitnessCalculator::getNodeCallbacks()
    {
        return networkOutputCallbacks;
    }

    void
    FitnessCalculator::resetControllerState()
    {
        for( size_t i = 0; i < controllerState.size(); ++i )
        {
            controllerState[i] = false;
        }
    }

    void
    FitnessCalculator::setParentFactory( const FitnessFactory * factory )
    {
        parentFactory = const_cast<FitnessFactory*>(factory);
    }


    // fitness factory

    FitnessFactory::FitnessFactory( const std::string& mario_rom, uint64_t steps_per_frame, size_t color_rings )
         :
        rom_path( mario_rom ),
        stepsPerFrame( steps_per_frame ),
        colorRings( color_rings )
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
        calc = std::make_shared<spkn::FitnessCalculator>( net, rom_path, stepsPerFrame, colorRings );

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

    uint64_t
    FitnessFactory::getTotalVBlanks() const
    {
        return totalVBlanks;
    }
}
