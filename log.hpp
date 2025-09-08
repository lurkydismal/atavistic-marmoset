#pragma once

#include <iostream>
#include <print>
#include <source_location>
#include <string_view>

#include "common.hpp"

namespace {

// Prefixes
#if defined( DEBUG )

constexpr std::string_view g_logDebugPrefix = "DEBUG: ";

#endif

constexpr std::string_view g_logInfoPrefix = "INFO: ";
constexpr std::string_view g_logWarningPrefix = "WARNING: ";
constexpr std::string_view g_logErrorPrefix = "ERROR: ";

} // namespace

namespace log {

template < typename... Arguments >
inline void debug( [[maybe_unused]] std::format_string< Arguments... > _format,
                   [[maybe_unused]] Arguments&&... _arguments ) {
#if defined( DEBUG )

    std::print( g_logDebugPrefix );
    std::println( _format, std::forward< Arguments >( _arguments )... );

#endif
}

template < typename T >
    requires( !std::is_pointer_v< T > )
inline void _variable( std::string_view _variableName,
                       const T& _variable,
                       const std::source_location _sourceLocation =
                           std::source_location::current() ) {
    debug( "{}:{} | {} = '{}'", _sourceLocation.file_name(),
           _sourceLocation.line(), _variableName, _variable );
}

template < typename T >
    requires( std::is_pointer_v< T > )
inline void _variable( std::string_view _variableName,
                       const T _variable,
                       const std::source_location _sourceLocation =
                           std::source_location::current() ) {
    debug( "{}:{} | {} = '0x{:016x}'", _sourceLocation.file_name(),
           _sourceLocation.line(), _variableName,
           std::bit_cast< uintptr_t >( _variable ) );
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

template < typename... Arguments >
inline void _error( std::format_string< Arguments... > _format,
                    const std::source_location _sourceLocation,
                    Arguments&&... _arguments ) {
    std::print( std::cerr, "{}\"{}\" {} : {} | ", g_logErrorPrefix,
                _sourceLocation.function_name(), _sourceLocation.file_name(),
                _sourceLocation.line() );
    std::println( std::cerr, _format,
                  std::forward< Arguments >( _arguments )... );
}

// Function file:line | message
template < typename... Arguments >
inline void error( std::format_string< Arguments... > _format,
                   Arguments&&... _arguments ) {
    _error( _format, std::source_location::current(),
            std::forward< Arguments >( _arguments )... );
}

} // namespace log
