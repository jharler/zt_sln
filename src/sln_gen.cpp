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

#define  SRC_FILE_UNITY_FULL      "unity_full.cpp"
#define  SRC_FILE_UNITY_LOADER    "unity_loader.cpp"
#define  SRC_FILE_GAME_LOADER     "main_loader.cpp"
#define  SRC_FILE_UNITY_DLL       "unity_dll.cpp"
#define  SRC_FILE_GAME_DLL        "main_dll.cpp"
#define  SRC_FILE_GAME            "main.cpp"
#define  SRC_FILE_GAME_H          "game.h"
#define  SRC_FILE_GAME_STATE_H    "game_state.h"
#define  SRC_FILE_GAME_STATE_MAIN "game_state_main.cpp"

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

				zt_fileWritef(&file, "    <ClCompile Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_H "\" />\n");
				zt_fileWritef(&file, "    <ClCompile Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_STATE_H "\" />\n");
				zt_fileWritef(&file, "    <None Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_STATE_MAIN "\" />\n");
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
				zt_fileWritef(&file, "    <ClCompile Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_STATE_H "\" />\n");
				zt_fileWritef(&file, "    <None Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_STATE_MAIN "\" />\n");
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
			"    <GenerateDebugInformation>true</GenerateDebugInformation>\n"
			"    <SubSystem>Windows</SubSystem>\n"
			"    <OutputFile>$(OutDir)%s$(TargetExt)</OutputFile>\n"
			"    <ImageHasSafeExceptionHandlers>false</ImageHasSafeExceptionHandlers>\n"
			"  </Link>\n"
			"</ItemDefinitionGroup>\n", proj_configs[i], optimization, runtime_library, proj_outdir_dbl[i], target_file
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
			"	HMODULE game_dll;\n"
			"	char    game_dll_name[ztFileMaxPath];\n"
			"\n"
			"	dll_settings_Func     *dll_settings;\n"
			"	dll_init_Func         *dll_init;\n"
			"	dll_reload_Func       *dll_reload;\n"
			"	dll_unload_Func       *dll_unload;\n"
			"	dll_cleanup_Func      *dll_cleanup;\n"
			"	dll_screenChange_Func *dll_screenChange;\n"
			"	dll_gameLoop_Func     *dll_gameLoop;\n"
			"\n"
			"	ztDirectoryMonitor dir_mon;\n"
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
			"ZT_FUNCTION_POINTER_REGISTER(ldr_buildDllConsoleCommand, ztInternal ZT_FUNC_DEBUG_CONSOLE_COMMAND(ldr_buildDllConsoleCommand))\n"
			"{\n"
			"	ldr_buildDll();\n"
			"}\n"
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
			"	zt_directoryMonitor(&g_dll.dir_mon, details->app_path, false, ztDirectoryMonitorFlags_All);\n"
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
			"	FreeLibrary(g_dll.game_dll);\n"
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
			"// ================================================================================================================================================================================================\n"
			"// All source files must be included here.\n"
			"// Remove them from compilation if they are added to the project\n"
			"// After generation, you may need to edit this file to trigger MSVC's file change detection in the included files\n\n"
			"#include \"" SRC_FILE_GAME_DLL "\"\n"
			"#include \"" SRC_FILE_GAME_STATE_MAIN "\"\n\n\n"
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
		}
		if (build_cfg->include_vr) {
			zt_fileWritef(&file, "#include \"zt_vr.h\"\n");
		}

		zt_fileWritef(&file,
			"#include \"game_state.h\"\n"
			"\n\n"
			"// types/enums/defines ============================================================================================================================================================================\n\n"
			"enum ReplayState_Enum\n"
			"{\n"
			"	ReplayState_None,\n"
			"	ReplayState_Recording,\n"
			"	ReplayState_Replaying,\n"
			"	ReplayState_Stepping,\n"
			"	ReplayState_Paused,\n\n"
			"	ReplayState_MAX,\n"
			"};\n\n"
			"// structs/classes ================================================================================================================================================================================\n\n"
			"// NOTE: do not assign values in the struct definition\n"
			"//       can cause problem with the stack when this struct grows large\n\n"
			"struct ztGame\n"
			"{\n"
			"	ztGameDetails         *details;\n"
			"	ztGameSettings        *settings;\n\n"
			"	ztInputReplayData      replay_data;\n"
			"	bool                   replaying;\n"
			"	ReplayState_Enum       replay_state;\n\n"
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
			"	GameState_Enum         game_state;\n"
			"	GameState_Enum         game_state_transition_to;\n"
			"	GameStateMain          game_state_main;\n\n"
			"	GameStateBegin_Func   *gsf_begin  [GameState_MAX];\n"
			"	GameStateCleanup_Func *gsf_cleanup[GameState_MAX];\n"
			"	GameStateUpdate_Func  *gsf_update [GameState_MAX];\n"
			"	GameStateRender_Func  *gsf_render [GameState_MAX];\n\n"
			"	ztTextureID            render_target;\n\n"
			"	ztMemoryArena         *asset_arena;\n"
			"	ztMemoryArena         *gui_arena;\n"
			"	ztMemoryArena         *temp_arena;\n"
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
	else if (zt_strEquals(src_file, "src\\" SRC_FILE_GAME_STATE_H)) {
		zt_fileWrite(&file,
			"#ifndef __game_state_h__\n"
			"#define __game_state_h__\n"
			"\n"
			"// headers ========================================================================================================================================================================================\n\n"
			"#include \"zt_game.h\"\n\n"
			"// types/enums/defines ============================================================================================================================================================================\n\n"
			"enum GameState_Enum\n"
			"{\n"
			"	GameState_Invalid,\n"
			"	GameState_MainLoading,\n"
			"	GameState_Main,\n\n"
			"	GameState_MAX,\n"
			"};\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"struct ztGame;\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"#define FUNC_GAME_STATE_BEGIN(name)	    bool name(ztGame *game, GameState_Enum transitioning_from)\n"
			"typedef FUNC_GAME_STATE_BEGIN(GameStateBegin_Func);\n"
			"\n"
			"#define FUNC_GAME_STATE_CLEANUP(name)	void name(ztGame *game, GameState_Enum transitioning_to)\n"
			"typedef FUNC_GAME_STATE_CLEANUP(GameStateCleanup_Func);\n"
			"\n"
			"#define FUNC_GAME_STATE_UPDATE(name)	bool name(ztGame *game, r32 dt, bool gui_input, bool input_this_frame, ztInputKeys *input_keys, ztInputController *input_controller, ztInputMouse *input_mouse)\n"
			"typedef FUNC_GAME_STATE_UPDATE(GameStateUpdate_Func);\n"
			"\n"
			"#define FUNC_GAME_STATE_RENDER(name)	void name(ztGame *game, ztTextureID final_render_target)\n"
			"typedef FUNC_GAME_STATE_RENDER(GameStateRender_Func);\n\n"
			"// ================================================================================================================================================================================================\n"
			"// ================================================================================================================================================================================================\n"
			"// ================================================================================================================================================================================================\n\n"
			"enum GameStateMainLoad_Enum\n"
			"{\n"
			"	GameStateMainLoad_Scene,\n\n"
			"	GameStateMainLoad_MAX,\n"
			"};\n\n"
			"struct GameStateMainLoad\n"
			"{\n"
			"	ztLoadInfo                     load_info[GameStateMainLoad_MAX];\n"
			"	int                            load_idx;\n"
			"	r32                            load_time;\n"
			"\n"
			"	// place variables required for loading here...\n"
			"};\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"struct GameStateMain\n"
			"{\n"
			"	ztGame                        *game;\n"
			"	GameStateMainLoad              load;\n"
			);

		if (build_cfg->include_3d_scene) {
			zt_fileWritef(&file,
				"\n	ztScene                       *scene;\n"
				"	ztLight                        scene_light;\n"
				"	ztCameraControllerFPS          camera_controller;\n"
				);
		}

		zt_fileWritef(&file,
			"};\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"FUNC_GAME_STATE_BEGIN   (gameStateMainLoadingBegin);\n"
			"FUNC_GAME_STATE_CLEANUP (gameStateMainLoadingCleanup);\n"
			"FUNC_GAME_STATE_UPDATE  (gameStateMainLoadingUpdate);\n"
			"FUNC_GAME_STATE_RENDER  (gameStateMainLoadingRender);\n"
			"\n"
			"FUNC_GAME_STATE_BEGIN   (gameStateMainBegin);\n"
			"FUNC_GAME_STATE_CLEANUP (gameStateMainCleanup);\n"
			"FUNC_GAME_STATE_UPDATE  (gameStateMainUpdate);\n"
			"FUNC_GAME_STATE_RENDER  (gameStateMainRender);\n"
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
	else if (zt_strEquals(src_file, "src\\" SRC_FILE_GAME_STATE_MAIN)) {
		zt_fileWritef(&file,
			"// headers ========================================================================================================================================================================================\n\n"
			"#include \"game.h\"\n\n"
			"// variables ======================================================================================================================================================================================\n\n"
			"// private functions ==============================================================================================================================================================================\n\n"
			"ztInternal void _gameStateMainDoLoad(ztGame *game, GameStateMain *gs, int idx)\n"
			"{\n"
			"	switch (idx)\n"
			"	{\n"
			"		case GameStateMainLoad_Scene: {\n"
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
			zt_fileWritef(&file,
				"						gs->scene = zt_sceneMake(zt_memGetGlobalArena());\n"
				"						{\n"
				"							ztMeshID   cube = zt_meshMakePrimitiveBox(1, 1, 1);\n"
				"							ztMaterial cube_mat = zt_materialMake(ztInvalidID, ztVec4(1, 1, 1, 1));\n"
				"							ztModel   *cube_model = zt_modelMake(zt_memGetGlobalArena(), cube, &cube_mat, zt_shaderGetDefault(ztShaderDefault_LitShadow), nullptr, ztModelFlags_CastsShadows | ztModelFlags_OwnsMaterials | ztModelFlags_OwnsMesh);\n"
				"							cube_model->transform.position = ztVec3(0, .5f, 0);\n"
				"							zt_sceneAddModel(gs->scene, cube_model);\n"
				"							gs->scene_light = zt_lightMakeDirectional(ztVec3(5, 10, 10), ztVec3::zero, 1, 0.25f);\n"
				"							zt_sceneAddLight(gs->scene, &gs->scene_light);\n"
				"						}\n"
				"						gs->camera_controller = zt_cameraControllerMakeFPS(&game->camera_3d);\n"
				);
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
			"FUNC_GAME_STATE_BEGIN(gameStateMainLoadingBegin)\n"
			"{\n"
			"	ZT_PROFILE_GAME(\"gameStateMainLoadingBegin\");\n\n"
			"	GameStateMain *gs = &game->game_state_main;\n\n"
			"	zt_fize(gs->load.load_info) {\n"
			"		gs->load.load_info[i] = {};\n"
			"		_gameStateMainDoLoad(game, gs, i);\n"
			"	}\n\n"
			"	gs->load.load_time = 0;\n"
			"	gs->load.load_idx  = 0;\n\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"FUNC_GAME_STATE_CLEANUP(gameStateMainLoadingCleanup)\n"
			"{\n"
			"	GameStateMain *gs = &game->game_state_main;\n"
			"	// cleanup here\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"FUNC_GAME_STATE_UPDATE(gameStateMainLoadingUpdate)\n"
			"{\n"
			"	ZT_PROFILE_GAME(\"gameStateMainLoadingUpdate\");\n\n"
			"	GameStateMain *gs = &game->game_state_main;\n\n"
			"	gs->load.load_time += dt;\n\n"
			"	if (gs->load.load_idx >= zt_elementsOf(gs->load.load_info)) {\n"
			"		if (gs->load.load_time > .025f) {\n"
			"			game->game_state_transition_to = GameState_Main;\n"
			"		}\n"
			"		return true;\n"
			"	}\n\n"
			"	_gameStateMainDoLoad(game, gs, gs->load.load_idx);\n\n"
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
			"FUNC_GAME_STATE_RENDER(gameStateMainLoadingRender)\n"
			"{\n"
			"	ZT_PROFILE_GAME(\"gameStateMainLoadingRender\");\n\n"
			"	GameStateMain *gs = &game->game_state_main;\n\n"
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
			"			ztVec2 load_ttl = ztVec2(6, .25f);\n"
			"			ztVec2 load_pct = ztVec2(6 * pct, .25f);\n\n"
			"			r32 diff = load_ttl.x - load_pct.x;\n"
			"			load_ttl.x += pix * 8;\n"
			"			load_ttl.y += pix * 8;\n\n"
			"			zt_drawListPushTexture(&game->draw_list, 0);\n"
			"			{\n"
			"				zt_drawListAddSolidRect2D(&game->draw_list, ztVec3(0, -.5f, 0), load_ttl, ztColor_Black);\n"
			"				zt_drawListAddSolidRect2D(&game->draw_list, ztVec3(-diff / 2, -.5f, 0), load_pct, ztColor_White);\n"
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
			"FUNC_GAME_STATE_BEGIN(gameStateMainBegin)\n"
			"{\n"
			"	GameStateMain *gs = &game->game_state_main;\n\n"
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
			"FUNC_GAME_STATE_CLEANUP(gameStateMainCleanup)\n"
			"{\n"
			"	GameStateMain *gs = &game->game_state_main;\n\n"
			);

		if (build_cfg->include_3d_scene) {
			zt_fileWritef(&file,
				"	zt_sceneFreeAllModels(gs->scene);\n"
				"	zt_sceneFree(gs->scene);\n\n"
				"	gs->scene = nullptr;\n"
				);
		}

		zt_fileWritef(&file,
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"FUNC_GAME_STATE_UPDATE(gameStateMainUpdate)\n"
			"{\n"
			"	ZT_PROFILE_GAME(\"gameStateMainUpdate\");\n\n"
			"	GameStateMain *gs = &game->game_state_main;\n\n"
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
			"FUNC_GAME_STATE_RENDER(gameStateMainRender)\n"
			"{\n"
			"	ZT_PROFILE_GAME(\"gameStateMainRender\");\n\n"
			"	GameStateMain *gs = &game->game_state_main;\n\n"
			);

			if (build_cfg->include_3d_scene) {
				zt_fileWritef(&file,
					"	zt_scenePrepare(gs->scene, &game->camera_3d);\n"
					"	zt_sceneOptimize(gs->scene, &game->camera_3d);\n"
					"	zt_sceneLighting(gs->scene, &game->camera_3d);\n\n"
					"	zt_textureRenderTargetPrepare(final_render_target);\n"
					"	{\n"
					"		zt_rendererClear(ztVec4(.4f, .4f, .4f, 1));\n"
					"		zt_rendererSetDepthTest(true, ztRendererDepthTestFunction_LessEqual);\n\n"
					"		zt_sceneRender(gs->scene, &game->camera_3d);\n\n"
					"		{\n"
					"			zt_drawListPushShader(&game->draw_list, zt_shaderGetDefault(ztShaderDefault_Unlit));\n"
					"			zt_drawListPushTexture(&game->draw_list, ztTextureDefault);\n\n"
					"			zt_drawListAddAxis(&game->draw_list, .0375f, game->camera_3d.position + game->camera_3d.direction);\n"
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
					"		zt_rendererClear(ztVec4(.4f, .4f, .4f, 1));\n"
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
	else if (zt_strEquals(src_file, "src\\" SRC_FILE_GAME_DLL)) {
		bool draw_list = build_cfg->camera_2d || build_cfg->include_gui;

		zt_fileWritef(&file,
			"// headers ========================================================================================================================================================================================\n\n"
			"#include \"game.h\"\n\n"
			"// variables ======================================================================================================================================================================================\n\n"
			"// private functions ==============================================================================================================================================================================\n\n"
			"ztInternal void _gameSetFunctionPointers(ztGame *game)\n"
			"{\n"
			"	zt_fiz(GameState_MAX) {\n"
			"		game->gsf_begin  [i] = nullptr;\n"
			"		game->gsf_cleanup[i] = nullptr;\n"
			"		game->gsf_update [i] = nullptr;\n"
			"		game->gsf_render [i] = nullptr;\n"
			"	}\n\n"
			"	game->gsf_begin  [GameState_MainLoading] = gameStateMainLoadingBegin;\n"
			"	game->gsf_cleanup[GameState_MainLoading] = gameStateMainLoadingCleanup;\n"
			"	game->gsf_update [GameState_MainLoading] = gameStateMainLoadingUpdate;\n"
			"	game->gsf_render [GameState_MainLoading] = gameStateMainLoadingRender;\n\n"
			"	game->gsf_begin  [GameState_Main       ] = gameStateMainBegin;\n"
			"	game->gsf_cleanup[GameState_Main       ] = gameStateMainCleanup;\n"
			"	game->gsf_update [GameState_Main       ] = gameStateMainUpdate;\n"
			"	game->gsf_render [GameState_Main       ] = gameStateMainRender;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ztInternal void _gameStateCleanup(ztGame *game)\n"
			"{\n"
			"	if (game->gsf_cleanup[game->game_state]) {\n"
			"		game->gsf_cleanup[game->game_state](game, game->game_state_transition_to);\n"
			"	}\n"
			"}\n\n"
			"// functions ======================================================================================================================================================================================\n\n"
			"ZT_DLLEXPORT bool dll_settings(ztGameDetails* details, ztGameSettings* settings)\n"
			"{\n"
			"	settings->memory   = zt_megabytes(128);\n"
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
			"ZT_DLLEXPORT bool dll_init(ztGameDetails* details, ztGameSettings* settings, void** game_memory)\n"
			"{\n"
			"	ztGame *game = zt_mallocStruct(ztGame);\n"
			"	*game = {};\n"
			"	*game_memory = game;\n\n"
			"	game->details     = details;\n"
			"	game->settings    = settings;\n\n"
			"	game->asset_arena = zt_memMakeArena(zt_megabytes(16), zt_memGetGlobalArena());\n"
			"	game->gui_arena   = zt_memMakeArena(zt_megabytes(16), zt_memGetGlobalArena());\n"
			"	game->temp_arena  = zt_memMakeArena(zt_megabytes(4), zt_memGetGlobalArena());\n"
			);

		if (build_cfg->default_dirs) {
			zt_fileWritef(&file,
				"\n"
				"	{\n"
				"		// app path and assets\n\n"
				"		zt_strMakePrintf(data_path, ztFileMaxPath, \"%%s%%cdata\", details->user_path, ztFilePathSeparator, ztFilePathSeparator);\n"
				"		if (!zt_directoryExists(data_path)) {\n"
				"			zt_strPrintf(data_path, ztFileMaxPath, \"%%s%%crun%%cdata\", details->user_path, ztFilePathSeparator, ztFilePathSeparator);\n"
				"		}\n\n"
				"		if (!zt_assetManagerLoadDirectory(&game->asset_manager, data_path)) {\n"
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
				"		game->gui_manager = zt_guiManagerMake(&game->camera_2d, nullptr, zt_memGetGlobalArena());\n"
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
				"		game->render_target = zt_textureMakeRenderTarget(settings->native_w, settings->native_h, ztTextureFlags_RenderTargetScreen);\n"
				"	}\n"
				);
		}

		zt_fileWritef(&file,
			"\n"
			"	_gameSetFunctionPointers(game);\n\n"
			"	zt_guiDebugMemoryInspectorAddArena(game->asset_arena, \"Asset Arena\");\n"
			"	zt_guiDebugMemoryInspectorAddArena(game->gui_arena,   \"Gui Arena\");\n"
			"	zt_guiDebugMemoryInspectorAddArena(game->temp_arena,  \"Temp Arena\");\n\n"
			"	game->game_state               = GameState_Invalid;\n"
			"	game->game_state_transition_to = GameState_MainLoading;\n\n"
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
			"	}\n\n"
			);

		/*
		if (build_cfg->include_3d_scene) {
			zt_fileWritef(&file,
				"	game->scene = zt_sceneMake(zt_memGetGlobalArena());\n"
				"	{\n"
				"		ztMeshID   cube       = zt_meshMakePrimitiveBox(1, 1, 1);\n"
				"		ztMaterial cube_mat   = zt_materialMake(ztInvalidID, ztVec4(1,1,1,1));\n"
				"		ztModel   *cube_model = zt_modelMake(zt_memGetGlobalArena(), cube, &cube_mat, zt_shaderGetDefault(ztShaderDefault_LitShadow), nullptr, ztModelFlags_CastsShadows | ztModelFlags_OwnsMaterials | ztModelFlags_OwnsMesh);\n"
				"		cube_model->transform.position = ztVec3(0, 0, 0);\n"
				"		zt_sceneAddModel(game->scene, cube_model);\n"
				"\n"
				"		game->scene_light = zt_lightMakeDirectional(ztVec3(5, 10, 0), ztVec3::zero, 1, 0.05f);\n"
				"		zt_sceneAddLight(game->scene, &game->scene_light);\n"
				"		game->camera_3d.rotation = ztQuat::makeFromEuler(35, 0, 0);\n"
				"		game->camera_3d.rotation *= ztQuat::makeFromEuler(0, -40, 0);\n"
				"		game->camera_3d.position = ztVec3(3, 3, 3);\n"
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
			"	_gameStateCleanup(game);\n"
			"	zt_inputReplayFree(&game->replay_data);\n"
			"	zt_textureFree(game->render_target);\n\n"
			);

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
			zt_fileWritef(&file, "	zt_guiManagerFree(game->gui_manager);\n");
		}


		zt_fileWritef(&file,
			"\n"
			"	zt_memFreeArena(game->temp_arena);\n"
			"	zt_memFreeArena(game->gui_arena);\n"
			"	zt_memFreeArena(game->asset_arena);\n"
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
				"	r32 view_distance = 500;\n"
				"	zt_cameraMakePersp(&game->camera_3d, game->settings->screen_w, game->settings->screen_h, zt_degreesToRadians(60), 0.1f, view_distance, game->camera_3d.position, game->camera_3d.rotation);\n"
				"	zt_cameraRecalcMatrices(&game->camera_3d);\n"
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
				"	}\n\n"
				);
		}

		zt_fileWritef(&file,
			"	if (game->game_state_transition_to != GameState_Invalid) {\n"
			"		ZT_PROFILE_GAME(\"gameLoop:gamestate transition\");\n"
			"		_gameStateCleanup(game);\n\n"
			"		GameState_Enum transitioning_from = game->game_state;\n"
			"		game->game_state = game->game_state_transition_to;\n"
			"		game->game_state_transition_to = GameState_Invalid;\n\n"
			"		if (game->gsf_begin[game->game_state]) {\n"
			"			if (!game->gsf_begin[game->game_state](game, transitioning_from)) {\n"
			"				return false;\n"
			"			}\n"
			"		}\n"
			"	}\n\n"
			"	{\n"
			"		if (game->replay_state != ReplayState_Paused) {\n"
			"			ZT_PROFILE_GAME(\"gameLoop:update\");\n"
			"			if (!game->gsf_update[game->game_state](game, dt, gui_input, input_this_frame, input_keys, &input_controller, &input_mouse)) {\n"
			"				return false;\n"
			"			}\n"
			"		}\n"
			"	}\n\n"
			"	{\n"
			"		ZT_PROFILE_GAME(\"gameLoop:render state\");\n"
			"		zt_rendererClear(ztVec4(0, 0, 0, 0));\n"
			"		game->gsf_render[game->game_state](game, game->render_target);\n"

			"		zt_drawListPushShader(&game->draw_list, zt_shaderGetDefault(ztShaderDefault_Unlit));\n"
			"		zt_drawListAddScreenRenderTexture(&game->draw_list, game->render_target, &game->camera_2d);\n"
			"		zt_renderDrawList(&game->camera_2d, &game->draw_list, ztVec4::zero, ztRenderDrawListFlags_NoDepthTest);\n"
			"		zt_drawListPopShader(&game->draw_list);\n\n"
			"	}\n\n"
			);

		if (build_cfg->include_gui) {
			zt_fileWritef(&file,
				"	{\n"
				"		ZT_PROFILE_GAME(\"gameLoop:render gui\");\n\n"
				"		zt_drawListPushShader(&game->draw_list, zt_shaderGetDefault(ztShaderDefault_Unlit));\n"
				"		zt_guiManagerRender(game->gui_manager, &game->draw_list, dt);\n"
				"		zt_drawListPopShader(&game->draw_list);\n\n"
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
				"			zt_drawListAddText2D(&game->draw_list, ztFontDefault, text, ztVec2(0, ext.y / -2 + .2f), ztAlign_Center | ztAlign_Bottom, ztAnchor_Center | ztAnchor_Bottom);\n"
				"			zt_drawListPopShader(&game->draw_list);\n"
				"		}\n\n"
				"		zt_renderDrawList(&game->camera_2d, &game->draw_list, ztVec4::zero, ztRenderDrawListFlags_NoDepthTest | ztRenderDrawListFlags_NoClear);\n"
				"	}\n"
				);
		}

		zt_fileWritef(&file,
			"	{\n"
			"		ZT_PROFILE_GAME(\"gameLoop:asset check\");\n"
			"		zt_assetManagerCheckForChanges(&game->asset_manager);\n"
			"	}\n\n"
			"	{\n"
			"		if (game->replay_state == ReplayState_Stepping) {\n"
			"			game->replay_state = ReplayState_Paused;\n"
			"		}\n"
			"		else if (game->replay_state == ReplayState_Paused) {\n"
			"			game->details->current_frame -= 1;\n"
			"		}\n"
			"	}\n"
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
					"				zt_rendererClear(ztVec4(0, 0, 0, 1));\n"
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
			sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME_STATE_H);
			sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME_STATE_MAIN);
			sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME_DLL);
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
		sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME_STATE_H);
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

bool sln_build(ztBuildConfig *build_cfg)
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

	if (build_cfg->dll_project) {
		sln_createBuildBatchFiles(build_cfg, proj_dir);
	}

	return true;
}

// ------------------------------------------------------------------------------------------------
