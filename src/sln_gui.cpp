#include "sln_gui.h"

#define ZT_GAME_GUI_INTERNAL_DECLARATIONS
#include "zt_game_gui.h"

// ------------------------------------------------------------------------------------------------

ztInternal void _sln_addControl(const char *label, ztGuiItem *control, ztGuiItem *sizer, int grow_direction = ztGuiItemOrient_Horz | ztGuiItemOrient_Vert)
{
	r32 label_min_x = 2.65f;

	ztGuiItem *sub_sizer = zt_guiMakeSizer(sizer, ztGuiItemOrient_Horz);

	ztGuiItem *label_id = zt_guiMakeStaticText(sub_sizer, label);
	zt_guiItemSetAlign(label_id, ztAlign_Left);
	zt_guiItemSetSize(label_id, zt_vec2(label_min_x, 0));

	zt_guiSizerAddItem(sub_sizer, label_id, 0, 1 / zt_pixelsPerUnit());
	zt_guiSizerAddItem(sub_sizer, control, 1, 1.f / zt_pixelsPerUnit(), ztAlign_Left, grow_direction);

	zt_guiSizerAddItem(sizer, sub_sizer, 0, 0 / zt_pixelsPerUnit());
}

// ------------------------------------------------------------------------------------------------

ztGuiItem *sln_buildConfig(ztBuildConfigGui *config_gui)
{
	//zt_guiMakeScrollWindow
	ztGuiItem *window_id = zt_guiMakeWindow("ZeroTolerance Project Configuration", ztGuiWindowBehaviorFlags_ShowTitle);
	zt_guiItemSetSize(window_id, zt_vec2(8.5f, 8.75f));

	ztGuiItem *root_sizer = zt_guiMakeSizer(zt_guiWindowGetContentParent(window_id), ztGuiItemOrient_Vert);
	zt_guiSizerSizeToParent(root_sizer);

	ztGuiItem *main_sizer = zt_guiMakeSizer(root_sizer, ztGuiItemOrient_Vert);
	zt_guiSizerAddItem(root_sizer, main_sizer, 1, 4 / zt_pixelsPerUnit());
	{
		char ini_file[ztFileMaxPath];
		zt_fileConcatFileToPath(ini_file, ztFileMaxPath, g_game->details->app_path, "settings.ini");

		{
			char root_dir[ztFileMaxPath];
			zt_iniFileGetValue(ini_file, "general", "proj_root_dir", g_game->details->app_path, root_dir, ztFileMaxPath);

			ztGuiItem *textbox_id = zt_guiMakeTextEdit(main_sizer, root_dir);
			//textbox_id->debug_highlight = ztColor(1, 0, 0, 1);
			
			_sln_addControl("Project Root", textbox_id, main_sizer);
			config_gui->project_root = textbox_id;
		}
		{
			ztGuiItem *textbox_id = zt_guiMakeTextEdit(main_sizer, "zt_project");
			_sln_addControl("Project Name", textbox_id, main_sizer);
			config_gui->project_name = textbox_id;
		}
		{
			char zt_dir[ztFileMaxPath];
			zt_iniFileGetValue(ini_file, "general", "zt_directory", "", zt_dir, ztFileMaxPath);

			ztGuiItem *textbox_id = zt_guiMakeTextEdit(main_sizer, zt_dir);
			_sln_addControl("ZT Project Directory", textbox_id, main_sizer);
			config_gui->zt_directory = textbox_id;
		}
		{
			ztGuiItem *checkbox_id = zt_guiMakeCheckbox(main_sizer, "(must exclude each file from compilation)", ztGuiCheckboxBehaviorFlags_RightText, &config_gui->unity_build);
			zt_guiCheckboxSetValue(checkbox_id, true);
			zt_guiItemHide(checkbox_id); // we're going to make this always true

			//_sln_addControl("Unity Build", checkbox_id, main_sizer, 0);
		}
		{
			ztGuiItem *checkbox_id = zt_guiMakeCheckbox(main_sizer, "(hotloading game code)", ztGuiCheckboxBehaviorFlags_RightText, &config_gui->dll_project);
			zt_guiCheckboxSetValue(checkbox_id, true);
			zt_guiItemHide(checkbox_id); // we're going to make this always true

			_sln_addControl("DLL Project", checkbox_id, main_sizer, 0);
		}
		{
			ztGuiItem *textbox_id = zt_guiMakeTextEdit(main_sizer, "%s_loader");
			_sln_addControl("  +- Loader Name", textbox_id, main_sizer);
			config_gui->dll_loader_name = textbox_id;
		}
		{
			ztGuiItem *textbox_id = zt_guiMakeTextEdit(main_sizer, "%s_dll");
			_sln_addControl("  +- DLL Name", textbox_id, main_sizer);
			config_gui->dll_dll_name = textbox_id;
		}
		{
			ztGuiItem *label_id = zt_guiMakeStaticText(main_sizer, "(%s will be replaced with Project Name)");
			_sln_addControl(" ", label_id, main_sizer);
		}
		{
			ztGuiItem *checkbox_id = zt_guiMakeCheckbox(main_sizer, "", ztGuiCheckboxBehaviorFlags_RightText, &config_gui->default_dirs);
			zt_guiCheckboxSetValue(checkbox_id, true);

			_sln_addControl("Default Directories", checkbox_id, main_sizer, 0);
		}
		{
			ztGuiItem *checkbox_id = zt_guiMakeCheckbox(main_sizer, "", ztGuiCheckboxBehaviorFlags_RightText, &config_gui->include_git_ignore);
			zt_guiCheckboxSetValue(checkbox_id, true);

			_sln_addControl("Include Git Ignore", checkbox_id, main_sizer, 0);
		}

		zt_guiSizerAddStretcher(main_sizer, 0, 20 / zt_pixelsPerUnit());

		{
			ztGuiItem *checkbox_id = zt_guiMakeCheckbox(main_sizer, "", ztGuiCheckboxBehaviorFlags_RightText, &config_gui->camera_2d);
			zt_guiCheckboxSetValue(checkbox_id, true);

			_sln_addControl("2D Camera", checkbox_id, main_sizer, 0);
		}
		{
			ztGuiItem *checkbox_id = zt_guiMakeCheckbox(main_sizer, "", ztGuiCheckboxBehaviorFlags_RightText, &config_gui->camera_3d);
			zt_guiCheckboxSetValue(checkbox_id, true);

			_sln_addControl("3D Camera", checkbox_id, main_sizer, 0);
		}
		{
			ztGuiItem *checkbox_id = zt_guiMakeCheckbox(main_sizer, "(will include 2D camera)", ztGuiCheckboxBehaviorFlags_RightText, &config_gui->include_gui);
			zt_guiCheckboxSetValue(checkbox_id, true);

			_sln_addControl("Include GUI", checkbox_id, main_sizer, 0);
		}
		{
			ztGuiItem *checkbox_id = zt_guiMakeCheckbox(main_sizer, "(will include 3D camera)", ztGuiCheckboxBehaviorFlags_RightText, &config_gui->include_3d_scene);
			zt_guiCheckboxSetValue(checkbox_id, true);

			_sln_addControl("Include 3D Scene", checkbox_id, main_sizer, 0);
		}
		{
			ztGuiItem *checkbox_id = zt_guiMakeCheckbox(main_sizer, "(will include 3D scene)", ztGuiCheckboxBehaviorFlags_RightText, &config_gui->include_pbr);
			zt_guiCheckboxSetValue(checkbox_id, true);

			_sln_addControl("Include PBR", checkbox_id, main_sizer, 0);
		}
		{
			ztGuiItem *checkbox_id = zt_guiMakeCheckbox(main_sizer, "", ztGuiCheckboxBehaviorFlags_RightText, &config_gui->include_vr);
			zt_guiCheckboxSetValue(checkbox_id, false);

			_sln_addControl("Include VR", checkbox_id, main_sizer, 0);
		}
		{
			ztGuiItem *checkbox_id = zt_guiMakeCheckbox(main_sizer, "", ztGuiCheckboxBehaviorFlags_RightText, &config_gui->log_file);
			zt_guiCheckboxSetValue(checkbox_id, true);

			_sln_addControl("Log File", checkbox_id, main_sizer, 0);
		}
	}

	{
		zt_guiSizerAddStretcher(main_sizer, 1, 0);
		ztGuiItem *button_id = zt_guiMakeButton(main_sizer, "Generate Project", 0, &config_gui->generate);
		zt_guiSizerAddItem(main_sizer, button_id, 0, 5 / zt_pixelsPerUnit());
	}

	return window_id;
}

