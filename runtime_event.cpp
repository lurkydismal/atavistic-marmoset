#include <ranges>

#include "log.hpp"
#include "runtime.hpp"

namespace {

auto onWindowResize( runtime::applicationState_t& _applicationState,
                     const float _width,
                     const float _height ) -> bool {
    bool l_returnValue = false;

    if ( !_width || !_height ) {
        goto EXIT;
    }

    {
        _applicationState.width = _width;
        _applicationState.height = _height;

        bgfx::reset( _width, _height );

        bgfx::setViewRect( 0, 0, 0, _width, _height );

        l_returnValue = true;
    }

EXIT:
    return ( l_returnValue );
}

auto handleKeyboardState( runtime::applicationState_t& _applicationState )
    -> bool {
    bool l_returnValue = false;

    {
        static size_t l_lastInputFrame = 0;
        const size_t l_totalFramesRendered =
            _applicationState.totalFramesRendered;

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
                            _applicationState.settings.controls.get(
                                l_scancode );

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

        l_returnValue = true;
    }

    return ( l_returnValue );
}

} // namespace

namespace runtime {

auto event( applicationState_t& _applicationState, const event_t& _event )
    -> bool {
    bool l_returnValue = false;

    {
        const bool l_isEventEmpty = ( _event.type == 0 );

        // Empty means last event on current frame
        if ( l_isEventEmpty ) {
            l_returnValue = handleKeyboardState( _applicationState );

            if ( !l_returnValue ) {
                log::error( "Handling keyboard state" );

                goto EXIT;
            }

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
    }

EXIT:
    return ( l_returnValue );
}

} // namespace runtime
