#ifndef __sln_gui_h__
#define __sln_gui_h__

// ------------------------------------------------------------------------------------------------

#include "zt_game_gui.h"
#include "sln_gen.h"

// ------------------------------------------------------------------------------------------------

struct ztBuildConfigGui
{
	ztGuiItem  *project_root  = nullptr;
	ztGuiItem  *project_name  = nullptr;
	ztGuiItem  *zt_directory  = nullptr;

	bool        include_gui         = true;
	bool        include_vr          = false;
	bool        include_git_ignore  = true;
	bool        unity_build         = true;
	bool        dll_project         = true;
	ztGuiItem  *dll_loader_name     = nullptr;
	ztGuiItem  *dll_dll_name        = nullptr;
	bool        camera_2d           = true;
	bool        camera_3d           = true;
	bool        include_3d_scene    = true;
	bool        include_pbr         = true;
	bool        include_entity      = true;
	bool        default_dirs        = true;
	bool        log_file            = true;


	bool generate = false;
};

// ------------------------------------------------------------------------------------------------

ztGuiItem *sln_buildConfig(ztBuildConfigGui *config_gui);
void sln_cleanupConfig(ztGuiItem *config);

void sln_populateConfig(ztBuildConfigGui *config_gui, ztBuildConfig *config);

// ------------------------------------------------------------------------------------------------

#endif // include guard