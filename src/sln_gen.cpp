#include "sln_gen.h"
#include "bin_zt_icon_ico.h"

// ------------------------------------------------------------------------------------------------

enum SlnProjFileType_Enum
{
	SlnProjFileType_Standard,
	SlnProjFileType_DllLoader,
	SlnProjFileType_DllDll,
};

// ------------------------------------------------------------------------------------------------

#define  SRC_FILE_UNITY_FULL          "unity_full.cpp"
#define  SRC_FILE_UNITY_LOADER        "unity_loader.cpp"
#define  SRC_FILE_GAME_LOADER         "main_loader.cpp"
#define  SRC_FILE_UNITY_DLL           "unity_dll.cpp"
#define  SRC_FILE_GAME_DLL            "main_dll.cpp"
#define  SRC_FILE_GAME                "main.cpp"
#define  SRC_FILE_GAME_H              "game.h"
#define  SRC_FILE_GAME_SCENE_H        "game_scene.h"
#define  SRC_FILE_GAME_SCENE_MAIN     "game_scene_main.cpp"
#define  SRC_FILE_GAME_SCENE_SPLASH   "game_scene_splash.cpp"
#define  SRC_FILE_GAME_SCENE_MENU     "game_scene_menu.cpp"
#define  SRC_FILE_GUI_THEME_H         "gui_theme.h"
#define  SRC_FILE_GUI_THEME           "gui_theme.cpp"

// ------------------------------------------------------------------------------------------------

void sln_generateGuid(const char *file, char *buffer, int buffer_size)
{
	char file_only[ztFileMaxPath];
	zt_fileGetFileName(file, file_only, ztFileMaxPath);

	srand(zt_strHash(file_only));
	zt_strPrintf(buffer, buffer_size, "%08X-%04X-%04X-%04X-%04X%08X", rand(), rand() % 0xFFFF, rand() % 0xFFFF, rand() % 0xFFFF, rand() % 0xFFFF, rand());

	zt_logDebug("generated GUID for: %s = %s", file_only, buffer);
}

// ------------------------------------------------------------------------------------------------

void sln_generateGuidProj(const char *proj, char *buffer, int buffer_size)
{
	zt_strMakePrintf(file_name, 256, "\\%s.vcxproj", proj);
	sln_generateGuid(file_name, buffer, buffer_size);
}

// ------------------------------------------------------------------------------------------------

bool sln_writeProjectFile(const char *proj_file, SlnProjFileType_Enum type, ztBuildConfig *build_cfg)
{
	char file_path[ztFileMaxPath];
	zt_fileGetFullPath(proj_file, file_path, ztFileMaxPath);
	if (!zt_directoryExists(file_path)) {
		zt_directoryMake(file_path);
	}

	ztFile file;
	zt_fileOpen(&file, proj_file, ztFileOpenMethod_WriteOver);

	zt_strMakePrintf(user_file, ztFileMaxPath, "%s.user", proj_file);
	ztFile ufile;
	zt_fileOpen(&ufile, user_file, ztFileOpenMethod_WriteOver);
	

	zt_fileWrite(&file,
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
		"<Project DefaultTargets=\"Build\" ToolsVersion=\"12.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n"
		"  <ItemGroup Label=\"ProjectConfigurations\">\n"
		);

	zt_fileWritef(&ufile,
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
		"<Project ToolsVersion=\"12.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n"
		);


	char *proj_configs[4] = {
		"Debug|Win32",
		"Debug|x64",
		"Release|Win32",
		"Release|x64",
	};

	char *proj_outdir[4] = {
		"..\\..\\bin\\debug\\x86\\",
		"..\\..\\bin\\debug\\x64\\",
		"..\\..\\bin\\release\\x86\\",
		"..\\..\\bin\\release\\x64\\",
	};

	char *proj_outdir_dbl[4] = {
		"..\\\\..\\\\bin\\\\debug\\\\x86\\\\",
		"..\\\\..\\\\bin\\\\debug\\\\x64\\\\",
		"..\\\\..\\\\bin\\\\release\\\\x86\\\\",
		"..\\\\..\\\\bin\\\\release\\\\x64\\\\",
	};

	int proj_configs_cnt = 4; // build_cfg->include_x64 ? 4 : 2;

	zt_fiz(proj_configs_cnt) {
		char config[16]; zt_strGetBetween(config, 16, proj_configs[i], "", "|");
		char platform[16]; zt_strGetBetween(platform, 16, proj_configs[i], "|", "");

		zt_fileWritef(&file,
			"    <ProjectConfiguration Include=\"%s\">\n"
			"      <Configuration>%s</Configuration>\n"
			"      <Platform>%s</Platform>\n"
			"    </ProjectConfiguration>\n", proj_configs[i], config, platform);


		zt_fileWritef(&ufile,
			"  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)' == '%s'\">\n"
			"    <LocalDebuggerWorkingDirectory>$(ProjectDir)\\..\\..\\..\\..</LocalDebuggerWorkingDirectory>\n"
			"    <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>\n"
			"  </PropertyGroup>\n", proj_configs[i]
			);
	}

	zt_fileWritef(&ufile,
		"</Project>\n"
		);

	zt_fileWrite(&file,
		"  </ItemGroup>\n"
		"  <ItemGroup>\n"
		);

	if (build_cfg->unity_build) {
		switch (type)
		{
			case SlnProjFileType_Standard: { 
				if (build_cfg->dll_project) {
					zt_fileWritef(&file, "    <ClCompile Include=\"..\\..\\..\\src\\" SRC_FILE_UNITY_FULL "\" />\n");
				}
				else {
					zt_fileWritef(&file, "    <ClCompile Include=\"..\\..\\..\\src\\" SRC_FILE_GAME "\" />\n");
				}
			} break;

			case SlnProjFileType_DllLoader: {
				zt_fileWritef(&file, "    <ClCompile Include=\"..\\..\\..\\src\\" SRC_FILE_UNITY_LOADER "\" />\n");
				zt_fileWritef(&file, "    <None Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_LOADER "\" />\n");
			} break;

			case SlnProjFileType_DllDll: {
				zt_fileWritef(&file, "    <ClCompile Include=\"..\\..\\..\\src\\" SRC_FILE_UNITY_DLL "\" />\n");
				zt_fileWritef(&file, "    <None Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_DLL "\" />\n");

				zt_fileWritef(&file, "    <None Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_H "\" />\n");
				zt_fileWritef(&file, "    <None Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_SCENE_H "\" />\n");
				zt_fileWritef(&file, "    <None Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_SCENE_SPLASH"\" />\n");
				zt_fileWritef(&file, "    <None Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_SCENE_MENU"\" />\n");
				zt_fileWritef(&file, "    <None Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_SCENE_MAIN "\" />\n");

				if (build_cfg->include_gui) {
					zt_fileWritef(&file, "    <None Include=\"..\\..\\..\\src\\" SRC_FILE_GUI_THEME_H "\" />\n");
					zt_fileWritef(&file, "    <None Include=\"..\\..\\..\\src\\" SRC_FILE_GUI_THEME "\" />\n");
				}

			} break;
		}
	}
	else {
		switch (type)
		{
			case SlnProjFileType_Standard: {
				if (build_cfg->dll_project) {
					zt_fileWritef(&file, "    <ClCompile Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_LOADER "\" />\n");
					zt_fileWritef(&file, "    <ClCompile Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_DLL "\" />\n");
				}
				else {
					zt_fileWritef(&file, "    <ClCompile Include=\"..\\..\\..\\src\\" SRC_FILE_GAME "\" />\n");
				}
			} break;

			case SlnProjFileType_DllLoader: {
				zt_fileWritef(&file, "    <ClCompile Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_LOADER "\" />\n");
			} break;

			case SlnProjFileType_DllDll: {
				zt_fileWritef(&file, "    <ClCompile Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_DLL "\" />\n");
				zt_fileWritef(&file, "    <ClCompile Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_H "\" />\n");
				zt_fileWritef(&file, "    <ClCompile Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_SCENE_H "\" />\n");
				zt_fileWritef(&file, "    <ClCompile Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_SCENE_SPLASH "\" />\n");
				zt_fileWritef(&file, "    <ClCompile Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_SCENE_MENU "\" />\n");
				zt_fileWritef(&file, "    <ClCompile Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_SCENE_MAIN "\" />\n");

				if (build_cfg->include_gui) {
					zt_fileWritef(&file, "    <None Include=\"..\\..\\..\\src\\" SRC_FILE_GUI_THEME_H "\" />\n");
					zt_fileWritef(&file, "    <None Include=\"..\\..\\..\\src\\" SRC_FILE_GUI_THEME "\" />\n");
				}
			} break;
		}
	}

	zt_fileWrite(&file,
		"  </ItemGroup>\n"
		"  <ItemGroup>\n"
		"    <ResourceCompile Include=\"..\\..\\..\\src\\resources.rc\"/>\n"
		"  </ItemGroup>\n"
		"  <ItemGroup>\n"
		"    <Image Include=\"..\\..\\..\\src\\resources_icon.ico\"/>\n"
		"  </ItemGroup>\n"
		);

	char *proj_name = nullptr;
	char *cfg_type = nullptr;
	char *target_file = nullptr;

	if (type == SlnProjFileType_Standard) { proj_name = build_cfg->project_name; cfg_type = "Application"; target_file = proj_name; }
	if (type == SlnProjFileType_DllLoader) { proj_name = build_cfg->dll_loader_name; cfg_type = "Application"; target_file = proj_name; }
	if (type == SlnProjFileType_DllDll) { proj_name = build_cfg->dll_dll_name; cfg_type = "DynamicLibrary"; target_file = build_cfg->project_name; }

	char guid[128] = { 0 };
	sln_generateGuid(proj_file, guid, zt_elementsOf(guid));

	zt_fileWritef(&file,
		"  <PropertyGroup Label=\"Globals\">\n"
		"    <ProjectGuid>{%s}</ProjectGuid>\n"
		"    <RootNamespace>%s</RootNamespace>\n"
		"  </PropertyGroup>\n"
		"  <Import Project = \"$(VCTargetsPath)\\Microsoft.Cpp.Default.props\"/>\n",
		guid, proj_name
		);

	zt_fiz(proj_configs_cnt) {
		char *debug = zt_strCount(proj_configs[i], "Debug") > 0 ? "true" : "false";
		char *release = zt_strEquals(debug, "false") ? "true" : "false";

		zt_fileWritef(&file,
			"  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='%s'\" Label=\"Configuration\">\n"
			"    <ConfigurationType>%s</ConfigurationType>\n"
			"    <UseDebugLibraries>%s</UseDebugLibraries>\n"
			"    <PlatformToolset>v120</PlatformToolset>\n"
			"    <WholeProgramOptimization>%s</WholeProgramOptimization>\n"
			"    <CharacterSet>MultiByte</CharacterSet>\n"
			"  </PropertyGroup>\n", proj_configs[i], cfg_type, debug, release
			);
	}

	zt_fileWrite(&file,
		"  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.props\"/>\n"
		"  <ImportGroup Label=\"ExtensionSettings\">\n"
		"  </ImportGroup>\n"
		);

	zt_fiz(proj_configs_cnt) {
		zt_fileWritef(&file,
			"  <ImportGroup Label=\"PropertySheets\" Condition=\"'$(Configuration)|$(Platform)'=='%s'\">\n"
			"    <Import Project=\"$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\" Condition=\"exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')\" Label=\"LocalAppDataPlatform\"/>\n"
			"  </ImportGroup>\n", proj_configs[i]
			);
	}

	zt_fileWrite(&file,
		"  <PropertyGroup Label=\"UserMacros\"/>\n"
		);

	zt_fiz(proj_configs_cnt) {
		zt_fileWritef(&file,
			"  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='%s'\">\n"
			"    <IncludePath>$(VC_IncludePath);$(WindowsSDK_IncludePath);%s;</IncludePath>\n"
			"    <OutDir>$(SolutionDir)%s\\</OutDir>\n"
			"    <TargetName>%s</TargetName>\n"
			"  </PropertyGroup>\n", proj_configs[i], build_cfg->zt_directory, proj_outdir[i], target_file
			);
	}

	zt_fiz(proj_configs_cnt) {
		char *optimization = zt_strCount(proj_configs[i], "Debug") > 0 ? "Disabled" : "MaxSpeed";
		char *debug_info = zt_strCount(proj_configs[i], "Debug") > 0 ? "true" : "false";
		char *runtime_library = zt_strCount(proj_configs[i], "Debug") > 0 ? "MultiThreadedDebug" : "MultiThreaded";

		zt_fileWritef(&file,
			"<ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='%s'\">\n"
			"  <ClCompile>\n"
			"    <WarningLevel>Level3</WarningLevel>\n"
			"    <Optimization>%s</Optimization>\n"
			"    <SDLCheck>true</SDLCheck>\n"
			"    <ExceptionHandling>false</ExceptionHandling>\n"
			"    <RuntimeLibrary>%s</RuntimeLibrary>\n" // !
			"    <PreprocessorDefinitions>_MBCS;%%(PreprocessorDefinitions);ZT_BUILD_DESTINATION=\"%s\"</PreprocessorDefinitions>\n"
			"  </ClCompile>\n"
			"  <Link>\n"
			"    <GenerateDebugInformation>%s</GenerateDebugInformation>\n"
			"    <SubSystem>Windows</SubSystem>\n"
			"    <OutputFile>$(OutDir)%s$(TargetExt)</OutputFile>\n"
			"    <ImageHasSafeExceptionHandlers>false</ImageHasSafeExceptionHandlers>\n"
			"  </Link>\n"
			"</ItemDefinitionGroup>\n", proj_configs[i], optimization, runtime_library, proj_outdir_dbl[i], debug_info, target_file
			);
	}

	zt_fileWrite(&file,
		"<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\"/>\n"
		"<ImportGroup Label=\"ExtensionTargets\">\n"
		"</ImportGroup>\n"
		"</Project>\n"
		);

	zt_fileClose(&file);
	zt_fileClose(&ufile);
	return true;
}

// ------------------------------------------------------------------------------------------------

bool sln_createProjects(ztBuildConfig *build_cfg, const char *proj_dir)
{
	if (build_cfg->dll_project) {
		char *files[] = {
			build_cfg->project_name,
			build_cfg->dll_loader_name,
			build_cfg->dll_dll_name,
		};

		zt_fize(files) {
			zt_strMakePrintf(proj_file, 512, "\\build\\msvc\\%s\\%s.vcxproj", files[i], files[i]);

			char proj_full[ztFileMaxPath];
			zt_fileConcatFileToPath(proj_full, ztFileMaxPath, proj_dir, proj_file);

			sln_writeProjectFile(proj_full, (SlnProjFileType_Enum)i, build_cfg);

			zt_strMakePrintf(user_file, 512, "\\build\\msvc\\%s\\%s.vcxproj.user", files[i], files[i]);
		}
	}
	else {
		zt_strMakePrintf(proj_file, 256, "\\build\\msvc\\%s\\%s.vcxproj", build_cfg->project_name, build_cfg->project_name);

		char proj_full[ztFileMaxPath];
		zt_fileConcatFileToPath(proj_full, ztFileMaxPath, proj_dir, proj_file);

		sln_writeProjectFile(proj_full, SlnProjFileType_Standard, build_cfg);
	}

	return true;
}

// ------------------------------------------------------------------------------------------------

