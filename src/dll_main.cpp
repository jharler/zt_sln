#include "zt_tools.h"
#include "zt_game.h"
#include "zt_game_gui.h"

#include "sln_gui.h"
#include "sln_gen.h"

// ------------------------------------------------------------------------------------------------

struct ztGame
{
	ztGameDetails *details;
	ztGameSettings *settings;

	ztAssetManager asset_mgr;

	ztCamera camera, gui_camera;

	ztGuiManager *gui_manager;

	ztDrawList draw_list;

	ztTextureID tex_background;

	ztGuiItem *gui_config;
	ztBuildConfigGui gui_config_data;
};

ztGame *g_game = nullptr;

// ------------------------------------------------------------------------------------------------

void makeBinaryIncludeFiles(ztGameDetails* details, const char **files, int files_count, bool force)
{
	zt_fiz(files_count) {
		char src_name[ztFileMaxPath], dst_name[ztFileMaxPath];
		zt_fileConcatFileToPath(src_name, ztFileMaxPath, details->user_path, files[i]);

		char src_file[256];
		zt_fileGetFileName(src_name, src_file, zt_elementsOf(src_file));

		char var_name[256];
		zt_strCpy(var_name, zt_elementsOf(var_name), src_file, zt_strLen(src_file));
		zt_fizr(zt_strLen(var_name)) {
			if (var_name[i] == '.') {
				var_name[i] = '_';
			}
		}

		zt_strPrintf(dst_name, zt_elementsOf(dst_name), "%s\\src\\bin_%s.h", details->user_path, var_name);

		if (!force && (!zt_fileExists(src_name) || zt_fileExists(dst_name))) {
			continue;
		}

		i32 size = 0;
		byte *data = (byte*)zt_readEntireFile(src_name, &size);

		ztFile file;
		if (zt_fileOpen(&file, dst_name, ztFileOpenMethod_WriteOver)) {
			zt_fileWritef(&file, "i32  bin_%s_size = %d;\nbyte bin_%s[] = {\n", var_name, size, var_name);

			char buff[32];
			zt_fiz(size) {
				if (i == 0) {
					zt_strPrintf(buff, zt_sizeof(buff), "\t0x%02x", data[i]);
				}
				else if (i % 1000 == 0){
					zt_strPrintf(buff, zt_sizeof(buff), ",\n\t0x%02x", data[i]);
				}
				else {
					zt_strPrintf(buff, zt_sizeof(buff), ",0x%02x", data[i]);
				}

				zt_fileWrite(&file, buff, zt_strSize(buff) - 1);
			}

			zt_fileWrite(&file, "\n};");
			zt_fileClose(&file);
		}

		zt_free(data);
	}
}

// ------------------------------------------------------------------------------------------------

ZT_DLLEXPORT bool zt_dllSettings(ztGameDetails* details, ztGameSettings* settings)
{
	zt_logDebug("DLL Settings");

	zt_strMakePrintf(ini_file, ztFileMaxPath, "%s%csettings.cfg", details->user_path, ztFilePathSeparator);
	settings->memory = zt_iniFileGetValue(ini_file, "general", "app_memory", (i32)zt_megabytes(128));

	settings->native_w = settings->screen_w = zt_iniFileGetValue(ini_file, "general", "resolution_w", (i32)600);
	settings->native_h = settings->screen_h = zt_iniFileGetValue(ini_file, "general", "resolution_h", (i32)600);
	settings->renderer = ztRenderer_OpenGL;

	settings->renderer_flags = ztRendererFlags_Windowed;

	zt_inputMouseLook(false);

	return true;
}

// ------------------------------------------------------------------------------------------------

ZT_DLLEXPORT bool zt_dllInit(ztGameDetails* details, ztGameSettings* settings, void** game_memory)
{
	g_game = zt_mallocStruct(ztGame);
	*game_memory = g_game;

	*g_game = {};
	g_game->details = details;
	g_game->settings = settings;
	g_game->gui_config_data = {};

	zt_strMakePrintf(data_path, ztFileMaxPath, "%s%cdata", details->user_path, ztFilePathSeparator, ztFilePathSeparator);
	if (!zt_directoryExists(data_path)) {
		zt_strPrintf(data_path, ztFileMaxPath, "%s%crun%cdata", details->user_path, ztFilePathSeparator, ztFilePathSeparator);
	}

	if (!zt_assetManagerLoadDirectory(&g_game->asset_mgr, data_path)) {
		zt_logCritical("Unable to load game assets");
		return false;
	}

	if (!zt_drawListMake(&g_game->draw_list, 1024 * 128)) {
		zt_logCritical("Unable to initialize draw list");
		return false;
	}

	g_game->gui_manager = zt_guiManagerMake(&g_game->gui_camera, nullptr, zt_memGetGlobalArena());
	//zt_guiInitDebug(g_game->gui_manager);

	g_game->gui_config = sln_buildConfig(&g_game->gui_config_data);

	g_game->tex_background = zt_textureMake(&g_game->asset_mgr, zt_assetLoad(&g_game->asset_mgr, "textures/background.png"));


	const char *files[] = {
		"src\\zt_icon.ico"
	};

	makeBinaryIncludeFiles(details, files, zt_elementsOf(files), true);
	return true;
}

// ------------------------------------------------------------------------------------------------

