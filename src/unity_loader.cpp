#define ZT_NO_DIRECTX
#define ZT_OPENGL_DEBUGGING

#ifndef ZT_DLL_LOADER
#define ZT_DLL_LOADER
#endif


#include "loader_main.cpp"

#define ZT_TOOLS_IMPLEMENTATION
#include "zt_tools.h"

#define ZT_GAME_IMPLEMENTATION
#include "zt_game.h"

#define ZT_GAME_GUI_IMPLEMENTATION
#include "zt_game_gui.h"
