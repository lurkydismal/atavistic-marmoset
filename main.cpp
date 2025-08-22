#include <SDL3/SDL.h>

#include <bitset>
#include <iostream>
#include <ranges>
#include <thread>

namespace {

// Function-like macros
// Universal
#define STRINGIFY( _value ) #_value
#define MACRO_TO_STRING( _macro ) STRINGIFY( _macro )

// Constants
// Universal
static inline constinit const size_t g_oneSecondInMilliseconds = 1000;
static inline constinit const size_t g_oneMillisecondInNanoseconds = 1000000;

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
    return ( _milliseconds * g_oneMillisecondInNanoseconds );
}

namespace log {

namespace {

// Prefixes
#if defined( DEBUG )

inline constinit const std::string_view g_logDebugPrefix = "DEBUG: ";

#endif

inline constinit const std::string_view g_logInfoPrefix = "INFO: ";
inline constinit const std::string_view g_logWarningPrefix = "WARNING: ";
inline constinit const std::string_view g_logErrorPrefix = "ERROR: ";

} // namespace

void debug( const std::string_view& _message ) {
#if defined( DEBUG )

    std::cout << g_logDebugPrefix << _message << "\n";

#endif
}

void info( const std::string_view& _message ) {
    std::cout << g_logInfoPrefix << _message << "\n";
}

void warning( const std::string_view& _message ) {
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

// Software vsync implementation
namespace vsync {

namespace {

vsync_t g_vsyncType = vsync_t::unknownVsync;
float g_desiredFPS = 0;

} // namespace

static struct timespec g_sleepTime, g_startTime, g_endTime;

auto init( const vsync_t _vsyncType,
           const float _desiredFPS,
           SDL_Renderer* _renderer ) -> bool {
    bool l_returnValue = false;

    if ( g_desiredFPS ) {
        log::error( "Alraedy initialized" );

        goto EXIT;
    }

    if ( !_renderer ) {
        log::error( "Invalid argument" );

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
                log::error( std ::format( "Setting renderer vsync: '{}'",
                                          SDL_GetError() ) );

                goto EXIT;
            }
        }

        log::info(
            std ::format( "Setting renderer vsync to {} FPS", _desiredFPS ) );

        log::debug( std ::format( "Vsync sleep time set to {} nanoseconds",
                                  g_sleepTime.tv_nsec ) );

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
    up = 0b1,
    down = 0b10,
    left = 0b100,
    right = 0b1000,
};

inline auto operator|=( direction_t& _lhs, direction_t _rhs ) -> direction_t& {
    _lhs = static_cast< direction_t >(
        static_cast< std::underlying_type_t< direction_t > >( _lhs ) |
        static_cast< std::underlying_type_t< direction_t > >( _rhs ) );

    return ( _lhs );
}

inline auto operator|( direction_t _lhs, direction_t _rhs ) -> direction_t {
    return ( static_cast< direction_t >(
        static_cast< std::underlying_type_t< direction_t > >( _lhs ) |
        static_cast< std::underlying_type_t< direction_t > >( _rhs ) ) );
}

inline auto operator&( direction_t _lhs, direction_t _rhs ) -> direction_t {
    return ( static_cast< direction_t >(
        static_cast< std::underlying_type_t< direction_t > >( _lhs ) &
        static_cast< std::underlying_type_t< direction_t > >( _rhs ) ) );
}

enum class button_t : uint8_t {
    none = 0,
};

inline auto operator|=( button_t& _lhs, button_t _rhs ) -> button_t& {
    _lhs = static_cast< button_t >(
        static_cast< std::underlying_type_t< button_t > >( _lhs ) |
        static_cast< std::underlying_type_t< button_t > >( _rhs ) );

    return ( _lhs );
}

inline auto operator|( button_t _lhs, button_t _rhs ) -> button_t {
    return ( static_cast< button_t >(
        static_cast< std::underlying_type_t< button_t > >( _lhs ) |
        static_cast< std::underlying_type_t< button_t > >( _rhs ) ) );
}

inline auto operator&( button_t _lhs, button_t _rhs ) -> button_t {
    return ( static_cast< button_t >(
        static_cast< std::underlying_type_t< button_t > >( _lhs ) &
        static_cast< std::underlying_type_t< button_t > >( _rhs ) ) );
}

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

    inline constexpr auto check( const SDL_Scancode _scancode ) -> bool {
        return ( scancode == _scancode );
    }

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

    inline constexpr auto get( const SDL_Scancode _scancode )
        -> const control_t {
        if ( up.check( _scancode ) ) {
            return ( up );

        } else if ( down.check( _scancode ) ) {
            return ( down );

        } else if ( left.check( _scancode ) ) {
            return ( left );

        } else if ( right.check( _scancode ) ) {
            return ( right );
        }

        return {};
    }

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

namespace FPS {

namespace {

std::jthread g_loggerThread;

void logger( const std::stop_token& _stopToken,
             std::atomic< size_t >& _frameCount ) {
    using clock = std::chrono::steady_clock;
    using namespace std::chrono_literals;

    auto l_timeLast = clock::now();

    while ( !_stopToken.stop_requested() ) {
        std::this_thread::sleep_for( 1s );

        const auto l_timeNow = clock::now();

        {
            const auto l_framesCount = _frameCount.exchange( 0 );

            const auto l_frameDuration = ( l_timeNow - l_timeLast );
            const std::chrono::duration< double > l_frameDurationInSeconds =
                l_frameDuration;

            double l_FPS = 0;

            if ( l_frameDurationInSeconds > 0s ) {
                l_FPS = ( l_framesCount / l_frameDurationInSeconds.count() );
            }

            log::info( std::format( "FPS: {:.2f}", l_FPS ) );
        }

        l_timeLast = l_timeNow;
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

namespace runtime {

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
    input_t currentInput;
    size_t logicalWidth = 1280;
    size_t logicalHeight = 720;
    std::atomic< size_t > totalFramesRendered = 0;
    bool isPaused = false;
    bool status = false;
};

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
                    log::error( std::format( "Setting render scale: '{}'",
                                             SDL_GetError() ) );

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
                    log::error( std::format(
                        "Window or Renderer creation: '{}'", SDL_GetError() ) );

                    goto EXIT;
                }
            }

            // Default scale mode
            if ( !SDL_SetDefaultTextureScaleMode( _applicationState.renderer,
                                                  SDL_SCALEMODE_LINEAR ) ) {
                log::error(
                    std::format( "Setting render nearest scale mode: '{}'",
                                 SDL_GetError() ) );

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
                    log::error( std::format( "Setting render scale: '{}'",
                                             SDL_GetError() ) );

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

        if ( !l_errorMessage.empty() ) {
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

            if ( !l_errorMessage.empty() ) {
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

using event_t = SDL_Event;

namespace {

auto onWindowResize( applicationState_t& _applicationState,
                     const float _width,
                     const float _height ) -> bool {
    bool l_returnValue = false;

    {
        static size_t l_lastResizeFrame = 0;
        const size_t l_totalFramesRendered =
            _applicationState.totalFramesRendered;

        if ( l_lastResizeFrame < l_totalFramesRendered ) {
            const float l_logicalWidth = _applicationState.logicalWidth;
            const float l_logicalHeigth = _applicationState.logicalHeight;

            const float l_scaleX = ( _width / l_logicalWidth );
            const float l_scaleY = ( _height / l_logicalHeigth );

            if ( !SDL_SetRenderScale( _applicationState.renderer, l_scaleX,
                                      l_scaleY ) ) {
                l_returnValue = false;

                log::error( std::format( "Setting render scale: '{}'",
                                         SDL_GetError() ) );

                goto EXIT;
            }
        }

        l_lastResizeFrame = l_totalFramesRendered;

        l_returnValue = true;
    }

EXIT:
    return ( l_returnValue );
}

auto handleKeyboardState( applicationState_t& _applicationState ) -> bool {
    bool l_returnValue = false;

    {
        static size_t l_lastInputFrame = 0;
        const size_t l_totalFramesRendered =
            _applicationState.totalFramesRendered;

        if ( l_lastInputFrame < l_totalFramesRendered ) {
            input_t l_input;

            {
                std::bitset< SDL_SCANCODE_COUNT > l_keysState{};

                constexpr const size_t l_SDLScancodeFirst =
                    ( SDL_SCANCODE_UNKNOWN + 1 );

                // Build keys state
                {
                    std::bitset< SDL_SCANCODE_COUNT >& l_temp = l_keysState;

                    int l_keysAmount = 0;
                    const bool* l_keysState =
                        SDL_GetKeyboardState( &l_keysAmount );

                    __builtin_assume( l_keysAmount == SDL_SCANCODE_COUNT );

                    for ( size_t _index : std::views::iota(
                              l_SDLScancodeFirst, ( size_t )l_keysAmount ) ) {
                        l_temp.set( l_keysState[ _index ] );
                    }
                }

                if ( l_keysState.any() ) {
                    for ( size_t _index : std::views::iota(
                              l_SDLScancodeFirst, l_keysState.size() ) ) {
                        // If pressed
                        if ( l_keysState.test( _index ) ) {
                            auto l_scancode = ( SDL_Scancode )_index;

                            const control_t& l_control =
                                _applicationState.settings.controls.get(
                                    l_scancode );

                            if ( l_control.scancode != SDL_SCANCODE_UNKNOWN ) {
                                l_input.direction |= l_control.input.direction;
                                l_input.button |= l_control.input.button;
                            }
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

auto event( applicationState_t& _applicationState, const event_t& _event )
    -> bool {
    bool l_returnValue = false;

    {
        const bool l_isEventEmpty = ( _event.type == 0 );

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

// TODO: Implement
auto iterate( applicationState_t& _applicationState ) -> bool {
    bool l_returnValue = false;

    {
        if ( !_applicationState.isPaused ) {
            // TODO: Camera
        }

        // Render
        {
            SDL_RenderClear( _applicationState.renderer );

            // TODO: Background
            // TODO: Scene

            SDL_RenderPresent( _applicationState.renderer );
        }

        if ( !_applicationState.isPaused ) {
            // TODO: Logic
            // TODO: Handle application current input
        }

        l_returnValue = true;
    }

    return ( l_returnValue );
}

} // namespace runtime

} // namespace

auto main() -> int {
    runtime::applicationState_t l_applicationState;

    {
        if ( !runtime::init( l_applicationState ) ) {
            goto EXIT;
        }

        for ( ;; ) {
            vsync::begin();

            SDL_PumpEvents();

            runtime::event_t l_event{};

            while ( SDL_PollEvent( &l_event ) ) {
                if ( !runtime::event( l_applicationState, l_event ) ) {
                    goto EXIT;
                }
            }

            // NULL means last event on current frame
            if ( !runtime::event( l_applicationState, {} ) ) {
                break;
            }

            if ( !runtime::iterate( l_applicationState ) ) {
                break;
            }

            vsync::end();

            ( l_applicationState.totalFramesRendered )++;
        }
    }

EXIT:
    runtime::quit( l_applicationState );

    return ( ( l_applicationState.status ) ? ( EXIT_SUCCESS )
                                           : ( EXIT_FAILURE ) );
}
