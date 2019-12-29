#pragma once

#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdlib>
#include <tchar.h>
#include <string>
#include <cassert>
#include <memory>
#include <vector>
#include <array>

#include "ReleasedUniquePtr.h"
#include "glm.h"

constexpr uint32_t kNumFramesInFlight = 2u;

template<typename T>
using IFFArray = std::array<T, kNumFramesInFlight>;