bool sln_createSourceFile(ztBuildConfig *build_cfg, const char *proj_dir, const char *src_file)
{
	char *header =
		"/**************************************************************************************************\n"
		" ** file: %s\n"
		" **\n"
		" ** This file was automatically generated.\n"
		" **************************************************************************************************/\n\n";

	char file_name[ztFileMaxPath];
	zt_fileConcatFileToPath(file_name, ztFileMaxPath, proj_dir, src_file);

	zt_logInfo("SLN Build: Generating file: %s", file_name);

	ztFile file;
	zt_fileOpen(&file, file_name, ztFileOpenMethod_WriteOver);

	zt_fileWritef(&file, header, src_file);

	bool need_impls = false;
	bool need_endif = false;

	if (zt_strEquals(src_file, "src\\" SRC_FILE_UNITY_FULL)) {
		if (build_cfg->dll_project) {
			zt_fileWritef(&file,
				"#define ZT_NO_LOADER\n\n"
				"#include \"" SRC_FILE_UNITY_LOADER "\"\n"
				"#include \"" SRC_FILE_UNITY_DLL "\"\n\n"
				);
		}
		else {
			zt_fileWritef(&file,
				"#include \"" SRC_FILE_GAME "\"\n\n"
				"#define ZT_TOOLS_IMPLEMENTATION\n"
				"#include \"zt_tools.h\"\n\n"
				"#define ZT_GAME_IMPLEMENTATION\n"
				"#include \"zt_game.h\"\n\n"
				);

			need_impls = true;
		}

	}
	else if (zt_strEquals(src_file, "src\\" SRC_FILE_UNITY_LOADER)) {
		zt_fileWritef(&file,
			"#if !defined(ZT_LOADER) && !defined(ZT_NO_LOADER)\n"
			"#define ZT_LOADER\n"
			"#endif\n\n"
			"#ifndef ZT_NO_DIRECTX\n"
			"#define ZT_NO_DIRECTX\n"
			"#endif\n\n"
			"//#define ZT_OPENGL_DEBUGGING\n"
			"//#define ZT_NO_PROFILE\n\n"
			"// ================================================================================================================================================================================================\n"
			"// All source files must be included here.\n"
			"// Remove them from compilation if they are added to the project\n\n"
			"#include \"" SRC_FILE_GAME_LOADER "\"\n\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"#define ZT_TOOLS_IMPLEMENTATION\n"
			"#include \"zt_tools.h\"\n\n"
			"#define ZT_GAME_IMPLEMENTATION\n"
			"#include \"zt_game.h\"\n\n"
			);

		need_impls = true;
	}
	else if (zt_strEquals(src_file, "src\\" SRC_FILE_GAME_LOADER)) {
		zt_fileWritef(&file,
			"// headers ========================================================================================================================================================================================\n"
			"\n"
			"#define ZT_GAME_NAME                \"%s\"\n"
			"#define ZT_GAME_LOCAL_ONLY\n"
			"#define ZT_GAME_FUNC_SETTINGS       ldr_gameSettings\n"
			"#define ZT_GAME_FUNC_INIT           ldr_gameInit\n"
			"#define ZT_GAME_FUNC_SCREENCHANGE   ldr_gameScreenChange\n"
			"#define ZT_GAME_FUNC_CLEANUP        ldr_gameCleanup\n"
			"#define ZT_GAME_FUNC_LOOP           ldr_gameLoop\n"
			"\n", build_cfg->project_name
			);

		zt_fileWritef(&file,
			"#include \"zt_tools.h\"\n"
			"#include \"zt_game.h\"\n"
			);

		if (build_cfg->include_gui) {
			zt_fileWritef(&file,
				"#include \"zt_game_gui.h\"\n"
				);
		}

		zt_fileWritef(&file,
			"\n\n"
			"// types/enums/defines ============================================================================================================================================================================\n"
			"\n"
			"#define LDR_DLL_NAME	\"%s.dll\"\n", build_cfg->project_name
			);

		zt_fileWritef(&file,
			"\n"
			"typedef bool (dll_settings_Func    ) (ztGameDetails *, ztGameSettings *);\n"
			"typedef bool (dll_init_Func        ) (ztGameDetails *, ztGameSettings *, void **game_memory);\n"
			"typedef bool (dll_reload_Func      ) (void *);\n"
			"typedef bool (dll_unload_Func      ) (void *);\n"
			"typedef void (dll_cleanup_Func     ) (void *);\n"
			"typedef void (dll_screenChange_Func) (ztGameSettings *, void *);\n"
			"typedef bool (dll_gameLoop_Func    ) (void *, r32);\n"
			"\n"
			"#if !defined(ZT_LOADER)\n"
			"bool dll_settings     (ztGameDetails *, ztGameSettings *);\n"
			"bool dll_init         (ztGameDetails *, ztGameSettings *, void **game_memory);\n"
			"void dll_cleanup      (void *);\n"
			"void dll_screenChange (ztGameSettings *, void *);\n"
			"bool dll_gameLoop     (void *, r32);\n"
			"#endif\n"
			"\n"
			"\n"
			"// structs/classes ================================================================================================================================================================================\n"
			"\n"
			"struct ztDll\n"
			"{\n"
			"	void *game_memory;\n"
			"\n"
			"#	if defined(ZT_LOADER)\n"
			"	HMODULE game_dll;\n"
			"	char    game_dll_name[ztFileMaxPath];\n"\
			"	ztDirectoryMonitor dir_mon;\n"
			"#	endif\n"
			"\n"
			"	dll_settings_Func     *dll_settings;\n"
			"	dll_init_Func         *dll_init;\n"
			"	dll_reload_Func       *dll_reload;\n"
			"	dll_unload_Func       *dll_unload;\n"
			"	dll_cleanup_Func      *dll_cleanup;\n"
			"	dll_screenChange_Func *dll_screenChange;\n"
			"	dll_gameLoop_Func     *dll_gameLoop;\n"
			"\n"
			"	ztGameDetails *details;\n"
			"};\n"
			"\n"
			"\n"
			"// variables ======================================================================================================================================================================================\n"
			"\n"
			"ztInternal ztDll g_dll;\n"
			);

		if (build_cfg->log_file) {
			zt_fileWrite(&file,
				"ztInternal ztFile g_log;\n"
				);
		}

		zt_fileWrite(&file,
			"\n"
			"\n"
			"// private functions ==============================================================================================================================================================================\n"
			"\n"
			"// ================================================================================================================================================================================================\n"
			"\n"
			);

		if (build_cfg->log_file) {
			zt_fileWritef(&file,
				"ZT_FUNC_LOG_CALLBACK(ldr_logCallback)\n"
				"{\n"
				"	zt_fileWrite(&g_log, message, zt_strSize(message) - 1);\n"
				"	zt_fileWrite(&g_log, \"\\n\", 1);\n"
				"}\n"
				"\n"
				"// ================================================================================================================================================================================================\n"
				"\n"
				);
		}

		zt_fileWrite(&file, 
			"void ldr_buildDllCleanup()\n"
			"{\n"
			"#	if defined(ZT_LOADER)\n"
			"	char cleanup_file[ztFileMaxPath];\n"
			"	zt_fileConcatFileToPath(cleanup_file, zt_elementsOf(cleanup_file), g_dll.details->app_path, \"..\\\\..\\\\..\\\\build\\\\msvc\\\\cleanup.bat\");\n"
			"\n"
			"	if (zt_fileExists(cleanup_file)) {\n"
			"		char cleanup_path[ztFileMaxPath];\n"
			"		zt_fileGetFullPath(cleanup_file, cleanup_path, zt_elementsOf(cleanup_path));\n"
			"\n"
			"		char current_path[ztFileMaxPath];\n"
			"		zt_fileGetCurrentPath(current_path, zt_elementsOf(current_path));\n"
			"		zt_fileSetCurrentPath(cleanup_path);\n"
			"\n"
			"		zt_strMakePrintf(cleanup_cmd, ztFileMaxPath, \"cmd.exe /c \\\"%s\\\"\", cleanup_file);\n"
			"		zt_processRun(cleanup_cmd);\n"
			"\n"
			"		zt_fileSetCurrentPath(current_path);\n"
			"	}\n"
			"	else {\n"
			"		zt_logCritical(\"cleanup file not found: %s\", cleanup_file);\n"
			"	}\n"
			"#	endif\n"
			"}\n"
			"\n"
			"// ================================================================================================================================================================================================\n"
			"\n"
			"bool ldr_loadDll(const char *dll_name, bool initial_load)\n"
			"{\n"
			"#	if defined(ZT_LOADER)\n"
			"	if (initial_load) {\n"
			"		ldr_buildDllCleanup();\n"
			"	}\n"
			"\n"
			"	zt_logInfo(\"Loading game DLL: %s\", dll_name);\n"
			"\n"
			"	if (g_dll.game_dll != NULL) {\n"
			"		if (g_dll.dll_unload) {\n"
			"			g_dll.dll_unload(g_dll.game_memory);\n"
			"		}\n"
			"		FreeLibrary(g_dll.game_dll);\n"
			"	}\n"
			"\n"
			"	if (!initial_load) {\n"
			"		zt_sleep(.2f);\n"
			"	}\n"
			"\n"
			"	HMODULE dll = LoadLibraryA(dll_name);\n"
			"\n"
			"	if (dll == NULL) {\n"
			"		for (int i = 0; i < 100 && dll == NULL && GetLastError() == 32; ++i) {\n"
			"			zt_sleep(.1f);\n"
			"			dll = LoadLibraryA(dll_name);\n"
			"		}\n"
			"		if (dll == NULL) {\n"
			"			zt_logCritical(\"Unable to load DLL: %s (%d)\", dll_name, GetLastError());\n"
			"			return false;\n"
			"		}\n"
			"	}\n"
			"\n"
			"	g_dll.game_dll = dll;\n"
			"\n"
			"	zt_dllSetGlobals_Func *zt_dllSetGlobals = (zt_dllSetGlobals_Func *)GetProcAddress(g_dll.game_dll, \"zt_dllSetGlobals\");\n"
			"	if (!zt_dllSetGlobals) {\n"
			"		zt_logCritical(\"Game DLL does not contain a SetGlobals function\");\n"
			"		return false;\n"
			"	}\n"
			"	zt_dllSendGlobals(zt_dllSetGlobals);\n"
			"\n"
			"	zt_dllSetGameGlobals_Func *zt_dllSetGameGlobals = (zt_dllSetGameGlobals_Func *)GetProcAddress(g_dll.game_dll, \"zt_dllSetGameGlobals\");\n"
			"	if (!zt_dllSetGameGlobals) {\n"
			"		zt_logCritical(\"Game DLL does not contain a SetGameGlobals function\");\n"
			"		return false;\n"
			"	}\n"
			"\n"
			"#	if defined(ZT_OPENGL)\n"
			"	zt_dllSetOpenGLGlobals_Func *zt_dllSetOpenGLGlobals = (zt_dllSetOpenGLGlobals_Func *)GetProcAddress(g_dll.game_dll, \"zt_dllSetOpenGLGlobals\");\n"
			"	if (!zt_dllSetOpenGLGlobals) {\n"
			"		zt_logCritical(\"Game DLL does not contain a SetOpenGLGlobals function\");\n"
			"		return false;\n"
			"	}\n"
			"	zt_dllSendGameGlobals(zt_dllSetGameGlobals, zt_dllSetOpenGLGlobals);\n"
			"\n"
			"	if(!initial_load) {\n"
			"		zt_dllSendOpenGLGlobals(zt_dllSetOpenGLGlobals);\n"
			"	}\n"
			"#	else\n"
			"	zt_dllSendGameGlobals(zt_dllSetGameGlobals);\n"
			"#	endif // ZT_OPENGL\n"
			"\n"
			"	zt_dllSetGameGuiGlobals_Func *zt_dllSetGameGuiGlobals = (zt_dllSetGameGuiGlobals_Func *)GetProcAddress(g_dll.game_dll, \"zt_dllSetGameGuiGlobals\");\n"
			"	if (zt_dllSetGameGuiGlobals) {\n"
			"		zt_dllSendGameGuiGlobals(zt_dllSetGameGuiGlobals);\n"
			"	}\n"
			"\n"
			"	g_dll.dll_settings     = (dll_settings_Func     *)GetProcAddress(g_dll.game_dll, \"dll_settings\");\n"
			"	g_dll.dll_init         = (dll_init_Func         *)GetProcAddress(g_dll.game_dll, \"dll_init\");\n"
			"	g_dll.dll_reload       = (dll_reload_Func       *)GetProcAddress(g_dll.game_dll, \"dll_reload\");\n"
			"	g_dll.dll_unload       = (dll_unload_Func       *)GetProcAddress(g_dll.game_dll, \"dll_unload\");\n"
			"	g_dll.dll_cleanup      = (dll_cleanup_Func      *)GetProcAddress(g_dll.game_dll, \"dll_cleanup\");\n"
			"	g_dll.dll_screenChange = (dll_screenChange_Func *)GetProcAddress(g_dll.game_dll, \"dll_screenChange\");\n"
			"	g_dll.dll_gameLoop     = (dll_gameLoop_Func     *)GetProcAddress(g_dll.game_dll, \"dll_gameLoop\");\n"
			"\n"
			"	if (!initial_load && g_dll.dll_reload) {\n"
			"		g_dll.dll_reload(g_dll.game_memory);\n"
			"	}\n"
			"\n"
			"	zt_strCpy(g_dll.game_dll_name, zt_elementsOf(g_dll.game_dll_name), dll_name);\n"
			"\n"
			"	zt_logInfo(\"Game DLL successfully loaded.\");\n"
			"\n"
			"	ldr_buildDllCleanup();\n"
			"\n"
			"#	else\n"
			"	g_dll.dll_settings     = dll_settings;\n"
			"	g_dll.dll_init         = dll_init;\n"
			"	g_dll.dll_cleanup      = dll_cleanup;\n"
			"	g_dll.dll_screenChange = dll_screenChange;\n"
			"	g_dll.dll_gameLoop     = dll_gameLoop;\n"
			"#	endif\n"
			"\n"
			"	return true;\n"
			"}\n"
			"\n"
			"// ================================================================================================================================================================================================\n"
			"\n"
			"void ldr_buildDll()\n"
			"{\n"
			"#	if defined(ZT_LOADER)\n"
			"	zt_logInfo(\"Building game DLL\");\n"
			"\n"
			"#	if defined(ZT_64BIT)\n"
			"	char *build_file_name = zt_debugOnly(\"..\\\\..\\\\..\\\\build\\\\msvc\\\\build_d64.bat\") zt_releaseOnly(\"..\\\\..\\\\..\\\\build\\\\msvc\\\\build_r64.bat\");\n"
			"#	else\n"
			"	char *build_file_name = zt_debugOnly(\"..\\\\..\\\\..\\\\build\\\\msvc\\\\build_d.bat\") zt_releaseOnly(\"..\\\\..\\\\..\\\\build\\\\msvc\\\\build_r.bat\");\n"
			"#	endif\n"
			"\n"
			"	char build_file[ztFileMaxPath];\n"
			"	zt_fileConcatFileToPath(build_file, zt_elementsOf(build_file), g_dll.details->app_path, build_file_name);\n"
			"\n"
			"	if (zt_fileExists(build_file)) {\n"
			"		char build_path[ztFileMaxPath];\n"
			"		zt_fileGetFullPath(build_file, build_path, zt_elementsOf(build_path));\n"
			"\n"
			"		char current_path[ztFileMaxPath];\n"
			"		zt_fileGetCurrentPath(current_path, zt_elementsOf(current_path));\n"
			"		zt_fileSetCurrentPath(build_path);\n"
			"\n"
			"		int size = 1024 * 32;\n"
			"		char *build_results = zt_mallocStructArrayArena(char, size, zt_memGetGlobalArena());\n"
			"\n"
			"		zt_strMakePrintf(build_cmd, ztFileMaxPath, \"cmd.exe /c \\\"\\\"%s\\\" \\\"\"ZT_BUILD_DESTINATION\"\\\"\\\"\", build_file);\n"
			"\n"
			"		zt_processRun(build_cmd, build_results, size);\n"
			"\n"
			"		ztToken lines[100];\n"
			"		int lines_count = zt_strTokenize(build_results, \"\\r\\n\", lines, zt_elementsOf(lines));\n"
			"\n"
			"		zt_fiz(zt_min(lines_count, zt_elementsOf(lines))) {\n"
			"			char line[1024];\n"
			"			zt_strCpy(line, zt_elementsOf(line), zt_strMoveForward(build_results, lines[i].beg), lines[i].len);\n"
			"\n"
			"			if (zt_strFindPos(line, \") : error \", 0) != ztStrPosNotFound) {\n"
			"				zt_logCritical(line);\n"
			"			}\n"
			"			else {\n"
			"				zt_logInfo(line);\n"
			"			}\n"
			"		}\n"
			"\n"
			"		if (lines_count >= zt_elementsOf(lines)) {\n"
			"			zt_logInfo(\"%d more lines...\", lines_count - zt_elementsOf(lines));\n"
			"		}\n"
			"\n"
			"		zt_freeArena(build_results, zt_memGetGlobalArena());\n"
			"\n"
			"		zt_fileSetCurrentPath(current_path);\n"
			"	}\n"
			"	else {\n"
			"		zt_logCritical(\"build file not found: %s\", build_file);\n"
			"	}\n"
			"#	endif\n"
			"}\n"
			"\n"
			"// ================================================================================================================================================================================================\n"
			"\n"
			"#if defined(ZT_LOADER)\n"
			"ZT_FUNCTION_POINTER_REGISTER(ldr_buildDllConsoleCommand, ztInternal ZT_FUNC_DEBUG_CONSOLE_COMMAND(ldr_buildDllConsoleCommand))\n"
			"{\n"
			"	ldr_buildDll();\n"
			"}\n"
			"#endif\n"
			"\n"
			"\n"
			"// functions ======================================================================================================================================================================================\n"
			"\n"
			"bool ldr_gameSettings(ztGameDetails* details, ztGameSettings* settings)\n"
			"{\n"
			);

		if (build_cfg->log_file) {
			zt_fileWrite(&file,
				"	zt_fileOpen(&g_log, \"log.txt\", ztFileOpenMethod_WriteOver);\n"
				"	zt_logAddCallback(ldr_logCallback, ztLogMessageLevel_Verbose);\n"
				"\n"
				);
		}

		zt_fileWrite(&file,
			"	g_dll.details = details;\n"
			"\n"
			"	zt_strMakePrintf(dll_name, ztFileMaxPath, \"%s\\\\\" LDR_DLL_NAME, details->app_path);\n"
			"	if (!ldr_loadDll(dll_name, true)) {\n"
			"		return false;\n"
			"	}\n"
			"\n"
			"	if (g_dll.dll_settings == nullptr || g_dll.dll_init == nullptr) {\n"
			"		zt_logCritical(\"Game DLL does not contain a startup or init function\");\n"
			"		return false;\n"
			"	}\n"
			"\n"
			"	if (!g_dll.dll_settings(details, settings)) {\n"
			"		zt_logCritical(\"Game DLL failed to startup\");\n"
			"		return false;\n"
			"	}\n"
			"	else {\n"
			"		char bytes[128];\n"
			"		zt_strBytesToString(bytes, zt_elementsOf(bytes), settings->memory);\n"
			"		zt_logInfo(\"Game DLL requesting %s of memory\", bytes);\n"
			"	}\n"
			"\n"
			"#	if defined(ZT_LOADER)\n"
			"	zt_directoryMonitor(&g_dll.dir_mon, details->app_path, false, ztDirectoryMonitorFlags_All);\n"
			"#	endif\n"
			"\n"
			"	return true;\n"
			"}\n"
			"\n"
			"// ================================================================================================================================================================================================\n"
			"\n"
			"bool ldr_gameInit(ztGameDetails* details, ztGameSettings* settings)\n"
			"{\n"
			"	zt_guiInitGlobalMemory(zt_memGetGlobalArena());\n"
			"\n"
			"	if (!g_dll.dll_init(details, settings, &g_dll.game_memory)) {\n"
			"		zt_logCritical(\"Game DLL failed to initialize\");\n"
			"		return false;\n"
			"	}\n"
			"\n"
			"#	if defined(ZT_LOADER)\n"
			"	zt_debugConsoleAddCommand(\"build\", \"Compiles the DLL\", ldr_buildDllConsoleCommand_FunctionID, ztInvalidID);\n"
			"#	endif\n"
			"\n"
			"	return true;\n"
			"}\n"
			"\n"
			"// ================================================================================================================================================================================================\n"
			"\n"
			"void ldr_gameCleanup()\n"
			"{\n"
			"	if (g_dll.dll_cleanup) {\n"
			"		g_dll.dll_cleanup(g_dll.game_memory);\n"
			"	}\n"
			"	if (g_dll.dll_unload) {\n"
			"		g_dll.dll_unload(g_dll.game_memory);\n"
			"	}\n"
			"\n"
			"#	if defined(ZT_LOADER)\n"
			"	FreeLibrary(g_dll.game_dll);\n"
			"#	endif\n"
			"\n"
			"	ldr_buildDllCleanup();\n"
			"\n"
			"	g_dll.dll_settings     = nullptr;\n"
			"	g_dll.dll_init         = nullptr;\n"
			"	g_dll.dll_reload       = nullptr;\n"
			"	g_dll.dll_unload       = nullptr;\n"
			"	g_dll.dll_cleanup      = nullptr;\n"
			"	g_dll.dll_screenChange = nullptr;\n"
			"	g_dll.dll_gameLoop     = nullptr;\n"
			"\n"
			"	zt_logRemoveCallback(ldr_logCallback);\n"
			"	zt_fileClose(&g_log);\n"
			"}\n"
			"\n"
			"// ================================================================================================================================================================================================\n"
			"\n"
			"void ldr_gameScreenChange(ztGameSettings *settings)\n"
			"{\n"
			"	if (g_dll.dll_screenChange) {\n"
			"		g_dll.dll_screenChange(settings, g_dll.game_memory);\n"
			"	}\n"
			"}\n"
			"\n"
			"// ================================================================================================================================================================================================\n"
			"\n"
			"bool ldr_gameLoop(r32 dt)\n"
			"{\n"
			"	ztInputKeys *keys = zt_inputKeysAccessState();\n"
			"	if (keys[ztInputKeys_Control].pressed() && keys[ztInputKeys_Menu].pressed() && keys[ztInputKeys_Shift].pressed()) {\n"
			"		if (keys[ztInputKeys_B].justPressed()) {\n"
			"			ldr_buildDll();\n"
			"		}\n"
			"	}\n"
			"\n"
			"#	if defined(ZT_LOADER)\n"
			"	if (zt_directoryMonitorHasChanged(&g_dll.dir_mon)) {\n"
			"		while (zt_directoryMonitorHasChanged(&g_dll.dir_mon)) {\n"
			"			;\n"
			"		}\n"
			"\n"
			"		char file_buffer[1024 * 16];\n"
			"		zt_getDirectoryFiles(g_dll.details->app_path, file_buffer, zt_elementsOf(file_buffer), false);\n"
			"\n"
			"		const char *files = file_buffer;\n"
			"		while (files) {\n"
			"			int end = zt_strFindPos(files, \"\\n\", 0);\n"
			"			if (end == ztStrPosNotFound) end = zt_strLen(files);\n"
			"			if (end == 0) break;\n"
			"\n"
			"			if (zt_striEndsWith(files, end, \".hotload.\" LDR_DLL_NAME, zt_strLen(\".hotload.\" LDR_DLL_NAME))) {\n"
			"				char file[ztFileMaxPath];\n"
			"				zt_strCpy(file, ztFileMaxPath, files, end);\n"
			"\n"
			"				if (!zt_strEquals(file, g_dll.game_dll_name)) {\n"
			"					if (!ldr_loadDll(file, false)) {\n"
			"						zt_logCritical(\"Unable to reload game DLL\");\n"
			"						return false;\n"
			"					}\n"
			"\n"
			"					break;\n"
			"				}\n"
			"			}\n"
			"\n"
			"			files = zt_strMoveForward(files, end + 1);\n"
			"		}\n"
			"	}\n"
			"#	endif\n"
			"\n"
			"	return g_dll.dll_gameLoop(g_dll.game_memory, dt);\n"
			"}\n"
			"\n"
			"// ================================================================================================================================================================================================\n"
			);
	}
	else if (zt_strEquals(src_file, "src\\" SRC_FILE_UNITY_DLL)) {
		zt_fileWritef(&file,
			"#if !defined(ZT_DLL) && !defined(ZT_NO_LOADER)\n"
			"#define ZT_DLL\n"
			"#endif\n\n"
			"#ifndef ZT_NO_DIRECTX\n"
			"#define ZT_NO_DIRECTX\n"
			"#endif\n\n"
			"//#define ZT_OPENGL_DEBUGGING\n"
			"//#define ZT_NO_PROFILE\n\n"
			"// ================================================================================================================================================================================================\n"
			"// All source files must be included here.\n"
			"// Remove them from compilation if they are added to the project\n"
			"// After generation, you may need to edit this file to trigger MSVC's file change detection in the included files\n\n"
			"#include \"" SRC_FILE_GAME_DLL "\"\n"
			"#include \"" SRC_FILE_GAME_SCENE_SPLASH "\"\n"
			"#include \"" SRC_FILE_GAME_SCENE_MENU "\"\n"
			"#include \"" SRC_FILE_GAME_SCENE_MAIN "\"\n"
			);

		if (build_cfg->include_gui) {
			zt_fileWritef(&file,
				"#include \"" SRC_FILE_GUI_THEME "\"\n"
				);
		}

		zt_fileWritef(&file,
			"\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"#ifndef ZT_LOADER\n\n"
			"#define ZT_TOOLS_IMPLEMENTATION\n"
			"#include \"zt_tools.h\"\n\n"
			"#define ZT_GAME_IMPLEMENTATION\n"
			"#include \"zt_game.h\"\n\n"
			);

		need_impls = true;
		need_endif = true;
	}
	else if (zt_strEquals(src_file, "src\\" SRC_FILE_GAME_H)) {
		zt_fileWrite(&file,
			"#ifndef __game_h__\n"
			"#define __game_h__\n"
			"\n"
			"// headers ========================================================================================================================================================================================\n\n"
			"#include \"zt_tools.h\"\n"
			"#include \"zt_game.h\"\n"
			);

		if (build_cfg->include_gui) {
			zt_fileWritef(&file, "#include \"zt_game_gui.h\"\n");
			zt_fileWritef(&file, "#include \"gui_theme.h\"\n");
		}
		if (build_cfg->include_vr) {
			zt_fileWritef(&file, "#include \"zt_vr.h\"\n");
		}

		zt_fileWritef(&file,
			"#include \"game_scene.h\"\n"
			"\n\n"
			"// types/enums/defines ============================================================================================================================================================================\n\n"
			"#if defined(ZT_INPUT_REPLAY)\n"
			"enum ReplayState_Enum\n"
			"{\n"
			"	ReplayState_None,\n"
			"	ReplayState_Recording,\n"
			"	ReplayState_Replaying,\n"
			"	ReplayState_Stepping,\n"
			"	ReplayState_Paused,\n\n"
			"	ReplayState_MAX,\n"
			"};\n"
			"#endif\n"
			"\n"
			"// structs/classes ================================================================================================================================================================================\n\n"
			"// NOTE: do not assign values in the struct definition\n"
			"//       can cause problem with the stack when this struct grows large\n\n"
			"struct ztGame\n"
			"{\n"
			"	ztGameDetails         *details;\n"
			"	ztGameSettings        *settings;\n\n"
			"#	if defined(ZT_INPUT_REPLAY)\n"
			"	ztInputReplayData      replay_data;\n"
			"	bool                   replaying;\n"
			"	ReplayState_Enum       replay_state;\n"
			"#	endif\n\n"
			);

		bool draw_list = false;
		if (build_cfg->camera_2d || build_cfg->include_gui) {
			zt_fileWritef(&file, "	ztCamera               camera_2d;\n");
			draw_list = true;
		}
		if (build_cfg->camera_3d || build_cfg->include_3d_scene) {
			zt_fileWritef(&file, "	ztCamera               camera_3d;\n");
		}
		if (build_cfg->include_gui) {
			zt_fileWritef(&file, "	ztGuiManager          *gui_manager;\n");
			zt_fileWritef(&file, "	ztGuiTheme             gui_theme;\n");
			zt_fileWritef(&file, "	GuiThemeData          *gui_theme_data;\n");
		}
		if (build_cfg->include_vr) {
			zt_fileWritef(&file, "	ztVrSystem            *vr;\n");
		}
		if (build_cfg->default_dirs) {
			zt_fileWritef(&file, "	ztAssetManager         asset_manager;\n");
		}

		if (draw_list) {
			zt_fileWritef(&file, "	ztDrawList             draw_list;\n");
		}

		zt_fileWritef(&file,
			"	GameScene_Enum         game_scene;\n"
			"	GameScene_Enum         game_scene_transition_to;\n"
			"	r32                    game_scene_transition_time;\n"
			"	GameSceneSplash        game_scene_splash;\n"
			"	GameSceneMenu          game_scene_menu;\n"
			"	GameSceneMain          game_scene_main;\n\n"
			"	GameSceneBegin_Func   *gsf_begin  [GameScene_MAX];\n"
			"	GameSceneCleanup_Func *gsf_cleanup[GameScene_MAX];\n"
			"	GameSceneUpdate_Func  *gsf_update [GameScene_MAX];\n"
			"	GameSceneRender_Func  *gsf_render [GameScene_MAX];\n\n"
			"	ztTextureID            render_target;\n\n"
			);

		if (build_cfg->include_pbr) {
			zt_fileWritef(&file,
				"ztTextureID            render_target_attach_position;\n"
				"ztTextureID            render_target_attach_normal;\n\n"
				"ztTextureID            render_target_bright;\n"
				"ztTextureID            render_target_blurred;\n"
				"ztTextureID            texture_random;\n"
				"ztTextureID            render_target_ao;\n\n"
				"ztShaderID             shader_hdr_tonemap;\n"
				"ztShaderID             shader_hdr_bright;\n"
				"ztShaderID             shader_hdr_bloom_blur_1;\n"
				"ztShaderID             shader_hdr_bloom_blur_2;\n"
				"ztShaderID             shader_ao;\n\n"
			);
		}

		zt_fileWritef(&file,
			"	r32                    fade_time;\n"
			"	r32                    fade_time_max;\n\n"
			"	ztTweenManager         tween_manager_state;\n"
			"	ztTweenManager         tween_manager_permanent;\n"
			"};\n\n\n"
			"// external variables =============================================================================================================================================================================\n\n"
			"// function prototypes ============================================================================================================================================================================\n\n"
			);

		if (build_cfg->camera_3d || build_cfg->include_3d_scene) {
			zt_fileWritef(&file,
				"void gameAdjustCamera(ztGame *game);\n\n\n"
				);
		}

		zt_fileWritef(&file,
			"// inline functions ===============================================================================================================================================================================\n\n"
			"#endif // include guard\n"
			);
	}
	else if (zt_strEquals(src_file, "src\\" SRC_FILE_GAME_SCENE_H)) {
		zt_fileWrite(&file,
			"#ifndef __game_scene_h__\n"
			"#define __game_scene_h__\n"
			"\n"
			"// headers ========================================================================================================================================================================================\n\n"
			"#include \"zt_game.h\"\n\n"
			"// types/enums/defines ============================================================================================================================================================================\n\n"
			"enum GameScene_Enum\n"
			"{\n"
			"	GameScene_Invalid,\n"
			"	GameScene_SplashLoading,\n"
			"	GameScene_Splash,\n"
			"	GameScene_Menu,\n"
			"	GameScene_MainLoading,\n"
			"	GameScene_Main,\n\n"
			"	GameScene_MAX,\n"
			"};\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"struct ztGame;\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"#define FUNC_GAME_SCENE_BEGIN(name)	    bool name(ztGame *game, GameScene_Enum transitioning_from)\n"
			"typedef FUNC_GAME_SCENE_BEGIN(GameSceneBegin_Func);\n"
			"\n"
			"#define FUNC_GAME_SCENE_CLEANUP(name)	void name(ztGame *game, GameScene_Enum transitioning_to)\n"
			"typedef FUNC_GAME_SCENE_CLEANUP(GameSceneCleanup_Func);\n"
			"\n"
			"#define FUNC_GAME_SCENE_UPDATE(name)	bool name(ztGame *game, r32 dt, bool gui_input, bool input_this_frame, ztInputKeys *input_keys, ztInputController *input_controller, ztInputMouse *input_mouse)\n"
			"typedef FUNC_GAME_SCENE_UPDATE(GameSceneUpdate_Func);\n"
			"\n"
			"#define FUNC_GAME_SCENE_RENDER(name)	void name(ztGame *game, ztTextureID final_render_target)\n"
			"typedef FUNC_GAME_SCENE_RENDER(GameSceneRender_Func);\n\n"
			"// ================================================================================================================================================================================================\n"
			"// ================================================================================================================================================================================================\n"
			"// ================================================================================================================================================================================================\n\n"
			"enum GameSceneSplashLoad_Enum\n"
			"{\n"
			"	GameSceneSplashLoad_Scene,\n\n"
			"	GameSceneSplashLoad_MAX,\n"
			"};\n\n"
			"struct GameSceneSplashLoad\n"
			"{\n"
			"	ztLoadInfo                     load_info[GameSceneSplashLoad_MAX];\n"
			"	int                            load_idx;\n"
			"	r32                            load_time;\n"
			"\n"
			"	// place variables required for loading here...\n"
			"};\n\n"
			"\n"
			"// ================================================================================================================================================================================================\n\n"
			"struct GameSceneSplash\n"
			"{\n"
			"	GameSceneSplashLoad load;\n"
			"	r32                 time;\n"
			"};\n\n"
			"\n"
			"FUNC_GAME_SCENE_BEGIN   (gameSceneSplashLoadingBegin);\n"
			"FUNC_GAME_SCENE_CLEANUP (gameSceneSplashLoadingCleanup);\n"
			"FUNC_GAME_SCENE_UPDATE  (gameSceneSplashLoadingUpdate);\n"
			"FUNC_GAME_SCENE_RENDER  (gameSceneSplashLoadingRender);\n"
			"\n"
			"FUNC_GAME_SCENE_BEGIN   (gameSceneSplashBegin);\n"
			"FUNC_GAME_SCENE_CLEANUP (gameSceneSplashCleanup);\n"
			"FUNC_GAME_SCENE_UPDATE  (gameSceneSplashUpdate);\n"
			"FUNC_GAME_SCENE_RENDER  (gameSceneSplashRender);\n"
			"\n"
			"// ================================================================================================================================================================================================\n"
			"// ================================================================================================================================================================================================\n"
			"// ================================================================================================================================================================================================\n\n"
			"struct GameSceneMenu\n"
			"{\n"
			"};\n\n"
			"\n"
			"FUNC_GAME_SCENE_BEGIN   (gameSceneMenuBegin);\n"
			"FUNC_GAME_SCENE_CLEANUP (gameSceneMenuCleanup);\n"
			"FUNC_GAME_SCENE_UPDATE  (gameSceneMenuUpdate);\n"
			"FUNC_GAME_SCENE_RENDER  (gameSceneMenuRender);\n"
			"\n"
			"// ================================================================================================================================================================================================\n"
			"// ================================================================================================================================================================================================\n"
			"// ================================================================================================================================================================================================\n\n"
			"enum GameSceneMainLoad_Enum\n"
			"{\n"
			"	GameSceneMainLoad_Scene,\n\n"
			"	GameSceneMainLoad_MAX,\n"
			"};\n\n"
			"struct GameSceneMainLoad\n"
			"{\n"
			"	ztLoadInfo                     load_info[GameSceneMainLoad_MAX];\n"
			"	int                            load_idx;\n"
			"	r32                            load_time;\n"
			"\n"
			"	// place variables required for loading here...\n"
			"};\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"struct GameSceneMain\n"
			"{\n"
			"	ztGame                        *game;\n"
			"	GameSceneMainLoad              load;\n"
			);

		if (build_cfg->include_3d_scene) {
			zt_fileWritef(&file,
				"\n	ztScene                       *scene;\n"
				"	ztLight                        scene_light;\n"
				"	ztCameraControllerFPS          camera_controller;\n"
				"	ztModel                        models[128];\n"
				"	int                            models_used;\n"
				"	ztLight                        lights[9];\n"
				"	int                            lights_used;\n"
			);
		}

		zt_fileWritef(&file,
			"};\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"FUNC_GAME_SCENE_BEGIN   (gameSceneMainLoadingBegin);\n"
			"FUNC_GAME_SCENE_CLEANUP (gameSceneMainLoadingCleanup);\n"
			"FUNC_GAME_SCENE_UPDATE  (gameSceneMainLoadingUpdate);\n"
			"FUNC_GAME_SCENE_RENDER  (gameSceneMainLoadingRender);\n"
			"\n"
			"FUNC_GAME_SCENE_BEGIN   (gameSceneMainBegin);\n"
			"FUNC_GAME_SCENE_CLEANUP (gameSceneMainCleanup);\n"
			"FUNC_GAME_SCENE_UPDATE  (gameSceneMainUpdate);\n"
			"FUNC_GAME_SCENE_RENDER  (gameSceneMainRender);\n"
			"\n"
			"// used by serialization functions using ztLoadState.  allows for easy error handling while checking every read/write call\n"
			"// there must be an on_error label that sets the ztLoadState state to error\n"
			"#define _serialValidate(system, cond, saving)           if(!(cond)) { zt_logCritical(\"serial: %%s : Unable to %%s file(%%d)\", system, ((saving) ? \"write to\" : \"read from\"), __LINE__); goto on_error; }\n"
			"#define _serialValidateMsg(system, cond, error)         if(!(cond)) { zt_logCritical(\"serial: %%s: %%s (%%d)\", system, error, __LINE__); goto on_error; }\n\n"
			"// ================================================================================================================================================================================================\n"
			"// ================================================================================================================================================================================================\n"
			"// ================================================================================================================================================================================================\n\n"
			"#endif // include guard\n"
			);
	}
	else if (zt_strEquals(src_file, "src\\" SRC_FILE_GAME_SCENE_MAIN)) {
		zt_fileWritef(&file,
			"// headers ========================================================================================================================================================================================\n\n"
			"#include \"game.h\"\n\n"
			"// variables ======================================================================================================================================================================================\n\n"
			"// private functions ==============================================================================================================================================================================\n\n"
			"ztInternal void _gameSceneMainDoLoad(ztGame *game, GameSceneMain *gs, int idx)\n"
			"{\n"
			"	switch (idx)\n"
			"	{\n"
			"		case GameSceneMainLoad_Scene: {\n"
			"			if (gs->load.load_info[idx].state == ztLoadState_Query) {\n"
			"				gs->load.load_info[idx].total_steps  = 2;\n"
			"				gs->load.load_info[idx].current_step = 0;\n"
			"				gs->load.load_info[idx].state        = ztLoadState_Loading;\n"
			"			}\n"
			"			else if (gs->load.load_info[idx].state == ztLoadState_Loading) {\n"
			"				gs->game = game;\n\n"
			"				switch (gs->load.load_info[idx].current_step++)\n"
			"				{\n"
			"					case 0: {\n"
			);

		if (build_cfg->include_3d_scene) {
			if (build_cfg->include_pbr) {
				zt_fileWritef(&file,
					"						gs->scene = zt_sceneMake(zt_memGetGlobalArena());\n"
					"						{\n"
					"							ztShaderPhysicallyBasedRenderingSettings settings = {};\n"
					"							ztShaderID shader_pbr = zt_shaderMakePhysicallyBasedRendering(&settings);\n\n"
					"							// environment maps\n"
					"							{\n"
					"								ztTextureID tex = zt_textureMakeCubeMapFromHDR(&game->asset_manager, zt_assetLoad(&game->asset_manager, \"textures/environment.hdr\"), 1024, 1024);\n"
					"								if (tex) {\n"
					"									gs->scene->tex_irradiance_map = zt_textureMakeIrradianceCubeMapFromCubeMap(tex);\n"
					"									gs->scene->tex_prefilter_map = zt_textureMakePrefilterCubeMapFromCubeMap(tex);\n\n"
					"									ztModel *model_skybox = &gs->models[gs->models_used++];\n"
					"									if (zt_modelMakeSkybox(model_skybox, tex, true)) {\n"
					"										zt_sceneSetSkybox(gs->scene, model_skybox);\n"
					"									}\n"
					"								}\n\n"
					"								gs->scene->tex_brdf_lut = zt_textureMakeBidirectionalReflectanceDistributionFunctionLUT(512, 512);\n"
					"							}\n\n"
					"							// models\n"
					"							{\n"
					"								ztMaterial statue_mat = zt_materialMake();\n"
					"								ztMeshID statue_mesh;\n\n"
					"								if (zt_meshLoadOBJ(&game->asset_manager, zt_assetLoad(&game->asset_manager, \"models/pbr_test_statue.obj\"), &statue_mesh, &statue_mat, 1, ztVec3::one * .5f) != 1) {\n"
					"									gs->load.load_info[idx].state = ztLoadState_Error;\n"
					"									break;\n"
					"								}\n\n"
					"								statue_mat = zt_materialMake(\n"
					"								zt_textureMake(&game->asset_manager, zt_assetLoad(&game->asset_manager, \"models/pbr_test_statue_albedo.png\"), ztTextureFlags_MipMaps), ztColor_White, ztMaterialFlags_OwnsTexture,\n"
					"								zt_textureMake(&game->asset_manager, zt_assetLoad(&game->asset_manager, \"models/pbr_test_statue_metallic.png\"), ztTextureFlags_MipMaps), ztColor_White, ztMaterialFlags_OwnsTexture,\n"
					"								zt_textureMake(&game->asset_manager, zt_assetLoad(&game->asset_manager, \"models/pbr_test_statue_normal.png\"), ztTextureFlags_MipMaps), ztMaterialFlags_OwnsTexture,\n"
					"								zt_textureMake(&game->asset_manager, zt_assetLoad(&game->asset_manager, \"models/pbr_test_statue_height.png\"), ztTextureFlags_MipMaps), ztMaterialFlags_OwnsTexture,\n"
					"								zt_textureMake(&game->asset_manager, zt_assetLoad(&game->asset_manager, \"models/pbr_test_statue_roughness.png\"), ztTextureFlags_MipMaps), ztMaterialFlags_OwnsTexture);\n\n"
					"								ztModel *statue_model = &gs->models[gs->models_used++];\n"
					"								zt_modelMakeFromMesh(statue_model, statue_mesh, &statue_mat, shader_pbr, nullptr, ztModelFlags_CastsShadows);\n"
					"								statue_model->aabb_center = ztVec3::zero;\n"
					"								statue_model->aabb_size = zt_vec3(1, .1f, 1);\n"
					"								statue_model->transform.rotation = ztQuat::makeFromEuler(0, 180, 0);\n"
					"								zt_sceneAddModel(gs->scene, statue_model);\n"
					"							}\n"
					"							{\n"
					"								ztMaterial floor_mat = zt_materialMake();\n"
					"								ztMeshID floor_panel = zt_meshMakePrimitivePlane(3, 3, 3, 3);\n\n"
					"								floor_mat = zt_materialMake(\n"
					"									zt_textureMake(&game->asset_manager, zt_assetLoad(&game->asset_manager, \"models/floor_panel_albedo.png\"), ztTextureFlags_MipMaps), ztColor_White, ztMaterialFlags_OwnsTexture,\n"
					"									zt_textureMake(&game->asset_manager, zt_assetLoad(&game->asset_manager, \"models/floor_panel_metallic.png\"), ztTextureFlags_MipMaps), ztColor_White, ztMaterialFlags_OwnsTexture,\n"
					"									zt_textureMake(&game->asset_manager, zt_assetLoad(&game->asset_manager, \"models/floor_panel_normal.png\"), ztTextureFlags_MipMaps), ztMaterialFlags_OwnsTexture,\n"
					"									zt_textureMake(&game->asset_manager, zt_assetLoad(&game->asset_manager, \"models/floor_panel_height.png\"), ztTextureFlags_MipMaps), ztMaterialFlags_OwnsTexture,\n"
					"									zt_textureMake(&game->asset_manager, zt_assetLoad(&game->asset_manager, \"models/floor_panel_roughness.png\"), ztTextureFlags_MipMaps), ztMaterialFlags_OwnsTexture);\n\n"
					"								ztModel *floor_model = &gs->models[gs->models_used++];\n"
					"								zt_modelMakeFromMesh(floor_model, floor_panel, &floor_mat, shader_pbr, nullptr, ztModelFlags_OwnsMaterials | ztModelFlags_OwnsMesh);\n"
					"								floor_model->aabb_center = ztVec3::zero;\n"
					"								floor_model->aabb_size = zt_vec3(3, .1f, 3);\n"
					"								floor_model->transform.position.y = .01f;\n"
					"								zt_sceneAddModel(gs->scene, floor_model);\n"
					"							}\n\n"
					"							// lights\n"
					"							{\n"
					"								ztLight *light = &gs->lights[gs->lights_used++];\n"
					"								*light = zt_lightMakeDirectional(zt_vec3(3, 10, 3), ztVec3::zero, 1, 0.25f);\n"
					"								zt_sceneAddLight(gs->scene, light);\n\n"
					"								light = &gs->lights[gs->lights_used++];\n"
					"								*light = zt_lightMakeArea(zt_vec3(-3, 5, -3), 1.f, true, ztColor_Cyan);\n"
					"								zt_sceneAddLight(gs->scene, light);\n"
					"							}\n"
					"						}\n"
					"						gs->camera_controller = zt_cameraControllerMakeFPS(&game->camera_3d);\n"
					);
			}
			else {
				zt_fileWritef(&file,
					"						gs->scene = zt_sceneMake(zt_memGetGlobalArena());\n"
					"						{\n"
					"							ztMeshID   cube = zt_meshMakePrimitiveBox(1, 1, 1);\n"
					"							ztMaterial cube_mat = zt_materialMake(ztInvalidID, zt_vec4(1, 1, 1, 1));\n"
					"							ztModel *cube_model = &gs->models[gs->models_used++];\n"
					"							zt_modelMakeFromMesh(cube_model, cube, &cube_mat, zt_shaderGetDefault(ztShaderDefault_LitShadow), nullptr, ztModelFlags_CastsShadows | ztModelFlags_OwnsMaterials | ztModelFlags_OwnsMesh);\n"
					"							cube_model->transform.position = zt_vec3(0, .5f, 0);\n"
					"							zt_sceneAddModel(gs->scene, cube_model);\n"
					"							gs->scene_light = zt_lightMakeDirectional(zt_vec3(5, 10, 10), ztVec3::zero, 1, 0.25f);\n"
					"							zt_sceneAddLight(gs->scene, &gs->scene_light);\n"
					"						}\n"
					"						gs->camera_controller = zt_cameraControllerMakeFPS(&game->camera_3d);\n"
					);
			}
		}

		zt_fileWritef(&file,
			"					} break;\n\n"
			"					default: {\n"
			"						gs->load.load_info[idx].state = ztLoadState_Complete;\n"
			"					} break;\n"
			"				}\n"
			"			}\n"
			"		} break;\n\n"
			"		default: zt_assert(false);\n"
			"	}\n"
			"}\n\n"
			"// functions ======================================================================================================================================================================================\n\n"
			"FUNC_GAME_SCENE_BEGIN(gameSceneMainLoadingBegin)\n"
			"{\n"
			"	ZT_PROFILE_GAME(\"gameSceneMainLoadingBegin\");\n\n"
			"	GameSceneMain *gs = &game->game_scene_main;\n\n"
			"	zt_fize(gs->load.load_info) {\n"
			"		gs->load.load_info[i] = {};\n"
			"		_gameSceneMainDoLoad(game, gs, i);\n"
			"	}\n\n"
			"	gs->load.load_time = 0;\n"
			"	gs->load.load_idx  = 0;\n\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"FUNC_GAME_SCENE_CLEANUP(gameSceneMainLoadingCleanup)\n"
			"{\n"
			"	GameSceneMain *gs = &game->game_scene_main;\n"
			"	// cleanup here\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"FUNC_GAME_SCENE_UPDATE(gameSceneMainLoadingUpdate)\n"
			"{\n"
			"	ZT_PROFILE_GAME(\"gameSceneMainLoadingUpdate\");\n\n"
			"	GameSceneMain *gs = &game->game_scene_main;\n\n"
			"	if (game->game_scene_transition_to != GameScene_Invalid || game->fade_time > 0) {\n"
			"			return true;\n"
			"	}\n"
			"\n"
			"	gs->load.load_time += dt;\n\n"
			"	if (gs->load.load_idx >= zt_elementsOf(gs->load.load_info)) {\n"
			"		if (gs->load.load_time > .025f) {\n"
			"			gameTransitionTo(game, GameScene_Main, false);\n"
			"		}\n"
			"		return true;\n"
			"	}\n\n"
			"	_gameSceneMainDoLoad(game, gs, gs->load.load_idx);\n\n"
			"	if (gs->load.load_info[gs->load.load_idx].state == ztLoadState_Complete) {\n"
			"		if (++gs->load.load_idx >= zt_elementsOf(gs->load.load_info)) {\n"
			"			gs->load.load_time = 0;\n"
			"		}\n"
			"	}\n"
			"	else if (gs->load.load_info[gs->load.load_idx].state == ztLoadState_Error) {\n"
			"		if (gs->load.load_time > 3) {\n"
			"			return false;\n"
			"		}\n"
			"	}\n\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"FUNC_GAME_SCENE_RENDER(gameSceneMainLoadingRender)\n"
			"{\n"
			"	ZT_PROFILE_GAME(\"gameSceneMainLoadingRender\");\n\n"
			"	GameSceneMain *gs = &game->game_scene_main;\n\n"
			"	zt_drawListPushShader(&game->draw_list, zt_shaderGetDefault(ztShaderDefault_Unlit));\n"
			"	{\n"
			"		if (gs->load.load_idx < zt_elementsOf(gs->load.load_info) && gs->load.load_info[gs->load.load_idx].state == ztLoadState_Error) {\n"
			"			zt_drawListAddFancyText2D(&game->draw_list, ztFontDefault, \"Error loading resources\", ztVec2::zero, ztAlign_Center, ztAnchor_Center, nullptr, ztColor_Black);\n"
			"		}\n"
			"		else {\n"
			"			zt_drawListAddFancyText2D(&game->draw_list, ztFontDefault, \"Loading\", ztVec2::zero, ztAlign_Center, ztAnchor_Center, nullptr, ztColor_Black);\n"
			"			int total = 0, current = 0;\n\n"
			"			zt_fize(gs->load.load_info) {\n"
			"				total += gs->load.load_info[i].total_steps;\n"
			"				current += gs->load.load_info[i].current_step;\n"
			"			}\n\n"
			"			r32 pct = (r32)current / total;\n"
			"			r32 pix = 1 / zt_pixelsPerUnit();\n\n"
			"			ztVec2 load_ttl = zt_vec2(6, .25f);\n"
			"			ztVec2 load_pct = zt_vec2(6 * pct, .25f);\n\n"
			"			r32 diff = load_ttl.x - load_pct.x;\n"
			"			load_ttl.x += pix * 8;\n"
			"			load_ttl.y += pix * 8;\n\n"
			"			zt_drawListPushTexture(&game->draw_list, 0);\n"
			"			{\n"
			"				zt_drawListAddSolidRect2D(&game->draw_list, zt_vec3(0, -.5f, 0), load_ttl, ztColor_Black);\n"
			"				zt_drawListAddSolidRect2D(&game->draw_list, zt_vec3(-diff / 2, -.5f, 0), load_pct, ztColor_White);\n"
			"			}\n"
			"			zt_drawListPopTexture(&game->draw_list);\n"
			"		}\n"
			"	}\n"
			"	zt_drawListPopShader(&game->draw_list);\n\n"
			"	zt_renderDrawList(&game->camera_2d, &game->draw_list, ztColor_White, ztRenderDrawListFlags_NoDepthTest, final_render_target);\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n"
			"// ================================================================================================================================================================================================\n"
			"// ================================================================================================================================================================================================\n\n"
			"FUNC_GAME_SCENE_BEGIN(gameSceneMainBegin)\n"
			"{\n"
			"	GameSceneMain *gs = &game->game_scene_main;\n\n"
			"	//zt_profilerPause(); // uncomment to profile startup\n\n"
			);

		if (build_cfg->include_3d_scene) {
			zt_fileWritef(&file,
				"	zt_inputMouseLook(true);\n"
				"	zt_inputMouseSetCursor(ztInputMouseCursor_None);\n"
				);
		}

		zt_fileWritef(&file,
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"FUNC_GAME_SCENE_CLEANUP(gameSceneMainCleanup)\n"
			"{\n"
			"	GameSceneMain *gs = &game->game_scene_main;\n\n"
			);

		if (build_cfg->include_3d_scene) {
			zt_fileWritef(&file,
				"	zt_sceneFree(gs->scene);\n\n"
				"	zt_fiz(gs->models_used) {\n"
				"		zt_modelFree(&gs->models[i]);\n"
				"	}\n"
				"	gs->models_used = 0;\n\n"
				"	gs->scene = nullptr;\n"
				);
		}

		zt_fileWritef(&file,
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"FUNC_GAME_SCENE_UPDATE(gameSceneMainUpdate)\n"
			"{\n"
			"	ZT_PROFILE_GAME(\"gameSceneMainUpdate\");\n\n"
			"	GameSceneMain *gs = &game->game_scene_main;\n\n"
			);

		if (build_cfg->include_3d_scene) {
			zt_fileWritef(&file,
				"	if (input_keys[ztInputKeys_Escape].justPressed()) {\n"
				"		if (zt_inputMouseIsLook()) {\n"
				"			zt_inputMouseLook(false);\n"
				"			zt_inputMouseSetCursor(ztInputMouseCursor_Arrow);\n"
				"		}\n"
				"		else {\n"
				"			zt_inputMouseLook(true);\n"
				"			zt_inputMouseSetCursor(ztInputMouseCursor_None);\n"
				"		}\n"
				"	}\n\n"
				"	if (input_keys[ztInputKeys_Control].pressed() && input_keys[ztInputKeys_Shift].pressed() && input_keys[ztInputKeys_Menu].pressed()) {\n"
				"		// put special debugging stuff here\n"
				"	}\n\n"
				"	if (zt_inputMouseIsLook()) {\n"
				"		zt_cameraControlUpdateWASD(&gs->camera_controller, input_mouse, input_keys, dt);\n"
				"	}\n\n"
				);
		}

		zt_fileWritef(&file,
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"FUNC_GAME_SCENE_RENDER(gameSceneMainRender)\n"
			"{\n"
			"	ZT_PROFILE_GAME(\"gameSceneMainRender\");\n\n"
			"	GameSceneMain *gs = &game->game_scene_main;\n\n"
			);

		if (build_cfg->include_3d_scene) {
			zt_fileWritef(&file,
				"	zt_scenePrepare(gs->scene, &game->camera_3d);\n"
				"	zt_sceneOptimize(gs->scene, &game->camera_3d);\n"
				"	zt_sceneLighting(gs->scene, &game->camera_3d);\n\n"
				"	zt_textureRenderTargetPrepare(final_render_target);\n"
				"	{\n"
				"		zt_rendererClear(zt_vec4(.4f, .4f, .4f, 1));\n"
				"		zt_rendererSetDepthTest(true, ztRendererDepthTestFunction_LessEqual);\n\n"
				"		zt_sceneRender(gs->scene, &game->camera_3d);\n\n"
				"		{\n"
				"			zt_drawListPushShader(&game->draw_list, zt_shaderGetDefault(ztShaderDefault_Unlit));\n"
				"			zt_drawListPushTexture(&game->draw_list, ztTextureDefault);\n\n"
				"			zt_drawListAddAxis(&game->draw_list, .0075f, game->camera_3d.position + game->camera_3d.direction * .125f);\n"
				"			zt_drawListAddFloorGrid(&game->draw_list, ztVec3::zero, 10, 10);\n"
				"			zt_drawListPopTexture(&game->draw_list);\n"
				"			zt_drawListPopShader(&game->draw_list);\n\n"
				"			zt_renderDrawList(&game->camera_3d, &game->draw_list, ztVec4::zero, ztRenderDrawListFlags_NoClear);\n"
				"		}\n"
				"	}\n"
				"	zt_textureRenderTargetCommit(final_render_target);\n"
				);
		}
		else {
			zt_fileWritef(&file,
				"	zt_textureRenderTargetPrepare(final_render_target);\n"
				"	{\n"
				"		zt_rendererClear(zt_vec4(.4f, .4f, .4f, 1));\n"
				"		// render scene\n"
				"	}\n"
				"	zt_textureRenderTargetCommit(final_render_target);\n"
				);

		}

		zt_fileWritef(&file,
			"}\n\n"
			"// ================================================================================================================================================================================================\n"
			);
	}
	else if (zt_strEquals(src_file, "src\\" SRC_FILE_GAME_SCENE_SPLASH)) {
		zt_fileWritef(&file,
			"// headers ========================================================================================================================================================================================\n\n"
			"#include \"game.h\"\n\n"
			"// variables ======================================================================================================================================================================================\n\n"
			"#define SPLASH_SCREEN_TIME       .05f\n\n"
			"// private functions ==============================================================================================================================================================================\n\n"
			"ztInternal void _gameStatSplashDoLoad(ztGame *game, GameSceneSplash *gs, int idx)\n"
			"{\n"
			"	switch (idx)\n"
			"	{\n"
			"		case GameSceneSplashLoad_Scene: {\n"
			"			if (gs->load.load_info[idx].state == ztLoadState_Query) {\n"
			"				gs->load.load_info[idx].total_steps  = 2;\n"
			"				gs->load.load_info[idx].current_step = 0;\n"
			"				gs->load.load_info[idx].state        = ztLoadState_Loading;\n"
			"			}\n"
			"			else if (gs->load.load_info[idx].state == ztLoadState_Loading) {\n"
			"				switch (gs->load.load_info[idx].current_step++)\n"
			"				{\n"
			"					case 0: {\n"
			"					} break;\n\n"
			"					default: {\n"
			"						gs->load.load_info[idx].state = ztLoadState_Complete;\n"
			"					} break;\n"
			"				}\n"
			"			}\n"
			"		} break;\n\n"
			"		default: zt_assert(false);\n"
			"	}\n"
			"}\n\n"
			"// functions ======================================================================================================================================================================================\n\n"
			"FUNC_GAME_SCENE_BEGIN(gameSceneSplashLoadingBegin)\n"
			"{\n"
			"	ZT_PROFILE_GAME(\"GameSceneSplashLoadingBegin\");\n\n"
			"	GameSceneSplash *gs = &game->game_scene_splash;\n\n"
			"	zt_fize(gs->load.load_info) {\n"
			"		gs->load.load_info[i] = {};\n"
			"		_gameStatSplashDoLoad(game, gs, i);\n"
			"	}\n\n"
			"	gs->load.load_time = 0;\n"
			"	gs->load.load_idx  = 0;\n\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"FUNC_GAME_SCENE_CLEANUP(gameSceneSplashLoadingCleanup)\n"
			"{\n"
			"	GameSceneSplash *gs = &game->game_scene_splash;\n"
			"	// cleanup here\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"FUNC_GAME_SCENE_UPDATE(gameSceneSplashLoadingUpdate)\n"
			"{\n"
			"	ZT_PROFILE_GAME(\"GameSceneSplashLoadingUpdate\");\n\n"
			"	GameSceneSplash *gs = &game->game_scene_splash;\n\n"
			"	if (game->game_scene_transition_to != GameScene_Invalid || game->fade_time > 0) {\n"
			"			return true;\n"
			"	}\n"
			"\n"
			"	gs->load.load_time += dt;\n\n"
			"	if (gs->load.load_idx >= zt_elementsOf(gs->load.load_info)) {\n"
			"		if (gs->load.load_time > .025f) {\n"
			"			gameTransitionTo(game, GameScene_Splash, false);\n"
			"		}\n"
			"		return true;\n"
			"	}\n\n"
			"	_gameStatSplashDoLoad(game, gs, gs->load.load_idx);\n\n"
			"	if (gs->load.load_info[gs->load.load_idx].state == ztLoadState_Complete) {\n"
			"		if (++gs->load.load_idx >= zt_elementsOf(gs->load.load_info)) {\n"
			"			gs->load.load_time = 0;\n"
			"		}\n"
			"	}\n"
			"	else if (gs->load.load_info[gs->load.load_idx].state == ztLoadState_Error) {\n"
			"		if (gs->load.load_time > 3) {\n"
			"			return false;\n"
			"		}\n"
			"	}\n\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"FUNC_GAME_SCENE_RENDER(gameSceneSplashLoadingRender)\n"
			"{\n"
			"	ZT_PROFILE_GAME(\"GameSceneSplashLoadingRender\");\n\n"
			"	GameSceneSplash *gs = &game->game_scene_splash;\n\n"
			"	zt_drawListPushShader(&game->draw_list, zt_shaderGetDefault(ztShaderDefault_Unlit));\n"
			"	{\n"
			"		if (gs->load.load_idx < zt_elementsOf(gs->load.load_info) && gs->load.load_info[gs->load.load_idx].state == ztLoadState_Error) {\n"
			"			zt_drawListAddFancyText2D(&game->draw_list, ztFontDefault, \"Error loading resources\", ztVec2::zero, ztAlign_Center, ztAnchor_Center, nullptr, ztColor_Black);\n"
			"		}\n"
			"		else {\n"
			"			zt_drawListAddFancyText2D(&game->draw_list, ztFontDefault, \"Loading\", ztVec2::zero, ztAlign_Center, ztAnchor_Center, nullptr, ztColor_Black);\n"
			"			int total = 0, current = 0;\n\n"
			"			zt_fize(gs->load.load_info) {\n"
			"				total += gs->load.load_info[i].total_steps;\n"
			"				current += gs->load.load_info[i].current_step;\n"
			"			}\n\n"
			"			r32 pct = (r32)current / total;\n"
			"			r32 pix = 1 / zt_pixelsPerUnit();\n\n"
			"			ztVec2 load_ttl = zt_vec2(6, .25f);\n"
			"			ztVec2 load_pct = zt_vec2(6 * pct, .25f);\n\n"
			"			r32 diff = load_ttl.x - load_pct.x;\n"
			"			load_ttl.x += pix * 8;\n"
			"			load_ttl.y += pix * 8;\n\n"
			"			zt_drawListPushTexture(&game->draw_list, 0);\n"
			"			{\n"
			"				zt_drawListAddSolidRect2D(&game->draw_list, zt_vec3(0, -.5f, 0), load_ttl, ztColor_Black);\n"
			"				zt_drawListAddSolidRect2D(&game->draw_list, zt_vec3(-diff / 2, -.5f, 0), load_pct, ztColor_White);\n"
			"			}\n"
			"			zt_drawListPopTexture(&game->draw_list);\n"
			"		}\n"
			"	}\n"
			"	zt_drawListPopShader(&game->draw_list);\n\n"
			"	zt_renderDrawList(&game->camera_2d, &game->draw_list, ztColor_White, ztRenderDrawListFlags_NoDepthTest, final_render_target);\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n"
			"// ================================================================================================================================================================================================\n"
			"// ================================================================================================================================================================================================\n\n"
			"FUNC_GAME_SCENE_BEGIN(gameSceneSplashBegin)\n"
			"{\n"
			"	GameSceneSplash *gs = &game->game_scene_splash;\n\n"
			"	//zt_profilerPause(); // uncomment to profile startup\n\n"
			"	gs->time = 0;\n\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"FUNC_GAME_SCENE_CLEANUP(gameSceneSplashCleanup)\n"
			"{\n"
			"	GameSceneSplash *gs = &game->game_scene_splash;\n\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"FUNC_GAME_SCENE_UPDATE(gameSceneSplashUpdate)\n"
			"{\n"
			"	ZT_PROFILE_GAME(\"GameSceneSplashUpdate\");\n\n"
			"	GameSceneSplash *gs = &game->game_scene_splash;\n\n"
			"	gs->time += dt;\n"
			"	if (gs->time >= SPLASH_SCREEN_TIME && game->game_scene_transition_to == GameScene_Invalid) {\n"
			"		//gameTransitionTo(game, GameScene_Menu, false);\n"
			"		gameTransitionTo(game, GameScene_MainLoading, false);\n"
			"	}\n\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"FUNC_GAME_SCENE_RENDER(gameSceneSplashRender)\n"
			"{\n"
			"	ZT_PROFILE_GAME(\"gameSceneMainRender\");\n\n"
			"	GameSceneSplash *gs = &game->game_scene_splash;\n\n"
			"	zt_textureRenderTargetPrepare(final_render_target);\n"
			"	{\n"
			"		zt_rendererClear(zt_vec4(.4f, .4f, .4f, 1));\n"
			"		// draw splash screen here\n"
			"	}\n"
			"	zt_textureRenderTargetCommit(final_render_target);\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n"
			);
	}
	else if (zt_strEquals(src_file, "src\\" SRC_FILE_GAME_SCENE_MENU)) {
		zt_fileWritef(&file,
			"// headers ========================================================================================================================================================================================\n\n"
			"#include \"game.h\"\n\n"
			"// variables ======================================================================================================================================================================================\n\n"
			"// private functions ==============================================================================================================================================================================\n\n"
			"// functions ======================================================================================================================================================================================\n\n"
			"FUNC_GAME_SCENE_BEGIN(gameSceneMenuBegin)\n"
			"{\n"
			"	GameSceneMenu *gs = &game->game_scene_menu;\n\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"FUNC_GAME_SCENE_CLEANUP(gameSceneMenuCleanup)\n"
			"{\n"
			"	GameSceneMenu *gs = &game->game_scene_menu;\n\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"FUNC_GAME_SCENE_UPDATE(gameSceneMenuUpdate)\n"
			"{\n"
			"	ZT_PROFILE_GAME(\"GameSceneMenuUpdate\");\n\n"
			"	GameSceneMenu *gs = &game->game_scene_menu;\n\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"FUNC_GAME_SCENE_RENDER(gameSceneMenuRender)\n"
			"{\n"
			"	ZT_PROFILE_GAME(\"GameSceneMenuRender\");\n\n"
			"	GameSceneMenu *gs = &game->game_scene_menu;\n\n"
			"	zt_textureRenderTargetPrepare(final_render_target);\n"
			"	{\n"
			"		zt_rendererClear(zt_vec4(.4f, .4f, .4f, 1));\n"
			"		// render menu\n"
			"	}\n"
			"	zt_textureRenderTargetCommit(final_render_target);\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n"
			);
	}
	else if (zt_strEquals(src_file, "src\\" SRC_FILE_GAME_DLL)) {
		bool draw_list = build_cfg->camera_2d || build_cfg->include_gui;

		zt_fileWritef(&file,
			"// headers ========================================================================================================================================================================================\n\n"
			"#include \"game.h\"\n\n"
			"// variables ======================================================================================================================================================================================\n\n"
			"// private functions ==============================================================================================================================================================================\n\n"
			"ztInternal void _gameSetFunctionPointers(ztGame *game)\n"
			"{\n"
			"	zt_fiz(GameScene_MAX) {\n"
			"		game->gsf_begin  [i] = nullptr;\n"
			"		game->gsf_cleanup[i] = nullptr;\n"
			"		game->gsf_update [i] = nullptr;\n"
			"		game->gsf_render [i] = nullptr;\n"
			"	}\n\n"
			"	game->gsf_begin  [GameScene_SplashLoading] = gameSceneSplashLoadingBegin;\n"
			"	game->gsf_cleanup[GameScene_SplashLoading] = gameSceneSplashLoadingCleanup;\n"
			"	game->gsf_update [GameScene_SplashLoading] = gameSceneSplashLoadingUpdate;\n"
			"	game->gsf_render [GameScene_SplashLoading] = gameSceneSplashLoadingRender;\n\n"
			"	game->gsf_begin  [GameScene_Splash       ] = gameSceneSplashBegin;\n"
			"	game->gsf_cleanup[GameScene_Splash       ] = gameSceneSplashCleanup;\n"
			"	game->gsf_update [GameScene_Splash       ] = gameSceneSplashUpdate;\n"
			"	game->gsf_render [GameScene_Splash       ] = gameSceneSplashRender;\n\n"
			"	game->gsf_begin  [GameScene_Menu         ] = gameSceneMenuBegin;\n"
			"	game->gsf_cleanup[GameScene_Menu         ] = gameSceneMenuCleanup;\n"
			"	game->gsf_update [GameScene_Menu         ] = gameSceneMenuUpdate;\n"
			"	game->gsf_render [GameScene_Menu         ] = gameSceneMenuRender;\n\n"
			"	game->gsf_begin  [GameScene_MainLoading  ] = gameSceneMainLoadingBegin;\n"
			"	game->gsf_cleanup[GameScene_MainLoading  ] = gameSceneMainLoadingCleanup;\n"
			"	game->gsf_update [GameScene_MainLoading  ] = gameSceneMainLoadingUpdate;\n"
			"	game->gsf_render [GameScene_MainLoading  ] = gameSceneMainLoadingRender;\n\n"
			"	game->gsf_begin  [GameScene_Main         ] = gameSceneMainBegin;\n"
			"	game->gsf_cleanup[GameScene_Main         ] = gameSceneMainCleanup;\n"
			"	game->gsf_update [GameScene_Main         ] = gameSceneMainUpdate;\n"
			"	game->gsf_render [GameScene_Main         ] = gameSceneMainRender;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ztInternal void _gameSceneCleanup(ztGame *game)\n"
			"{\n"
			"	if (game->gsf_cleanup[game->game_scene]) {\n"
			"		game->gsf_cleanup[game->game_scene](game, game->game_scene_transition_to);\n"
			"	}\n"
			"}\n\n"
			"// functions ======================================================================================================================================================================================\n\n"
			"ZT_DLLEXPORT bool dll_settings(ztGameDetails* details, ztGameSettings* settings)\n"
			"{\n"
			"	settings->memory   = zt_megabytes(256);\n"
			"	settings->native_w = settings->screen_w = 1280;\n"
			"	settings->native_h = settings->screen_h =  720;\n\n"
			"	settings->threaded_frame_jobs      = 4;\n"
			"	settings->threaded_background_jobs = 2;\n\n"
			"	if (!zt_cmdHasArg(details->argv, details->argc, \"no-vsync\", \"no-vsync\") && !zt_cmdHasArg(details->argv, details->argc, \"novsync\", \"novsync\")) {\n"
			"		settings->renderer_flags |= ztRendererFlags_Vsync;\n"
			"	}\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ztInternal void _gameCreateRenderTargets(ztGame *game, bool free_first)\n"
			"{\n"
			);

		if (build_cfg->include_pbr) {
			zt_fileWrite(&file,
				"	if (free_first) {\n"
				"		if (zt_textureGetSize(game->render_target) == zt_vec2i(game->camera_2d.width, game->camera_2d.height)) {\n"
				"			return;\n"
				"		}\n\n"
				"		zt_textureFree(game->render_target_attach_position);\n"
				"		zt_textureFree(game->render_target_attach_normal);\n"
				"		zt_textureFree(game->render_target_bright);\n"
				"		zt_textureFree(game->render_target_blurred);\n"
				"		zt_textureFree(game->render_target_ao);\n"
				"		zt_textureFree(game->render_target);\n"
				"	}\n\n"
				"	game->render_target                 = zt_textureMakeRenderTarget(game->settings->native_w, game->settings->native_h, ztTextureFlags_HDR | ztTextureFlags_Multisample);\n"
				"	game->render_target_attach_position = zt_textureRenderTargetAddAttachment(game->render_target, ztTextureColorFormat_RGBA16F);\n"
				"	game->render_target_attach_normal   = zt_textureRenderTargetAddAttachment(game->render_target, ztTextureColorFormat_RGBA16F);\n\n"
				"	game->render_target_bright          = zt_textureMakeRenderTarget(game->settings->native_w / 2, game->settings->native_h / 2, ztTextureFlags_HDR);\n"
				"	game->render_target_blurred         = zt_textureMakeRenderTarget(game->settings->native_w / 2, game->settings->native_h / 2, ztTextureFlags_HDR);\n"
				"	game->render_target_ao              = zt_textureMakeRenderTarget(game->settings->native_w / 2, game->settings->native_h / 2);\n"
				);
		}
		else {
			zt_fileWrite(&file,
				"	if (free_first) {\n"
				"		if (zt_textureGetSize(game->render_target) == zt_vec2i(game->camera_2d.width, game->camera_2d.height)) {\n"
				"			return;\n"
				"		}\n\n"
				"		zt_textureFree(game->render_target);\n"
				"	}\n\n"
				"	game->render_target = zt_textureMakeRenderTarget(game->settings->native_w, game->settings->native_h, 0);\n"
				);
		}

		zt_fileWrite(&file,
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_DLLEXPORT bool dll_init(ztGameDetails* details, ztGameSettings* settings, void** game_memory)\n"
			"{\n"
			"	ztGame *game = zt_mallocStruct(ztGame);\n"
			"	*game = {};\n"
			"	*game_memory = game;\n\n"
			"	game->details     = details;\n"
			"	game->settings    = settings;\n\n"
			);

		if (build_cfg->default_dirs) {
			zt_fileWritef(&file,
				"\n"
				"	{\n"
				"		// app path and assets\n\n"
				"		char data_path[ztFileMaxPath];\n"
				"		zt_strCpy(data_path, ztFileMaxPath, details->data_path);\n"
				"		if (!zt_directoryExists(data_path)) {\n"
				"			zt_fileConcatFileToPath(data_path, ztFileMaxPath, details->data_path, \"data\");\n"
				"			if (!zt_directoryExists(data_path)) {\n"
				"				zt_fileConcatFileToPath(data_path, ztFileMaxPath, details->data_path, \"run\" ztFilePathSeparatorStr \"data\");\n"
				"			}\n"
				"		}\n"
				"		if (!zt_assetManagerLoadDirectory(&game->asset_manager, data_path, zt_memGetGlobalArena())) {\n"
				"			zt_logCritical(\"Unable to load game assets\");\n"
				"			return false;\n"
				"		}\n"
				"	}\n"
				);
		}

		if (build_cfg->include_gui) {
			zt_fileWritef(&file,
				"\n"
				"	{\n"
				"		// gui\n\n"
				"		if (!guiThemeMake(&game->gui_theme, &game->asset_manager, game)) {\n"
				"			return false;\n"
				"		}\n\n"
				"		game->gui_manager = zt_guiManagerMake(&game->camera_2d, &game->gui_theme, zt_memGetGlobalArena());\n"
				"		zt_guiInitDebug(game->gui_manager);\n"
				"	}\n\n"
				);
		}

		zt_fileWritef(&file,
			"	zt_debuggingInit(\"debug.cfg\");\n\n"
			);

		if (build_cfg->include_vr) {
			zt_fileWrite(&file,
				"	game->vr = zt_vrIsHeadsetPresent() ? zt_vrMake() : nullptr;\n\n"
				);
		}

		if (draw_list) {
			zt_fileWritef(&file,
				"	{\n"
				"		if (!zt_drawListMake(&game->draw_list, 1024 * 128)) {\n"
				"			zt_logCritical(\"Unable to initialize draw list\");\n"
				"			return false;\n"
				"		}\n\n"
				);

			if (build_cfg->include_pbr) {
				zt_fileWritef(&file,
					"		_gameCreateRenderTargets(game, false);\n\n"
					"		ztRandom random;\n"
					"		zt_randomInit(&random);\n"
					"		game->texture_random = zt_textureMakeRandom(&random, 4, 4);\n\n"
					"		ztShaderTonemapSettings tonemap_settings = {};\n"
					"		tonemap_settings.bloom_enabled = true;\n"
					"		tonemap_settings.ao_enabled = true;\n\n"
					"		game->shader_hdr_tonemap = zt_shaderBuildTonemap(&tonemap_settings);\n"
					"		game->shader_hdr_bright = zt_shaderGetDefault(ztShaderDefault_Bright);\n"
					"		game->shader_hdr_bloom_blur_1 = zt_shaderGetDefault(ztShaderDefault_BlurHorz);\n"
					"		game->shader_hdr_bloom_blur_2 = zt_shaderGetDefault(ztShaderDefault_BlurVert);\n\n"
					"		ztShaderAmbientOcclusionSettings ao_settings = {};\n"
					"		ao_settings.generate_noise_tex = false;\n\n"
					"		game->shader_ao = zt_shaderBuildAmbientOcclusion(&ao_settings);\n\n"
					"		if (game->shader_hdr_tonemap == ztInvalidID || game->shader_hdr_bright == ztInvalidID || game->shader_hdr_bloom_blur_1 == ztInvalidID || game->shader_hdr_bloom_blur_2 == ztInvalidID || game->shader_ao == ztInvalidID) {\n"
					"			return false;\n"
					"		}\n"
					"		zt_shaderSetVariableVec2(game->shader_ao, zt_strHash(\"noise_scale\"), zt_vec2(settings->native_w / 4.0f, settings->native_h / 4.0f));\n"
					"	}\n\n"
					);
			}
			else {
				zt_fileWritef(&file,
					"		_gameCreateRenderTargets(game, false);\n"
					"	}\n"
					);
			}
		}

		zt_fileWritef(&file,
			"\n"
			"	_gameSetFunctionPointers(game);\n\n"
			"	game->game_scene                 = GameScene_Invalid;\n"
			"	game->game_scene_transition_to   = GameScene_SplashLoading;\n"
			"	game->game_scene_transition_time = 0;\n\n"
			"	game->fade_time = game->fade_time_max = 0;\n\n"
			"	zt_tweenManagerMake(&game->tween_manager_state);\n"
			"	zt_tweenManagerMake(&game->tween_manager_permanent);\n\n"
			"#	if defined(ZT_INPUT_REPLAY)\n"
			"	game->replay_state = ReplayState_None;\n"
			"	{\n"
			"		// setup input recording/playback\n\n"
			"		if (zt_cmdHasArg(details->argv, details->argc, \"i\", \"input\")) {\n"
			"			char replay_file_name[ztFileMaxPath];\n"
			"			if (zt_cmdGetArg(details->argv, details->argc, \"i\", \"input\", replay_file_name, ztFileMaxPath)) {\n"
			"				char replay_file[ztFileMaxPath];\n"
			"				if (zt_fileExists(replay_file_name)) {\n"
			"					zt_strCpy(replay_file, zt_elementsOf(replay_file), replay_file_name, zt_strLen(replay_file_name));\n"
			"				}\n"
			"				else {\n"
			"					zt_fileConcatFileToPath(replay_file, ztFileMaxPath, details->user_path, replay_file_name);\n"
			"					if (!zt_fileExists(replay_file)) {\n"
			"						zt_fileConcatFileToPath(replay_file, ztFileMaxPath, details->app_path, replay_file_name);\n"
			"					}\n"
			"				}\n\n"
			"				if (!zt_fileExists(replay_file)) {\n"
			"					zt_logCritical(\"Could not locate input replay file: %%s\", replay_file_name);\n"
			"				}\n"
			"				else {\n"
			"					zt_inputReplayMakeReader(&game->replay_data, replay_file);\n"
			"					game->replaying = true;\n"
			"					game->replay_state = ReplayState_Replaying;\n\n"
			"					zt_guiDebugShowDetails();\n"
			"				}\n"
			"			}\n"
			"		}\n"
			"		else if (!zt_cmdHasArg(details->argv, details->argc, \"no-vsync\", \"no-vsync\")) {\n"
			"			char replay_file[ztFileMaxPath];\n\n"
			"			// we will keep the last 5 replays, so that we don't accidentally lose a replay\n"
			"			for (int i = 5; i > 1; --i) {\n"
			"				zt_strMakePrintf(file_name, 64, \"input_replay.%%d.dat\", i);\n"
			"				zt_fileConcatFileToPath(replay_file, ztFileMaxPath, details->user_path, file_name);\n\n"
			"				zt_strPrintf(file_name, 64, \"input_replay.%%d.dat\", i - 1);\n"
			"				char next_replay_file[ztFileMaxPath];\n"
			"				zt_fileConcatFileToPath(next_replay_file, ztFileMaxPath, details->user_path, file_name);\n\n"
			"				if (zt_fileExists(replay_file)) {\n"
			"					zt_fileDelete(replay_file);\n"
			"				}\n"
			"				if (zt_fileExists(next_replay_file)) {\n"
			"					zt_fileCopy(next_replay_file, replay_file);\n"
			"				}\n"
			"			}\n\n"
			"			zt_fileConcatFileToPath(replay_file, ztFileMaxPath, details->user_path, \"input_replay.1.dat\");\n"
			"			zt_fileDelete(replay_file);\n\n"
			"			zt_inputReplayMakeWriter(&game->replay_data, replay_file);\n"
			"			game->replaying = false;\n"
			"			game->replay_state = ReplayState_Recording;\n"
			"		}\n"
			"	}\n"
			"#	endif\n"
			"\n"
			);

		/*
		if (build_cfg->include_3d_scene) {
			zt_fileWritef(&file,
				"	game->scene = zt_sceneMake(zt_memGetGlobalArena());\n"
				"	{\n"
				"		ztMeshID   cube       = zt_meshMakePrimitiveBox(1, 1, 1);\n"
				"		ztMaterial cube_mat   = zt_materialMake(ztInvalidID, zt_vec4(1,1,1,1));\n"
				"		ztModel   *cube_model = zt_modelMake(zt_memGetGlobalArena(), cube, &cube_mat, zt_shaderGetDefault(ztShaderDefault_LitShadow), nullptr, ztModelFlags_CastsShadows | ztModelFlags_OwnsMaterials | ztModelFlags_OwnsMesh);\n"
				"		cube_model->transform.position = zt_vec3(0, 0, 0);\n"
				"		zt_sceneAddModel(game->scene, cube_model);\n"
				"\n"
				"		game->scene_light = zt_lightMakeDirectional(zt_vec3(5, 10, 0), ztVec3::zero, 1, 0.05f);\n"
				"		zt_sceneAddLight(game->scene, &game->scene_light);\n"
				"		game->camera_3d.rotation = ztQuat::makeFromEuler(35, 0, 0);\n"
				"		game->camera_3d.rotation *= ztQuat::makeFromEuler(0, -40, 0);\n"
				"		game->camera_3d.position = zt_vec3(3, 3, 3);\n"
				"	}\n"
				);
		}
		*/
		zt_fileWritef(&file,
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_DLLEXPORT void dll_cleanup(void *memory)\n"
			"{\n"
			"	ztGame *game = (ztGame*)memory;\n\n"
			"	_gameSceneCleanup(game);\n"
			"#	if defined(ZT_INPUT_REPLAY)\n"
			"	zt_inputReplayFree(&game->replay_data);\n"
			"#	endif\n"
			"	zt_textureFree(game->render_target);\n\n"
			);

		if (build_cfg->include_pbr) {
			zt_fileWritef(&file,
				"	zt_textureFree(game->render_target_attach_position);\n"
				"	zt_textureFree(game->render_target_attach_normal);\n"
				"	zt_textureFree(game->render_target_bright);\n"
				"	zt_textureFree(game->render_target_blurred);\n"
				"	zt_textureFree(game->render_target_ao);\n\n"
				"	zt_textureFree(game->render_target);\n\n"
				"	zt_textureFree(game->texture_random);\n"
				"	zt_shaderFree(game->shader_hdr_tonemap);\n"
				"	zt_shaderFree(game->shader_hdr_bright);\n"
				"	zt_shaderFree(game->shader_hdr_bloom_blur_1);\n"
				"	zt_shaderFree(game->shader_hdr_bloom_blur_2);\n"
				"	zt_shaderFree(game->shader_ao);\n\n"
				);
		}

		if (build_cfg->include_vr) {
			zt_fileWrite(&file,
				"	if(game->vr) {\n"
				);

			if (build_cfg->include_3d_scene) {
				zt_fileWrite(&file,
					"		if (game->vr->headset.model) {\n"
					"			zt_sceneRemoveModel(game->scene, game->vr->headset.model);\n"
					"		}\n"
					"		zt_fiz(game->vr->controllers_count) {\n"
					"			if (game->vr->controllers[i].model && zt_sceneHasModel(game->scene, game->vr->controllers[i].model)) {\n"
					"				zt_sceneRemoveModel(game->scene, game->vr->controllers[i].model);\n"
					"			}\n"
					"		}\n"
					);
			}
			zt_fileWrite(&file,
				"		zt_vrFree(game->vr);\n"
				"	}\n"
				);
		}

/*		if (build_cfg->include_3d_scene) {
			zt_fileWritef(&file,
				"	{\n"
				"		zt_sceneFreeAllModels(game->scene);\n"
				"		zt_sceneFree(game->scene);\n"
				"	}\n"
				);
		}*/

		if (draw_list) {
			zt_fileWritef(&file, "	zt_drawListFree(&game->draw_list);\n");
		}

		if (build_cfg->default_dirs) {
			zt_fileWritef(&file, "	zt_assetManagerFree(&game->asset_manager);\n");
		}

		if (build_cfg->include_gui) {
			zt_fileWritef(&file,
				"	guiThemeFree(&game->gui_theme);\n"
				"	zt_guiManagerFree(game->gui_manager);\n"
				);
		}


		zt_fileWritef(&file,
			"\n"
			"	zt_free(game);\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_DLLEXPORT bool dll_reload(void *memory)\n"
			"{\n"
			"	ztGame *game = (ztGame*)memory;\n"
			"#	if defined(ZT_DLL)\n"
			);

		if (build_cfg->include_gui) {
			zt_fileWritef(&file,
				"	zt_dllGuiLoad();\n"
				);
		}

		zt_fileWritef(&file,
			"#	endif\n"
			"	_gameSetFunctionPointers(game);\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_DLLEXPORT bool dll_unload(void *memory)\n"
			"{\n"
			"#	if defined(ZT_DLL)\n"
			"	ztGame *game = (ztGame*)memory;\n"
			);

		if (build_cfg->include_gui) {
			zt_fileWritef(&file,
				"	zt_dllGuiUnload();\n"
				);
		}

		zt_fileWritef(&file,
			"#	endif\n"
			"	return true;\n"
			"}\n\n"
			);

		if (build_cfg->camera_3d || build_cfg->include_3d_scene) {
			zt_fileWritef(&file,
				"// ================================================================================================================================================================================================\n\n"
				"void gameAdjustCamera(ztGame *game)\n"
				"{\n"
				"	r32 view_distance = 50;\n"
				"	zt_cameraMakePersp(&game->camera_3d, game->settings->screen_w, game->settings->screen_h, zt_degreesToRadians(60), 0.1f, view_distance, game->camera_3d.position, game->camera_3d.rotation);\n"
				"	zt_cameraRecalcMatrices(&game->camera_3d);\n"
				"	_gameCreateRenderTargets(game, true);\n"
				"}\n\n"
				);
		}

		zt_fileWritef(&file,
			"// ================================================================================================================================================================================================\n\n"
			"ZT_DLLEXPORT void dll_screenChange(ztGameSettings *settings, void *memory)\n"
			"{\n"
			"	ztGame *game = (ztGame*)memory;\n"
			);

		if (build_cfg->camera_2d || build_cfg->include_gui) {
			zt_fileWritef(&file,
				"	zt_cameraMakeOrtho(&game->camera_2d, settings->screen_w, settings->screen_h, settings->native_w, settings->native_h, 0.1f, 100.f, game->camera_2d.position);\n"
				"	zt_cameraRecalcMatrices(&game->camera_2d);\n"
				);
		}
		if (build_cfg->camera_3d || build_cfg->include_3d_scene) {
			zt_fileWritef(&file,
				"	gameAdjustCamera(game);\n"
				);
		}

		zt_fileWritef(&file,
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"void gameTransitionTo(ztGame *game, GameScene_Enum state, bool immediately)\n"
			"{\n"
			"	game->game_scene_transition_to = state;\n"
			"\n"
			"	if (immediately) {\n"
			"		game->game_scene_transition_time = 0;\n"
			"	}\n"
			"	else {\n"
			"		r32 transition_time = .25f;\n"
			"		game->game_scene_transition_time = transition_time;\n"
			"		game->fade_time = game->fade_time_max = transition_time * 2;\n"
			"	}\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_DLLEXPORT bool dll_gameLoop(void *memory, r32 dt)\n"
			"{\n"
			"	ztGame *game = (ztGame*)memory;\n\n"
			"	ztInputKeys       input_keys[ztInputKeys_MAX];\n"
			"	ztInputMouse      input_mouse;\n"
			"	ztInputController input_controller;\n"
			"	ztInputKeys_Enum  input_keystrokes[16];\n"
			"	bool              input_this_frame;\n"
			"	zt_inputGetKeyStrokes(input_keystrokes);\n\n"
			);

		if (build_cfg->include_gui) {
			zt_fileWritef(&file,
				"	bool gui_input;\n"
				);
		}

		zt_fileWritef(&file,
			"	{\n"
			"		ZT_PROFILE_GAME(\"gameLoop:input\");\n"
			"		zt_inputKeysCopyState(input_keys);\n"
			"		zt_inputMouseCopyState(&input_mouse);\n"
			"		zt_inputControllerCopyState(&input_controller, 0);\n"
			"		zt_inputGetKeyStrokes(input_keystrokes);\n"
			"		input_this_frame = zt_inputThisFrame();\n\n"
			"		if (input_keys[ztInputKeys_Control].pressed() && input_keys[ztInputKeys_Shift].pressed() && input_keys[ztInputKeys_Menu].pressed()) {\n"
			"			if (input_keys[ztInputKeys_P].justPressed()) {\n"
			"				if (zt_profilerIsPaused()) {\n"
			"					zt_profilerResume();\n"
			"				}\n"
			"				else {\n"
			"					zt_profilerPause();\n"
			"				}\n"
			"			}\n\n"
			"			if (input_keys[ztInputKeys_D].justPressed()) {\n"
			"				zt_guiDebugToggle();\n"
			"			}\n"
			"		}\n"
			"#		if defined(ZT_INPUT_REPLAY)\n"
			"		if (game->replay_state == ReplayState_Replaying) {\n"
			"			if (input_keys[ztInputKeys_Space].justPressed()) {\n"
			"				game->replay_state = ReplayState_Paused;\n"
			"			}\n"
			"		}\n"
			"		else if (game->replay_state == ReplayState_Paused) {\n"
			"			if (input_keys[ztInputKeys_Right].justPressedOrRepeated()) {\n"
			"				game->replay_state = ReplayState_Stepping;\n"
			"			}\n"
			"			if (input_keys[ztInputKeys_Space].justPressed()) {\n"
			"				game->replay_state = ReplayState_Replaying;\n"
			"			}\n"
			"		}\n\n"
			"		if (game->replay_state != ReplayState_Paused) {\n"
			"			if (!zt_inputReplayProcessFrame(&game->replay_data, game->details->current_frame, &input_this_frame, input_keys, &input_mouse, &input_controller, input_keystrokes)) {\n"
			"				game->replaying = false;\n"
			"				game->replay_state = ReplayState_None;\n"
			"			}\n"
			"#		else\n"
			"		{\n"
			"#		endif\n"
			);

		if (build_cfg->include_gui) {
			zt_fileWritef(&file,
				"			gui_input = zt_guiManagerHandleInput(game->gui_manager, input_keys, input_keystrokes, &input_mouse);\n"
				"			if (!gui_input) {\n"
				"				if (input_keys[ztInputKeys_Tilda].justPressed()) {\n"
				"					bool console_shown = false;\n"
				"					zt_debugConsoleToggle(&console_shown);\n"
				"					if (console_shown) {\n"
				"						zt_guiManagerSetKeyboardFocus(game->gui_manager);\n"
				"					}\n"
				"				}\n"
				"			}\n"
				"		}\n"
				"	}\n"
				"\n"
				);
		}

		zt_fileWritef(&file,
			"	if (game->game_scene_transition_to != GameScene_Invalid) {\n"
			"		game->game_scene_transition_time -= dt;\n"
			"		if (game->game_scene_transition_time < 0) {\n"
			"			ZT_PROFILE_GAME(\"gameLoop:gamescene transition\");\n"
			"			game->game_scene_transition_time = 0;\n"
			"			_gameSceneCleanup(game);\n\n"
			"			GameScene_Enum transitioning_from = game->game_scene;\n"
			"			game->game_scene = game->game_scene_transition_to;\n"
			"			game->game_scene_transition_to = GameScene_Invalid;\n\n"
			"			zt_tweenManagerReset(&game->tween_manager_state);\n\n"
			"			if (game->gsf_begin[game->game_scene]) {\n"
			"				if (!game->gsf_begin[game->game_scene](game, transitioning_from)) {\n"
			"					return false;\n"
			"				}\n"
			"			}\n"
			"		}\n"
			"	}\n\n"
			"	{\n"
			"#		if defined(ZT_INPUT_REPLAY)\n"
			"		if (game->replay_state != ReplayState_Paused)\n"
			"#		endif\n"
			"		{\n"
			"			ZT_PROFILE_GAME(\"gameLoop:update\");\n"
			"			if (!game->gsf_update[game->game_scene](game, dt, gui_input, input_this_frame, input_keys, &input_controller, &input_mouse)) {\n"
			"				return false;\n"
			"			}\n"
			"			zt_tweenManagerUpdate(&game->tween_manager_state, dt);\n"
			"			zt_tweenManagerUpdate(&game->tween_manager_permanent, dt);\n"
			"		}\n"
			"	}\n\n"
			"	{\n"
			"		ZT_PROFILE_GAME(\"gameLoop:render state\");\n"
			"		zt_rendererClear(zt_vec4(0, 0, 0, 0));\n"
			"		game->gsf_render[game->game_scene](game, game->render_target);\n"
			);

		if (build_cfg->include_pbr) {
			zt_fileWritef(&file,
				"\n"
				"		// render ambient occlusion\n"
				"		{\n"
				"			{\n"
				"				ZT_PROFILE_GAME(\"gameLoop:ao\");\n"
				"				static u32 position_tex_hash = zt_strHash(\"position_tex\");\n"
				"				static u32 normal_tex_hash   = zt_strHash(\"normal_tex\");\n"
				"				static u32 noise_tex_hash    = zt_strHash(\"noise_tex\");\n"
				"				static u32 cam_proj_hash     = zt_strHash(\"cam_proj\");\n"
				"		\n"
				"				zt_shaderSetVariableTex (game->shader_ao, position_tex_hash, game->render_target_attach_position);\n"
				"				zt_shaderSetVariableTex (game->shader_ao, normal_tex_hash,   game->render_target_attach_normal);\n"
				"				zt_shaderSetVariableTex (game->shader_ao, noise_tex_hash,    game->texture_random);\n"
				"				zt_shaderSetVariableMat4(game->shader_ao, cam_proj_hash,     game->camera_3d.mat_proj);\n"
				"		\n"
				"				zt_drawListAddScreenRenderTexture(&game->draw_list, game->render_target, &game->camera_2d, .5f, game->shader_ao);\n"
				"				zt_renderDrawList(&game->camera_2d, &game->draw_list, ztVec4::zero, ztRenderDrawListFlags_NoDepthTest, game->render_target_bright);\n"
				"			}\n"
				"			{\n"
				"				ZT_PROFILE_GAME(\"gameLoop:ao_blur\");\n"
				"				static u32 texel_size_hash = zt_strHash(\"texel_size\");\n\n"
				"				zt_shaderSetVariableFloat(game->shader_hdr_bloom_blur_1, texel_size_hash, 1.f / zt_textureGetSize(game->render_target_bright).x * 1.f);\n"
				"				zt_shaderSetVariableFloat(game->shader_hdr_bloom_blur_2, texel_size_hash, 1.f / zt_textureGetSize(game->render_target_bright).x * 1.f);\n\n"
				"				int blur_iters = 2;\n"
				"				zt_fiz(blur_iters) {\n"
				"					zt_drawListAddScreenRenderTexture(&game->draw_list, game->render_target_bright, &game->camera_2d, .5f, game->shader_hdr_bloom_blur_1);\n"
				"					zt_renderDrawList(&game->camera_2d, &game->draw_list, ztVec4::zero, ztRenderDrawListFlags_NoDepthTest, game->render_target_blurred);\n\n"
				"					zt_drawListAddScreenRenderTexture(&game->draw_list, game->render_target_blurred, &game->camera_2d, .5f, game->shader_hdr_bloom_blur_2);\n"
				"					zt_renderDrawList(&game->camera_2d, &game->draw_list, ztVec4::zero, ztRenderDrawListFlags_NoDepthTest, i == blur_iters - 1 ? game->render_target_ao : game->render_target_bright);\n"
				"				}\n"
				"			}\n"
				"		}\n"
				"		// draw the bright areas of the screen for bloom\n"
				"		{\n"
				"			ZT_PROFILE_GAME(\"gameLoop:bloom\");\n"
				"			zt_drawListAddScreenRenderTexture(&game->draw_list, game->render_target, &game->camera_2d, .5f, game->shader_hdr_bright);\n"
				"			zt_renderDrawList(&game->camera_2d, &game->draw_list, ztVec4::zero, ztRenderDrawListFlags_NoDepthTest, game->render_target_bright);\n\n"
				"			static u32 texel_size_hash = zt_strHash(\"texel_size\");\n"
				"			zt_shaderSetVariableFloat(game->shader_hdr_bloom_blur_1, texel_size_hash, 1.f / zt_textureGetSize(game->render_target_bright).x * 1.f);\n"
				"			zt_shaderSetVariableFloat(game->shader_hdr_bloom_blur_2, texel_size_hash, 1.f / zt_textureGetSize(game->render_target_bright).x * 1.f);\n\n"
				"			zt_fiz(2) {\n"
				"				zt_drawListAddScreenRenderTexture(&game->draw_list, game->render_target_bright, &game->camera_2d, .5f, game->shader_hdr_bloom_blur_1);\n"
				"				zt_renderDrawList(&game->camera_2d, &game->draw_list, ztVec4::zero, ztRenderDrawListFlags_NoDepthTest, game->render_target_blurred);\n\n"
				"				zt_drawListAddScreenRenderTexture(&game->draw_list, game->render_target_blurred, &game->camera_2d, .5f, game->shader_hdr_bloom_blur_2);\n"
				"				zt_renderDrawList(&game->camera_2d, &game->draw_list, ztVec4::zero, ztRenderDrawListFlags_NoDepthTest, game->render_target_bright);\n"
				"			}\n"
				"		}\n"
				"		static u32 bloom_tex_hash = zt_strHash(\"bloom_tex\");\n"
				"		static u32 ao_tex_hash    = zt_strHash(\"ao_tex\");\n\n"
				"		zt_shaderSetVariableTex  (game->shader_hdr_tonemap, bloom_tex_hash, game->render_target_blurred);\n"
				"		zt_shaderSetVariableTex  (game->shader_hdr_tonemap, ao_tex_hash,    game->render_target_ao);\n\n"
				"		zt_drawListAddScreenRenderTexture(&game->draw_list, game->render_target, &game->camera_2d, 1, game->shader_hdr_tonemap);\n\n"
			);
		}
		else {
			zt_fileWritef(&file,
				"		zt_drawListAddScreenRenderTexture(&game->draw_list, game->render_target, &game->camera_2d);\n"
				);
		}

		zt_fileWritef(&file,
			"		if (game->fade_time > 0) {\n"
			"			game->fade_time -= dt;\n"
			"			if (game->fade_time < 0) {\n"
			"				game->fade_time = 0;\n"
			"			}\n\n"
			"			r32 alpha = 1;\n"
			"			r32 fade_time_half = game->fade_time_max / 2.f;\n"
			"			if (game->fade_time < fade_time_half) {\n"
			"				alpha = zt_linearRemapAndClamp(game->fade_time, 0, fade_time_half, 0, 1);\n"
			"			}\n"
			"			else {\n"
			"				alpha = zt_linearRemapAndClamp(game->fade_time, fade_time_half, game->fade_time_max, 1, 0);\n"
			"			}\n\n"
			"			zt_drawListPushShader(&game->draw_list, zt_shaderGetDefault(ztShaderDefault_Unlit));\n"
			"			zt_drawListAddSolidRect2D(&game->draw_list, ztVec3::zero, zt_vec2(128, 128), zt_color(0, 0, 0, alpha));\n"
			"			zt_drawListPopShader(&game->draw_list);\n"
			"		}\n\n"
			"		zt_renderDrawList(&game->camera_2d, &game->draw_list, ztVec4::zero, ztRenderDrawListFlags_NoDepthTest);\n"
			"	}\n\n"
			);

		if (build_cfg->include_gui) {
			zt_fileWritef(&file,
				"	{\n"
				"		ZT_PROFILE_GAME(\"gameLoop:render gui\");\n\n"
				"		zt_drawListPushShader(&game->draw_list, zt_shaderGetDefault(ztShaderDefault_Unlit));\n"
				"		zt_guiManagerRender(game->gui_manager, &game->draw_list, dt);\n"
				"		zt_drawListPopShader(&game->draw_list);\n\n"
				"#		if defined(ZT_INPUT_REPLAY)\n"
				"		if (game->replaying) {\n"
				"			ztVec2 mpos = zt_cameraOrthoScreenToWorld(&game->camera_2d, input_mouse.screen_x, input_mouse.screen_y);\n"
				"			r32 ppu = zt_pixelsPerUnit();\n\n"
				"			zt_drawListPushShader(&game->draw_list, zt_shaderGetDefault(ztShaderDefault_Unlit));\n"
				"			zt_drawListAddSolidCircle2D(&game->draw_list, mpos, 7 / ppu, 10, ztColor_DarkRed);\n"
				"			zt_drawListAddSolidCircle2D(&game->draw_list, mpos, 2 / ppu, 10, ztColor_Red);\n\n"
				"			ztVec2 ext = zt_cameraOrthoGetViewportSize(&game->camera_2d);\n\n"
				"			char *text = \"[Replaying]\\nPress Space to Pause\";\n"
				"			if (game->replay_state == ReplayState_Paused || game->replay_state == ReplayState_Stepping) {\n"
				"				text = \"[Replay Paused]\\nPress Space to Resume, Press -> to Step Forward One Frame\";\n"
				"			}\n\n"
				"			zt_drawListAddText2D(&game->draw_list, ztFontDefault, text, zt_vec2(0, ext.y / -2 + .2f), ztAlign_Center | ztAlign_Bottom, ztAnchor_Center | ztAnchor_Bottom);\n"
				"			zt_drawListPopShader(&game->draw_list);\n"
				"		}\n"
				"#		endif\n\n"
				"		zt_renderDrawList(&game->camera_2d, &game->draw_list, ztVec4::zero, ztRenderDrawListFlags_NoDepthTest | ztRenderDrawListFlags_NoClear);\n"
				"	}\n"
				);
		}

		zt_fileWritef(&file,
			"	{\n"
			"		ZT_PROFILE_GAME(\"gameLoop:asset check\");\n"
			"		zt_assetManagerCheckForChanges(&game->asset_manager);\n"
			"	}\n\n"
			"#	if defined(ZT_INPUT_REPLAY)\n"
			"	{\n"
			"		if (game->replay_state == ReplayState_Stepping) {\n"
			"			game->replay_state = ReplayState_Paused;\n"
			"		}\n"
			"		else if (game->replay_state == ReplayState_Paused) {\n"
			"			game->details->current_frame -= 1;\n"
			"		}\n"
			"	}\n"
			"#	endif\n"
			"	return true;\n"
			"}\n\n"
			);

		/*
		if (build_cfg->include_vr) {
			zt_fileWrite(&file,
				"	if (game->vr) {\n"
				"		if (zt_vrUpdate(game->vr)) {\n"
				);

			if (build_cfg->include_3d_scene) {
				zt_fileWrite(&file,
					"			zt_vrUpdateScene(game->vr, game->scene, ztVrUpdateSceneFlags_IncludeControllers);\n"
					"\n"
					"			ztCamera *cameras[2] = { &game->vr->camera_left, &game->vr->camera_right };\n"
					"			ztTextureID textures[2] = { game->vr->tex_left, game->vr->tex_right };\n"
					"\n"
					"			zt_fiz(2) {\n"
					"				zt_sceneOptimize(game->scene, cameras[i]);\n"
					"				zt_sceneLighting(game->scene, cameras[i]);\n"
					"\n"
					"				zt_textureRenderTargetPrepare(textures[i]);\n"
					"				zt_rendererClear(zt_vec4(0, 0, 0, 1));\n"
					"				zt_sceneRender(game->scene, cameras[i]);\n"
					"				zt_textureRenderTargetCommit(textures[i]);\n"
					"			}\n"
					"\n"
					"			zt_vrSubmit(game->vr);\n"
					);
			}

			zt_fileWrite(&file,
				"		}\n"
				"	}\n\n"
				);
		}

		if (build_cfg->include_3d_scene) {
			zt_fileWritef(&file,
				"	zt_sceneOptimize(game->scene, &game->camera_3d);\n"
				"	zt_sceneLighting(game->scene, &game->camera_3d);\n"
				"	zt_sceneRender(game->scene, &game->camera_3d);\n"
				);

		}

		if (build_cfg->include_gui) {
			zt_fileWritef(&file,
				"	{\n"
				"		zt_drawListPushShader(&game->draw_list, zt_shaderGetDefault(ztShaderDefault_Unlit));\n"
				"		zt_guiManagerRender(game->gui_manager, &game->draw_list, dt);\n"
				"		zt_drawListPopShader(&game->draw_list);\n"
				);

			if (build_cfg->include_3d_scene) {
				zt_fileWritef(&file,
					"		zt_renderDrawList(&game->camera_2d, &game->draw_list, ztVec4::zero, ztRenderDrawListFlags_NoDepthTest | ztRenderDrawListFlags_NoClear);\n"
					"	}\n"
					);
			}
			else {
				zt_fileWritef(&file,
					"		zt_renderDrawList(&game->camera_2d, &game->draw_list, ztVec4::zero, ztRenderDrawListFlags_NoDepthTest);\n"
					"	}\n"
					);
			}
		}

		if (build_cfg->default_dirs) {
			zt_fileWritef(&file,
				"	zt_assetManagerCheckForChanges(&game->asset_manager);\n"
				);
		}

		zt_fileWritef(&file, 
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n"
			);
			*/
	}
	else if (zt_strEquals(src_file, "src\\" SRC_FILE_GAME)) {

	}
	else if (zt_strEquals(src_file, "src\\" SRC_FILE_GUI_THEME_H)) {
		zt_fileWritef(&file,
			"/**************************************************************************************************************************************************************************************************\n"
			"** file: src/gui_theme.h\n"
			"**\n"
			" ** This file was automatically generated.\n"
			"***************************************************************************************************************************************************************************************************/\n"
			"\n"
			"#ifndef __gui_theme_h__\n"
			"#define __gui_theme_h__\n"
			"\n"
			"// ================================================================================================================================================================================================\n"
			"\n"
			"#include \"zt_game_gui.h\"\n"
			"\n"
			"// ================================================================================================================================================================================================\n"
			"\n"
			"enum GuiThemeItemFlags_Enum\n"
			"{\n"
			"	// use these to identify different controls\n"
			"	GuiThemeItemFlags_        = (1<<0),\n"
			"};\n"
			"\n"
			"enum GuiThemeStates_Enum\n"
			"{\n"
			"	// use these to give gui controls custom states\n"
			"	GuiThemeStates_ = ztGuiItemStates_USER,\n"
			"};\n"
			"\n"
			"// ================================================================================================================================================================================================\n"
			"\n"
			"struct ztGame;\n"
			"\n"
			"// ================================================================================================================================================================================================\n"
			"\n"
			"enum GuiSpriteNineSlice_Enum\n"
			"{\n"
			"	GuiSpriteNineSlice_WindowFrame,\n"
			"	GuiSpriteNineSlice_WindowFrameDisabled,\n"
			"	GuiSpriteNineSlice_PanelFrame,\n"
			"	GuiSpriteNineSlice_PanelFrameDisabled,\n"
			"	GuiSpriteNineSlice_CollapsingPanelFrame,\n"
			"	GuiSpriteNineSlice_CollapsingPanelFrameDisabled,\n"
			"	GuiSpriteNineSlice_ButtonNormal,\n"
			"	GuiSpriteNineSlice_ButtonHighlight,\n"
			"	GuiSpriteNineSlice_ButtonPressed,\n"
			"	GuiSpriteNineSlice_ButtonDisabled,\n"
			"	GuiSpriteNineSlice_ButtonToggleNormal,\n"
			"	GuiSpriteNineSlice_ButtonToggleHighlight,\n"
			"	GuiSpriteNineSlice_ButtonTogglePressed,\n"
			"	GuiSpriteNineSlice_ButtonToggleDisabled,\n"
			"	GuiSpriteNineSlice_ButtonToggleOn,\n"
			"	GuiSpriteNineSlice_ButtonToggleOnDisabled,\n"
			"	GuiSpriteNineSlice_Menu,\n"
			"	GuiSpriteNineSlice_MenuBar,\n"
			"	GuiSpriteNineSlice_MenuBarDisabled,\n"
			"	GuiSpriteNineSlice_TextEdit,\n"
			"	GuiSpriteNineSlice_TextEditDisabled,\n"
			"	GuiSpriteNineSlice_ScrollbarVertBackground,\n"
			"	GuiSpriteNineSlice_ScrollbarVertBackgroundDisabled,\n"
			"	GuiSpriteNineSlice_ScrollbarVertHandle,\n"
			"	GuiSpriteNineSlice_ScrollbarVertHandleHighlight,\n"
			"	GuiSpriteNineSlice_ScrollbarVertHandlePressed,\n"
			"	GuiSpriteNineSlice_ScrollbarVertHandleDisabled,\n"
			"	GuiSpriteNineSlice_ScrollbarHorzBackground,\n"
			"	GuiSpriteNineSlice_ScrollbarHorzBackgroundDisabled,\n"
			"	GuiSpriteNineSlice_ScrollbarHorzHandle,\n"
			"	GuiSpriteNineSlice_ScrollbarHorzHandleHighlight,\n"
			"	GuiSpriteNineSlice_ScrollbarHorzHandlePressed,\n"
			"	GuiSpriteNineSlice_ScrollbarHorzHandleDisabled,\n"
			"	GuiSpriteNineSlice_SliderVertBar,\n"
			"	GuiSpriteNineSlice_SliderVertBarDisabled,\n"
			"	GuiSpriteNineSlice_SliderHorzBar,\n"
			"	GuiSpriteNineSlice_SliderHorzBarDisabled,\n"
			"	GuiSpriteNineSlice_ComboBox,\n"
			"	GuiSpriteNineSlice_ComboBoxHighlight,\n"
			"	GuiSpriteNineSlice_ComboBoxPressed,\n"
			"	GuiSpriteNineSlice_ComboBoxDisabled,\n"
			"	GuiSpriteNineSlice_CycleBoxBody,\n"
			"	GuiSpriteNineSlice_CycleBoxBodyDisabled,\n"
			"	GuiSpriteNineSlice_CycleBoxLeft,\n"
			"	GuiSpriteNineSlice_CycleBoxLeftHighlight,\n"
			"	GuiSpriteNineSlice_CycleBoxLeftPressed,\n"
			"	GuiSpriteNineSlice_CycleBoxLeftDisabled,\n"
			"	GuiSpriteNineSlice_CycleBoxRight,\n"
			"	GuiSpriteNineSlice_CycleBoxRightHighlight,\n"
			"	GuiSpriteNineSlice_CycleBoxRightPressed,\n"
			"	GuiSpriteNineSlice_CycleBoxRightDisabled,\n"
			"	GuiSpriteNineSlice_Solid,\n"
			"\n"
			"	GuiSpriteNineSlice_MAX,\n"
			"};\n"
			"\n"
			"// ================================================================================================================================================================================================\n"
			"\n"
			"enum GuiSprite_Enum\n"
			"{\n"
			"	GuiSprite_CheckBox,\n"
			"	GuiSprite_CheckBoxHighlight,\n"
			"	GuiSprite_CheckBoxPressed,\n"
			"	GuiSprite_CheckBoxDisabled,\n"
			"	GuiSprite_Radio,\n"
			"	GuiSprite_RadioHighlight,\n"
			"	GuiSprite_RadioPressed,\n"
			"	GuiSprite_RadioDisabled,\n"
			"	GuiSprite_RadioSelect,\n"
			"	GuiSprite_SliderHorzHandle,\n"
			"	GuiSprite_SliderHorzHandleHighlight,\n"
			"	GuiSprite_SliderHorzHandlePressed,\n"
			"	GuiSprite_SliderHorzHandleDisabled,\n"
			"	GuiSprite_SliderVertHandle,\n"
			"	GuiSprite_SliderVertHandleHighlight,\n"
			"	GuiSprite_SliderVertHandlePressed,\n"
			"	GuiSprite_SliderVertHandleDisabled,\n"
			"	GuiSprite_Resizer,\n"
			"	GuiSprite_HorzLine,\n"
			"	GuiSprite_IconPlus,\n"
			"	GuiSprite_IconMinus,\n"
			"	GuiSprite_IconArrowLeft,\n"
			"	GuiSprite_IconArrowRight,\n"
			"	GuiSprite_IconArrowUp,\n"
			"	GuiSprite_IconArrowDown,\n"
			"	GuiSprite_IconHandGrab,\n"
			"	GuiSprite_IconHandPoint,\n"
			"	GuiSprite_IconPencil,\n"
			"	GuiSprite_IconTrash,\n"
			"	GuiSprite_IconUndo,\n"
			"	GuiSprite_IconCopy,\n"
			"	GuiSprite_IconCut,\n"
			"	GuiSprite_IconNew,\n"
			"	GuiSprite_IconSave,\n"
			"	GuiSprite_IconOpen,\n"
			"	GuiSprite_IconWarning,\n"
			"	GuiSprite_IconMenu,\n"
			"	GuiSprite_IconCopyright,\n"
			"	GuiSprite_IconCheck,\n"
			"	GuiSprite_IconX,\n"
			"	GuiSprite_IconGear,\n"
			"	GuiSprite_IconCancel,\n"
			"	GuiSprite_IconPlay,\n"
			"	GuiSprite_IconPause,\n"
			"	GuiSprite_IconRewind,\n"
			"	GuiSprite_IconFastForward,\n"
			"	GuiSprite_IconBegin,\n"
			"	GuiSprite_IconEnd,\n"
			"\n"
			"	GuiSprite_MAX,\n"
			"};\n"
			"\n"
			"// ================================================================================================================================================================================================\n"
			"\n"
			"enum GuiAudio_Enum\n"
			"{\n"
			"	GuiAudio_,\n"
			"\n"
			"	GuiAudio_MAX,\n"
			"};\n"
			"\n"
			"// ================================================================================================================================================================================================\n"
			"\n"
			"struct GuiThemeData\n"
			"{\n"
			"	ztTextureID        texture;\n"
			"\n"
			"	ztSpriteManager    sprite_manager;\n"
			"\n"
			"	ztSpriteNineSlice *sprite_nine_slice[GuiSpriteNineSlice_MAX];\n"
			"	ztSprite          *sprite           [GuiSprite_MAX];\n"
			"	ztAudioClipID      audio            [GuiAudio_MAX];\n"
			"\n"
			"	ztGame            *game;\n"
			"	ztCamera          *gui_camera;\n"
			"\n"
			"	ztFontID           font;\n"
			"	ztFontID           font_monospace;\n"
			"};\n"
			"\n"
			"// ================================================================================================================================================================================================\n"
			"\n"
			"bool guiThemeMake(ztGuiTheme *theme, ztAssetManager *asset_manager, ztGame *game);\n"
			"void guiThemeFree(ztGuiTheme *theme);\n"
			"\n"
			"// ================================================================================================================================================================================================\n"
			"\n"
			"#endif // include guard\n"
			);
	}
	else if (zt_strEquals(src_file, "src\\" SRC_FILE_GUI_THEME)) {
		zt_fileWrite(&file,
			"/**************************************************************************************************************************************************************************************************\n"
			"** file: src/gui_theme.cpp\n"
			"**\n"
			"** Responsible for drawing the gui elements\n"
			"***************************************************************************************************************************************************************************************************/\n"
			"\n"
			"#define ZT_GAME_GUI_INTERNAL_DECLARATIONS\n"
			"#include \"zt_game_gui.h\"\n"
			"\n"
			"#include \"gui_theme.h\"\n"
			"#include \"game.h\"\n"
			"\n"
			"// ================================================================================================================================================================================================\n"
			"\n"
			"#define ZT_GUI_HIGHLIGHT_COLOR	zt_color(1.f, .5f, 0.f, .3f)\n"
			"\n"
			"// ================================================================================================================================================================================================\n"
			"\n"
			"ztInternal bool _guiThemeIsCustom(ztGuiItem *item)\n"
			"{\n"
			"	if (zt_guiItemGetTopLevelParent(item)->custom_flags == 0) {\n"
			"	}\n"
			"\n"
			"	return true;\n"
			"}\n"
			"\n"
			"// ================================================================================================================================================================================================\n"
			"\n"
			"ZT_FUNCTION_POINTER_REGISTER(guiThemeGetRValue, ztInternal ZT_FUNC_THEME_GET_RVALUE(guiThemeGetRValue))\n"
			"{\n"
			"	ZT_PROFILE_GUI(\"guiThemeGetRValue\");\n"
			"\n"
			"	if (!_guiThemeIsCustom(item)) {\n"
			"		return false;\n"
			"	}\n"
			"\n"
			"	r32 ppu = zt_pixelsPerUnit();\n"
			"\n"
			"	switch (value)\n"
			"	{\n"
			"		case ztGuiThemeValue_r32_Padding:                     *result = 6 / ppu; break;\n"
			"\n"
			"		case ztGuiThemeValue_r32_WindowTitleHeight:           *result = 24 / ppu; break;\n"
			"		case ztGuiThemeValue_r32_WindowCollapseButtonOffsetX: *result = 5 / ppu; break;\n"
			"		case ztGuiThemeValue_r32_WindowCollapseButtonOffsetY: *result = -1 / ppu; break;\n"
			"		case ztGuiThemeValue_r32_WindowCloseButtonOffsetX:    *result = -5 / ppu; break;\n"
			"		case ztGuiThemeValue_r32_WindowCloseButtonOffsetY:    *result = -1 / ppu; break;\n"
			"		case ztGuiThemeValue_r32_WindowPaddingX:              *result = 3 / ppu; break;\n"
			"		case ztGuiThemeValue_r32_WindowPaddingY:              *result = 3 / ppu; break;\n"
			"		case ztGuiThemeValue_r32_WindowCornerResizeW:         *result = 16 / ppu; break;\n"
			"		case ztGuiThemeValue_r32_WindowCornerResizeH:         *result = 16 / ppu; break;\n"
			"\n"
			"		case ztGuiThemeValue_r32_CheckboxW:                   *result = 20 / ppu; break;\n"
			"		case ztGuiThemeValue_r32_CheckboxH:                   *result = 20 / ppu; break;\n"
			"\n"
			"		case ztGuiThemeValue_r32_SliderHandleSize:            *result = 10 / ppu; break;\n"
			"		case ztGuiThemeValue_r32_SliderHandleMinHeight:       *result = 10 / ppu; break;\n"
			"\n"
			"		case ztGuiThemeValue_r32_ScrollbarButtonW:            *result = 0 / ppu; break;\n"
			"		case ztGuiThemeValue_r32_ScrollbarButtonH:            *result = 0 / ppu; break;\n"
			"		case ztGuiThemeValue_r32_ScrollbarMinHandleSize:      *result = 10 / ppu; break;\n"
			"		case ztGuiThemeValue_r32_ScrollbarMinWidth:           *result = 10 / ppu; break;\n"
			"\n"
			"		case ztGuiThemeValue_r32_ComboboxButtonW:             *result = 27 / ppu; break;\n"
			"\n"
			"		case ztGuiThemeValue_r32_CycleBoxButtonW:             *result = 27 / ppu; break;\n"
			"\n"
			"		default: return false;  // return false to use the default\n"
			"	}\n"
			"\n"
			"	return true;\n"
			"}\n"
			"\n"
			"// ------------------------------------------------------------------------------------------------\n"
			"\n"
			"ZT_FUNCTION_POINTER_REGISTER(guiThemeGetIValue, ztInternal ZT_FUNC_THEME_GET_IVALUE(guiThemeGetIValue))\n"
			"{\n"
			"	ZT_PROFILE_GUI(\"guiThemeGetIValue\");\n"
			"\n"
			"	if (!_guiThemeIsCustom(item)) {\n"
			"		return false;\n"
			"	}\n"
			"\n"
			"	GuiThemeData *theme_data = (GuiThemeData*)theme->theme_data;\n"
			"\n"
			"	switch (value)\n"
			"	{\n"
			"		case ztGuiThemeValue_i32_WindowCloseButtonAlign:            *result = ztAlign_Right | ztAlign_Top; break;\n"
			"		case ztGuiThemeValue_i32_WindowCloseButtonAnchor:           *result = ztAnchor_Right | ztAnchor_Top; break;\n"
			"		case ztGuiThemeValue_i32_WindowCloseButtonBehaviorFlags:    *result = ztGuiButtonBehaviorFlags_NoBackground | ztGuiButtonBehaviorFlags_OnPressDip; break;\n"
			"\n"
			"\n"
			"		case ztGuiThemeValue_i32_MenuFontID: {\n"
			"			*result = theme_data->font;\n"
			"			return true;\n"
			"		} break;\n"
			"\n"
			"		default: return false;  // return false to use the default\n"
			"	}\n"
			"\n"
			"	return true;\n"
			"}\n"
			"\n"
			"// ------------------------------------------------------------------------------------------------\n"
			"\n"
			"ZT_FUNCTION_POINTER_REGISTER(guiThemeUpdateItem, ztInternal ZT_FUNC_THEME_UPDATE_ITEM(guiThemeUpdateItem))\n"
			"{\n"
			"	ZT_PROFILE_GUI(\"guiThemeUpdateItem\");\n"
			"\n"
			"	r32 ppu = zt_pixelsPerUnit();\n"
			"\n"
			"	//	switch (item->type)\n"
			"	//	{\n"
			"	//\n"
			"	//		default: return true;  // return false to use the default (this is the default though)\n"
			"	//	}\n"
			"\n"
			"	return false;\n"
			"}\n"
			"\n"
			"// ------------------------------------------------------------------------------------------------\n"
			"\n"
			"ZT_FUNCTION_POINTER_REGISTER(guiThemeUpdateSubitem, ztInternal ZT_FUNC_THEME_UPDATE_SUBITEM(guiThemeUpdateSubitem))\n"
			"{\n"
			"	ZT_PROFILE_GUI(\"guiThemeUpdateSubitem\");\n"
			"\n"
			"	if (!_guiThemeIsCustom(item)) {\n"
			"		return false;\n"
			"	}\n"
			"	GuiThemeData *theme_data = (GuiThemeData*)theme->theme_data;\n"
			"	r32 ppu = zt_pixelsPerUnit();\n"
			"\n"
			"	switch (item->type)\n"
			"	{\n"
			"		case ztGuiItemType_Window: {\n"
			"			switch (subitem->type)\n"
			"			{\n"
			"				case ztGuiItemType_Button: {\n"
			"					char *name = zt_guiItemGetName(subitem);\n"
			"\n"
			"					if (zt_strEquals(name, \"Close\")) {\n"
			"						zt_guiButtonSetIcon(subitem, theme_data->sprite[GuiSprite_IconX]);\n"
			"					}\n"
			"					else if (zt_strEquals(name, \"Collapse\")) {\n"
			"						if (zt_bitIsSet(item->state_flags, zt_bit(ztGuiWindowInternalStates_Collapsed))) {\n"
			"							zt_guiButtonSetIcon(subitem, theme_data->sprite[GuiSprite_IconArrowRight]);\n"
			"						}\n"
			"						else {\n"
			"							zt_guiButtonSetIcon(subitem, theme_data->sprite[GuiSprite_IconArrowDown]);\n"
			"						}\n"
			"					}\n"
			"					zt_guiItemSetSize(subitem, zt_vec2(26 / ppu, 26 / ppu));\n"
			"				} break;\n"
			"			}\n"
			"		} break;\n"
			"\n"
			"		// ================================================================================================================================================================================================\n"
			"\n"
			"		case ztGuiItemType_CollapsingPanel: {\n"
			"			switch (subitem->type)\n"
			"			{\n"
			"				case ztGuiItemType_Button: {\n"
			"					if (zt_bitIsSet(item->state_flags, zt_bit(ztGuiCollapsingPanelInternalStates_Collapsed))) {\n"
			"						zt_guiButtonSetIcon(subitem, theme_data->sprite[GuiSprite_IconArrowRight]);\n"
			"					}\n"
			"					else {\n"
			"						zt_guiButtonSetIcon(subitem, theme_data->sprite[GuiSprite_IconArrowDown]);\n"
			"					}\n"
			"					zt_guiItemSetSize(subitem, zt_vec2(21 / ppu, 21 / ppu));\n"
			"				} break;\n"
			"			}\n"
			"\n"
			"		} break;\n"
			"\n"
			"		// ================================================================================================================================================================================================\n"
			"\n"
			"		case ztGuiItemType_Tree: {\n"
			"			switch (subitem->type)\n"
			"			{\n"
			"				case ztGuiItemType_Button: {\n"
			"					if (data == nullptr || *((bool*)data) == true) {\n"
			"						zt_guiButtonSetIcon(subitem, theme_data->sprite[GuiSprite_IconMinus]);\n"
			"					}\n"
			"					else {\n"
			"						zt_guiButtonSetIcon(subitem, theme_data->sprite[GuiSprite_IconPlus]);\n"
			"					}\n"
			"					zt_guiItemSetSize(subitem, zt_vec2(17 / ppu, 17 / ppu));\n"
			"				} break;\n"
			"			}\n"
			"		} break;\n"
			"\n"
			"		// ================================================================================================================================================================================================\n"
			"\n"
			"		case ztGuiItemType_CycleBox: {\n"
			"			r32 button_w = zt_guiThemeGetRValue(zt_guiItemGetTheme(item), ztGuiThemeValue_r32_CycleBoxButtonW, item);\n"
			"\n"
			"			subitem->size.x = button_w;\n"
			"			subitem->size.y = item->size.y;\n"
			"\n"
			"			switch (subitem->type)\n"
			"			{\n"
			"				case ztGuiItemType_Button: {\n"
			"					subitem->behavior_flags |= ztGuiButtonBehaviorFlags_NoBackground;\n"
			"\n"
			"					if (((ztDirection_Enum)(i32)data) == ztDirection_Left) {\n"
			"						zt_guiButtonSetIcon(subitem, theme_data->sprite[GuiSprite_IconArrowLeft]);\n"
			"						zt_guiItemSetPosition(subitem, ztAlign_Left, ztAnchor_Left, ztVec2::zero);\n"
			"					}\n"
			"					else if (((ztDirection_Enum)(i32)data) == ztDirection_Right) {\n"
			"						zt_guiButtonSetIcon(subitem, theme_data->sprite[GuiSprite_IconArrowRight]);\n"
			"						zt_guiItemSetPosition(subitem, ztAlign_Right, ztAnchor_Right, ztVec2::zero);\n"
			"					}\n"
			"				} break;\n"
			"			}\n"
			"\n"
			"		} break;\n"
			"\n"
			"		// ================================================================================================================================================================================================\n"
			"\n"
			"		default: return false; // return false to use the default\n"
			"	}\n"
			"\n"
			"	return true;\n"
			"}\n"
			"\n"
			);

		zt_fileWrite(&file,
			"// ================================================================================================================================================================================================\n"
			"\n"
			"ZT_FUNCTION_POINTER_REGISTER(guiThemeSizeItem, ztInternal ZT_FUNC_THEME_SIZE_ITEM(guiThemeSizeItem))\n"
			"{\n"
			"	ZT_PROFILE_GUI(\"guiThemeSizeItem\");\n"
			"\n"
			"	if (!_guiThemeIsCustom(item)) {\n"
			"		return false;\n"
			"	}\n"
			"\n"
			"	GuiThemeData *theme_data = (GuiThemeData*)theme->theme_data;\n"
			"\n"
			"	r32 ppu = zt_pixelsPerUnit();\n"
			"\n"
			"	switch (item->type)\n"
			"	{\n"
			"		case ztGuiItemType_Button: {\n"
			"			ztVec2 icon_size = item->button.icon ? item->button.icon->half_size * 2.f : ztVec2::zero;\n"
			"			ztFontID font = theme_data->font;\n"
			"			item->size = zt_bitIsSet(item->behavior_flags, ztGuiStaticTextBehaviorFlags_Fancy) ? zt_fontGetExtentsFancy(font, item->label) : zt_fontGetExtents(font, item->label);\n"
			"			item->size.x = zt_max(icon_size.x, item->size.x);\n"
			"			item->size.y += icon_size.y;\n"
			"\n"
			"			item->size += zt_vec2(21 / ppu, 21 / ppu);\n"
			"		} break;\n"
			"\n"
			"		// ================================================================================================================================================================================================\n"
			"\n"
			"		case ztGuiItemType_StaticText: {\n"
			"			ztFontID font = zt_bitIsSet(item->behavior_flags, ztGuiStaticTextBehaviorFlags_MonoSpaced) ? theme_data->font_monospace : theme_data->font;\n"
			"			item->size = zt_bitIsSet(item->behavior_flags, ztGuiStaticTextBehaviorFlags_Fancy) ? zt_fontGetExtentsFancy(font, item->label) : zt_fontGetExtents(font, item->label);\n"
			"		} break;\n"
			"\n"
			"		// ================================================================================================================================================================================================\n"
			"\n"
			"		case ztGuiItemType_Slider: {\n"
			"			if (item->slider.orient == ztGuiItemOrient_Horz) {\n"
			"				item->size = zt_vec2(5, zt_guiThemeGetRValue(theme, ztGuiThemeValue_r32_SliderHandleSize, item));\n"
			"			}\n"
			"			else {\n"
			"				item->size = zt_vec2(zt_guiThemeGetRValue(theme, ztGuiThemeValue_r32_SliderHandleSize, item), 5);\n"
			"			}\n"
			"		} break;\n"
			"\n"
			"		// ================================================================================================================================================================================================\n"
			"\n"
			"		case ztGuiItemType_Checkbox:\n"
			"		case ztGuiItemType_RadioButton: {\n"
			"			ztVec2 txt_size = zt_fontGetExtents(theme_data->font, item->label);\n"
			"			r32 padding     = zt_guiThemeGetRValue(theme, ztGuiThemeValue_r32_Padding, item);\n"
			"			r32 checkbox_w  = zt_guiThemeGetRValue(theme, ztGuiThemeValue_r32_CheckboxW, item);\n"
			"			r32 checkbox_h  = zt_guiThemeGetRValue(theme, ztGuiThemeValue_r32_CheckboxW, item);\n"
			"			item->size.x = txt_size.x + padding + checkbox_w;\n"
			"			item->size.y = zt_max(txt_size.y, checkbox_h);\n"
			"		} break;\n"
			"\n"
			"		// ================================================================================================================================================================================================\n"
			"\n"
			"		case ztGuiItemType_ComboBox: {\n"
			"			r32 base_width = 38 / ppu;\n"
			"\n"
			"			item->size.x = 160 / ppu;\n"
			"			item->size.y = zt_guiThemeGetRValue(theme, ztGuiThemeValue_r32_ComboboxButtonW, item);\n"
			"\n"
			"			r32 padding = zt_guiThemeGetRValue(theme, ztGuiThemeValue_r32_Padding, item);\n"
			"			ztFontID font = theme_data->font;\n"
			"\n"
			"			zt_fiz(item->combobox.contents_count) {\n"
			"				ztVec2 ext = zt_fontGetExtents(font, item->combobox.contents[i]);\n"
			"				item->size.x = zt_max(item->size.x, base_width + ext.x);\n"
			"				item->size.y = zt_max(item->size.y, ext.y + padding * 2);\n"
			"			}\n"
			"		} break;\n"
			"\n"
			"		// ================================================================================================================================================================================================\n"
			"\n"
			"		case ztGuiItemType_Menu: {\n"
			"			ztVec2 icon_orig = zt_vec2(zt_guiThemeGetRValue(theme, ztGuiThemeValue_r32_MenuSubmenuIconX, item), zt_guiThemeGetRValue(theme, ztGuiThemeValue_r32_MenuSubmenuIconY, item));\n"
			"			ztVec2 icon = icon_orig;\n"
			"			r32 padding = zt_guiThemeGetRValue(theme, ztGuiThemeValue_r32_Padding, item);\n"
			"			zt_fiz(item->menu.item_count) {\n"
			"				if (item->menu.icons[i] != nullptr) {\n"
			"					icon.x = zt_max(icon.x, item->menu.icons[i]->half_size.x * 2.f + padding * 2.f);\n"
			"					icon.y = zt_max(icon.y, item->menu.icons[i]->half_size.y * 2.f);\n"
			"				}\n"
			"			}\n"
			"\n"
			"			item->size = ztVec2::zero;\n"
			"\n"
			"			ztFontID font = theme_data->font;\n"
			"\n"
			"			zt_fiz(item->menu.item_count) {\n"
			"				ztVec2 ext = zt_fontGetExtents(font, item->menu.display[i]);\n"
			"\n"
			"				if (item->menu.ids[i] == ztInvalidID && zt_strStartsWith(item->menu.display[i], \"__\")) {\n"
			"					ext.y = zt_guiThemeGetRValue(theme, ztGuiThemeValue_r32_MenuSeparatorHeight, item);\n"
			"				}\n"
			"\n"
			"				item->size.y += zt_max(zt_max(icon.y, ext.y), icon_orig.y) + padding;\n"
			"				item->size.x = zt_max(item->size.x, ext.x + icon.x + icon_orig.x + padding * 3.f);\n"
			"			}\n"
			"			item->size.y += padding;\n"
			"		} break;\n"
			"\n"
			"		// ================================================================================================================================================================================================\n"
			"\n"
			"		case ztGuiItemType_CycleBox: {\n"
			"			r32 base_width = 144 / ppu;\n"
			"\n"
			"			item->size.x = 180 / ppu;\n"
			"			item->size.y = zt_guiThemeGetRValue(theme, ztGuiThemeValue_r32_TextEditDefaultH, item);\n"
			"\n"
			"			r32 padding = zt_guiThemeGetRValue(theme, ztGuiThemeValue_r32_Padding, item);\n"
			"			ztFontID font = theme_data->font;\n"
			"			zt_fiz(item->cyclebox.contents_count) {\n"
			"				ztVec2 ext = zt_fontGetExtents(font, item->cyclebox.contents[i]);\n"
			"				item->size.x = zt_max(item->size.x, base_width + ext.x);\n"
			"				item->size.y = zt_max(item->size.y, ext.y + padding * 2);\n"
			"			}\n"
			"\n"
			"		} break;\n"
			"\n"
			"		// ================================================================================================================================================================================================\n"
			"\n"
			"		default: return false; // return false to use the default\n"
			"	}\n"
			"	\n"
			"	return true;\n"
			"}\n"
			"\n"
			);

		zt_fileWrite(&file,
			"// ================================================================================================================================================================================================\n"
			"\n"
			"ZT_FUNCTION_POINTER_REGISTER(guiThemeRenderItem, ztInternal ZT_FUNC_THEME_RENDER_ITEM(guiThemeRenderItem))\n"
			"{\n"
			"	ZT_PROFILE_GUI(\"guiThemeRenderItem\");\n"
			"\n"
			"	GuiThemeData *theme_data = (GuiThemeData*)theme->theme_data;\n"
			"\n"
			"	r32 ppu = zt_pixelsPerUnit();\n"
			"\n"
			"	bool has_focus   = zt_bitIsSet(item->state_flags, zt_bit(ztGuiItemStates_HasFocus));\n"
			"	bool highlighted = zt_bitIsSet(item->gm->item_cache_flags[item->id], ztGuiManagerItemCacheFlags_MouseOver);\n"
			"	bool pressed     = (highlighted && item->gm->item_has_mouse == item) || (!highlighted && zt_bitIsSet(item->state_flags, zt_bit(ztGuiButtonInternalStates_IsToggled)));\n"
			"	bool enabled     = !zt_bitIsSet(item->state_flags, zt_bit(ztGuiItemStates_Disabled));\n"
			"\n"
			"	struct local\n"
			"	{\n"
			"		static i32 imageIndex(i32 img_normal, i32 img_highlight, i32 img_pressed, i32 img_disabled, ztGuiItem *item)\n"
			"		{\n"
			"			if (img_highlight < 0) img_highlight = img_normal;\n"
			"			if (img_pressed < 0) img_pressed = img_normal;\n"
			"			if (img_disabled < 0) img_pressed = img_normal;\n"
			"\n"
			"			bool disabled = zt_bitIsSet(item->state_flags, zt_bit(ztGuiItemStates_Disabled));\n"
			"			ztGuiItem *parent = item->parent;\n"
			"			while (!disabled && parent) {\n"
			"				if (zt_bitIsSet(parent->state_flags, zt_bit(ztGuiItemStates_Disabled))) {\n"
			"					disabled = true;\n"
			"					break;\n"
			"				}\n"
			"				parent = parent->parent;\n"
			"			}\n"
			"\n"
			"			if (disabled) return img_disabled;\n"
			"\n"
			"			bool highlighted = zt_bitIsSet(item->gm->item_cache_flags[item->id], ztGuiManagerItemCacheFlags_MouseOver);\n"
			"			bool pressed = (highlighted && item->gm->item_has_mouse == item);\n"
			"\n"
			"			if (pressed) return img_pressed;\n"
			"			if (highlighted) return img_highlight;\n"
			"\n"
			"			return img_normal;\n"
			"		}\n"
			"	};\n"
			"\n"
			"	switch (item->type)\n"
			"	{\n"
			"		case ztGuiItemType_Window: {\n"
			"			ZT_PROFILE_GUI(\"guiThemeRenderItem:Window\");\n"
			"\n"
			"			if (zt_bitIsSet(item->behavior_flags, ztGuiWindowBehaviorFlags_Modal)) {\n"
			"				zt_drawListPushColor(draw_list, zt_color(0,0,0, .75f));\n"
			"				zt_drawListAddSpriteNineSlice(draw_list, theme_data->sprite_nine_slice[GuiSpriteNineSlice_Solid], ztVec2::zero, zt_cameraOrthoGetViewportSize(item->gm->gui_camera));\n"
			"				zt_drawListPopColor(draw_list);\n"
			"			}\n"
			"\n"
			"			if (zt_bitIsSet(item->behavior_flags, ztGuiWindowBehaviorFlags_ShowTitle)) {\n"
			"				zt_drawListAddSpriteNineSlice(draw_list, theme_data->sprite_nine_slice[local::imageIndex(GuiSpriteNineSlice_WindowFrame, -1, -1, GuiSpriteNineSlice_WindowFrameDisabled, item)], item->pos, item->size);\n"
			"				zt_drawListAddText2D(draw_list, theme_data->font, item->label, zt_vec2(pos.x, pos.y + item->size.y / 2 - 6 / ppu), ztAlign_Center | ztAlign_Top, ztAnchor_Center | ztAnchor_Top);\n"
			"			}\n"
			"			else {\n"
			"				zt_drawListAddSpriteNineSlice(draw_list, theme_data->sprite_nine_slice[local::imageIndex(GuiSpriteNineSlice_PanelFrame, -1, -1, GuiSpriteNineSlice_PanelFrameDisabled, item)], pos, item->size);\n"
			"			}\n"
			"		} break;\n"
			"\n"
			"		// ================================================================================================================================================================================================\n"
			"\n"
			"		case ztGuiItemType_Panel: {\n"
			"			ZT_PROFILE_GUI(\"guiThemeRenderItem:Window\");\n"
			"\n"
			"			if (zt_bitIsSet(item->behavior_flags, ztGuiPanelBehaviorFlags_DrawBackground)) {\n"
			"				zt_drawListAddSpriteNineSlice(draw_list, theme_data->sprite_nine_slice[local::imageIndex(GuiSpriteNineSlice_PanelFrame, -1, -1, GuiSpriteNineSlice_PanelFrameDisabled, item)], pos, item->size);\n"
			"			}\n"
			"		} break;\n"
			"\n"
			"		// ================================================================================================================================================================================================\n"
			"\n"
			"		case ztGuiItemType_CollapsingPanel: {\n"
			"			ZT_PROFILE_GUI(\"guiThemeRenderItem:CollapsingPanel\");\n"
			"\n"
			"			if (zt_bitIsSet(item->state_flags, zt_bit(ztGuiCollapsingPanelInternalStates_Collapsed))) {\n"
			"				zt_drawListAddSpriteNineSlice(draw_list, theme_data->sprite_nine_slice[local::imageIndex(GuiSpriteNineSlice_PanelFrame, -1, -1, GuiSpriteNineSlice_PanelFrameDisabled, item)], pos, item->size);\n"
			"			}\n"
			"			else {\n"
			"				zt_drawListAddSpriteNineSlice(draw_list, theme_data->sprite_nine_slice[local::imageIndex(GuiSpriteNineSlice_CollapsingPanelFrame, -1, -1, GuiSpriteNineSlice_CollapsingPanelFrameDisabled, item)], pos, item->size);\n"
			"			}\n"
			"\n"
			"			if (item->label != nullptr) {\n"
			"				r32 panel_height = zt_guiThemeGetRValue(theme, ztGuiThemeValue_r32_CollapsingPanelHeight, item);\n"
			"\n"
			"				ztVec2 title_pos = zt_vec2(pos.x - ((item->size.x) / 2) + 25 / ppu, pos.y + (item->size.y - (panel_height)) / 2);\n"
			"				ztVec2 title_size;\n"
			"\n"
			"				//				if (!zt_bitIsSet(item->state_flags, zt_bit(ztGuiCollapsingPanelInternalStates_Collapsed)) {\n"
			"				//				}\n"
			"\n"
			"				zt_drawListAddText2D(draw_list, 0, item->label, title_pos, ztAlign_Left, ztAnchor_Left, &title_size);\n"
			"			}\n"
			"		} break;\n"
			"\n"
			"		// ================================================================================================================================================================================================\n"
			"\n"
			"		case ztGuiItemType_ToggleButton:\n"
			"		case ztGuiItemType_Button: {\n"
			"			ZT_PROFILE_GUI(\"guiThemeRenderItem:Button\");\n"
			"			ztFontID font = theme_data->font;\n"
			"			r32 scale = 1;\n"
			"\n"
			"			if (font != ztInvalidID) {\n"
			"				ztVec2 ext = zt_bitIsSet(item->behavior_flags, ztGuiStaticTextBehaviorFlags_Fancy) ? zt_fontGetExtentsFancy(font, item->label) : zt_fontGetExtents(font, item->label);\n"
			"				ztVec2 off = pos;\n"
			"\n"
			"				if (item->button.icon) {\n"
			"					off.y -= item->button.icon->half_size.y * 1;\n"
			"				}\n"
			"\n"
			"				ext *= scale;\n"
			"\n"
			"				if (item->align_flags != 0) {\n"
			"					if (zt_bitIsSet(item->align_flags, ztAlign_Left)) off.x -= (item->size.x - ext.x) / 2.f;\n"
			"					if (zt_bitIsSet(item->align_flags, ztAlign_Right)) off.x += (item->size.x - ext.x) / 2.f;\n"
			"					if (zt_bitIsSet(item->align_flags, ztAlign_Top)) off.y += (item->size.y - ext.y) / 2.f;\n"
			"					if (zt_bitIsSet(item->align_flags, ztAlign_Bottom)) off.y -= (item->size.y - ext.y) / 2.f;\n"
			"				}\n"
			"\n"
			"				ztVec2 text_ext = ztVec2::zero;\n"
			"\n"
			"				if (!zt_bitIsSet(item->behavior_flags, ztGuiButtonBehaviorFlags_NoBackground) || highlighted) {\n"
			"					if (item->type == ztGuiItemType_ToggleButton) {\n"
			"						bool toggled = zt_bitIsSet(item->state_flags, zt_bit(ztGuiButtonInternalStates_IsToggled));\n"
			"						zt_drawListAddSpriteNineSlice(draw_list, theme_data->sprite_nine_slice[local::imageIndex(toggled ? GuiSpriteNineSlice_ButtonToggleOn : GuiSpriteNineSlice_ButtonToggleNormal, GuiSpriteNineSlice_ButtonToggleHighlight, GuiSpriteNineSlice_ButtonTogglePressed, toggled ? GuiSpriteNineSlice_ButtonToggleOnDisabled : GuiSpriteNineSlice_ButtonToggleDisabled, item)], pos, item->size);\n"
			"					}\n"
			"					else {\n"
			"						zt_drawListAddSpriteNineSlice(draw_list, theme_data->sprite_nine_slice[local::imageIndex(GuiSpriteNineSlice_ButtonNormal, GuiSpriteNineSlice_ButtonHighlight, GuiSpriteNineSlice_ButtonPressed, GuiSpriteNineSlice_ButtonDisabled, item)], pos, item->size);\n"
			"					}\n"
			"				}\n"
			"\n"
			"				if (zt_bitIsSet(item->behavior_flags, ztGuiStaticTextBehaviorFlags_Fancy)) {\n"
			"					zt_drawListAddFancyText2D(draw_list, font, item->label, off, zt_vec2(scale, scale), item->align_flags, item->anchor_flags, &text_ext);\n"
			"				}\n"
			"				else {\n"
			"					zt_drawListAddText2D(draw_list, font, item->label, off, zt_vec2(scale, scale), item->align_flags, item->anchor_flags, &text_ext);\n"
			"				}\n"
			"\n"
			"				if (item->button.icon) {\n"
			"					if (text_ext.y > 0) {\n"
			"						off.y += 3 / ppu;\n"
			"					}\n"
			"\n"
			"					ztVec3 icon_pos = zt_vec3(off + zt_vec2(0, text_ext.y / 2.f + item->button.icon->half_size.y), 0);\n"
			"					zt_drawListAddSpriteFast(draw_list, item->button.icon, icon_pos, ztVec3::zero, zt_vec3(scale, scale, scale));\n"
			"				}\n"
			"			}\n"
			"			else return false;\n"
			"		} break;\n"
			"\n"
			"		// ================================================================================================================================================================================================\n"
			"\n"
			"		case ztGuiItemType_StaticText: {\n"
			"			ZT_PROFILE_GUI(\"guiThemeRenderItem:StaticText\");\n"
			"			ztFontID font = zt_bitIsSet(item->behavior_flags, ztGuiStaticTextBehaviorFlags_MonoSpaced) ? theme_data->font_monospace : theme_data->font;\n"
			"			r32 scale = 1;\n"
			"\n"
			"			if (font != ztInvalidID) {\n"
			"				ztVec2 ext = zt_bitIsSet(item->behavior_flags, ztGuiStaticTextBehaviorFlags_Fancy) ? zt_fontGetExtentsFancy(font, item->label) : zt_fontGetExtents(font, item->label);\n"
			"				ztVec2 off = pos;\n"
			"\n"
			"\n"
			"				ext *= scale;\n"
			"\n"
			"				if (item->align_flags != 0) {\n"
			"					if (zt_bitIsSet(item->align_flags, ztAlign_Left)) off.x -= (item->size.x - ext.x) / 2.f;\n"
			"					if (zt_bitIsSet(item->align_flags, ztAlign_Right)) off.x += (item->size.x - ext.x) / 2.f;\n"
			"					if (zt_bitIsSet(item->align_flags, ztAlign_Top)) off.y += (item->size.y - ext.y) / 2.f;\n"
			"					if (zt_bitIsSet(item->align_flags, ztAlign_Bottom)) off.y -= (item->size.y - ext.y) / 2.f;\n"
			"				}\n"
			"\n"
			"				ztVec2 text_ext = ztVec2::zero;\n"
			"\n"
			"				if (zt_bitIsSet(item->behavior_flags, ztGuiStaticTextBehaviorFlags_Fancy)) {\n"
			"					zt_drawListAddFancyText2D(draw_list, font, item->label, off, zt_vec2(scale, scale), item->align_flags, item->anchor_flags, &text_ext);\n"
			"				}\n"
			"				else {\n"
			"					zt_drawListAddText2D(draw_list, font, item->label, off, zt_vec2(scale, scale), item->align_flags, item->anchor_flags, &text_ext);\n"
			"				}\n"
			"			}\n"
			"			else return false;\n"
			"		} break;\n"
			"\n"
			"		// ================================================================================================================================================================================================\n"
			"\n"
			"		case ztGuiItemType_Checkbox:\n"
			"		case ztGuiItemType_RadioButton: {\n"
			"			ZT_PROFILE_GUI(\"guiThemeRenderItem:CheckRadio\");\n"
			"\n"
			"			bool checkbox = zt_bitIsSet(item->behavior_flags, ztGuiButtonInternalBehaviorFlags_IsCheckbox);\n"
			"			bool radio    = zt_bitIsSet(item->behavior_flags, ztGuiButtonInternalBehaviorFlags_IsRadio);\n"
			"\n"
			"			r32 checkbox_w = zt_guiThemeGetRValue(theme, item->type == ztGuiItemType_Checkbox ? ztGuiThemeValue_r32_CheckboxW : ztGuiThemeValue_r32_RadiobuttonW, item);\n"
			"			r32 checkbox_h = zt_guiThemeGetRValue(theme, item->type == ztGuiItemType_Checkbox ? ztGuiThemeValue_r32_CheckboxW : ztGuiThemeValue_r32_RadiobuttonW, item);\n"
			"			r32 padding    = zt_guiThemeGetRValue(theme, ztGuiThemeValue_r32_Padding, item);\n"
			"\n"
			"			ztVec2 box_size = zt_vec2(checkbox_w, checkbox_h);\n"
			"			ztVec2 box_pos, txt_size, txt_pos;\n"
			"\n"
			"			txt_size = item->label == nullptr ? ztVec2::zero : zt_fontGetExtents(theme_data->font, item->label);\n"
			"			if (zt_bitIsSet(item->behavior_flags, ztGuiCheckboxBehaviorFlags_RightText)) {\n"
			"				box_pos = zt_vec2((item->size.x - box_size.x) / -2.f, 0);\n"
			"				txt_pos = zt_vec2(box_pos.x + box_size.x / 2.f + padding, 0);\n"
			"			}\n"
			"			else {\n"
			"				txt_pos = zt_vec2(item->size.x / -2.f, 0);\n"
			"				box_pos = zt_vec2((item->size.x / 2.f) - (box_size.x / 2.f), 0);\n"
			"			}\n"
			"\n"
			"			zt_drawListAddText2D(draw_list, theme_data->font, item->label, zt_strLen(item->label), txt_pos + pos, ztAlign_Left, ztAnchor_Left);\n"
			"\n"
			"\n"
			"			box_pos += pos;\n"
			"\n"
			"			if (checkbox) {\n"
			"				zt_drawListAddSprite(draw_list, theme_data->sprite[local::imageIndex(GuiSprite_CheckBox, GuiSprite_CheckBoxHighlight, GuiSprite_CheckBoxPressed, GuiSprite_CheckBoxDisabled, item)], zt_vec3(box_pos, 0));\n"
			"\n"
			"				if (zt_bitIsSet(item->state_flags, zt_bit(ztGuiButtonInternalStates_IsToggled))) {\n"
			"					zt_drawListAddSprite(draw_list, theme_data->sprite[GuiSprite_IconCheck], zt_vec3(box_pos, 0));\n"
			"				}\n"
			"			}\n"
			"			else if (radio) {\n"
			"				zt_drawListAddSprite(draw_list, theme_data->sprite[local::imageIndex(GuiSprite_Radio, GuiSprite_RadioHighlight, GuiSprite_RadioPressed, GuiSprite_RadioDisabled, item)], zt_vec3(box_pos, 0));\n"
			"\n"
			"				if (zt_bitIsSet(item->state_flags, zt_bit(ztGuiButtonInternalStates_IsToggled))) {\n"
			"					zt_drawListAddSprite(draw_list, theme_data->sprite[GuiSprite_RadioSelect], zt_vec3(box_pos, 0));\n"
			"				}\n"
			"			}\n"
			"		} break;\n"
			"\n"
			);

		zt_fileWrite(&file,
			"		// ================================================================================================================================================================================================\n"
			"\n"
			"		case ztGuiItemType_TextEdit: {\n"
			"			ZT_PROFILE_GUI(\"guiThemeRenderItem:TextEdit\");\n"
			"\n"
			"			zt_drawListAddSpriteNineSlice(draw_list, theme_data->sprite_nine_slice[local::imageIndex(GuiSpriteNineSlice_TextEdit, -1, -1, GuiSpriteNineSlice_TextEditDisabled, item)], pos, item->size);\n"
			"\n"
			"			ztVec2 text_pos = zt_vec2(item->textedit.text_pos[0], item->textedit.text_pos[1] + (-4 / zt_pixelsPerUnit()));\n"
			"\n"
			"			if (item->textedit.select_beg != item->textedit.select_end) {\n"
			"				int sel_beg = zt_min(item->textedit.select_beg, item->textedit.select_end);\n"
			"				int sel_end = zt_max(item->textedit.select_beg, item->textedit.select_end);\n"
			"\n"
			"				while (sel_beg < sel_end) {\n"
			"					ztVec2 pos_beg = zt_guiTextEditGetCharacterPos(item, sel_beg, false);\n"
			"\n"
			"					int idx_end_line = zt_strFindPos(item->textedit.text_buffer, \"\\n\", sel_beg);\n"
			"					if (idx_end_line == ztStrPosNotFound || idx_end_line > sel_end) {\n"
			"						idx_end_line = sel_end;\n"
			"					}\n"
			"\n"
			"					ztVec2 pos_end = zt_guiTextEditGetCharacterPos(item, idx_end_line, true);\n"
			"\n"
			"					ztVec2 pos_size = zt_vec2(pos_end.x - pos_beg.x, pos_beg.y - pos_end.y);\n"
			"					ztVec2 pos_center = zt_vec2(pos_beg.x + pos_size.x / 2.f, pos_beg.y - pos_size.y / 2.f);\n"
			"\n"
			"					zt_drawListPushColor(draw_list, ZT_GUI_HIGHLIGHT_COLOR);\n"
			"					zt_drawListAddSpriteNineSlice(draw_list, theme_data->sprite_nine_slice[GuiSpriteNineSlice_Solid], text_pos + pos_center, pos_size);\n"
			"					zt_drawListPopColor(draw_list);\n"
			"\n"
			"					sel_beg = idx_end_line + 1;\n"
			"				}\n"
			"			}\n"
			"\n"
			"			ztVec3 dlpos = zt_vec3(text_pos, 0);\n"
			"			zt_alignToPixel(&dlpos, zt_pixelsPerUnit());\n"
			"			zt_drawListAddDrawList(draw_list, item->draw_list, dlpos);\n"
			"\n"
			"			if (item->gm->focus_item == item) {\n"
			"				if (item->textedit.cursor_vis) {\n"
			"					ztFontID font = theme_data->font;\n"
			"					ztVec2 cursor_pos = text_pos + zt_vec2(item->textedit.cursor_xy[0], item->textedit.cursor_xy[1]);\n"
			"					ztVec2 cursor_size = zt_fontGetExtents(font, \"|\");\n"
			"					cursor_pos.x -= cursor_size.x / 2;\n"
			"					zt_drawListAddText2D(draw_list, font, \"|\", cursor_pos, ztAlign_Left | ztAlign_Top, ztAnchor_Left | ztAnchor_Top);\n"
			"				}\n"
			"			}\n"
			"\n"
			"		} break;\n"
			"\n"
			"		// ================================================================================================================================================================================================\n"
			"\n"
			"		case ztGuiItemType_Slider: {\n"
			"			ZT_PROFILE_GUI(\"guiThemeRenderItem:Slider\");\n"
			"			ztVec2 handle_pos, handle_size;\n"
			"\n"
			"			if (item->slider.drag_state.dragging) {\n"
			"				highlighted = true;\n"
			"			}\n"
			"\n"
			"			r32 handle_w = zt_guiThemeGetRValue(theme, ztGuiThemeValue_r32_SliderHandleSize, item);\n"
			"			r32 scale = 1.f;\n"
			"\n"
			"			if (item->slider.orient == ztGuiItemOrient_Horz) {\n"
			"				handle_pos = pos + zt_vec2(item->slider.handle_pos, 0);\n"
			"				handle_size = zt_vec2(zt_max(8 / ppu, item->slider.handle_size), item->size.y);\n"
			"\n"
			"				zt_drawListAddSpriteNineSlice(draw_list, theme_data->sprite_nine_slice[local::imageIndex(GuiSpriteNineSlice_SliderHorzBar, -1, -1, GuiSpriteNineSlice_SliderHorzBarDisabled, item)], pos, zt_vec2(item->size.x, 10 / ppu));\n"
			"				zt_drawListAddSpriteFast(draw_list, theme_data->sprite[local::imageIndex(GuiSprite_SliderHorzHandle, GuiSprite_SliderHorzHandleHighlight, GuiSprite_SliderHorzHandlePressed, GuiSprite_SliderHorzHandleDisabled, item)], zt_vec3(handle_pos, 0), ztVec3::zero, zt_vec3(scale, scale, 1));\n"
			"			}\n"
			"			else {\n"
			"				handle_pos = pos + zt_vec2(0, item->slider.handle_pos);\n"
			"				handle_size = zt_vec2(item->size.x, zt_max(10 / ppu, item->slider.handle_size));\n"
			"\n"
			"				zt_drawListAddSpriteNineSlice(draw_list, theme_data->sprite_nine_slice[local::imageIndex(GuiSpriteNineSlice_SliderVertBar, -1, -1, GuiSpriteNineSlice_SliderHorzBarDisabled, item)], pos, zt_vec2(10 / ppu, item->size.y));\n"
			"				zt_drawListAddSpriteFast(draw_list, theme_data->sprite[local::imageIndex(GuiSprite_SliderVertHandle, GuiSprite_SliderVertHandleHighlight, GuiSprite_SliderVertHandlePressed, GuiSprite_SliderVertHandleDisabled, item)], zt_vec3(handle_pos, 0), ztVec3::zero, zt_vec3(scale, scale, 1));\n"
			"			}\n"
			"		} break;\n"
			"\n"
			"		// ================================================================================================================================================================================================\n"
			"\n"
			"		case ztGuiItemType_Scrollbar: {\n"
			"			ZT_PROFILE_GUI(\"guiThemeRenderItem:ScrollbarSlider\");\n"
			"\n"
			"			ztVec2 handle_pos, handle_size;\n"
			"\n"
			"			if (item->slider.drag_state.dragging) {\n"
			"				highlighted = true;\n"
			"			}\n"
			"\n"
			"			r32 scrollbar_button_w = zt_guiThemeGetRValue(theme, ztGuiThemeValue_r32_ScrollbarButtonW, item);\n"
			"\n"
			"			if (item->slider.orient == ztGuiItemOrient_Horz) {\n"
			"				handle_pos = pos + zt_vec2(item->slider.handle_pos, 0);\n"
			"				handle_size = zt_vec2(zt_max(10 / ppu, item->slider.handle_size - (2 / ppu)), item->size.y - (2 / ppu));\n"
			"\n"
			"				zt_drawListAddSpriteNineSlice(draw_list, theme_data->sprite_nine_slice[local::imageIndex(GuiSpriteNineSlice_ScrollbarHorzBackground, -1, -1, GuiSpriteNineSlice_ScrollbarHorzBackgroundDisabled, item)], pos, zt_vec2(item->size.x, 10 / ppu));\n"
			"				zt_drawListAddSpriteNineSlice(draw_list, theme_data->sprite_nine_slice[local::imageIndex(GuiSpriteNineSlice_ScrollbarHorzHandle, GuiSpriteNineSlice_ScrollbarHorzHandleHighlight, GuiSpriteNineSlice_ScrollbarHorzHandlePressed, GuiSpriteNineSlice_ScrollbarHorzHandleDisabled, item)], handle_pos, handle_size);\n"
			"			}\n"
			"			else {\n"
			"				handle_pos = pos + zt_vec2(0, item->slider.handle_pos);\n"
			"				handle_size = zt_vec2(item->size.x - (2 / ppu), zt_max(10 / ppu, item->slider.handle_size - (2 / ppu)));\n"
			"\n"
			"				zt_drawListAddSpriteNineSlice(draw_list, theme_data->sprite_nine_slice[local::imageIndex(GuiSpriteNineSlice_ScrollbarVertBackground, -1, -1, GuiSpriteNineSlice_ScrollbarVertBackgroundDisabled, item)], pos, zt_vec2(10 / ppu, item->size.y));\n"
			"				zt_drawListAddSpriteNineSlice(draw_list, theme_data->sprite_nine_slice[local::imageIndex(GuiSpriteNineSlice_ScrollbarVertHandle, GuiSpriteNineSlice_ScrollbarVertHandleHighlight, GuiSpriteNineSlice_ScrollbarVertHandlePressed, GuiSpriteNineSlice_ScrollbarVertHandleDisabled, item)], handle_pos, handle_size);\n"
			"			}\n"
			"		} break;\n"
			"\n"
			"		// ================================================================================================================================================================================================\n"
			"\n"
			"		case ztGuiItemType_ComboBox: {\n"
			"			ZT_PROFILE_GUI(\"guiThemeRenderItem:ComboBox\");\n"
			"\n"
			"			zt_drawListAddSpriteNineSlice(draw_list, theme_data->sprite_nine_slice[local::imageIndex(GuiSpriteNineSlice_ComboBox, GuiSpriteNineSlice_ComboBoxHighlight, GuiSpriteNineSlice_ComboBoxPressed, GuiSpriteNineSlice_ComboBoxDisabled, item)], pos, item->size);\n"
			"\n"
			"			r32 padding = zt_guiThemeGetRValue(theme, ztGuiThemeValue_r32_Padding, item);\n"
			"\n"
			"			r32 button_w = zt_guiThemeGetRValue(theme, ztGuiThemeValue_r32_ComboboxButtonW, item);\n"
			"\n"
			"			if (item->combobox.selected >= 0 && item->combobox.selected < item->combobox.contents_count) {\n"
			"				r32 width = item->size.x;\n"
			"\n"
			"				pos.x = (pos.x) + (width / -2.f + padding * 2.f);\n"
			"\n"
			"				if (item->combobox.popup != nullptr) {\n"
			"					if (item->combobox.popup->menu.icons[item->combobox.selected]) {\n"
			"						ztSprite *sprite = item->combobox.popup->menu.icons[item->combobox.selected];\n"
			"\n"
			"						zt_drawListAddSprite(draw_list, sprite, zt_vec3(pos.x + sprite->half_size.x, pos.y, 0));\n"
			"\n"
			"						pos.x += sprite->half_size.x * 2 + padding;\n"
			"					}\n"
			"				}\n"
			"				zt_drawListAddFancyText2D(draw_list, theme_data->font, item->combobox.contents[item->combobox.selected], pos, ztAlign_Left, ztAnchor_Left);\n"
			"			}\n"
			"\n"
			"		} break;\n"
			"\n"
			"		// ================================================================================================================================================================================================\n"
			"\n"
			"		case ztGuiItemType_MenuBar: {\n"
			"			ZT_PROFILE_GUI(\"guiThemeRenderItem:MenuBar\");\n"
			"\n"
			"			zt_drawListAddSpriteNineSlice(draw_list, theme_data->sprite_nine_slice[local::imageIndex(GuiSpriteNineSlice_MenuBar, -1, -1, GuiSpriteNineSlice_MenuBarDisabled, item)], pos, item->size);\n"
			"\n"
			"			r32 padding = zt_guiThemeGetRValue(theme, ztGuiThemeValue_r32_Padding, item);\n"
			"			pos.x -= (item->size.x - padding * 2.f) / 2.f;\n"
			"			pos.y += (item->size.y - padding * 2.f) / 2.f;\n"
			"\n"
			"			ztVec2 icon;\n"
			"			icon.x = zt_guiThemeGetRValue(theme, ztGuiThemeValue_r32_MenuSubmenuIconX, item);\n"
			"			icon.y = zt_guiThemeGetRValue(theme, ztGuiThemeValue_r32_MenuSubmenuIconY, item);\n"
			"\n"
			"			zt_fiz(item->menu.item_count) {\n"
			"				if (item->menu.icons[i] != nullptr) {\n"
			"					icon.x = zt_max(icon.x, item->menu.icons[i]->half_size.x * 2.f + padding * 2.f);\n"
			"					icon.y = zt_max(icon.y, item->menu.icons[i]->half_size.y * 2.f);\n"
			"				}\n"
			"			}\n"
			"\n"
			"			zt_fiz(item->menu.item_count) {\n"
			"				ztVec2 ext = zt_fontGetExtents(theme_data->font, item->menu.display[i]);\n"
			"\n"
			"				if (icon.y > ext.y) ext.y = icon.y;\n"
			"				if (!(item->menu.ids[i] == ztInvalidID && zt_strStartsWith(item->menu.display[i], \"__\"))) {\n"
			"					if (item->menu.highlighted == i && highlighted) {\n"
			"						zt_drawListPushColor(draw_list, ZT_GUI_HIGHLIGHT_COLOR);\n"
			"						zt_drawListAddSpriteNineSlice(draw_list, theme_data->sprite_nine_slice[GuiSpriteNineSlice_Solid], zt_vec2(pos.x + ext.x / 2, pos.y - ext.y / 2.f), zt_vec2(ext.x + padding * 2, ext.y + 6 / ppu));\n"
			"						zt_drawListPopColor(draw_list);\n"
			"					}\n"
			"					zt_drawListAddText2D(draw_list, theme_data->font, item->menu.display[i], zt_vec2(pos.x + icon.x, pos.y - ext.y / 2.f), ztAlign_Left, ztAnchor_Left);\n"
			"\n"
			"					if (item->menu.icons[i] != nullptr) {\n"
			"						r32 y = zt_max(item->menu.icons[i]->half_size.y, ext.y / 2.f);\n"
			"						zt_drawListAddSprite(draw_list, item->menu.icons[i], zt_vec3(pos.x + padding + item->menu.icons[i]->half_size.x, pos.y - y, 0));\n"
			"					}\n"
			"				}\n"
			"\n"
			"				pos.x += icon.x + padding + ext.x + padding;\n"
			"			}\n"
			"		} break;\n"
			"\n"
			"		// ================================================================================================================================================================================================\n"
			"\n"
			"		case ztGuiItemType_Menu: {\n"
			"			ZT_PROFILE_GUI(\"guiThemeRenderItem:Menu\");\n"
			"\n"
			"			zt_drawListAddSpriteNineSlice(draw_list, theme_data->sprite_nine_slice[local::imageIndex(GuiSpriteNineSlice_Menu, -1, -1, -1, item)], pos, item->size);\n"
			"\n"
			"			r32 padding = zt_guiThemeGetRValue(theme, ztGuiThemeValue_r32_Padding, item);\n"
			"			pos.x -= (item->size.x - padding * 2.f) / 2.f;\n"
			"			pos.y += (item->size.y - padding * 2.f) / 2.f;\n"
			"\n"
			"			ztVec2 icon;\n"
			"			icon.x = zt_guiThemeGetRValue(theme, ztGuiThemeValue_r32_MenuSubmenuIconX, item);\n"
			"			icon.y = zt_guiThemeGetRValue(theme, ztGuiThemeValue_r32_MenuSubmenuIconY, item);\n"
			"\n"
			"			zt_fiz(item->menu.item_count) {\n"
			"				if (item->menu.icons[i] != nullptr) {\n"
			"					icon.x = zt_max(icon.x, item->menu.icons[i]->half_size.x * 2.f + padding * 2.f);\n"
			"					icon.y = zt_max(icon.y, item->menu.icons[i]->half_size.y * 2.f);\n"
			"				}\n"
			"			}\n"
			"\n"
			"			ztVec2 size = item->size;\n"
			"\n"
			"			if (zt_guiItemIsVisible(item->menu.scrollbar_vert)) {\n"
			"				pos.y += (item->menu.full_size.y - item->size.y) * zt_guiScrollbarGetValue(item->menu.scrollbar_vert);\n"
			"				size.x -= item->menu.scrollbar_vert->size.x;\n"
			"			}\n"
			"			if (zt_guiItemIsVisible(item->menu.scrollbar_horz)) {\n"
			"				pos.x -= (item->menu.full_size.x - item->size.x) * zt_guiScrollbarGetValue(item->menu.scrollbar_horz);\n"
			"			}\n"
			"\n"
			"			ztFontID font = theme_data->font;\n"
			"\n"
			"			zt_fiz(item->menu.item_count) {\n"
			"				ztVec2 ext = zt_fontGetExtents(font, item->menu.display[i]);\n"
			"\n"
			"				if (item->menu.ids[i] == ztInvalidID && zt_strStartsWith(item->menu.display[i], \"__\")) {\n"
			"					ext.y = zt_guiThemeGetRValue(theme, ztGuiThemeValue_r32_MenuSeparatorHeight, item);\n"
			"\n"
			"					ztVec2 lpos = pos;\n"
			"					lpos.y -= ext.y / 2.f;\n"
			"					zt_drawListAddSprite(draw_list, theme_data->sprite[GuiSprite_HorzLine], zt_vec3(pos.x + item->size.x / 2 - padding, pos.y, 0), ztVec3::zero, zt_vec3(item->size.x, 1, 1));\n"
			"				}\n"
			"				else {\n"
			"					if (item->menu.highlighted == i && highlighted) {\n"
			"						zt_drawListPushColor(draw_list, ZT_GUI_HIGHLIGHT_COLOR);\n"
			"						zt_drawListAddSpriteNineSlice(draw_list, theme_data->sprite_nine_slice[GuiSpriteNineSlice_Solid], zt_vec2(pos.x + size.x / 2 - padding, pos.y - ext.y / 2.f + 1 / ppu), zt_vec2(size.x - padding * 2.f, ext.y + 4 / ppu));\n"
			"						zt_drawListPopColor(draw_list);\n"
			"					}\n"
			"					zt_drawListAddText2D(draw_list, font, item->menu.display[i], zt_vec2(pos.x + icon.x + padding, pos.y - ext.y / 2.f), ztAlign_Left, ztAnchor_Left);\n"
			"\n"
			"					if (item->menu.icons[i] != nullptr) {\n"
			"						r32 y = zt_max(item->menu.icons[i]->half_size.y, ext.y / 2.f);\n"
			"						zt_drawListAddSprite(draw_list, item->menu.icons[i], zt_vec3(pos.x + padding + item->menu.icons[i]->half_size.x, pos.y - y, 0));\n"
			"					}\n"
			"				}\n"
			"\n"
			"\n"
			"				if (item->menu.submenus[i] != nullptr) {\n"
			"					r32 icon_x = zt_guiThemeGetRValue(theme, ztGuiThemeValue_r32_MenuSubmenuIconX, item);\n"
			"					r32 icon_y = zt_guiThemeGetRValue(theme, ztGuiThemeValue_r32_MenuSubmenuIconY, item);\n"
			"					r32 y = zt_max(icon_y, ext.y) / 2.f;\n"
			"					zt_drawListAddText2D(draw_list, font, \">\", zt_vec2((item->size.x + pos.x) - (padding * 3 + icon_x / 2.f), pos.y - y + padding));\n"
			"				}\n"
			"\n"
			"				pos.y -= zt_max(icon.y, ext.y) + padding;\n"
			"			}\n"
			"		} break;\n"
			"\n"
			);

		zt_fileWrite(&file,
			"		// ================================================================================================================================================================================================\n"
			"\n"
			"		case ztGuiItemType_Tree: {\n"
			"			ZT_PROFILE_GUI(\"guiThemeRenderItem:Tree\");\n"
			"\n"
			"			zt_drawListAddSpriteNineSlice(draw_list, theme_data->sprite_nine_slice[local::imageIndex(GuiSpriteNineSlice_PanelFrame, -1, -1, GuiSpriteNineSlice_PanelFrameDisabled, item)], pos, item->size);\n"
			"\n"
			"			if (item->tree.active_item != nullptr) {\n"
			"				bool visible = true;\n"
			"				ztGuiItem::ztTreeItem *parent = item->tree.active_item->parent;\n"
			"				while (parent) {\n"
			"					if (!parent->expanded) {\n"
			"						visible = false;\n"
			"						break;\n"
			"					}\n"
			"					parent = parent->parent;\n"
			"				}\n"
			"				if (visible) {\n"
			"					ztGuiItem *active = item->tree.active_item->item;\n"
			"					if (active) {\n"
			"						ztVec2 npos = zt_guiItemPositionLocalToScreen(active, ztVec2::zero);\n"
			"\n"
			"						zt_drawListPushColor(draw_list, ZT_GUI_HIGHLIGHT_COLOR);\n"
			"						zt_drawListAddSpriteNineSlice(draw_list, theme_data->sprite_nine_slice[GuiSpriteNineSlice_Solid], zt_vec2(pos.x, npos.y), zt_vec2(item->size.x, active->size.y));\n"
			"						zt_drawListPopColor(draw_list);\n"
			"					}\n"
			"				}\n"
			"			}\n"
			"\n"
			"		} break;\n"
			"\n"
			"		// ================================================================================================================================================================================================\n"
			"\n"
			"		case ztGuiItemType_SpriteDisplay: {\n"
			"			ZT_PROFILE_GUI(\"guiThemeRenderItem:sprite\");\n"
			"\n"
			"			if (*(ztVec4*)item->sprite_display.bgcolor != ztVec4::zero) {\n"
			"				zt_drawListAddSpriteNineSlice(draw_list, theme_data->sprite_nine_slice[local::imageIndex(GuiSpriteNineSlice_PanelFrame, -1, -1, GuiSpriteNineSlice_PanelFrameDisabled, item)], pos, item->size);\n"
			"			}\n"
			"\n"
			"			if (item->sprite_display.sprite_anim_controller) {\n"
			"				ztSprite *sprite = zt_spriteAnimControllerActiveSprite(item->sprite_display.sprite_anim_controller);\n"
			"				if (sprite != nullptr) {\n"
			"					zt_drawListAddSprite(draw_list, sprite, zt_vec3(pos, 0), ztVec3::zero, zt_vec3(item->sprite_display.scale[0], item->sprite_display.scale[1], 1));\n"
			"				}\n"
			"			}\n"
			"			else {\n"
			"				zt_drawListAddGuiThemeSprite(draw_list, item->sprite_display.sprite, pos, item->size);\n"
			"			}\n"
			"		} break;\n"
			"\n"
			"		// ================================================================================================================================================================================================\n"
			"\n"
			"		case ztGuiItemType_Spinner: {\n"
			"			ZT_PROFILE_GUI(\"guiThemeRenderItem:Spinner\");\n"
			"\n"
			"\n"
			"			zt_drawListPushColor(draw_list, ztColor_DarkGray);\n"
			"			zt_drawListAddSpriteNineSlice(draw_list, theme_data->sprite_nine_slice[GuiSpriteNineSlice_Solid], pos, item->size);\n"
			"			zt_drawListPopColor(draw_list);\n"
			"\n"
			"			zt_drawListAddSprite(draw_list, theme_data->sprite[GuiSprite_HorzLine], zt_vec3(pos.x, pos.y, 0), ztVec3::zero, zt_vec3(item->size.x - 0 / ppu, 1, 1));\n"
			"\n"
			"			r32 padding = zt_guiThemeGetRValue(theme, ztGuiThemeValue_r32_Padding, item);\n"
			"			\n"
			"			ztVec3 pos_txt_p = zt_vec3(pos.x, (pos.y + (item->size.y / 2)) - (6 / ppu), 0);\n"
			"			ztVec3 pos_txt_m = zt_vec3(pos.x, (pos.y - (item->size.y / 2)) + (6 / ppu), 0);\n"
			"\n"
			"			zt_drawListAddSprite(draw_list, theme_data->sprite[GuiSprite_IconArrowUp], pos_txt_p);\n"
			"			zt_drawListAddSprite(draw_list, theme_data->sprite[GuiSprite_IconArrowDown], pos_txt_m);\n"
			"		} break;\n"
			"\n"
			"		// ================================================================================================================================================================================================\n"
			"\n"
			"		case ztGuiItemType_ListBox: {\n"
			"			ZT_PROFILE_GUI(\"guiThemeRenderItem:ListBox\");\n"
			"\n"
			"			zt_drawListAddSpriteNineSlice(draw_list, theme_data->sprite_nine_slice[local::imageIndex(GuiSpriteNineSlice_PanelFrame, -1, -1, GuiSpriteNineSlice_PanelFrameDisabled, item)], pos, item->size);\n"
			"\n"
			"			r32 padding = zt_guiThemeGetRValue(theme, ztGuiThemeValue_r32_Padding, item);\n"
			"\n"
			"			int item_drawn = 0;\n"
			"			zt_fiz(item->listbox.item_count) {\n"
			"				item_drawn += 1;\n"
			"\n"
			"				if (!zt_guiItemIsVisible(item->listbox.items[i])) {\n"
			"					if (item->listbox.hidden[i]) {\n"
			"						item_drawn -= 1;\n"
			"					}\n"
			"					continue;\n"
			"				}\n"
			"\n"
			"				if (item->listbox.selected[i]) {\n"
			"					zt_drawListPushColor(draw_list, ZT_GUI_HIGHLIGHT_COLOR);\n"
			"					zt_drawListAddSpriteNineSlice(draw_list, theme_data->sprite_nine_slice[GuiSpriteNineSlice_Solid], pos + item->listbox.container->pos + zt_vec2(0, item->listbox.items[i]->pos.y), zt_vec2(item->listbox.container->size.x - 2 / ppu, item->listbox.items[i]->size.y + 2 / ppu));\n"
			"					zt_drawListPopColor(draw_list);\n"
			"				}\n"
			"				else if (zt_bitIsSet(item->behavior_flags, ztGuiListBoxBehaviorFlags_AlternateRowColor) && item_drawn % 2 == 0) {\n"
			"					zt_drawListAddSolidRect2D(draw_list, pos + item->listbox.container->pos + zt_vec2(0, item->listbox.items[i]->pos.y), zt_vec2(item->listbox.container->size.x, item->listbox.items[i]->size.y), zt_color(.5f, .5f, 1, .125f));\n"
			"				}\n"
			"			}\n"
			"		} break;\n"
			"\n"
			"		// ================================================================================================================================================================================================\n"
			"\n"
			"		case ztGuiItemType_CycleBox: {\n"
			"			if (item->cyclebox.selected >= 0 && item->cyclebox.selected < item->cyclebox.contents_count) {\n"
			"				zt_drawListAddFancyText2D(draw_list, theme_data->font, item->cyclebox.contents[item->cyclebox.selected], pos);\n"
			"			}\n"
			"		} break;\n"
			"\n"
			"		// ================================================================================================================================================================================================\n"
			"\n"
			"		default: return false; // return false to use the default (this is the default though)\n"
			"	}\n"
			"\n"
			"	return true;\n"
			"}\n"
			"\n"
			"// ------------------------------------------------------------------------------------------------\n"
			"\n"
			"ZT_FUNCTION_POINTER_REGISTER(guiThemePostRenderItem, ztInternal ZT_FUNC_THEME_POSTRENDER_ITEM(guiThemePostRenderItem))\n"
			"{\n"
			"	ZT_PROFILE_GUI(\"guiThemePostRenderItem\");\n"
			"\n"
			"	GuiThemeData *theme_data = (GuiThemeData*)theme->theme_data;\n"
			"\n"
			"	return false;\n"
			"}\n"
			"\n"
			"// ================================================================================================================================================================================================\n"
			"\n"
			"ztInternal ztGuiThemeSprite _guiThemeMakeThemeSprite(ztSprite s)\n"
			"{\n"
			"	ztGuiThemeSprite gts;\n"
			"	gts.type = ztGuiThemeSpriteType_Sprite;\n"
			"	gts.s = s;\n"
			"	return gts;\n"
			"}\n"
			"\n"
			"// ================================================================================================================================================================================================\n"
			"\n"
			"ztInternal ztGuiThemeSprite _guiThemeMakeThemeSprite(ztSpriteNineSlice sns)\n"
			"{\n"
			"	ztGuiThemeSprite gts;\n"
			"	gts.type = ztGuiThemeSpriteType_SpriteNineSlice;\n"
			"	gts.sns = sns;\n"
			"	return gts;\n"
			"}\n"
			"\n"
			"// ================================================================================================================================================================================================\n"
			"\n"
			"ZT_FUNCTION_POINTER_REGISTER(guiThemeRenderPassBegin, ZT_FUNC_THEME_RENDER_PASS_BEGIN(guiThemeRenderPassBegin))\n"
			"{\n"
			"}\n"
			"\n"
			"// ================================================================================================================================================================================================\n"
			"\n"
			"ZT_FUNCTION_POINTER_REGISTER(guiThemeRenderPassEnd, ZT_FUNC_THEME_RENDER_PASS_END(guiThemeRenderPassEnd))\n"
			"{\n"
			"}\n"
			"\n"
			"// ================================================================================================================================================================================================\n"
			"\n"
			);

		zt_fileWrite(&file,
			"bool guiThemeMake(ztGuiTheme *theme, ztAssetManager *asset_manager, ztGame *game)\n"
			"{\n"
			"	zt_returnValOnNull(theme, false);\n"
			"	zt_returnValOnNull(asset_manager, false);\n"
			"\n"
			"	GuiThemeData *theme_data = zt_mallocStruct(GuiThemeData);\n"
			"\n"
			"	theme_data->texture = zt_textureMake(asset_manager, zt_assetLoad(asset_manager, \"textures/gui.png\"), ztTextureFlags_PixelPerfect);\n"
			"	if (theme_data->texture == ztInvalidID) {\n"
			"		return false;\n"
			"	}\n"
			"\n"
			"	zt_spriteManagerMake(&theme_data->sprite_manager, GuiSpriteNineSlice_MAX + GuiSprite_MAX + 1);\n"
			"\n"
			"	if (!zt_spriteManagerLoad(&theme_data->sprite_manager, asset_manager, zt_assetLoad(asset_manager, \"textures/gui.spr\"), theme_data->texture)) {\n"
			"		return false;\n"
			"	}\n"
			"\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_WindowFrame                     ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_window_frame\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_WindowFrameDisabled             ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_window_frame_disabled\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_PanelFrame                      ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_panel_frame\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_PanelFrameDisabled              ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_panel_frame_disabled\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_CollapsingPanelFrame            ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_collapsing_panel_frame\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_CollapsingPanelFrameDisabled    ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_collapsing_panel_frame_disabled\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_ButtonNormal                    ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_button_normal\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_ButtonHighlight                 ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_button_highlight\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_ButtonPressed                   ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_button_pressed\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_ButtonDisabled                  ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_button_disabled\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_ButtonToggleNormal              ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_toggle_button_normal\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_ButtonToggleHighlight           ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_toggle_button_highlight\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_ButtonTogglePressed             ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_toggle_button_pressed\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_ButtonToggleDisabled            ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_toggle_button_disabled\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_ButtonToggleOn                  ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_toggle_button_on\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_ButtonToggleOnDisabled          ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_toggle_button_on_disabled\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_Menu                            ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_menu\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_MenuBar                         ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_menubar\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_MenuBarDisabled                 ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_menubar_disabled\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_TextEdit                        ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_textedit\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_TextEditDisabled                ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_textedit_disabled\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_ScrollbarVertBackground         ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_scrollbar_vert_background\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_ScrollbarVertBackgroundDisabled ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_scrollbar_vert_background_disabled\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_ScrollbarVertHandle             ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_scrollbar_vert_handle\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_ScrollbarVertHandleHighlight    ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_scrollbar_vert_handle_highlight\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_ScrollbarVertHandlePressed      ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_scrollbar_vert_handle_pressed\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_ScrollbarVertHandleDisabled     ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_scrollbar_vert_handle_disabled\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_ScrollbarHorzBackground         ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_scrollbar_horz_background\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_ScrollbarHorzBackgroundDisabled ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_scrollbar_horz_background_disabled\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_ScrollbarHorzHandle             ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_scrollbar_horz_handle\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_ScrollbarHorzHandleHighlight    ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_scrollbar_horz_handle_highlight\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_ScrollbarHorzHandlePressed      ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_scrollbar_horz_handle_pressed\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_ScrollbarHorzHandleDisabled     ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_scrollbar_horz_handle_disabled\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_SliderVertBar                   ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_slider_vert_bar\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_SliderVertBarDisabled           ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_slider_vert_bar_disabled\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_SliderHorzBar                   ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_slider_horz_bar\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_SliderHorzBarDisabled           ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_slider_horz_bar_disabled\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_ComboBox                        ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_combobox\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_ComboBoxHighlight               ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_combobox_highlight\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_ComboBoxPressed                 ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_combobox_pressed\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_ComboBoxDisabled                ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_combobox_disabled\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_CycleBoxBody                    ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_cyclebox_body\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_CycleBoxBodyDisabled            ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_cyclebox_body_disabled\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_CycleBoxLeft                    ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_cyclebox_left\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_CycleBoxLeftHighlight           ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_cyclebox_left_highlight\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_CycleBoxLeftPressed             ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_cyclebox_left_pressed\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_CycleBoxLeftDisabled            ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_cyclebox_left_disabled\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_CycleBoxRight                   ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_cyclebox_right\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_CycleBoxRightHighlight          ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_cyclebox_right_highlight\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_CycleBoxRightPressed            ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_cyclebox_right_pressed\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_CycleBoxRightDisabled           ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_cyclebox_right_disabled\");\n"
			"	theme_data->sprite_nine_slice[GuiSpriteNineSlice_Solid                           ] = zt_spriteManagerGetSpriteNineSlice(&theme_data->sprite_manager, \"gui_solid\");\n"
			"\n"
			"	zt_fize(theme_data->sprite_nine_slice) {\n"
			"		if (theme_data->sprite_nine_slice[i] == nullptr) {\n"
			"			zt_logCritical(\"Missing GUI theme sprite (1.%d)\", i);\n"
			"			return false;\n"
			"		}\n"
			"	}\n"
			"\n"
			"	theme_data->sprite           [GuiSprite_CheckBox                                 ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_checkbox\");\n"
			"	theme_data->sprite           [GuiSprite_CheckBoxHighlight                        ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_checkbox_highlight\");\n"
			"	theme_data->sprite           [GuiSprite_CheckBoxPressed                          ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_checkbox_pressed\");\n"
			"	theme_data->sprite           [GuiSprite_CheckBoxDisabled                         ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_checkbox_disabled\");\n"
			"	theme_data->sprite           [GuiSprite_Radio                                    ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_radio\");\n"
			"	theme_data->sprite           [GuiSprite_RadioHighlight                           ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_radio_highlight\");\n"
			"	theme_data->sprite           [GuiSprite_RadioPressed                             ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_radio_pressed\");\n"
			"	theme_data->sprite           [GuiSprite_RadioDisabled                            ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_radio_disabled\");\n"
			"	theme_data->sprite           [GuiSprite_RadioSelect                              ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_radio_select\");\n"
			"	theme_data->sprite           [GuiSprite_SliderHorzHandle                         ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_slider_horz_handle\");\n"
			"	theme_data->sprite           [GuiSprite_SliderHorzHandleHighlight                ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_slider_horz_handle_highlight\");\n"
			"	theme_data->sprite           [GuiSprite_SliderHorzHandlePressed                  ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_slider_horz_handle_pressed\");\n"
			"	theme_data->sprite           [GuiSprite_SliderHorzHandleDisabled                 ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_slider_horz_handle_disabled\");\n"
			"	theme_data->sprite           [GuiSprite_SliderVertHandle                         ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_slider_vert_handle\");\n"
			"	theme_data->sprite           [GuiSprite_SliderVertHandleHighlight                ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_slider_vert_handle_highlight\");\n"
			"	theme_data->sprite           [GuiSprite_SliderVertHandlePressed                  ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_slider_vert_handle_pressed\");\n"
			"	theme_data->sprite           [GuiSprite_SliderVertHandleDisabled                 ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_slider_vert_handle_disabled\");\n"
			"	theme_data->sprite           [GuiSprite_Resizer                                  ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_resizer\");\n"
			"	theme_data->sprite           [GuiSprite_HorzLine                                 ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_horz_line\");\n"
			"	theme_data->sprite           [GuiSprite_IconPlus                                 ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_icon_plus\");\n"
			"	theme_data->sprite           [GuiSprite_IconMinus                                ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_icon_minus\");\n"
			"	theme_data->sprite           [GuiSprite_IconArrowLeft                            ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_icon_arrow_left\");\n"
			"	theme_data->sprite           [GuiSprite_IconArrowRight                           ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_icon_arrow_right\");\n"
			"	theme_data->sprite           [GuiSprite_IconArrowUp                              ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_icon_arrow_up\");\n"
			"	theme_data->sprite           [GuiSprite_IconArrowDown                            ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_icon_arrow_down\");\n"
			"	theme_data->sprite           [GuiSprite_IconHandGrab                             ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_icon_hand_grab\");\n"
			"	theme_data->sprite           [GuiSprite_IconHandPoint                            ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_icon_hand_point\");\n"
			"	theme_data->sprite           [GuiSprite_IconPencil                               ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_icon_pencil\");\n"
			"	theme_data->sprite           [GuiSprite_IconTrash                                ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_icon_trash\");\n"
			"	theme_data->sprite           [GuiSprite_IconUndo                                 ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_icon_undo\");\n"
			"	theme_data->sprite           [GuiSprite_IconCopy                                 ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_icon_copy\");\n"
			"	theme_data->sprite           [GuiSprite_IconCut                                  ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_icon_cut\");\n"
			"	theme_data->sprite           [GuiSprite_IconNew                                  ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_icon_new\");\n"
			"	theme_data->sprite           [GuiSprite_IconSave                                 ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_icon_save\");\n"
			"	theme_data->sprite           [GuiSprite_IconOpen                                 ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_icon_open\");\n"
			"	theme_data->sprite           [GuiSprite_IconWarning                              ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_icon_warning\");\n"
			"	theme_data->sprite           [GuiSprite_IconMenu                                 ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_icon_menu\");\n"
			"	theme_data->sprite           [GuiSprite_IconCopyright                            ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_icon_copyright\");\n"
			"	theme_data->sprite           [GuiSprite_IconCheck                                ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_icon_check\");\n"
			"	theme_data->sprite           [GuiSprite_IconX                                    ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_icon_x\");\n"
			"	theme_data->sprite           [GuiSprite_IconGear                                 ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_icon_gear\");\n"
			"	theme_data->sprite           [GuiSprite_IconCancel                               ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_icon_cancel\");\n"
			"	theme_data->sprite           [GuiSprite_IconPlay                                 ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_icon_play\");\n"
			"	theme_data->sprite           [GuiSprite_IconPause                                ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_icon_pause\");\n"
			"	theme_data->sprite           [GuiSprite_IconRewind                               ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_icon_rewind\");\n"
			"	theme_data->sprite           [GuiSprite_IconFastForward                          ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_icon_fastforward\");\n"
			"	theme_data->sprite           [GuiSprite_IconBegin                                ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_icon_begin\");\n"
			"	theme_data->sprite           [GuiSprite_IconEnd                                  ] = zt_spriteManagerGetSprite(&theme_data->sprite_manager, \"gui_icon_end\");\n"
			"\n"
			"	zt_fize(theme_data->sprite_nine_slice) {\n"
			"		if (theme_data->sprite_nine_slice[i] == nullptr) {\n"
			"			zt_logCritical(\"Missing GUI theme sprite (2.%d)\", i);\n"
			"			return false;\n"
			"		}\n"
			"	}\n"
			"\n"
			"\n"
			"	zt_fize(theme_data->audio) {\n"
			"		theme_data->audio[i] = ztInvalidID;\n"
			"	}\n"
			"\n"
			"	{\n"
			"		// default font\n"
			"		char *data = \"info face=ZeroToleranceGui size=16 bold=0 italic=0 charset=unicode=stretchH=100 smooth=1 aa=1 padding=0,0,0,0 spacing=0,0 outline=0\\ncommon lineHeight=16 base=13 scaleW=512 scaleH=64 pages=1 packed=0\\npage id=0 file=\\\\\\\".\\\\\\\"\\nchars count=94\\nchar id=33 x=11 y=0 width=5 height=16 xoffset=0 yoffset=0 xadvance=5 page=0 chnl=15\\nchar id=34 x=15 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=35 x=23 y=0 width=11 height=16 xoffset=0 yoffset=0 xadvance=11 page=0 chnl=15\\nchar id=36 x=35 y=0 width=8 height=16 xoffset=0 yoffset=0 xadvance=8 page=0 chnl=15\\nchar id=37 x=44 y=0 width=10 height=16 xoffset=0 yoffset=0 xadvance=10 page=0 chnl=15\\nchar id=38 x=55 y=0 width=9 height=16 xoffset=0 yoffset=0 xadvance=9 page=0 chnl=15\\nchar id=39 x=65 y=0 width=4 height=16 xoffset=0 yoffset=0 xadvance=4 page=0 chnl=15\\nchar id=40 x=71 y=0 width=6 height=16 xoffset=0 yoffset=0 xadvance=6 page=0 chnl=15\\nchar id=41 x=78 y=0 width=6 height=16 xoffset=0 yoffset=0 xadvance=6 page=0 chnl=15\\nchar id=42 x=86 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=43 x=94 y=0 width=10 height=16 xoffset=0 yoffset=0 xadvance=10 page=0 chnl=15\\nchar id=44 x=106 y=0 width=4 height=16 xoffset=0 yoffset=0 xadvance=4 page=0 chnl=15\\nchar id=45 x=111 y=0 width=5 height=16 xoffset=0 yoffset=0 xadvance=5 page=0 chnl=15\\nchar id=46 x=118 y=0 width=4 height=16 xoffset=0 yoffset=0 xadvance=4 page=0 chnl=15\\nchar id=47 x=124 y=0 width=8 height=16 xoffset=0 yoffset=0 xadvance=8 page=0 chnl=15\\nchar id=48 x=133 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=49 x=142 y=0 width=5 height=16 xoffset=0 yoffset=0 xadvance=5 page=0 chnl=15\\nchar id=50 x=149 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=51 x=157 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=52 x=166 y=0 width=8 height=16 xoffset=0 yoffset=0 xadvance=8 page=0 chnl=15\\nchar id=53 x=175 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=54 x=183 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=55 x=191 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=56 x=200 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=57 x=208 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=58 x=217 y=0 width=4 height=16 xoffset=0 yoffset=0 xadvance=4 page=0 chnl=15\\nchar id=59 x=223 y=0 width=4 height=16 xoffset=0 yoffset=0 xadvance=4 page=0 chnl=15\\nchar id=60 x=230 y=0 width=10 height=16 xoffset=0 yoffset=0 xadvance=10 page=0 chnl=15\\nchar id=61 x=240 y=0 width=10 height=16 xoffset=0 yoffset=0 xadvance=10 page=0 chnl=15\\nchar id=62 x=252 y=0 width=10 height=16 xoffset=0 yoffset=0 xadvance=10 page=0 chnl=15\\nchar id=63 x=263 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=64 x=271 y=0 width=10 height=16 xoffset=0 yoffset=0 xadvance=10 page=0 chnl=15\\nchar id=65 x=281 y=0 width=8 height=16 xoffset=0 yoffset=0 xadvance=8 page=0 chnl=15\\nchar id=66 x=291 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=67 x=300 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=68 x=309 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=69 x=318 y=0 width=6 height=16 xoffset=0 yoffset=0 xadvance=6 page=0 chnl=15\\nchar id=70 x=326 y=0 width=6 height=16 xoffset=0 yoffset=0 xadvance=6 page=0 chnl=15\\nchar id=71 x=334 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=72 x=343 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=73 x=353 y=0 width=4 height=16 xoffset=0 yoffset=0 xadvance=4 page=0 chnl=15\\nchar id=74 x=358 y=0 width=5 height=16 xoffset=0 yoffset=0 xadvance=5 page=0 chnl=15\\nchar id=75 x=365 y=0 width=8 height=16 xoffset=0 yoffset=0 xadvance=8 page=0 chnl=15\\nchar id=76 x=374 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=77 x=382 y=0 width=9 height=16 xoffset=0 yoffset=0 xadvance=9 page=0 chnl=15\\nchar id=78 x=393 y=0 width=8 height=16 xoffset=0 yoffset=0 xadvance=8 page=0 chnl=15\\nchar id=79 x=403 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=80 x=412 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=81 x=420 y=0 width=8 height=16 xoffset=0 yoffset=0 xadvance=8 page=0 chnl=15\\nchar id=82 x=429 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=83 x=438 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=84 x=447 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=85 x=455 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=86 x=464 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=87 x=473 y=0 width=10 height=16 xoffset=0 yoffset=0 xadvance=10 page=0 chnl=15\\nchar id=88 x=484 y=0 width=8 height=16 xoffset=0 yoffset=0 xadvance=8 page=0 chnl=15\\nchar id=89 x=493 y=0 width=8 height=16 xoffset=0 yoffset=0 xadvance=8 page=0 chnl=15\\nchar id=90 x=501 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=91 x=510 y=0 width=6 height=16 xoffset=0 yoffset=0 xadvance=6 page=0 chnl=15\\nchar id=92 x=517 y=0 width=8 height=16 xoffset=0 yoffset=0 xadvance=8 page=0 chnl=15\\nchar id=93 x=527 y=0 width=6 height=16 xoffset=0 yoffset=0 xadvance=6 page=0 chnl=15\\nchar id=94 x=535 y=0 width=10 height=16 xoffset=0 yoffset=0 xadvance=10 page=0 chnl=15\\nchar id=95 x=547 y=0 width=9 height=16 xoffset=0 yoffset=0 xadvance=9 page=0 chnl=15\\nchar id=96 x=558 y=0 width=4 height=16 xoffset=0 yoffset=0 xadvance=4 page=0 chnl=15\\nchar id=97 x=564 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=98 x=572 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=99 x=581 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=100 x=589 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=101 x=598 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=102 x=606 y=0 width=6 height=16 xoffset=0 yoffset=0 xadvance=6 page=0 chnl=15\\nchar id=103 x=613 y=0 width=8 height=16 xoffset=0 yoffset=0 xadvance=8 page=0 chnl=15\\nchar id=104 x=622 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=105 x=631 y=0 width=4 height=16 xoffset=0 yoffset=0 xadvance=4 page=0 chnl=15\\nchar id=106 x=636 y=0 width=5 height=16 xoffset=0 yoffset=0 xadvance=5 page=0 chnl=15\\nchar id=107 x=642 y=0 width=8 height=16 xoffset=0 yoffset=0 xadvance=8 page=0 chnl=15\\nchar id=108 x=651 y=0 width=4 height=16 xoffset=0 yoffset=0 xadvance=4 page=0 chnl=15\\nchar id=109 x=657 y=0 width=10 height=16 xoffset=0 yoffset=0 xadvance=10 page=0 chnl=15\\nchar id=110 x=668 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=111 x=677 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=112 x=685 y=0 width=6 height=16 xoffset=0 yoffset=0 xadvance=6 page=0 chnl=15\\nchar id=113 x=693 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=114 x=702 y=0 width=6 height=16 xoffset=0 yoffset=0 xadvance=6 page=0 chnl=15\\nchar id=115 x=709 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=116 x=717 y=0 width=6 height=16 xoffset=0 yoffset=0 xadvance=6 page=0 chnl=15\\nchar id=117 x=724 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=118 x=734 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=119 x=741 y=0 width=9 height=16 xoffset=0 yoffset=0 xadvance=9 page=0 chnl=15\\nchar id=120 x=751 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=121 x=759 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=122 x=767 y=0 width=6 height=16 xoffset=0 yoffset=0 xadvance=6 page=0 chnl=15\\nchar id=123 x=775 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=124 x=784 y=0 width=4 height=16 xoffset=0 yoffset=0 xadvance=4 page=0 chnl=15\\nchar id=125 x=789 y=0 width=7 height=16 xoffset=0 yoffset=0 xadvance=7 page=0 chnl=15\\nchar id=126 x=798 y=0 width=11 height=16 xoffset=0 yoffset=0 xadvance=11 page=0 chnl=15\\n\";\n"
			"		theme_data->font = zt_fontMakeFromBmpFontData(data, theme_data->texture, zt_vec2i(0, 20 + 473));\n"
			"	}\n"
			"\n"
			"	{\n"
			"		// monospaced default\n"
			"		char *data = \"info face=ZeroToleranceDefaultMono size=16 bold=0 italic=0 charset=unicode stretchH=100 smooth=1 aa=1 padding=0,0,0,0 spacing=0,0 outline=0\\ncommon lineHeight=16 base=13 scaleW=1024 scaleH=32 pages=1 packed=0\\npage id=0 file=\\\\\\\".\\\\\\\"\\nchars count=94\\nchar id=33 x=8 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=34 x=16 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=35 x=24 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=36 x=32 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=37 x=40 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=38 x=48 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=39 x=56 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=40 x=64 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=41 x=72 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=42 x=80 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=43 x=88 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=44 x=96 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=45 x=104 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=46 x=112 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=47 x=120 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=48 x=128 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=49 x=136 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=50 x=144 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=51 x=152 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=52 x=160 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=53 x=168 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=54 x=176 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=55 x=184 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=56 x=192 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=57 x=200 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=58 x=208 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=59 x=216 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=60 x=224 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=61 x=232 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=62 x=240 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=63 x=248 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=64 x=256 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=65 x=264 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=66 x=272 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=67 x=280 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=68 x=288 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=69 x=296 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=70 x=304 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=71 x=312 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=72 x=320 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=73 x=328 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=74 x=336 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=75 x=344 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=76 x=352 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=77 x=360 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=78 x=368 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=79 x=376 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=80 x=384 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=81 x=392 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=82 x=400 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=83 x=408 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=84 x=416 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=85 x=424 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=86 x=432 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=87 x=440 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=88 x=448 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=89 x=456 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=90 x=464 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=91 x=472 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=92 x=480 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=93 x=488 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=94 x=496 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=95 x=504 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=96 x=512 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=97 x=520 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=98 x=528 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=99 x=536 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=100 x=544 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=101 x=552 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=102 x=560 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=103 x=568 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=104 x=576 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=105 x=584 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=106 x=592 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=107 x=600 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=108 x=608 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=109 x=616 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=110 x=624 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=111 x=632 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=112 x=640 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=113 x=648 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=114 x=656 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=115 x=664 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=116 x=672 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=117 x=680 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=118 x=688 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=119 x=696 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=120 x=704 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=121 x=712 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=122 x=720 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=123 x=728 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=124 x=736 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=125 x=744 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\nchar id=126 x=744 y=0 width=8 height=16 xoffset=0 yoffset=3 xadvance=8 page=0 chnl=15\\n\";\n"
			"		theme_data->font_monospace = zt_fontMakeFromBmpFontData(data, theme_data->texture, zt_vec2i(0, 1 + 473));\n"
			"	}\n"
			"\n"
			"\n"
			"	game->gui_theme_data   = theme_data;\n"
			"\n"
			"	theme_data->game       = game;\n"
			"	theme->theme_data      = theme_data;\n"
			"\n"
			"#if 1\n"
			"	theme->get_rvalue      = ZT_FUNCTION_POINTER_TO_VAR(guiThemeGetRValue);\n"
			"	theme->get_ivalue      = ZT_FUNCTION_POINTER_TO_VAR(guiThemeGetIValue);\n"
			"	theme->update_item     = ZT_FUNCTION_POINTER_TO_VAR(guiThemeUpdateItem);\n"
			"	theme->update_subitem  = ZT_FUNCTION_POINTER_TO_VAR(guiThemeUpdateSubitem);\n"
			"	theme->size_item       = ZT_FUNCTION_POINTER_TO_VAR(guiThemeSizeItem);\n"
			"	theme->render_item     = ZT_FUNCTION_POINTER_TO_VAR(guiThemeRenderItem);\n"
			"	theme->postrender_item = ZT_FUNCTION_POINTER_TO_VAR(guiThemePostRenderItem);\n"
			"	theme->pass_begin      = ZT_FUNCTION_POINTER_TO_VAR(guiThemeRenderPassBegin);\n"
			"	theme->pass_end        = ZT_FUNCTION_POINTER_TO_VAR(guiThemeRenderPassEnd);\n"
			"#else\n"
			"	theme->get_rvalue      = ztInvalidID;\n"
			"	theme->get_ivalue      = ztInvalidID;\n"
			"	theme->update_item     = ztInvalidID;\n"
			"	theme->update_subitem  = ztInvalidID;\n"
			"	theme->size_item       = ztInvalidID;\n"
			"	theme->render_item     = ztInvalidID;\n"
			"#endif\n"
			"	theme->flags |= ztGuiThemeFlags_SeparatePasses;\n"
			"\n"
			"	return true;\n"
			"}\n"
			"\n"
			"// ================================================================================================================================================================================================\n"
			"\n"
			"void guiThemeFree(ztGuiTheme *theme)\n"
			"{\n"
			"	if (theme == nullptr) {\n"
			"		return;\n"
			"	}\n"
			"\n"
			"	GuiThemeData *theme_data = (GuiThemeData*)theme->theme_data;\n"
			"\n"
			"	zt_fontFree(theme_data->font);\n"
			"	zt_fontFree(theme_data->font_monospace);\n"
			"\n"
			"	zt_fize(theme_data->audio) {\n"
			"		if (theme_data->audio[i] != ztInvalidID) {\n"
			"			zt_audioClipFree(theme_data->audio[i]);\n"
			"			theme_data->audio[i] = ztInvalidID;\n"
			"		}\n"
			"	}\n"
			"\n"
			"	zt_fize(theme_data->sprite) {\n"
			"		theme_data->sprite[i] = nullptr;\n"
			"	}\n"
			"	zt_fize(theme_data->sprite_nine_slice) {\n"
			"		theme_data->sprite_nine_slice[i] = nullptr;\n"
			"	}\n"
			"\n"
			"	zt_spriteManagerFree(&theme_data->sprite_manager);\n"
			"	zt_textureFree(theme_data->texture);\n"
			"\n"
			"	zt_fize(theme_data->audio) {\n"
			"		zt_audioClipFree(theme_data->audio[i]);\n"
			"	}\n"
			"\n"
			"	zt_free(theme_data);\n"
			"}\n"
			"\n"
			"// ================================================================================================================================================================================================\n"
			);
	}

	if (need_impls) {
		if (build_cfg->include_gui) {
			zt_fileWritef(&file,
				"#define ZT_GAME_GUI_IMPLEMENTATION\n"
				"#include \"zt_game_gui.h\"\n\n"
				);
		}
		if (build_cfg->include_vr) {
			zt_fileWritef(&file,
				"#define ZT_VR_IMPLEMENTATION\n"
				"#include \"openvr/openvr.h\"\n"
				"#include \"zt_vr.h\"\n\n"
				);
		}
	}

	if (need_endif) {
		zt_fileWritef(&file, "#endif\n");
	}

	zt_fileClose(&file);

	return true;
}

