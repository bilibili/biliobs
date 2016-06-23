
#pragma once

#include "util/profiler.hpp"
#include "util/platform.h"
#include "util/dstr.h"
#include "util/util.hpp"
#include "qt-wrappers.hpp"
#include "platform.hpp"
#include <fstream>
#include <sstream>
#include <time.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#endif

using namespace std;

int GetUserDataPath(char *path, size_t size, const char *name);
bool do_mkdir(const char *path);
char *GetConfigPathPtr(const char *name);
string GenerateTimeDateFilename(const char *extension);
string CurrentTimeString();
vector<pair<string, string>> GetLocaleNames();
