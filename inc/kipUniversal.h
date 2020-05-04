#pragma once

#ifdef DLL_PROJECT
#define DLLMODE __declspec(dllexport)
#else
#define DLLMODE __declspec(dllimport)
#endif