// ------------------------------------------------------------------------------------------------

bool sln_createResources(ztBuildConfig *build_cfg, const char *proj_dir)
{
	{ // icon
		char icon_file[ztFileMaxPath];
		zt_fileConcatFileToPath(icon_file, ztFileMaxPath, proj_dir, "\\src\\resources_icon.ico");
		zt_writeEntireFile(icon_file, bin_zt_icon_ico, bin_zt_icon_ico_size);
	}

	{ // .rc file
		char rc_file[ztFileMaxPath];
		zt_fileConcatFileToPath(rc_file, ztFileMaxPath, proj_dir, "\\src\\resources.rc");

		ztFile file;
		zt_fileOpen(&file, rc_file, ztFileOpenMethod_WriteOver);

		zt_fileWrite(&file,
			"#include \"resources.h\"\n"
			"\n"
			"#define APSTUDIO_READONLY_SYMBOLS\n"
			"#include \"winres.h\"\n"
			"#undef APSTUDIO_READONLY_SYMBOLS\n"
			"\n"
			"#if !defined(AFX_RESOURCE_DLL) || defined(AFX_TARG_ENU)\n"
			"LANGUAGE LANG_ENGLISH, SUBLANG_ENGLISH_US\n"
			"\n"
			"#ifdef APSTUDIO_INVOKED\n"
			"\n"
			"1 TEXTINCLUDE \n"
			"BEGIN\n"
			"    \"resource2.h\\0\"\n"
			"END\n"
			"\n"
			"2 TEXTINCLUDE \n"
			"BEGIN\n"
			"    \"#include \"\"winres.h\"\"\\r\\n\"\n"
			"    \"\\0\"\n"
			"END\n"
			"\n"
			"3 TEXTINCLUDE \n"
			"BEGIN\n"
			"    \"\\r\\n\"\n"
			"    \"\\0\"\n"
			"END\n"
			"\n"
			"#endif  // APSTUDIO_INVOKED\n"
			"\n"
			"IDI_ICON    ICON    \"resources_icon.ico\"\n"
			"\n"
			"#endif  // English (United States) resources\n"
			""
			);

		zt_fileClose(&file);
	}

	{ // resources.h
		char  h_file[ztFileMaxPath];
		zt_fileConcatFileToPath(h_file, ztFileMaxPath, proj_dir, "\\src\\resources.h");

		ztFile file;
		zt_fileOpen(&file, h_file, ztFileOpenMethod_WriteOver);

		zt_fileWrite(&file,
			"//{{NO_DEPENDENCIES}}\n"
			"// Used by resources.rc\n"
			"//\n"
			"#define IDI_ICON    101\n"
			"\n"
			"// Next default values for new objects\n"
			"// \n"
			"#ifdef APSTUDIO_INVOKED\n"
			"#ifndef APSTUDIO_READONLY_SYMBOLS\n"
			"#define _APS_NEXT_RESOURCE_VALUE        102\n"
			"#define _APS_NEXT_COMMAND_VALUE         40001\n"
			"#define _APS_NEXT_CONTROL_VALUE         1001\n"
			"#define _APS_NEXT_SYMED_VALUE           101\n"
			"#endif\n"
			"#endif\n"
			);

		zt_fileClose(&file);
	}

	return true;
}

