#pragma once

// ============================================================================
// Platform detection
// ============================================================================

#if defined(_WIN32) || defined(_WIN64)
#define SKIF_PLATFORM_WINDOWS 1
#elif defined(__linux__)
#define SKIF_PLATFORM_LINUX 1
#elif defined(__APPLE__)
#include <TargetConditionals.h>
#define SKIF_PLATFORM_MACOS 1
#endif

// ============================================================================
// Compiler detection
// ============================================================================

#if defined(_MSC_VER)
#define SKIF_COMPILER_MSVC    1
#define SKIF_COMPILER_VERSION _MSC_VER
#elif defined(__clang__)
#define SKIF_COMPILER_CLANG   1
#define SKIF_COMPILER_VERSION (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#elif defined(__GNUC__)
#define SKIF_COMPILER_GCC     1
#define SKIF_COMPILER_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

// ============================================================================
// Plugin export/import macros
// ============================================================================

#if defined(SKIF_PLATFORM_WINDOWS)
#if defined(SKIF_PLUGIN_EXPORTS)
#define SKIF_PLUGIN_API __declspec(dllexport)
#else
#define SKIF_PLUGIN_API __declspec(dllimport)
#endif
#else
#if defined(SKIF_PLUGIN_EXPORTS)
#define SKIF_PLUGIN_API __attribute__((visibility("default")))
#else
#define SKIF_PLUGIN_API
#endif
#endif

#define SKIF_PLUGIN_EXPORT extern "C" SKIF_PLUGIN_API

// ============================================================================
// Utility macros
// ============================================================================

/// Suppress unused variable warnings
#define SKIF_UNUSED(x) (void)(x)

/// Mark a function as deprecated
#if defined(SKIF_COMPILER_MSVC)
#define SKIF_DEPRECATED(msg) __declspec(deprecated(msg))
#else
#define SKIF_DEPRECATED(msg) [[deprecated(msg)]]
#endif

namespace skif::rmlui
{
// Placeholder namespace for future global types
} // namespace skif::rmlui
