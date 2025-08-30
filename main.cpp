#include <SDL3/SDL.h>
#include <bgfx/bgfx.h>

#include <algorithm>
#include <experimental/scope>
#include <ranges>
#include <vector>

#include "log.hpp"
#include "runtime.hpp"
#include "vsync.hpp"

static void printSupportedRenderers() {
    using rendererType_t = bgfx::RendererType::Enum;

    std::array< rendererType_t, rendererType_t::Count > l_supportedRenderers{};

    bgfx::getSupportedRenderers( bgfx::getSupportedRenderers(),
                                 l_supportedRenderers.data() );

    log::debug( "Supported renderers:" );

    for ( const std::string_view _rendererName :
          l_supportedRenderers |
              std::views::filter( [ & ]( rendererType_t _rendererType ) {
                  return ( _rendererType != rendererType_t::Noop );
              } ) |
              std::views::transform( bgfx::getRendererName ) ) {
        log::debug( " - {}", _rendererName );
    }
}

auto main() -> int {
    runtime::applicationState_t l_applicationState;

    printSupportedRenderers();

    do {
        if ( !runtime::init( l_applicationState ) ) {
            break;
        }

        auto l_onExit = std::experimental::scope_exit(
            [ & ] { runtime::quit( l_applicationState ); } );

        std::vector< runtime::event_t > l_events( 16 );

        // Main loop
        for ( ;; ) {
            vsync::begin();

            const auto l_handleEvents = [ & ] {
                // Poll events
                {
                    SDL_PumpEvents();

                    l_events.clear();

                    runtime::event_t l_event{};

                    while ( SDL_PollEvent( &l_event ) ) {
                        l_events.emplace_back( l_event );
                    }
                }

                return ( std::ranges::all_of(
                             l_events,
                             [ & ]( const runtime::event_t& _event ) {
                                 return ( runtime::event( l_applicationState,
                                                          _event ) );
                             } ) &&
                         (
                             // Empty means last event on current frame
                             runtime::event( l_applicationState, {} ) ) );
            };

            if ( !l_handleEvents() ) {
                break;
            }

            if ( !runtime::iterate( l_applicationState ) ) {
                break;
            }

            vsync::end();

            ( l_applicationState.totalFramesRendered )++;
        }
    } while ( false );

    return ( ( l_applicationState.status ) ? ( EXIT_SUCCESS )
                                           : ( EXIT_FAILURE ) );
}
