#include <iomanip>
#include <sstream>
#include <cmath>

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
        screen_data_queue_out(),
        blankScreenData( nullptr )
    {
        blankScreenData = std::make_shared<sf::Image>();
        blankScreenData->create( sn::NESVideoWidth, sn::NESVideoHeight, sf::Color{ 127,127,127,255 } );

        while( virtual_screens.size() < num_previews )
        {
            virtual_screens.push_back( sn::VirtualScreen() );
            virtual_screens.back().create( sn::NESVideoWidth, sn::NESVideoHeight, pixelSize, sf::Color::Blue );
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
    PreviewWindow::addScreenData( std::shared_ptr<sf::Image> data )
    {
        screen_data_queue_in.push( data );
    }

    void
    PreviewWindow::removeScreenData( std::shared_ptr<sf::Image> data )
    {
        screen_data_queue_out.push( data );
    }

    void
    PreviewWindow::close()
    {
        doRun = false;
    }

    void
    PreviewWindow::clearAllScreenData()
    {
        std::unique_lock<std::mutex>  vs_lock( virtual_screens_mutex );

        for( auto& vs : virtual_screens )
        {
            vs.setScreenData( blankScreenData );
        }

        screen_data_queue_in.clear();
        screen_data_queue_out.clear();
    }

    void
    PreviewWindow::setNumVBlanks( uint64_t vblanks )
    {
        numKnownVBlanks = vblanks;
    }

    void
    PreviewWindow::setNumProcessed( uint64_t numProcessed )
    {
        numIndividualsProcessed = numProcessed;
    }

    void
    PreviewWindow::setNumGenerations( uint64_t numGenerations )
    {
        numGenerationsProcessed = numGenerations;
    }

    void
    PreviewWindow::run()
    {
        sf::RenderWindow window;

        sf::VideoMode videoMode( ( (sn::NESVideoWidth+pixelGap) * virtual_screens.size()-pixelGap ) * pixelSize, sn::NESVideoHeight * pixelSize );
        window.create( videoMode, windowName, sf::Style::Titlebar );
        window.setVerticalSyncEnabled( true );
        window.setFramerateLimit( 60 );

        uint64_t windowUpdates = 0;

        auto startTime = std::chrono::steady_clock::now();

        uint64_t lastKnownVBlank = 0;
        auto lastUpdatedCurrentTime = startTime;

        sf::Event event;
        //bool focus = true;
        while( window.isOpen() && doRun )
        {
            while( window.pollEvent( event ) )
            {
                if( event.type == sf::Event::GainedFocus )
                {
                    window.setFramerateLimit( 60 );
                    //focus = true;
                }
                else if( event.type == sf::Event::LostFocus )
                {
                    window.setFramerateLimit( 30 );
                    //focus = false;
                }
            }

            processAddRequests();
            processRemoveRequests();


            // draw the shit
            {
                std::lock_guard<std::mutex> vs_lock( virtual_screens_mutex );

                window.clear();

                for( auto& vs : virtual_screens )
                {
                    window.draw( vs );
                }
                window.display();
            }

            if( numKnownVBlanks > lastKnownVBlank )
            {
                lastKnownVBlank = numKnownVBlanks;
                lastUpdatedCurrentTime = std::chrono::steady_clock::now();
            }

            ++windowUpdates;

            if( windowUpdates >= 10 )
            {
                windowUpdates = 0;

                auto currentTime = std::chrono::steady_clock::now();

                long double runTime = std::chrono::duration<long double>(currentTime-startTime).count();
                long double lastVBlankTime = std::chrono::duration<long double>(lastUpdatedCurrentTime-startTime).count();

                std::stringstream ss;
                ss << "SpikeyNES (";
                ss << "Gen=" << numGenerationsProcessed << ", ";
                ss << "Nets=" << numIndividualsProcessed << ", ";
                ss << "VBlanks=" << numKnownVBlanks << "(";
                ss << std::fixed << std::setprecision(2) << (long double)(numKnownVBlanks)/60.0 << "s), ";
                ss << "runtime=" << std::fixed << std::setprecision(1) << runTime << "s(";
                if( runTime >= 3600.0 ) ss << uint64_t(runTime/3600) << "h";
                if( runTime >= 60.0 )   ss << std::setw(2) << std::setfill('0') << uint64_t(runTime/60) % 60 << "m";
                                        ss << std::setw(4) << std::setfill('0') << fmod( runTime, 60.0 ) << "s";
                ss << "), ";
                ss << "NESs/s=" << std::fixed << std::setprecision(2) << ((long double)(numKnownVBlanks)/60.0) / lastVBlankTime << "s:1s";
                ss << "(" << std::fixed << std::setprecision(2) << ((long double)(numKnownVBlanks)/60.0/(long double)(virtual_screens.size())) / lastVBlankTime << "s:1s)";
                ss << ")";

                window.setTitle( ss.str() );
            }
        }
    }

    void
    PreviewWindow::processAddRequests()
    {
        std::unique_lock<std::mutex> vs_lock( virtual_screens_mutex );

        {
            for( auto& vs : virtual_screens )
            {
                std::shared_ptr<sf::Image> target( nullptr );
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
        std::unique_lock<std::mutex> vs_lock( virtual_screens_mutex );

        std::list< std::shared_ptr<sf::Image> > toReAdd;

        std::shared_ptr<sf::Image> target( nullptr );
        while( screen_data_queue_out.try_pop( target ) && target != nullptr )
        {
            bool was_removed = false;
            for( auto& vs : virtual_screens )
            {
                if( vs.getScreenData() == target )
                {
                    vs.setScreenData( blankScreenData );
                    was_removed = true;
                    break;
                }
            }

            if( !was_removed )
            {
                toReAdd.emplace_back( target );
            }

            target = nullptr;
        }

        while( !toReAdd.empty() )
        {
            addScreenData( toReAdd.front() );
            toReAdd.pop_front();
        }
    }

}
