#ifndef PREVIEW_WINDOW_HPP_INCLUDED
#define PREVIEW_WINDOW_HPP_INCLUDED

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <list>
#include <mutex>
#include <atomic>
#include <future>

#include "../simple_nes/include/VirtualScreen.h"

namespace spkn
{
    class PreviewWindow
    {
        private:

            sf::RenderWindow window;
            std::string windowName;
            float pixelSize;

            std::atomic_bool doRun;

            mutable std::mutex virtual_screens_mutex;
            std::vector< sn::VirtualScreen > virtual_screens;

            mutable std::mutex screen_data_queue_in_mutex;
            std::list< std::shared_ptr<std::vector<sf::Color>> > screen_data_queue_in;

            mutable std::mutex screen_data_queue_out_mutex;
            std::list< std::shared_ptr<std::vector<sf::Color>> > screen_data_queue_out;

            std::shared_ptr<std::vector<sf::Color>> blankScreenData;

            std::future< void > window_thread;

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