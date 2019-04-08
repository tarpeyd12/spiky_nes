#include "preview_window.hpp"

#include "../simple_nes/include/Emulator.h"

namespace spkn
{
    PreviewWindow::PreviewWindow( const std::string& window_name, size_t num_previews, float screen_size_ratio )
         :
        window(),
        windowName( window_name ),
        pixelSize( screen_size_ratio ),
        doRun( true ),
        virtual_screens_mutex(),
        virtual_screens(),
        screen_data_queue_in_mutex(),
        screen_data_queue_in(),
        screen_data_queue_out_mutex(),
        screen_data_queue_out(),
        blankScreenData( nullptr )
    {
        blankScreenData = std::make_shared<std::vector<sf::Color>>( sn::NESVideoWidth * sn::NESVideoHeight, sf::Color::Magenta );

        while( virtual_screens.size() < num_previews )
        {
            virtual_screens.push_back( sn::VirtualScreen() );
            virtual_screens.back().setScreenData( blankScreenData );
            virtual_screens.back().setScreenPosition( { (sn::NESVideoWidth + 1) * pixelSize * virtual_screens.size(), 0.0 } );
        }

        window_thread = std::async( std::launch::async, [&]{ run(); } );
    }

    PreviewWindow::~PreviewWindow()
    {
        close();
        window_thread.get();
    }

    void
    PreviewWindow::addScreenData( std::shared_ptr<std::vector<sf::Color>> data )
    {
        std::unique_lock<std::mutex> lock( screen_data_queue_in_mutex );

        screen_data_queue_in.push_back( data );
    }

    void
    PreviewWindow::removeScreenData( std::shared_ptr<std::vector<sf::Color>> data )
    {
        std::unique_lock<std::mutex> lock( screen_data_queue_out_mutex );

        screen_data_queue_out.push_back( data );
    }

    void
    PreviewWindow::close()
    {
        doRun = false;
    }

    void
    PreviewWindow::clearAllScreenData()
    {
        std::unique_lock<std::mutex>  vs_lock( virtual_screens_mutex,       std::defer_lock );
        std::unique_lock<std::mutex>  in_lock( screen_data_queue_in_mutex,  std::defer_lock );
        std::unique_lock<std::mutex> out_lock( screen_data_queue_out_mutex, std::defer_lock );
        std::lock( vs_lock, in_lock, out_lock );

        for( auto& vs : virtual_screens )
        {
            vs.setScreenData( blankScreenData );
        }

        while( screen_data_queue_in.size() )
        {
            screen_data_queue_in.pop_front();
        }

        while( screen_data_queue_out.size() )
        {
            screen_data_queue_out.pop_front();
        }
    }

    void
    PreviewWindow::run()
    {
        sf::VideoMode videoMode( (sn::NESVideoWidth+1) * pixelSize * virtual_screens.size(), sn::NESVideoHeight * pixelSize );

        window.create( videoMode, windowName, sf::Style::Titlebar );
        window.setVerticalSyncEnabled(true);

        while( window.isOpen() && doRun )
        {
            sf::Event event;
            while( window.pollEvent( event ) )
            {
                /*  */
            }

            {
                // draw the shit
                {
                    std::unique_lock<std::mutex> vs_lock( virtual_screens_mutex );
                    for( auto& vs : virtual_screens )
                    {
                        window.draw( vs );
                    }
                    window.display();
                }

                processRemoveRequests();
                processAddRequests();
            }
        }
    }

    void
    PreviewWindow::processAddRequests()
    {
        std::unique_lock<std::mutex> vs_lock( virtual_screens_mutex, std::defer_lock );
        std::unique_lock<std::mutex> lock( screen_data_queue_in_mutex, std::defer_lock );
        std::lock( vs_lock, lock );

        for( auto& vs : virtual_screens )
        {
            if( vs.getScreenData() == blankScreenData )
            {
                vs.setScreenData( screen_data_queue_in.front() );
                screen_data_queue_in.pop_front();
            }
        }
    }

    void
    PreviewWindow::processRemoveRequests()
    {
        std::unique_lock<std::mutex> vs_lock( virtual_screens_mutex, std::defer_lock );
        std::unique_lock<std::mutex> lock( screen_data_queue_out_mutex, std::defer_lock );
        std::lock( vs_lock, lock );

        std::list<std::list<std::shared_ptr<std::vector<sf::Color>>>::iterator> was_removed;

        // old school list traversal. I fucking love the auto keyword so much
        for( std::list<std::shared_ptr<std::vector<sf::Color>>>::iterator toRemove = screen_data_queue_out.begin(); toRemove != screen_data_queue_out.end(); ++toRemove )
        {
            for( auto& vs : virtual_screens )
            {
                if( vs.getScreenData() == *toRemove )
                {
                    vs.setScreenData( blankScreenData );
                    was_removed.push_back( toRemove );
                }
            }
        }

        for( auto rem : was_removed )
        {
            screen_data_queue_out.erase( rem );
        }
    }

}