// ------------------------------------------------------------------------------------------------

void sln_cleanupConfig(ztGuiItem *config)
{
	zt_guiItemFree(config);
}

// ------------------------------------------------------------------------------------------------

void sln_populateConfig(ztBuildConfigGui *config_gui, ztBuildConfig *config)
{
	zt_guiTextEditGetValue(config_gui->project_root, config->project_root, zt_elementsOf(config->project_root));
	zt_guiTextEditGetValue(config_gui->project_name, config->project_name, zt_elementsOf(config->project_name));
	zt_guiTextEditGetValue(config_gui->zt_directory, config->zt_directory, zt_elementsOf(config->zt_directory));

	zt_guiTextEditGetValue(config_gui->dll_loader_name, config->dll_loader_name, zt_elementsOf(config->dll_loader_name));
	zt_guiTextEditGetValue(config_gui->dll_dll_name, config->dll_dll_name, zt_elementsOf(config->dll_dll_name));

	if (zt_strFindPos(config->dll_loader_name, "%s", 0) != ztStrPosNotFound) {
		char copy[zt_elementsOf(config->dll_loader_name)];
		zt_strCpy(copy, zt_elementsOf(copy), config->dll_loader_name);

		zt_strPrintf(config->dll_loader_name, zt_elementsOf(config->dll_loader_name), copy, config->project_name);
	}

	if (zt_strFindPos(config->dll_dll_name, "%s", 0) != ztStrPosNotFound) {
		char copy[zt_elementsOf(config->dll_dll_name)];
		zt_strCpy(copy, zt_elementsOf(copy), config->dll_dll_name);

		zt_strPrintf(config->dll_dll_name, zt_elementsOf(config->dll_dll_name), copy, config->project_name);
	}

	config->include_gui = config_gui->include_gui;
	config->include_vr         = config_gui->include_vr;
	config->include_git_ignore = config_gui->include_git_ignore;
	config->unity_build        = config_gui->unity_build;
	config->dll_project        = config_gui->dll_project;
	config->camera_2d          = config_gui->camera_2d;
	config->camera_3d          = config_gui->camera_3d;
	config->include_3d_scene   = config_gui->include_3d_scene || config_gui->include_pbr;
	config->include_pbr        = config_gui->include_pbr;
	config->default_dirs       = config_gui->default_dirs;
	config->log_file           = config_gui->log_file;
}

// ------------------------------------------------------------------------------------------------