ZT_DLLEXPORT bool zt_dllReload(void *memory)
{
	g_game = (ztGame*)memory;

	zt_dllGuiLoad();
	zt_logDebug("DLL Reload");

	sln_cleanupConfig(g_game->gui_config);
	g_game->gui_config = sln_buildConfig(&g_game->gui_config_data);

	return true;
}

// ------------------------------------------------------------------------------------------------

ZT_DLLEXPORT bool zt_dllUnload(void *memory)
{
	zt_dllGuiUnload();
	zt_logDebug("DLL Unload");
	return true;
}

// ------------------------------------------------------------------------------------------------

ZT_DLLEXPORT void zt_dllScreenChange(ztGameSettings *settings, void *memory)
{
	g_game = (ztGame*)memory;

	if (g_game->details->current_frame == 1) {
		zt_cameraMakePersp(&g_game->camera, settings->screen_w, settings->screen_h, zt_degreesToRadians(60), 0.1f, 100.f, zt_vec3(9, 10, 8));
		zt_cameraMakeOrtho(&g_game->gui_camera, settings->screen_w, settings->screen_h, settings->native_w, settings->native_h, 0.1f, 100.f, zt_vec3(0, 0, 0));
	}
	else {
		zt_cameraMakePersp(&g_game->camera, settings->screen_w, settings->screen_h, zt_degreesToRadians(60), 0.1f, 100.f, g_game->camera.position, g_game->camera.rotation);
		zt_cameraMakeOrtho(&g_game->gui_camera, settings->screen_w, settings->screen_h, settings->native_w, settings->native_h, 0.1f, 100.f, g_game->gui_camera.position);
	}

	zt_cameraRecalcMatrices(&g_game->camera);
	zt_cameraRecalcMatrices(&g_game->gui_camera);
}

// ------------------------------------------------------------------------------------------------

ZT_DLLEXPORT void zt_dllCleanup(void *memory)
{
	g_game = (ztGame*)memory;

	zt_textureFree(g_game->tex_background);

	zt_guiManagerFree(g_game->gui_manager);

	zt_drawListFree(&g_game->draw_list);

	zt_assetManagerFree(&g_game->asset_mgr);
	zt_free(g_game);

	zt_logDebug("DLL Cleanup");
}

// ------------------------------------------------------------------------------------------------

ZT_DLLEXPORT bool zt_dllGameLoop(void *memory, r32 dt)
{
	g_game = (ztGame*)memory;

	ztInputKeys *input = zt_inputKeysAccessState();
	ztInputMouse *mouse = zt_inputMouseAccessState();
	ztInputController *controller = zt_inputControllerAccessState(0);
	ztInputKeys_Enum input_keystrokes[16];
	zt_inputGetKeyStrokes(input_keystrokes);

	bool gui_input = zt_guiManagerHandleInput(g_game->gui_manager, input, input_keystrokes, mouse);

	if (input[ztInputKeys_Tilda].justPressed()) {
		bool console_shown = false;
		zt_debugConsoleToggle(&console_shown);
		if (console_shown) {
			zt_guiManagerSetKeyboardFocus(g_game->gui_manager);
		}
	}

	if (g_game->gui_config_data.generate) {
		zt_logInfo("Generating project...");
		ztBuildConfig config;

		sln_populateConfig(&g_game->gui_config_data, &config);
		sln_build(&config, g_game->details->data_path);
	}

	zt_rendererClear(zt_vec4(0, 0, 0, 0));

	{ // draw background
		ztVec2 cam_size = zt_cameraOrthoGetViewportSize(&g_game->gui_camera);
		ztVec2 cam_min = zt_cameraOrthoGetMinExtent(&g_game->gui_camera);

		zt_drawListPushShader(&g_game->draw_list, zt_shaderGetDefault(ztShaderDefault_Unlit));
		zt_drawListPushTexture(&g_game->draw_list, g_game->tex_background);

		zt_drawListPushColor(&g_game->draw_list, zt_vec4(0, 1, 0, 1));

		ztVec2i tex_size = zt_textureGetSize(g_game->tex_background);
		ztVec2  tex_size_units = zt_vec2(tex_size.x / zt_pixelsPerUnit(), tex_size.y / zt_pixelsPerUnit());

		int cols = zt_convertToi32Ceil(cam_size.x / tex_size_units.x) + 1;
		int rows = zt_convertToi32Ceil(cam_size.y / tex_size_units.y) + 1;

		cam_min.x += tex_size_units.x / 2.f;
		cam_min.y += tex_size_units.y / 2.f;

		zt_fiz(rows) {
			zt_fjz(cols) {
				zt_drawListAddFilledRect2D(&g_game->draw_list, zt_vec3(cam_min.x + (j * tex_size_units.x), cam_min.y + (i * tex_size_units.y), 0), tex_size_units, zt_vec2(0, 0), zt_vec2(1, 1));
			}
		}
		zt_drawListPopColor(&g_game->draw_list);
		zt_drawListPopTexture(&g_game->draw_list);

		zt_guiManagerRender(g_game->gui_manager, &g_game->draw_list, dt);

		zt_drawListPopShader(&g_game->draw_list);
		zt_renderDrawList(&g_game->gui_camera, &g_game->draw_list, ztVec4::zero, ztRenderDrawListFlags_NoDepthTest);
	}

	zt_assetManagerCheckForChanges(&g_game->asset_mgr);
	return !input[ztInputKeys_Escape].justPressed();
	return true;
}


// ------------------------------------------------------------------------------------------------
