#ifndef SPKN_FITNESS_HPP_INCLUDED
#define SPKN_FITNESS_HPP_INCLUDED

#include "../simple_nes/include/Emulator.h"
#include "../spnn/spnn.hpp"

#include "preview_window.hpp"
#include "game_state.hpp"

namespace spkn
{
    class FitnessFactory;

    class FitnessCalculator : public neat::FitnessCalculator
    {
        private:

            // emulator states

            uint64_t networkStepsPerFrame;

            sn::Emulator emulator;
            std::array< bool, size_t(sn::Controller::TotalButtons) > controllerState;
            std::array< bool, size_t(sn::Controller::TotalButtons) > previousControllerState;
            bool controllerSet;
            long double actionsAvailable;
            long double actionsPerMinute;
            uint64_t numVBlanksWithoutButtonpress;
            double lastKnownScreenPosition;
            uint64_t numVBlanksWithoutMoving;

            GameState_SuperMarioBros gameStateExtractor;

            std::shared_ptr<Rand::RandomFunctor> random;

            // fitness state

            std::map< uint16_t, double > maxScreenPosPerLevel;
            uint16_t highestWorldLevel;
            mutable bool controllStopped;

            // callbacks

            std::vector< neat::NodeCallback > networkOutputCallbacks;

            // screen settings

            std::vector< double > screenInput;
            size_t spiralRings;
            double activationMaxValue;
            size_t NESpixelsPerNetworkPixel;

            // book-keeping

            FitnessFactory * parentFactory;

        public:

            FitnessCalculator( std::shared_ptr< neat::NetworkPhenotype > net, const std::string& rom_path, uint64_t stepsPerFrame, size_t colorRings, double maxActivationWeight, size_t downscaleRatio, double APM, std::shared_ptr<Rand::RandomFunctor> _rand = nullptr );
            virtual ~FitnessCalculator();

            FitnessCalculator( const FitnessCalculator& ) = delete;
            FitnessCalculator operator=( const FitnessCalculator& ) = delete;

            size_t numInputs();
            size_t numOutputs();

            // emulator stuff
            std::shared_ptr<sf::Image> getScreenData() const;
            uint64_t getNumVBlank() const;

        protected:

            // fitness calculator stuff

            long double getFitnessScore() const override;
            uint64_t getMaxNumTimesteps() const override;

            bool stopTest() const override;
            void testTick( uint64_t time ) override;

            uint64_t getInputValueCheckCadence() const override; // 0 means only once on init, 1 is every tick 2, every other etc.
            std::vector< double > getInputValues( uint64_t time ) override;
            std::vector< neat::NodeCallback > getNodeCallbacks() override;

            // controller stuff

            void resetControllerState();
            void activateButton( size_t button );

            // book-keeping stuff

            void setParentFactory( const FitnessFactory * factory );

            // friends
            friend class FitnessFactory;
    };

    class FitnessFactory : public neat::FitnessFactory
    {
        private:

            std::string rom_path;
            uint64_t stepsPerFrame;
            size_t colorRings;

            std::shared_ptr<Rand::RandomFunctor> random;

            std::atomic<uint64_t> totalVBlanks;
            std::atomic<uint64_t> individualsProcessed;
            std::atomic<uint64_t> generationsProcessed;

            std::shared_ptr<PreviewWindow> preview_window;

            double avtivationMaxValue;

            double actionsPerMinute;

            size_t NESpixelsPerNetworkPixel;

        public:

            FitnessFactory( const std::string& mario_rom, std::shared_ptr<PreviewWindow> window, double maxWeightForActivation, double APM, std::shared_ptr<Rand::RandomFunctor> _rand = nullptr, uint64_t steps_per_frame = 100, size_t color_rings = 5, size_t downscaleRatio = 2 );
            virtual ~FitnessFactory();

            uint64_t getTotalVBlanks() const;

            size_t numInputs();
            size_t numOutputs();

            void incrementGeneration();

        protected:

            std::shared_ptr< neat::FitnessCalculator > getNewFitnessCalculator( std::shared_ptr< neat::NetworkPhenotype > net, size_t testNum ) const override;
            size_t numTimesToTest() const override;

            // book-keeping stuff

            void addToTotalVBlanks( uint64_t num_vblanks );
            void regesterScreenData( std::shared_ptr<sf::Image> data );
            void unregesterScreenData( std::shared_ptr<sf::Image> data );

            // friends
            friend class FitnessCalculator;
    };
}

#endif // SPKN_FITNESS_HPP_INCLUDED