// ------------------------------------------------------------------------------------------------

bool sln_createSourceFiles(ztBuildConfig *build_cfg, const char *proj_dir)
{
	if (build_cfg->unity_build) {
		if (build_cfg->dll_project) {
			sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_UNITY_FULL);
			sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_UNITY_LOADER);
			sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME_LOADER);
			sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_UNITY_DLL);
			sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME_H);
			sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME_SCENE_H);
			sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME_SCENE_SPLASH);
			sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME_SCENE_MENU);
			sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME_SCENE_MAIN);
			sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME_DLL);

			if (build_cfg->include_gui) {
				sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GUI_THEME_H);
				sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GUI_THEME);
			}

		}
		else {
			sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_UNITY_FULL);
		}
	}
	else {
		if (build_cfg->dll_project) {
			sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME_LOADER);
			sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME_DLL);
		}
		else {
			sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME);
		}

		sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME_H);
		sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME_SCENE_H);
		sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME_SCENE_SPLASH);
		sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME_SCENE_MENU);
		sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME_SCENE_MAIN);

		if (build_cfg->include_gui) {
			sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GUI_THEME_H);
			sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GUI_THEME);
		}
	}

	sln_createResources(build_cfg, proj_dir);


	return true;
}

