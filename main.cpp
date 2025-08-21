#include <SDL3/SDL.h>

#include <chrono>
#include <iostream>
#include <stop_token>
#include <string_view>
#include <thread>

namespace {

// Function-like macros
// Universal
#define STRINGIFY( _value ) #_value
#define MACRO_TO_STRING( _macro ) STRINGIFY( _macro )

// Constants
// Universal
static inline constinit const size_t g_oneSecondInMilliseconds = 1000;

// Window
static inline constinit const std::string_view g_defaultWindowName =
    "atavistic-marmoset";

// Control
static inline constinit const std::string_view g_controlAsStringUnknown =
    "UNKNOWN";

// Settings
static inline constinit const std::string_view g_defaultSettingsVersion = "0.1";

template < typename T >
static inline constexpr auto millisecondsToNanoseconds( const T _milliseconds )
    -> size_t {
    return ( _milliseconds * g_oneSecondInMilliseconds );
}

namespace log {

namespace {

// Prefixes
inline constinit const std::string_view g_logInfoPrefix = "INFO: ";
inline constinit const std::string_view g_logWarningPrefix = "WARNING: ";
inline constinit const std::string_view g_logErrorPrefix = "ERROR: ";

} // namespace

void info( const std::string_view& _message ) {
    std::cout << g_logInfoPrefix << _message << "\n";
}

void warningg( const std::string_view& _message ) {
    std::cerr << g_logWarningPrefix << _message << "\n";
}

// Function file:line | message
inline void _error( const std::string_view& _message,
                    const char* _functionName,
                    const char* _fileName,
                    const char* _lineNumber ) {
    std::cerr << g_logErrorPrefix << "\"" << _functionName << "\"" << " "
              << _fileName << ":" << _lineNumber << " | " << _message << "\n";
}

#define error( _message )                                     \
    _error( ( _message ), __PRETTY_FUNCTION__, __FILE_NAME__, \
            MACRO_TO_STRING( __LINE__ ) )

} // namespace log

enum class vsync_t : uint8_t {
    off = 0,
    unknownVsync,
};

namespace vsync {

namespace {

vsync_t g_vsyncType;
float g_desiredFPS = 0;

} // namespace

static struct timespec g_sleepTime, g_startTime, g_endTime;

auto init( const vsync_t _vsyncType,
           const float _desiredFPS,
           SDL_Renderer* _renderer ) -> bool {
    bool l_returnValue = false;

    if ( !_renderer ) {
        log::error( "Invalid argument" );

        goto EXIT;
    }

    if ( g_desiredFPS ) {
        log::error( "Alraedy initialized" );

        goto EXIT;
    }

    {
        g_desiredFPS = _desiredFPS;
        g_vsyncType = _vsyncType;

        if ( _vsyncType == vsync_t::off ) {
            g_sleepTime.tv_sec = 0;
            g_sleepTime.tv_nsec = millisecondsToNanoseconds(
                g_oneSecondInMilliseconds / ( float )_desiredFPS );

            l_returnValue = SDL_SetRenderVSync(
                _renderer, SDL_WINDOW_SURFACE_VSYNC_DISABLED );

            if ( !l_returnValue ) {
                log::error( std ::string( "Setting renderer vsync: '%s'" ) +
                            SDL_GetError() );

                goto EXIT;
            }
        }

        l_returnValue = true;
    }

EXIT:
    return ( l_returnValue );
}

void quit() {
    g_desiredFPS = 0;

    if ( g_vsyncType == vsync_t::off ) {
        g_sleepTime.tv_nsec = 0;
    }
}

void begin() {
    if ( g_vsyncType == vsync_t::off ) {
        clock_gettime( CLOCK_MONOTONIC, &g_startTime );
    }
}

void end() {
    if ( g_vsyncType == vsync_t::off ) {
        clock_gettime( CLOCK_MONOTONIC, &g_endTime );

        struct timespec l_adjustedSleepTime{};

        {
            const size_t l_iterationTimeNano =
                ( g_endTime.tv_nsec - g_startTime.tv_nsec );

            long long l_adjustedSleepNano =
                ( g_sleepTime.tv_nsec - l_iterationTimeNano );

            l_adjustedSleepNano &= -( l_adjustedSleepNano > 0 );

            l_adjustedSleepTime.tv_sec = 0;
            l_adjustedSleepTime.tv_nsec = l_adjustedSleepNano;
        }

        clock_nanosleep( CLOCK_MONOTONIC, 0, &l_adjustedSleepTime, nullptr );
    }
}

} // namespace vsync

using window_t = struct window {
    window() = default;
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

enum class direction_t : uint8_t {
    none = 0,
    up = 8,
    right = 6,
    down = 2,
    left = 4,
};

enum class button_t : uint8_t {
    none = 0,
};

using input_t = struct input {
    input() = default;
    input( const input& ) = default;
    input( input&& ) = default;
    ~input() = default;
    auto operator=( const input& ) -> input& = default;
    auto operator=( input&& ) -> input& = default;

    direction_t direction = direction_t::none;
    button_t button = button_t::none;
    size_t duration = 0;
};

using control_t = struct control {
    control() = default;
    control( const control& ) = default;
    control( control&& ) = default;
    ~control() = default;
    auto operator=( const control& ) -> control& = default;
    auto operator=( control&& ) -> control& = default;

    SDL_Scancode scancode = SDL_SCANCODE_UNKNOWN;
    input_t input;
};

// All available controls
using controls_t = struct controls {
    controls() = default;
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
    settings() = default;
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
};

// TODO: Implement
using camera_t = struct camera {
    camera() = default;
    camera( const camera& ) = default;
    camera( camera&& ) = default;
    ~camera() = default;
    auto operator=( const camera& ) -> camera& = default;
    auto operator=( camera&& ) -> camera& = default;
};

using applicationState_t = struct applicationState {
    applicationState() = default;
    applicationState( const applicationState& ) = delete;
    applicationState( applicationState&& ) = delete;
    ~applicationState() = default;
    auto operator=( const applicationState& ) -> applicationState& = delete;
    auto operator=( applicationState&& ) -> applicationState& = delete;

    // TODO: Implement
    auto load() -> bool { return ( true ); }

    // TODO: Implement
    auto unload() -> bool { return ( true ); }

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    settings_t settings;
    camera_t camera;
    size_t logicalWidth = 1280;
    size_t logicalHeight = 720;
    std::atomic< size_t > totalFramesRendered = 0;
    bool isPaused = false;
    bool status = false;
};

namespace FPS {

namespace {

std::jthread g_loggerThread;

void logger( std::stop_token _stoken, std::atomic< size_t >& _frameCount ) {
    using clock = std::chrono::steady_clock;
    using namespace std::chrono_literals;

    auto l_lastTime = clock::now();

    while ( !_stoken.stop_requested() ) {
        std::this_thread::sleep_for( 1s );

        auto l_now = clock::now();
        auto l_count = _frameCount.exchange( 0 );

        auto l_duration =
            std::chrono::duration_cast< std::chrono::milliseconds >(
                l_now - l_lastTime )
                .count();
        double l_fps = l_count * 1000.0 / l_duration;

        log::info( std::format( "FPS: {}", l_fps ) );

        l_lastTime = l_now;
    }

    log::info( "FPS logger stopped." );
}

} // namespace

void init( std::atomic< size_t >& _frameCount ) {
    g_loggerThread = std::jthread( logger, std::ref( _frameCount ) );
}

void quit() {
    g_loggerThread.request_stop();

    if ( g_loggerThread.joinable() ) {
        g_loggerThread.join();
    }
}

} // namespace FPS

auto init( applicationState_t& _applicationState ) -> bool {
    bool l_returnValue = false;

    {
        // Generate application state
        {
            // Metadata
            {
                log::info( std::format(
                    "Window name: '{}', Version: '{}', Identifier: '{}'",
                    _applicationState.settings.window.name,
                    _applicationState.settings.version,
                    _applicationState.settings.identifier ) );

                if ( !SDL_SetAppMetadata(
                         std::string( _applicationState.settings.window.name )
                             .c_str(),
                         std::string( _applicationState.settings.version )
                             .c_str(),
                         std::string( _applicationState.settings.identifier )
                             .c_str() ) ) {
                    log::error( std::string( "Setting render scale: '%s'" ) +
                                SDL_GetError() );

                    goto EXIT;
                }
            }

            // Setup recources to load
            {
                // TODO: Implement
            }

            // Init SDL sub-systems
            SDL_Init( SDL_INIT_VIDEO );

            // Window and Renderer
            {
                if ( !SDL_CreateWindowAndRenderer(
                         std::string( _applicationState.settings.window.name )
                             .c_str(),
                         _applicationState.settings.window.width,
                         _applicationState.settings.window.height,
                         ( SDL_WINDOW_INPUT_FOCUS ),
                         &( _applicationState.window ),
                         &( _applicationState.renderer ) ) ) {
                    log::error(
                        std::string( "Window or Renderer creation: '%s'" ) +
                        SDL_GetError() );

                    goto EXIT;
                }
            }

            // Default scale mode
            if ( !SDL_SetDefaultTextureScaleMode( _applicationState.renderer,
                                                  SDL_SCALEMODE_NEAREST ) ) {
                log::error(
                    std::string( "Setting render nearest scale mode: '%s'" ) +
                    SDL_GetError() );

                goto EXIT;
            }

            // TODO: Set SDL3 logical resolution
            // TODO: Probably reduntant
            // Scaling
            {
                const float l_scaleX =
                    ( ( float )( _applicationState.settings.window.width ) /
                      ( float )( _applicationState.logicalWidth ) );
                const float l_scaleY =
                    ( ( float )( _applicationState.settings.window.height ) /
                      ( float )( _applicationState.logicalHeight ) );

                if ( !SDL_SetRenderScale( _applicationState.renderer, l_scaleX,
                                          l_scaleY ) ) {
                    log::error( std::string( "Setting render scale: '%s'" ) +
                                SDL_GetError() );

                    goto EXIT;
                }
            }

            // TODO: Set new SDL3 things

            // Load resources
            if ( !_applicationState.load() ) {
                log::error( "Loading application state" );

                goto EXIT;
            }
        }

        // Vsync
        if ( !vsync::init( _applicationState.settings.window.vsync,
                           _applicationState.settings.window.desiredFPS,
                           _applicationState.renderer ) ) {
            log::error( "Initializing Vsync" );

            goto EXIT;
        }

        // FPS
        // Does not fail
        FPS::init( _applicationState.totalFramesRendered );

        l_returnValue = true;
    }

EXIT:
    return ( l_returnValue );
}

void quit( applicationState_t& _applicationState ) {
    // Report if SDL error occured before quitting
    {
        const std::string_view& l_errorMessage = SDL_GetError();

        if ( !( l_errorMessage.empty() ) ) {
            log::error(
                std::format( "Application quit: '{}'", l_errorMessage ) );
        }
    }

    // FPS
    FPS::quit();

    // Vsync
    vsync::quit();

    // Application state
    {
        if ( !_applicationState.unload() ) {
            log::error( "Unloading application state" );
        }

        // Report if SDL error occured during quitting
        {
            const std::string_view& l_errorMessage = SDL_GetError();

            if ( !( l_errorMessage.empty() ) ) {
                log::error( std::format( "Application shutdown: '{}'",
                                         l_errorMessage ) );
            }
        }

        if ( _applicationState.renderer ) {
            SDL_DestroyRenderer( _applicationState.renderer );
        }

        if ( _applicationState.window ) {
            SDL_DestroyWindow( _applicationState.window );
        }
    }

    SDL_Quit();
}

} // namespace

auto main() -> int {
    applicationState_t l_applicationState;

    init( l_applicationState );

    std::this_thread::sleep_for( std::chrono::seconds( 5 ) );

    quit( l_applicationState );

    return ( 0 );
}
