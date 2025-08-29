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
    using rendererType_t = std::underlying_type_t< bgfx::RendererType::Enum >;

    std::array< bgfx::RendererType::Enum, bgfx::RendererType::Enum::Count >
        l_supportedRenderers{};

    const rendererType_t l_supportedRenderersAmount =
        bgfx::getSupportedRenderers( bgfx::getSupportedRenderers(),
                                     l_supportedRenderers.data() );

    log::debug( "Supported renderers:" );

    for ( rendererType_t _index :
          std::views::iota( static_cast< rendererType_t >( 0 ),
                            l_supportedRenderersAmount ) ) {
        log::debug( " - {}", bgfx::getRendererName(
                                 l_supportedRenderers.at( _index ) ) );
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

        for ( ;; ) {
            vsync::begin();

            SDL_PumpEvents();

            runtime::event_t l_event{};

            const auto l_handleEvents = [ & ] {
                std::vector< runtime::event_t > l_events( 16 );

                // Poll events
                while ( SDL_PollEvent( &l_event ) ) {
                    l_events.emplace_back( l_event );
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