// ------------------------------------------------------------------------------------------------

bool sln_createSlnFile(ztBuildConfig *build_cfg, const char *proj_dir)
{
	char file_name_only[256] = { 0 };
	zt_strPrintf(file_name_only, zt_elementsOf(file_name_only), "\\build\\msvc\\%s.sln", build_cfg->project_name);

	char file_name[ztFileMaxPath];
	zt_fileConcatFileToPath(file_name, ztFileMaxPath, proj_dir, file_name_only);

	ztFile file;
	zt_fileOpen(&file, file_name, ztFileOpenMethod_WriteOver);

	char sln_gui[128];
	sln_generateGuid(file_name, sln_gui, zt_elementsOf(sln_gui));

	zt_fileWritef(&file,
		"\n"
		"Microsoft Visual Studio Solution File, Format Version 12.00\n"
		"# Visual Studio 2013\n"
		"VisualStudioVersion = 12.0.40629.0\n"
		"MinimumVisualStudioVersion = 10.0.40219.1\n"
		);

	if (build_cfg->dll_project) {

		char guid_loader[128];
		sln_generateGuidProj(build_cfg->dll_loader_name, guid_loader, zt_elementsOf(guid_loader));

		char guid_dll[128];
		sln_generateGuidProj(build_cfg->dll_dll_name, guid_dll, zt_elementsOf(guid_dll));

		char guid_all[128];
		sln_generateGuidProj(build_cfg->project_name, guid_all, zt_elementsOf(guid_all));

		zt_fileWritef(&file,
			"Project(\"{%s}\") = \"%s\", \"%s\\%s.vcxproj\", \"{%s}\"\n"
			"	ProjectSection(ProjectDependencies) = postProject\n"
			"		{%s} = {%s}\n"
			"	EndProjectSection\n"
			"EndProject\n", sln_gui, build_cfg->dll_loader_name, build_cfg->dll_loader_name, build_cfg->dll_loader_name, guid_loader, guid_dll, guid_dll
			);

		zt_fileWritef(&file,
			"Project(\"{%s}\") = \"%s\", \"%s\\%s.vcxproj\", \"{%s}\"\n"
			"EndProject\n", sln_gui, build_cfg->dll_dll_name, build_cfg->dll_dll_name, build_cfg->dll_dll_name, guid_dll
			);

		zt_fileWritef(&file,
			"Project(\"{%s}\") = \"%s\", \"%s\\%s.vcxproj\", \"{%s}\"\n"
			"EndProject\n", sln_gui, build_cfg->project_name, build_cfg->project_name, build_cfg->project_name, guid_all
			);

		zt_fileWritef(&file,
			"Global\n"
			"	GlobalSection(SolutionConfigurationPlatforms) = preSolution\n"
			"		Debug|Win32 = Debug|Win32\n"
			"		Debug|x64 = Debug|x64\n"
			"		Release|Win32 = Release|Win32\n"
			"		Release|x64 = Release|x64\n"
			);

		zt_fileWritef(&file,
			"	EndGlobalSection\n"
			"	GlobalSection(ProjectConfigurationPlatforms) = postSolution\n"
			);

		char * guids[] = {
			guid_loader, guid_dll, guid_all
		};

		zt_fize(guids) {
			zt_fileWritef(&file,
				"		{%s}.Debug|Win32.ActiveCfg = Debug|Win32\n"
				"		{%s}.Debug|Win32.Build.0 = Debug|Win32\n"
				"		{%s}.Debug|x64.ActiveCfg = Debug|x64\n"
				"		{%s}.Debug|x64.Build.0 = Debug|x64\n"
				"		{%s}.Release|Win32.ActiveCfg = Release|Win32\n"
				"		{%s}.Release|Win32.Build.0 = Release|Win32\n"
				"		{%s}.Release|x64.ActiveCfg = Release|x64\n"
				"		{%s}.Release|x64.Build.0 = Release|x64\n", guids[i], guids[i], guids[i], guids[i], guids[i], guids[i], guids[i], guids[i]
				);
		}

		zt_fileWritef(&file, 
			"	EndGlobalSection\n"
			"	GlobalSection(SolutionProperties) = preSolution\n"
			"		HideSolutionNode = FALSE\n"
			"	EndGlobalSection\n"
			"	GlobalSection(Performance) = preSolution\n"
			"		HasPerformanceSessions = true\n"
			"	EndGlobalSection\n"
			"EndGlobal\n"
			);
	}
	else {

	}

	zt_fileClose(&file);

	return true;
}

