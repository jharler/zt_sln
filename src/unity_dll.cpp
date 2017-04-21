/*
 *
 *
 */

#ifndef ZT_DLL
#define ZT_DLL
#endif

#define ZT_NO_DIRECTX

#define ZT_OPENGL_DEBUGGING

#include "dll_main.cpp"
#include "sln_gui.cpp"
#include "sln_gen.cpp"

#define ZT_TOOLS_IMPLEMENTATION
#include "zt_tools.h"

#define ZT_GAME_IMPLEMENTATION
#include "zt_game.h"

#define ZT_GAME_GUI_IMPLEMENTATION
#include "zt_game_gui.h"
