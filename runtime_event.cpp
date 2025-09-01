#include <ranges>
#include <type_traits>

#include "controls.hpp"
#include "log.hpp"
#include "runtime.hpp"

namespace {

auto onWindowResize( runtime::applicationState_t& _applicationState,
                     const float _width,
                     const float _height ) -> bool {
    bool l_returnValue = false;

    do {
        if ( !_width || !_height ) {
            break;
        }

        _applicationState.width = _width;
        _applicationState.height = _height;

        bgfx::reset( _width, _height );

        bgfx::setViewRect( 0, 0, 0, _width, _height );

        l_returnValue = true;
    } while ( false );

    return ( l_returnValue );
}

void handleKeyboardState( runtime::applicationState_t& _applicationState ) {
    static size_t l_lastInputFrame = 0;
    const size_t l_totalFramesRendered = _applicationState.totalFramesRendered;

    if ( l_lastInputFrame < l_totalFramesRendered ) {
        controls::input_t l_input;

        {
            int l_keysAmount = 0;
            const bool* l_keysState = SDL_GetKeyboardState( &l_keysAmount );

            __builtin_assume( l_keysAmount == SDL_SCANCODE_COUNT );

            for ( auto [ _index, _isPressed ] :
                  std::span( l_keysState, l_keysAmount ) |
                      std::views::enumerate ) {
                if ( _isPressed ) {
                    auto l_scancode = static_cast< SDL_Scancode >( _index );

                    const controls::control_t& l_control =
                        _applicationState.settings.controls.get( l_scancode );

                    if ( l_control.scancode != SDL_SCANCODE_UNKNOWN ) {
                        l_input.direction |= l_control.input.direction;
                        l_input.button |= l_control.input.button;
                    }
                }
            }
        }

        _applicationState.currentInput = l_input;
    }

    l_lastInputFrame = l_totalFramesRendered;
}

} // namespace

namespace runtime {

// FIX: Remove goto
auto event( applicationState_t& _applicationState, const event_t& _event )
    -> bool {
    bool l_returnValue = false;

    do {
        const event_t l_emptyEVent{};

        const bool l_isEventEmpty =
            ( __builtin_memcmp( &_event, &l_emptyEVent, sizeof( _event ) ) ==
              0 );

        // Empty means last event on current frame
        if ( l_isEventEmpty ) {
            handleKeyboardState( _applicationState );

        } else {
            switch ( _event.type ) {
                case SDL_EVENT_QUIT: {
                    _applicationState.status = true;

                    l_returnValue = false;

                    goto EXIT;
                }

                case SDL_EVENT_WINDOW_RESIZED: {
                    const float l_newWidth = _event.window.data1;
                    const float l_newHeight = _event.window.data2;

                    l_returnValue = onWindowResize( _applicationState,
                                                    l_newWidth, l_newHeight );

                    if ( !l_returnValue ) {
                        log::error( "Handling window resize" );

                        goto EXIT;
                    }

                    break;
                }

                default: {
                }
            }
        }

        l_returnValue = true;
    EXIT:
    } while ( false );

    return ( l_returnValue );
}

} // namespace runtime
