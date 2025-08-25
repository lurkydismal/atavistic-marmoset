#pragma once

#include <iostream>
#include <sstream>
#include <string_view>

#include "common.hpp"

namespace {

// Prefixes
#if defined( DEBUG )

inline constexpr const std::string_view g_logDebugPrefix = "DEBUG: ";

#endif

inline constexpr const std::string_view g_logInfoPrefix = "INFO: ";
inline constexpr const std::string_view g_logWarningPrefix = "WARNING: ";
inline constexpr const std::string_view g_logErrorPrefix = "ERROR: ";

} // namespace

namespace log {

inline void debug( const std::string_view _message ) {
#if defined( DEBUG )

    std::cout << g_logDebugPrefix << _message << "\n";

#endif
}

template < typename T >
inline void _variable( const std::string_view _message, const T& _variable ) {
    std::ostringstream l_stream;

    l_stream << _message << " = '" << _variable << "'";

    const std::string l_message = l_stream.str();

    debug( l_message );
}

#define variable( _variableToLog )                                    \
    _variable( __FILE_NAME__                                          \
               ":" MACRO_TO_STRING( __LINE__ ) " | " #_variableToLog, \
               _variableToLog );

inline void info( const std::string_view _message ) {
    std::cout << g_logInfoPrefix << _message << "\n";
}

inline void warning( const std::string_view _message ) {
    std::cerr << g_logWarningPrefix << _message << "\n";
}

// Function file:line | message
inline void _error( const std::string_view _message,
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
