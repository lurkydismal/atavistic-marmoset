#include <SDL3/SDL.h>

#include <iostream>

// Log formats
#define LOG_INFO_FORMAT ""
#define LOG_WARNING_FORMAT ""

// Function file:line | message
#define LOG_ERROR_FORMAT                                               \
    "\"" << __PRETTY_FUNCTION__ << "\"" << " " << __FILE_NAME__ << ":" \
         << __LINE__ << " | "

#define logError( _message ) \
    std::cerr << LOG_ERROR_FORMAT << ( _message ) << "\n"

void log( const std::string_view& _message ) {
    std::cout << LOG_INFO_FORMAT << _message << "\n";
}

// Constants
// Log
static inline constinit const std::string_view g_logInfoPrefix = "INFO: ";
static inline constinit const std::string_view g_logWarningPrefix = "WARNING: ";
static inline constinit const std::string_view g_logErrorPrefix = "ERROR: ";

// Vsync
static inline constinit const std::string_view g_vsyncTypeAsStringOff = "OFF";
static inline constinit const std::string_view g_vsyncTypeAsStringUnknown =
    "UNKNOWN";

// Window
static inline constinit const std::string_view g_defaultWindowName =
    "atavistic-marmoset";

// Control
static inline constinit const std::string_view g_controlAsStringUnknown =
    "UNKNOWN";

// Settings
static inline constinit const std::string_view g_defaultSettingsVersion = "0.1";
static inline constinit const std::string_view g_defaultSettingsDescription =
    "brrr atavistic marmoset";
static inline constinit const std::string_view g_defaultSettingsContactAddress =
    "<lurkydismal@duck.com>";

using vsync_t = enum class vsync : uint8_t {
    off = 0,
    unknownVsync,
};

using window_t = struct window {
    window( const window& ) = default;
    window( window&& ) = default;
    ~window() = default;
    auto operator=( const window& ) -> window& = default;
    auto operator=( window&& ) -> window& = default;

    static inline constexpr const std::string_view& name = g_defaultWindowName;
    size_t width = 640;
    size_t height = 480;
    size_t desiredFPS = 60;
    vsync_t vsync = vsync_t::off;
};

// TODO: Implement input_t
using control_t = struct control {
    control( const control& ) = default;
    control( control&& ) = default;
    ~control() = default;
    auto operator=( const control& ) -> control& = default;
    auto operator=( control&& ) -> control& = default;

    SDL_Scancode scancode = SDL_SCANCODE_UNKNOWN;
    // input_t input;
};

// All available controls
using controls_t = struct controls {
    controls( const controls& ) = default;
    controls( controls&& ) = default;
    ~controls() = default;
    auto operator=( const controls& ) -> controls& = default;
    auto operator=( controls&& ) -> controls& = default;

    // Directions
    control_t up;
    control_t down;
    control_t left;
    control_t right;
};

// All available customization
using settings_t = struct settings {
    settings( const settings& ) = default;
    settings( settings&& ) = default;
    ~settings() = default;
    auto operator=( const settings& ) -> settings& = default;
    auto operator=( settings&& ) -> settings& = default;

    window_t window;
    controls_t controls;
    static inline constexpr const std::string_view& version =
        g_defaultSettingsVersion;
    static inline constexpr const std::string_view& identifier = window_t::name;
    static inline constexpr const std::string_view& description =
        g_defaultSettingsDescription;
    static inline constexpr const std::string_view& contactAddress =
        g_defaultSettingsContactAddress;
};

// TODO: Implement
using camera_t = struct camera {
    camera( const camera& ) = default;
    camera( camera&& ) = default;
    ~camera() = default;
    auto operator=( const camera& ) -> camera& = default;
    auto operator=( camera&& ) -> camera& = default;
};

using applicationState_t = struct applicationState {
    applicationState( const applicationState& ) = default;
    applicationState( applicationState&& ) = default;
    ~applicationState() = default;
    auto operator=( const applicationState& ) -> applicationState& = default;
    auto operator=( applicationState&& ) -> applicationState& = default;

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    settings_t settings;
    camera_t camera;
    size_t logicalWidth = 1280;
    size_t logicalHeight = 720;
    size_t totalFramesRendered = 0;
    bool isPaused = false;
    bool status = false;
};

auto init( applicationState_t* _applicationState ) -> bool {
    bool l_returnValue = false;

    if ( !_applicationState ) {
        logError( "Invalid argument" );

        goto EXIT;
    }

    {
        // Generate application state
        {
            // Metadata
            {
                log( std::format(
                    "Window name: '{}', Version: '{}', Identifier: '{}'",
                    _applicationState->settings.window.name,
                    _applicationState->settings.version,
                    _applicationState->settings.identifier ) );

                if ( !SDL_SetAppMetadata(
                         std::string( _applicationState->settings.window.name )
                             .c_str(),
                         std::string( _applicationState->settings.version )
                             .c_str(),
                         std::string( _applicationState->settings.identifier )
                             .c_str() ) ) {
                    logError( std::string( "Setting render scale: '%s'" )
                                  .append( SDL_GetError() ) );

                    goto EXIT;
                }
            }

            // Setup recources to load
            {
                // TODO: Implement
            }

            // Init SDL sub-systems
            {
                SDL_Init( SDL_INIT_VIDEO );
            }

            // Window and Renderer
            {
                if ( !SDL_CreateWindowAndRenderer(
                         std::string( _applicationState->settings.window.name )
                             .c_str(),
                         _applicationState->settings.window.width,
                         _applicationState->settings.window.height,
                         ( SDL_WINDOW_INPUT_FOCUS ),
                         &( _applicationState->window ),
                         &( _applicationState->renderer ) ) ) {
                    logError( std::string( "Window or Renderer creation: '%s'" )
                                  .append( SDL_GetError() ) );

                    goto EXIT;
                }
            }

            // Default scale mode
            {
                if ( !SDL_SetDefaultTextureScaleMode(
                         _applicationState->renderer,
                         SDL_SCALEMODE_NEAREST ) ) {
                    logError(
                        std::string( "Setting render nearest scale mode: '%s'" )
                            .append( SDL_GetError() ) );

                    goto EXIT;
                }
            }

            // TODO: Set SDL3 logical resolution
            // TODO: Set new SDL3 things

            // TODO: Probably reduntant
            // Scaling
            {
                const float l_scaleX =
                    ( ( float )( _applicationState->settings.window.width ) /
                      ( float )( _applicationState->logicalWidth ) );
                const float l_scaleY =
                    ( ( float )( _applicationState->settings.window.height ) /
                      ( float )( _applicationState->logicalHeight ) );

                if ( !SDL_SetRenderScale( _applicationState->renderer, l_scaleX,
                                          l_scaleY ) ) {
                    logError( std::string( "Setting render scale: '%s'" )
                                  .append( SDL_GetError() ) );

                    goto EXIT;
                }
            }

            // Load resources
            {
                if ( !applicationState_t$load( _applicationState ) ) {
                    logError( "Loading application state" );

                    goto EXIT;
                }
            }
        }

        // Vsync
        {
            if ( !vsync$init( _applicationState->settings.window.vsync,
                              _applicationState->settings.window.desiredFPS,
                              _applicationState->renderer ) ) {
                logError( "Initializing Vsync" );

                goto EXIT;
            }
        }

        // FPS
        {
            if ( !FPS$init( &( _applicationState->totalFramesRendered ) ) ) {
                logError( "Initializing FPS" );

                goto EXIT;
            }
        }

        l_returnValue = true;
    }

EXIT:
    return ( l_returnValue );
}

auto main() -> int {
    return ( 0 );
}