// ------------------------------------------------------------------------------------------------

bool sln_createBuildBatchFiles(ztBuildConfig *build_cfg, const char *proj_dir)
{
	char *file_names[] = { "build\\msvc\\build_d.bat", "build\\msvc\\build_r.bat", "build\\msvc\\build_d64.bat", "build\\msvc\\build_r64.bat" };
	char *compile_flags[] = { "-MTd", "-MT", "-MTd", "-MT" };
	char *vcvars_flags[] = { "", "", "amd64", "amd64" };

	zt_fiz(4) {
		char file_name[ztFileMaxPath];
		zt_fileConcatFileToPath(file_name, ztFileMaxPath, proj_dir, file_names[i]);

		ztFile file;
		zt_fileOpen(&file, file_name, ztFileOpenMethod_WriteOver);

		zt_fileWritef(&file,
			"@echo off\n"
			"set random_val=%%random%%\n"
			"\n"
			"set compile_flags=%s -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W3 -wd4201 -wd4100 -wd4189 -DZT_DLL -FC -Z7 -LD -I\"%s\"\n"
			"set link_flags=-incremental:no -opt:ref -PDB:..\\..\\bin\\%%1\\%%random_val%%.hotload.%s.pdb -OUT:..\\..\\bin\\%%1\\%%random_val%%.hotload.%s.dll user32.lib gdi32.lib winmm.lib shell32.lib\n"
			"\n"
			"call \"C:\\Program Files (x86)\\Microsoft Visual Studio 12.0\\VC\\vcvarsall.bat\" %s\n"
			"cl %%compile_flags%% ..\\..\\src\\unity_dll.cpp -Fm%s.map /link %%link_flags%%\n",
			compile_flags[i], build_cfg->zt_directory, build_cfg->project_name, build_cfg->project_name, vcvars_flags[i], build_cfg->project_name
			);

		zt_fileClose(&file);
	}

	char file_name[ztFileMaxPath];
	zt_fileConcatFileToPath(file_name, ztFileMaxPath, proj_dir, "build\\msvc\\cleanup.bat");

	ztFile file;
	zt_fileOpen(&file, file_name, ztFileOpenMethod_WriteOver);

	zt_fileWrite(&file,
		"@echo off\n"
		"del /Q ..\\..\\bin\\debug\\x86\\*.hotload.*\n"
		"del /Q ..\\..\\bin\\debug\\x64\\*.hotload.*\n"
		"del /Q ..\\..\\bin\\release\\x86\\*.hotload.*\n"
		"del /Q ..\\..\\bin\\release\\x64\\*.hotload.*\n"
		);

	zt_fileClose(&file);

	return true;
}

