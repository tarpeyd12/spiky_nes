#include "preview_window.hpp"

#include "../simple_nes/include/Emulator.h"

namespace spkn
{
    PreviewWindow::PreviewWindow( const std::string& window_name, size_t num_previews, float screen_size_ratio )
         :
        windowName( window_name ),
        pixelSize( screen_size_ratio ),
        doRun( true ),
        virtual_screens_mutex(),
        virtual_screens(),
        screen_data_queue_in(),
        screen_data_queue_out_mutex(),
        screen_data_queue_out(),
        blankScreenData( nullptr )
    {
        blankScreenData = std::make_shared<std::vector<sf::Color>>( sn::NESVideoWidth * sn::NESVideoHeight, sf::Color::Magenta );

        while( virtual_screens.size() < num_previews )
        {
            virtual_screens.push_back( sn::VirtualScreen() );
            virtual_screens.back().create( sn::NESVideoWidth, sn::NESVideoHeight, pixelSize, sf::Color::Cyan );
            virtual_screens.back().setScreenData( blankScreenData );
            virtual_screens.back().setScreenPosition( { (sn::NESVideoWidth + pixelGap) * pixelSize * ( virtual_screens.size() - 1 ), 0.0 } );
        }

        window_thread = std::thread( [&]{ run(); } );
    }

    PreviewWindow::~PreviewWindow()
    {
        clearAllScreenData();
        close();
        window_thread.join();
    }

    void
    PreviewWindow::addScreenData( std::shared_ptr<std::vector<sf::Color>> data )
    {
        screen_data_queue_in.push( data );
    }

    void
    PreviewWindow::removeScreenData( std::shared_ptr<std::vector<sf::Color>> data )
    {
        std::lock_guard<std::mutex> lock( screen_data_queue_out_mutex );

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
        std::unique_lock<std::mutex> out_lock( screen_data_queue_out_mutex, std::defer_lock );
        std::lock( vs_lock, out_lock );

        for( auto& vs : virtual_screens )
        {
            vs.setScreenData( blankScreenData );
        }

        screen_data_queue_in.clear();

        while( screen_data_queue_out.size() )
        {
            screen_data_queue_out.pop_front();
        }
    }

    void
    PreviewWindow::run()
    {
        sf::RenderWindow window;

        sf::VideoMode videoMode( ( (sn::NESVideoWidth+pixelGap) * virtual_screens.size()-pixelGap ) * pixelSize, sn::NESVideoHeight * pixelSize );
        window.create( videoMode, windowName, sf::Style::Titlebar );
        window.setVerticalSyncEnabled(true);

        sf::Event event;
        bool focus = true;
        while( window.isOpen() && doRun )
        {
            while( window.pollEvent( event ) )
            {
                if( event.type == sf::Event::GainedFocus )
                {
                    focus = true;
                }
                else if( event.type == sf::Event::LostFocus )
                {
                    focus = false;
                }
            }

            // draw the shit
            {
                window.clear();

                std::lock_guard<std::mutex> vs_lock( virtual_screens_mutex );
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

    void
    PreviewWindow::processAddRequests()
    {
        std::unique_lock<std::mutex> vs_lock( virtual_screens_mutex );

        {
            for( auto& vs : virtual_screens )
            {
                std::shared_ptr<std::vector<sf::Color>> target( nullptr );
                if( vs.getScreenData() == blankScreenData && screen_data_queue_in.try_pop( target ) && target != nullptr )
                {
                    vs.setScreenData( target );
                }
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
                    was_removed.emplace_back( toRemove );
                }
            }
        }

        for( auto rem : was_removed )
        {
            screen_data_queue_out.erase( rem );
        }
    }

}
