#include "FPS.hpp"
#include "log.hpp"
#include "runtime.hpp"

namespace runtime {

void quit( applicationState_t& _applicationState ) {
    // Report if SDL error occured before quitting
    {
        std::string_view l_errorMessage = SDL_GetError();

        if ( !l_errorMessage.empty() ) {
            log::error( "Application quit: '{}'", l_errorMessage );
        }
    }

    // FPS
    FPS::quit();

    // Vsync
    vsync::quit();

    // BGFX
    bgfx::shutdown();

    // Application state
    {
        if ( !_applicationState.unload() ) {
            log::error( "Unloading application state" );
        }

        // Report if SDL error occured during quitting
        {
            std::string_view l_errorMessage = SDL_GetError();

            if ( !l_errorMessage.empty() ) {
                log::error( "Application shutdown: '{}'", l_errorMessage );
            }
        }

        if ( _applicationState.window ) {
            SDL_DestroyWindow( _applicationState.window );
        }
    }

    SDL_Quit();

    log::debug( "Quitted" );
}

} // namespace runtime