// ------------------------------------------------------------------------------------------------

bool sln_createGitIgnore(ztBuildConfig *build_cfg, const char *proj_dir)
{
	if (!build_cfg->include_git_ignore) {
		return true;
	}

	char file_name[ztFileMaxPath];
	zt_fileConcatFileToPath(file_name, ztFileMaxPath, proj_dir, "\\.gitignore");

	ztFile file;
	zt_fileOpen(&file, file_name, ztFileOpenMethod_WriteOver);
	zt_fileWrite(&file,
		"## Ignore Visual Studio temporary files, build results, and files generated by popular Visual Studio add-ons.\n"
		"\n"
		"# User-specific files\n"
		"*.suo\n"
		"*.user\n"
		"*.userosscache\n"
		"*.sln.docstates\n"
		"*.vsp\n"
		"*.psess\n"
		"\n"
		"# Build results\n"
		"**/x86/[Dd]ebug/\n"
		"**/x86/[Rr]elease/\n"
		"**/x64/[Dd]ebug/\n"
		"**/x64/[Rr]elease/\n"
		"local\n"
		"\n"
		"**/bin/[Dd]ebug/*\n"
		"**/bin/[Rr]elease/*\n"
		"**/[Dd]ebug/*\n"
		"**/[Rr]elease/*\n"
		"**/msvc/*.exp\n"
		"**/msvc/*.lib\n"
		"**/msvc/*.obj\n"
		"**/msvc/*.map\n"
		"input_replay*\n"
		"debug.cfg\n"
		"\n"
		"# Visual C++ cache files\n"
		"*.aps\n"
		"*.ncb\n"
		"*.opensdf\n"
		"*.sdf\n"
		"*.cachefile\n"
		"\n"
		"*.tlog\n"
		"*.log\n"
		"*.pdb\n"
		"\n"
		"**/log.txt\n"
		"\n"
		"# =========================\n"
		"# Operating System Files\n"
		"# =========================\n"
		"\n"
		"# Windows image file caches\n"
		"Thumbs.db\n"
		"ehthumbs.db\n"
		"\n"
		"# Folder config file\n"
		"Desktop.ini\n"
		"\n"
		"# Recycle Bin used on file shares\n"
		"$RECYCLE.BIN/\n"
		"\n"
		"# Windows Installer files\n"
		"*.cab\n"
		"*.msi\n"
		"*.msm\n"
		"*.msp\n"
		"\n"
		"# Windows shortcuts\n"
		"*.lnk\n"
		);
	zt_fileClose(&file);
	return true;
}

