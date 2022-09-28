#pragma once

#define BIT(x) (1 << x)

#define global_variable static

#define ArraySize(arr) sizeof((arr)) / sizeof((arr)[0])

#define KB(x) ((uint64_t)1024 * x)
#define MB(x) ((uint64_t)1024 * KB(x))
#define GB(x) ((uint64_t)1024 * MB(x))

#define INVALID_IDX UINT32_MAX

#define C_PI 3.14159265358979323846

// clang-format on
// clang-format off

#define b8 bool