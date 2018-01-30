#ifndef __sln_gen_h__
#define __sln_gen_h__

// ------------------------------------------------------------------------------------------------

#include "zt_tools.h"

// ------------------------------------------------------------------------------------------------

struct ztBuildConfig
{
	char     project_root[ztFileMaxPath];
	char     project_name[256];
	char     zt_directory[ztFileMaxPath];
	bool     include_gui;
	bool     include_vr;
	bool     include_git_ignore;
	bool     unity_build;
	bool     dll_project;
	char     dll_loader_name[256];
	char     dll_dll_name[256];
	bool     camera_2d;
	bool     camera_3d;
	bool     include_3d_scene;
	bool     include_pbr;
	bool     default_dirs;
	bool     log_file;
};

// ------------------------------------------------------------------------------------------------

bool sln_build(ztBuildConfig *build_cfg, const char *data_dir);

// ------------------------------------------------------------------------------------------------

#endif // include guard