#pragma once

#include <stdio.h>

#define DEBUG_LEVEL_NONE 0
#define DEBUG_LEVEL_ERR 1
#define DEBUG_LEVEL_INFO 2
#define DEBUG_LEVEL_DETAILS 3
#define DEBUG_LEVEL_ALL 10

#ifndef DEBUG_LEVEL
#   define DEBUG_LEVEL DEBUG_LEVEL_DETAILS
#endif

#define DEBUG_DETAILS DEBUG_LEVEL >= DEBUG_LEVEL_DETAILS

#if DEBUG_LEVEL >= DEBUG_LEVEL_INFO
#   define dlogf(__str, ...) printf(__str, ##__VA_ARGS__)
#   define dlog(__str) printf(__str)
#   define dlogln(__str) printf(__str "\n")
#   define dlog_ef(__str, ...) fprintf(stderr, "debug-err: " __str, ##__VA_ARGS__)
#endif

#if DEBUG_LEVEL == DEBUG_LEVEL_ERR
#   define dlog_ef(__str, ...) fprintf(stderr, "debug-err: " __str, ##__VA_ARGS__)
#   define dlogf(__str, ...) do {} while(0)
#   define dlog(__str) do {} while(0)
#   define dlogln(__str) do {} while(0)
#endif

#if DEBUG_LEVEL == DEBUG_LEVEL_NONE
#   define dlogf(__str, ...) do {} while(0)
#   define dlog(__str) do {} while(0)
#   define dlogln(__str) do {} while(0)
#   define dlog_ef(__str, ...) do {} while(0)
#endif