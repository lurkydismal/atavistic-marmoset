#pragma once

#include <iostream>
#include <print>
#include <source_location>
#include <string_view>
#include <utility>

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

template < typename... Arguments >
inline void debug( std::format_string< Arguments... > _format,
                   Arguments&&... _arguments ) {
#if defined( DEBUG )

    std::print( g_logDebugPrefix );
    std::println( _format, std::forward< Arguments >( _arguments )... );

#else

    ( void )_message;

#endif
}

template < typename T >
inline void _variable( const std::string_view _variableName,
                       const T& _variable,
                       const std::source_location l_sourceLocation =
                           std::source_location::current() ) {
    debug( "{}:{} | {} = '{}'", l_sourceLocation.file_name(),
           l_sourceLocation.line(), _variableName, _variable );
}

#define variable( _variableToLog ) _variable( #_variableToLog, _variableToLog )

template < typename... Arguments >
inline void info( std::format_string< Arguments... > _format,
                  Arguments&&... _arguments ) {
    std::print( g_logInfoPrefix );
    std::println( _format, std::forward< Arguments >( _arguments )... );
}

template < typename... Arguments >
inline void warning( std::format_string< Arguments... > _format,
                     Arguments&&... _arguments ) {
    std::print( std::cerr, g_logWarningPrefix );
    std::println( _format, std::forward< Arguments >( _arguments )... );
}

// Function file:line | message
template < typename... Arguments >
inline void error( std::format_string< Arguments... > _format,
                   Arguments&&... _arguments,
                   const std::source_location l_sourceLocation =
                       std::source_location::current() ) {
    std::print( std::cerr, "{}\"{}\" {} : {} | ", g_logErrorPrefix,
                l_sourceLocation.function_name(), l_sourceLocation.file_name(),
                l_sourceLocation.line() );
    std::println( std::cerr, _format,
                  std::forward< Arguments >( _arguments )... );
}

} // namespace log
