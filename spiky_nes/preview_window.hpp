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

namespace spkn
{
    class PreviewWindow
    {
        private:

            const uint8_t pixelGap = 3;

            std::string windowName;
            float pixelSize;

            std::atomic_bool doRun;

            std::mutex virtual_screens_mutex;
            std::vector< sn::VirtualScreen > virtual_screens;

            tpl::safe_queue< std::shared_ptr<std::vector<sf::Color>> > screen_data_queue_in;

            std::mutex screen_data_queue_out_mutex;
            std::list< std::shared_ptr<std::vector<sf::Color>> > screen_data_queue_out;

            std::shared_ptr<std::vector<sf::Color>> blankScreenData;

            std::thread window_thread;

        public:

            PreviewWindow( const std::string& window_name, size_t num_previews, float screen_size_ratio = 2.0f );
            ~PreviewWindow();

            void addScreenData( std::shared_ptr<std::vector<sf::Color>> data );
            void removeScreenData( std::shared_ptr<std::vector<sf::Color>> data );

            void close();

            void clearAllScreenData();

        private:

            void run();

            void processAddRequests();
            void processRemoveRequests();
    };
}

#endif // PREVIEW_WINDOW_HPP_INCLUDED