// ------------------------------------------------------------------------------------------------

bool sln_build(ztBuildConfig *build_cfg, const char *data_dir)
{
	char proj_dir[ztFileMaxPath];
	zt_fileConcatFileToPath(proj_dir, ztFileMaxPath, build_cfg->project_root, build_cfg->project_name);
	zt_logInfo("SLN Build: Project directory: %s", proj_dir);

	if (!zt_directoryExists(proj_dir)) {
		zt_logInfo("SLN Build:   Directory does not exist.  Creating it.");
		zt_directoryMake(proj_dir);
	}

	{
		char *directories[] = {
			"build",
			"build\\msvc\\",
			"src",
		};

		zt_fize(directories) {
			char dir[ztFileMaxPath];
			zt_fileConcatFileToPath(dir, ztFileMaxPath, proj_dir, directories[i]);
			zt_logInfo("SLN Build: Required directory: %s", dir);
			if (!zt_directoryExists(dir)) {
				zt_logInfo("SLN Build:   Directory does not exist.  Creating it.");
				zt_directoryMake(dir);
			}
		}
	}

	if (build_cfg->default_dirs) {
		char *directories[] = {
			"assets",
			"local",
			"run",
			"run\\data",
			"run\\data\\audio",
			"run\\data\\fonts",
			"run\\data\\models",
			"run\\data\\shaders",
			"run\\data\\textures",
		};

		zt_fize(directories) {
			char dir[ztFileMaxPath];
			zt_fileConcatFileToPath(dir, ztFileMaxPath, proj_dir, directories[i]);
			zt_logInfo("SLN Build: Additional directory: %s", dir);
			if (!zt_directoryExists(dir)) {
				zt_logInfo("SLN Build:   Directory does not exist.  Creating it.");
				zt_directoryMake(dir);
			}
		}
	}

	sln_createProjects(build_cfg, proj_dir);
	sln_createSourceFiles(build_cfg, proj_dir);
	sln_createSlnFile(build_cfg, proj_dir);
	sln_createGitIgnore(build_cfg, proj_dir);

	struct local_copy
	{
		static void copy_files(const char *data_dir, char *proj_dir, char **src, int src_cnt, char **dst)
		{
			zt_fiz(src_cnt) {
				char file_name_src[ztFileMaxPath];
				zt_fileConcatFileToPath(file_name_src, ztFileMaxPath, data_dir, src[i]);

				char file_name_dst[ztFileMaxPath];
				zt_fileConcatFileToPath(file_name_dst, ztFileMaxPath, proj_dir, dst[i]);

				if (zt_fileExists(file_name_src)) {
					zt_fileCopy(file_name_src, file_name_dst);
				}
			}
		}
	};


	if (build_cfg->include_gui) {
		char *files_src[] = { "textures\\gui.png", "textures\\gui.spr" };
		char *files_dst[] = { "run\\data\\textures\\gui.png", "run\\data\\textures\\gui.spr" };

		local_copy::copy_files(data_dir, proj_dir, files_src, zt_elementsOf(files_src), files_dst);
	}

	if (build_cfg->include_pbr) {
		char *files_src[] = { "textures\\environment.hdr", "models\\pbr_test_statue.obj", "models\\pbr_test_statue_albedo.png", "models\\pbr_test_statue_height.png", "models\\pbr_test_statue_metallic.png", "models\\pbr_test_statue_normal.png", "models\\pbr_test_statue_roughness.png", "models\\floor_panel_albedo.png", "models\\floor_panel_height.png", "models\\floor_panel_metallic.png", "models\\floor_panel_normal.png", "models\\floor_panel_roughness.png" };
		char *files_dst[] = { "run\\data\\textures\\environment.hdr", "run\\data\\models\\pbr_test_statue.obj", "run\\data\\models\\pbr_test_statue_albedo.png", "run\\data\\models\\pbr_test_statue_height.png", "run\\data\\models\\pbr_test_statue_metallic.png", "run\\data\\models\\pbr_test_statue_normal.png", "run\\data\\models\\pbr_test_statue_roughness.png", "run\\data\\models\\floor_panel_albedo.png", "run\\data\\models\\floor_panel_height.png", "run\\data\\models\\floor_panel_metallic.png", "run\\data\\models\\floor_panel_normal.png", "run\\data\\models\\floor_panel_roughness.png" };

		local_copy::copy_files(data_dir, proj_dir, files_src, zt_elementsOf(files_src), files_dst);
	}

	if (build_cfg->dll_project) {
		sln_createBuildBatchFiles(build_cfg, proj_dir);
	}

	return true;
}

// ------------------------------------------------------------------------------------------------
