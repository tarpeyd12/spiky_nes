#ifndef PREVIEW_WINDOW_HPP_INCLUDED
#define PREVIEW_WINDOW_HPP_INCLUDED

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <list>
#include <thread>
#include <mutex>
#include <atomic>
#include <future>

#include "../simple_nes/include/VirtualScreen.h"
#include "../spnn/neat/thread_pool/safe_queue.hpp"
#include "../spnn/neat/thread_pool/thread_pool.hpp"

namespace spkn
{
    class PreviewWindow
    {
        private:

            const uint8_t pixelGap = 3;

            std::string windowName;
            float pixelSize;
            size_t width_inNES;
            size_t height_inNES;

            size_t populationSize;
            size_t runsPerNetwork;

            std::atomic_bool doRun;
            std::mutex virtual_screens_mutex;
            std::vector< sn::VirtualScreen > virtual_screens;
            tpl::safe_queue< std::shared_ptr<sf::Image> > screen_data_queue_in;
            tpl::safe_queue< std::shared_ptr<sf::Image> > screen_data_queue_out;
            std::list< std::shared_ptr<sf::Image> > screen_data_to_remove;
            std::shared_ptr<sf::Image> blankScreenData;
            std::thread window_thread;

            tpl::pool& working_thread_pool;

            uint64_t numKnownVBlanks;
            uint64_t numIndividualsProcessed;
            uint64_t numGenerationsProcessed;

        public:

            PreviewWindow( const std::string& window_name, size_t population_size, size_t num_previews, size_t num_columns, tpl::pool& thread_pool_to_limit, float screen_size_ratio = 2.0f );
            ~PreviewWindow();

            void addScreenData( std::shared_ptr<sf::Image> data );
            void removeScreenData( std::shared_ptr<sf::Image> data );

            void close();

            void clearAllScreenData();

            void setNumRunsPerNetwork( size_t runs_per_network );

            void setNumVBlanks( uint64_t vblanks );
            void setNumProcessed( uint64_t numProcessed );
            void setNumGenerations( uint64_t numGenerations );

        private:

            void run();

            void processAddRequests();
            void processRemoveRequests();
    };
}

#endif // PREVIEW_WINDOW_HPP_INCLUDED
