#pragma once

#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdlib>
#include <tchar.h>
#include <string>
#include <cassert>
#include <memory>
#include <vector>

#include "ReleasedUniquePtr.h"

constexpr uint32_t kNumFramesInFlight = 2u;
