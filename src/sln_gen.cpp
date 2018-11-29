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
#define  SRC_FILE_GAME_SCENE_MAIN_H   "game_scene_main.h"
#define  SRC_FILE_GAME_SCENE_MAIN     "game_scene_main.cpp"
#define  SRC_FILE_GAME_SCENE_EDITOR_H "game_scene_editor.h"
#define  SRC_FILE_GAME_SCENE_EDITOR   "game_scene_editor.cpp"
#define  SRC_FILE_GAME_SCENE_SPLASH_H "game_scene_splash.h"
#define  SRC_FILE_GAME_SCENE_SPLASH   "game_scene_splash.cpp"
#define  SRC_FILE_GAME_SCENE_MENU_H   "game_scene_menu.h"
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
				zt_fileWritef(&file, "    <None Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_SCENE_SPLASH_H "\" />\n");
				zt_fileWritef(&file, "    <None Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_SCENE_SPLASH"\" />\n");
				zt_fileWritef(&file, "    <None Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_SCENE_MENU_H "\" />\n");
				zt_fileWritef(&file, "    <None Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_SCENE_MENU"\" />\n");
				zt_fileWritef(&file, "    <None Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_SCENE_MAIN_H "\" />\n");
				zt_fileWritef(&file, "    <None Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_SCENE_MAIN "\" />\n");

				if (build_cfg->include_entity) {
					zt_fileWritef(&file, "    <None Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_SCENE_EDITOR "\" />\n");
				}

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
				zt_fileWritef(&file, "    <ClCompile Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_SCENE_SPLASH_H "\" />\n");
				zt_fileWritef(&file, "    <ClCompile Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_SCENE_SPLASH "\" />\n");
				zt_fileWritef(&file, "    <ClCompile Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_SCENE_MENU_H "\" />\n");
				zt_fileWritef(&file, "    <ClCompile Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_SCENE_MENU "\" />\n");
				zt_fileWritef(&file, "    <ClCompile Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_SCENE_MAIN_H "\" />\n");
				zt_fileWritef(&file, "    <ClCompile Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_SCENE_MAIN "\" />\n");

				if (build_cfg->include_entity) {
					zt_fileWritef(&file, "    <ClCompile Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_SCENE_EDITOR_H "\" />\n");
					zt_fileWritef(&file, "    <ClCompile Include=\"..\\..\\..\\src\\" SRC_FILE_GAME_SCENE_EDITOR "\" />\n");
				}

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
				"#define ZT_HOTSWAP_COMBINED_BUILD\n\n"
				"#define ZT_NO_PROFILE\n\n"
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
			"#define LDR_DLL_NAME \"%s.dll\"\n\n"
			"#define ZT_HOTSWAP_IMPLEMENTATION\n"
			"#define ZT_HOTSWAP_LOADER\n"
			"#include \"zt_hotswap.h\"\n\n"
			"// ================================================================================================================================================================================================\n",
			build_cfg->project_name
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

		if (build_cfg->include_entity) {
			zt_fileWritef(&file,
				"#include \"" SRC_FILE_GAME_SCENE_EDITOR "\"\n"
				);
		}

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
			"#include \"zt_game_gui.h\"\n"
			"#include \"zt_game_gui_default_theme.h\"\n");

		if (build_cfg->include_vr) {
			zt_fileWritef(&file, "#include \"zt_vr.h\"\n");
		}

		ztRandom random;
		zt_randomInit(&random);

		zt_fileWritef(&file,
			"\n"
			"// types/enums/defines ============================================================================================================================================================================\n\n"
			"\n"
			"// structs/classes ================================================================================================================================================================================\n\n"
			"// NOTE: do not assign values in the struct definition\n"
			"//       can cause problem with the stack when this struct grows large\n\n"
			"struct ztGame\n"
			"{\n"
			"	ztGameDetails         *details;\n"
			"	ztGameSettings        *settings;\n"
			"	ztGameSceneManager     scene_manager;\n\n"
			"	ztGuiManager          *gui_manager;\n"
			"	ztGuiTheme             gui_theme;\n"
			"	ztAssetManager         asset_manager;\n"
			"	ztInputRegistry        input_registry;\n"
			"	ztDrawList             draw_list;\n",
			zt_randomInt(&random, 0, ztInt32Max), zt_randomInt(&random, 0, ztInt32Max), zt_randomInt(&random, 0, ztInt32Max), zt_randomInt(&random, 0, ztInt32Max),
			zt_randomInt(&random, 0, ztInt32Max), zt_randomInt(&random, 0, ztInt32Max), zt_randomInt(&random, 0, ztInt32Max), zt_randomInt(&random, 0, ztInt32Max)
			);

		if (build_cfg->include_vr) {
			zt_fileWritef(&file, "	ztVrSystem            *vr;\n");
		}


		if (build_cfg->include_pbr) {
			zt_fileWritef(&file,
				"\n"
				"	ztPostProcessingStack  post_processing;\n"
				"	ztTextureID            texture_random;\n\n"
				);
		}

		zt_fileWritef(&file,
			"};\n\n\n"
			"// external variables =============================================================================================================================================================================\n\n"
			"// function prototypes ============================================================================================================================================================================\n\n"
			"// inline functions ===============================================================================================================================================================================\n\n"
			"#endif // include guard\n"
			);
	}
	else if (zt_strEquals(src_file, "src\\" SRC_FILE_GAME_SCENE_H)) {
	}
	else if (zt_strEquals(src_file, "src\\" SRC_FILE_GAME_SCENE_MAIN_H)) {
		ztRandom random;
		zt_randomInit(&random);

		zt_fileWritef(&file,
			"#ifndef __game_scene_main_h__\n"
			"#define __game_scene_main_h__\n"
			"// headers ========================================================================================================================================================================================\n\n"
			"#include \"game.h\"\n\n"
			"// types/enums/defines ============================================================================================================================================================================\n\n"
			"#define GAME_SCENE_MAIN_GUID     zt_guid(0x%08x, 0x%08x, 0x%08x, 0x%08x)\n\n"
			"// structs/classes ================================================================================================================================================================================\n\n"
			"struct GameSceneMain\n"
			"{\n"
			"	ztGame *game;\n"
			"};\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"// external variables =============================================================================================================================================================================\n\n"
			"// function prototypes ============================================================================================================================================================================\n\n"
			"void gs_mainInit(ztGame *game);\n\n"
			"// inline functions ===============================================================================================================================================================================\n\n"
			"#endif // include guard\n",
			zt_randomInt(&random, 0, ztInt32Max), zt_randomInt(&random, 0, ztInt32Max), zt_randomInt(&random, 0, ztInt32Max), zt_randomInt(&random, 0, ztInt32Max)
			);
	}
	else if (zt_strEquals(src_file, "src\\" SRC_FILE_GAME_SCENE_MAIN)) {
		zt_fileWritef(&file,
			"// headers ========================================================================================================================================================================================\n\n"
			"#include \"game_scene_main.h\"\n\n"
			"// variables ======================================================================================================================================================================================\n\n"
			"// private functions ==============================================================================================================================================================================\n\n"
			);

		if (false && build_cfg->include_entity) {
			ztRandom random;
			zt_randomInit(&random);

			zt_fileWritef(&file,
				"#define GSM_ENTITY_SERIAL_GUID           zt_guidMake(0x%08x, 0x%08x, 0x%08x, 0x%08x)\n"
				"#define GSM_ENTITY_SERIAL_VERSION        (i32)10001\n"
				"// ================================================================================================================================================================================================\n\n"
				"ZT_FUNCTION_POINTER_REGISTER(gameSceneMainEntitySerialExtraSave, ZT_FUNC_ENTITY_SERIALIZE_EXTRA_SAVE(gameSceneMainEntitySerialExtraSave))\n"
				"{\n"
				"	GameSceneMain *gs = (GameSceneMain*)user_data;\n\n"
				"	if (!zt_serialGroupPush(serial)) return false;\n"
				"	{\n"
				"		if (!zt_serialWriteGuidVersion(serial, GSM_ENTITY_SERIAL_GUID, GSM_ENTITY_SERIAL_VERSION)) return false;\n\n"
				"		// save cameras\n"
				"		if (!zt_serialGroupPush(serial)) return false;\n"
				"		{\n"
				"			if (!zt_cameraControllerFPSSave(&gs->camera_controller_fps, serial)) return false;\n"
				"			if (!zt_cameraControllerArcballSave(&gs->game->game_scene_editor.camera_controller_arc, serial)) return false;\n"
				"		}\n"
				"		if (!zt_serialGroupPop(serial)) return false;\n"
				"	}\n"
				"	if (!zt_serialGroupPop(serial)) return false;\n\n"
				"	return true;\n"
				"}\n\n"
				"// ================================================================================================================================================================================================\n\n"
				"ZT_FUNCTION_POINTER_REGISTER(gameSceneMainEntitySerialExtraLoad, ZT_FUNC_ENTITY_SERIALIZE_EXTRA_LOAD(gameSceneMainEntitySerialExtraLoad))\n"
				"{\n"
				"	GameSceneMain *gs = (GameSceneMain*)user_data;\n\n"
				"	if (!zt_serialGroupPush(serial)) return false;\n"
				"	{\n"
				"		if (!gs->running_from_editor) {\n"
				"			if (!zt_serialReadAndCheckGuidVersion(serial, GSM_ENTITY_SERIAL_GUID, GSM_ENTITY_SERIAL_VERSION)) return false;\n\n"
				"			// save cameras\n"
				"			if (!zt_serialGroupPush(serial)) return false;\n"
				"			{\n"
				"				if (!zt_cameraControllerFPSLoad(&gs->camera_controller_fps, serial)) return false;\n"
				"				if (!zt_cameraControllerArcballLoad(&gs->game->game_scene_editor.camera_controller_arc, serial)) return false;\n"
				"			}\n"
				"			if (!zt_serialGroupPop(serial)) return false;\n"
				"		}\n"
				"	}\n"
				"	if (!zt_serialGroupPop(serial)) return false;\n\n"
				"	return true;\n"
				"}\n\n", zt_randomInt(&random, 0, ztInt32Max), zt_randomInt(&random, 0, ztInt32Max), zt_randomInt(&random, 0, ztInt32Max), zt_randomInt(&random, 0, ztInt32Max)
			);
		}

		zt_fileWritef(&file,
			"// ================================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_LOAD_GET_DETAILS(gs_mainLoadGetDetails)\n"
			"{\n"
			"	*load_steps_needed = 0;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_LOAD_ITERATE(gs_mainLoadIterate)\n"
			"{\n"
			"	GameSceneMain *gs = (GameSceneMain*)game_scene->user_data;\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_UNLOAD(gs_mainUnload)\n"
			"{\n"
			"	GameSceneMain *gs = (GameSceneMain*)game_scene->user_data;\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_BEGIN(gs_mainBegin)\n"
			"{\n"
			"	GameSceneMain *gs = (GameSceneMain*)game_scene->user_data;\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_END(gs_mainEnd)\n"
			"{\n"
			"	GameSceneMain *gs = (GameSceneMain*)game_scene->user_data;\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_SCREEN_CHANGE(gs_mainScreenChange)\n"
			"{\n"
			"	GameSceneMain *gs = (GameSceneMain*)game_scene->user_data;\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_UPDATE_FRAME(gs_mainUpdateFrame)\n"
			"{\n"
			"	GameSceneMain *gs = (GameSceneMain*)game_scene->user_data;\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_UPDATE_TICK(gs_mainUpdateTick)\n"
			"{\n"
			"	GameSceneMain *gs = (GameSceneMain*)game_scene->user_data;\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_RENDER(gs_mainRender)\n"
			"{\n"
			"	GameSceneMain *gs = (GameSceneMain*)game_scene->user_data;\n"
			"	zt_drawListPushShader(draw_list, zt_shaderGetDefault(ztShaderDefault_Unlit));\n"
			"	zt_drawListAddSolidRect2D(draw_list, ztVec3::zero, zt_vec2(100, 100), ztColor_Blue);\n"
			"	zt_drawListPopShader(draw_list);\n"
			"	zt_renderDrawList(&game_scene->game_scene_manager->screen_camera, draw_list, ztVec4::zero, ztRenderDrawListFlags_NoClear | ztRenderDrawListFlags_NoDepthTest, final_render_target);\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_MAKE(gs_mainMake)\n"
			"{\n"
			"	ztGame *game = (ztGame*)user_data;\n\n"
			"	GameSceneMain *gs = zt_mallocStruct(GameSceneMain);\n"
			"	game_scene->user_data = gs;\n\n"
			"	gs->game = game;\n\n"
			"	game_scene->ticks_per_second = 20;\n\n"
			"	game_scene->callback_load_get_details = ZT_FUNCTION_POINTER_TO_VAR(gs_mainLoadGetDetails);\n"
			"	game_scene->callback_load_iterate     = ZT_FUNCTION_POINTER_TO_VAR(gs_mainLoadIterate);\n"
			"	game_scene->callback_unload           = ZT_FUNCTION_POINTER_TO_VAR(gs_mainUnload);\n"
			"	game_scene->callback_begin            = ZT_FUNCTION_POINTER_TO_VAR(gs_mainBegin);\n"
			"	game_scene->callback_end              = ZT_FUNCTION_POINTER_TO_VAR(gs_mainEnd);\n"
			"	game_scene->callback_screen_change    = ZT_FUNCTION_POINTER_TO_VAR(gs_mainScreenChange);\n"
			"	game_scene->callback_update_frame     = ZT_FUNCTION_POINTER_TO_VAR(gs_mainUpdateFrame);\n"
			"	game_scene->callback_update_tick      = ZT_FUNCTION_POINTER_TO_VAR(gs_mainUpdateTick);\n"
			"	game_scene->callback_render           = ZT_FUNCTION_POINTER_TO_VAR(gs_mainRender);\n\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_FREE(gs_mainFree)\n"
			"{\n"
			"	GameSceneMain *gs = (GameSceneMain*)game_scene->user_data;\n"
			"	zt_free(gs);\n"
			"	game_scene->user_data = nullptr;\n"
			"}\n\n"
			"// public functions ===============================================================================================================================================================================\n\n"
			"void gs_mainInit(ztGame *game)\n"
			"{\n"
			"	zt_gameSceneManagerAddScene(&game->scene_manager, GAME_SCENE_MAIN_GUID, ztGameSceneFlags_HdrScene | ztGameSceneFlags_UnloadOnEnd, ZT_FUNCTION_POINTER_TO_VAR(gs_mainMake), game, ZT_FUNCTION_POINTER_TO_VAR(gs_mainFree), game);\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			);
	}
	else if (zt_strEquals(src_file, "src\\" SRC_FILE_GAME_SCENE_EDITOR_H)) {
	}
	else if (zt_strEquals(src_file, "src\\" SRC_FILE_GAME_SCENE_EDITOR)) {
	}
	else if (zt_strEquals(src_file, "src\\" SRC_FILE_GAME_SCENE_SPLASH_H)) {
		ztRandom random;
		zt_randomInit(&random);

		zt_fileWritef(&file,
			"#ifndef __game_scene_splash_h__\n"
			"#define __game_scene_splash_h__\n"
			"// headers ========================================================================================================================================================================================\n\n"
			"#include \"game.h\"\n\n"
			"// types/enums/defines ============================================================================================================================================================================\n\n"
			"#define GAME_SCENE_SPLASH_GUID     zt_guid(0x%08x, 0x%08x, 0x%08x, 0x%08x)\n\n"
			"// structs/classes ================================================================================================================================================================================\n\n"
			"struct GameSceneSplash\n"
			"{\n"
			"	ztGame *game;\n"
			"};\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"// external variables =============================================================================================================================================================================\n\n"
			"// function prototypes ============================================================================================================================================================================\n\n"
			"void gs_splashInit(ztGame *game);\n\n"
			"// inline functions ===============================================================================================================================================================================\n\n"
			"#endif // include guard\n",
			zt_randomInt(&random, 0, ztInt32Max), zt_randomInt(&random, 0, ztInt32Max), zt_randomInt(&random, 0, ztInt32Max), zt_randomInt(&random, 0, ztInt32Max)
			);
	}
	else if (zt_strEquals(src_file, "src\\" SRC_FILE_GAME_SCENE_SPLASH)) {
		zt_fileWritef(&file,
			"// headers ========================================================================================================================================================================================\n\n"
			"#include \"game_scene_splash.h\"\n\n"
			"// variables ======================================================================================================================================================================================\n\n"
			"// private functions ==============================================================================================================================================================================\n\n"
			"// public function ================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_LOAD_GET_DETAILS(gs_splashLoadGetDetails)\n"
			"{\n"
			"	*load_steps_needed = 0;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_LOAD_ITERATE(gs_splashLoadIterate)\n"
			"{\n"
			"	GameSceneSplash *gs = (GameSceneSplash*)game_scene->user_data;\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_UNLOAD(gs_splashUnload)\n"
			"{\n"
			"	GameSceneSplash *gs = (GameSceneSplash*)game_scene->user_data;\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_BEGIN(gs_splashBegin)\n"
			"{\n"
			"	GameSceneSplash *gs = (GameSceneSplash*)game_scene->user_data;\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_END(gs_splashEnd)\n"
			"{\n"
			"	GameSceneSplash *gs = (GameSceneSplash*)game_scene->user_data;\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_SCREEN_CHANGE(gs_splashScreenChange)\n"
			"{\n"
			"	GameSceneSplash *gs = (GameSceneSplash*)game_scene->user_data;\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_UPDATE_FRAME(gs_splashUpdateFrame)\n"
			"{\n"
			"	GameSceneSplash *gs = (GameSceneSplash*)game_scene->user_data;\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_UPDATE_TICK(gs_splashUpdateTick)\n"
			"{\n"
			"	GameSceneSplash *gs = (GameSceneSplash*)game_scene->user_data;\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_RENDER(gs_splashRender)\n"
			"{\n"
			"	GameSceneSplash *gs = (GameSceneSplash*)game_scene->user_data;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_MAKE(gs_splashMake)\n"
			"{\n"
			"	ztGame *game = (ztGame*)user_data;\n\n"
			"	GameSceneSplash *gs = zt_mallocStruct(GameSceneSplash);\n"
			"	game_scene->user_data = gs;\n\n"
			"	gs->game = game;\n\n"
			"	game_scene->ticks_per_second = 20;\n\n"
			"	game_scene->callback_load_get_details = ZT_FUNCTION_POINTER_TO_VAR(gs_splashLoadGetDetails);\n"
			"	game_scene->callback_load_iterate     = ZT_FUNCTION_POINTER_TO_VAR(gs_splashLoadIterate);\n"
			"	game_scene->callback_unload           = ZT_FUNCTION_POINTER_TO_VAR(gs_splashUnload);\n"
			"	game_scene->callback_begin            = ZT_FUNCTION_POINTER_TO_VAR(gs_splashBegin);\n"
			"	game_scene->callback_end              = ZT_FUNCTION_POINTER_TO_VAR(gs_splashEnd);\n"
			"	game_scene->callback_screen_change    = ZT_FUNCTION_POINTER_TO_VAR(gs_splashScreenChange);\n"
			"	game_scene->callback_update_frame     = ZT_FUNCTION_POINTER_TO_VAR(gs_splashUpdateFrame);\n"
			"	game_scene->callback_update_tick      = ZT_FUNCTION_POINTER_TO_VAR(gs_splashUpdateTick);\n"
			"	game_scene->callback_render           = ZT_FUNCTION_POINTER_TO_VAR(gs_splashRender);\n\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_FREE(gs_splashFree)\n"
			"{\n"
			"	GameSceneSplash *gs = (GameSceneSplash*)game_scene->user_data;\n"
			"	zt_free(gs);\n"
			"	game_scene->user_data = nullptr;\n"
			"}\n\n"
			"// public functions ===============================================================================================================================================================================\n\n"
			"void gs_splashInit(ztGame *game)\n"
			"{\n"
			"	zt_gameSceneManagerAddScene(&game->scene_manager, GAME_SCENE_SPLASH_GUID, ztGameSceneFlags_HdrScene | ztGameSceneFlags_UnloadOnEnd, ZT_FUNCTION_POINTER_TO_VAR(gs_splashMake), game, ZT_FUNCTION_POINTER_TO_VAR(gs_splashFree), game);\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			);
	}
	else if (zt_strEquals(src_file, "src\\" SRC_FILE_GAME_SCENE_MENU_H)) {
		ztRandom random;
		zt_randomInit(&random);

		zt_fileWritef(&file,
			"#ifndef __game_scene_menu_h__\n"
			"#define __game_scene_menu_h__\n"
			"// headers ========================================================================================================================================================================================\n\n"
			"#include \"game.h\"\n\n"
			"// types/enums/defines ============================================================================================================================================================================\n\n"
			"#define GAME_SCENE_MENU_GUID     zt_guid(0x%08x, 0x%08x, 0x%08x, 0x%08x)\n\n"
			"// structs/classes ================================================================================================================================================================================\n\n"
			"struct GameSceneMenu\n"
			"{\n"
			"	ztGame *game;\n"
			"};\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"// external variables =============================================================================================================================================================================\n\n"
			"// function prototypes ============================================================================================================================================================================\n\n"
			"void gs_menuInit(ztGame *game);\n\n"
			"// inline functions ===============================================================================================================================================================================\n\n"
			"#endif // include guard\n",
			zt_randomInt(&random, 0, ztInt32Max), zt_randomInt(&random, 0, ztInt32Max), zt_randomInt(&random, 0, ztInt32Max), zt_randomInt(&random, 0, ztInt32Max)
			);
	}
	else if (zt_strEquals(src_file, "src\\" SRC_FILE_GAME_SCENE_MENU)) {
		zt_fileWritef(&file,
			"// headers ========================================================================================================================================================================================\n\n"
			"#include \"game_scene_menu.h\"\n\n"
			"// variables ======================================================================================================================================================================================\n\n"
			"// private functions ==============================================================================================================================================================================\n\n"
			"// public function ================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_LOAD_GET_DETAILS(gs_menuLoadGetDetails)\n"
			"{\n"
			"	*load_steps_needed = 0;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_LOAD_ITERATE(gs_menuLoadIterate)\n"
			"{\n"
			"	GameSceneMenu *gs = (GameSceneMenu*)game_scene->user_data;\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_UNLOAD(gs_menuUnload)\n"
			"{\n"
			"	GameSceneMenu *gs = (GameSceneMenu*)game_scene->user_data;\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_BEGIN(gs_menuBegin)\n"
			"{\n"
			"	GameSceneMenu *gs = (GameSceneMenu*)game_scene->user_data;\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_END(gs_menuEnd)\n"
			"{\n"
			"	GameSceneMenu *gs = (GameSceneMenu*)game_scene->user_data;\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_SCREEN_CHANGE(gs_menuScreenChange)\n"
			"{\n"
			"	GameSceneMenu *gs = (GameSceneMenu*)game_scene->user_data;\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_UPDATE_FRAME(gs_menuUpdateFrame)\n"
			"{\n"
			"	GameSceneMenu *gs = (GameSceneMenu*)game_scene->user_data;\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_UPDATE_TICK(gs_menuUpdateTick)\n"
			"{\n"
			"	GameSceneMenu *gs = (GameSceneMenu*)game_scene->user_data;\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_RENDER(gs_menuRender)\n"
			"{\n"
			"	GameSceneMenu *gs = (GameSceneMenu*)game_scene->user_data;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_MAKE(gs_menuMake)\n"
			"{\n"
			"	ztGame *game = (ztGame*)user_data;\n\n"
			"	GameSceneMenu *gs = zt_mallocStruct(GameSceneMenu);\n"
			"	game_scene->user_data = gs;\n\n"
			"	gs->game = game;\n\n"
			"	game_scene->ticks_per_second = 20;\n\n"
			"	game_scene->callback_load_get_details = ZT_FUNCTION_POINTER_TO_VAR(gs_menuLoadGetDetails);\n"
			"	game_scene->callback_load_iterate     = ZT_FUNCTION_POINTER_TO_VAR(gs_menuLoadIterate);\n"
			"	game_scene->callback_unload           = ZT_FUNCTION_POINTER_TO_VAR(gs_menuUnload);\n"
			"	game_scene->callback_begin            = ZT_FUNCTION_POINTER_TO_VAR(gs_menuBegin);\n"
			"	game_scene->callback_end              = ZT_FUNCTION_POINTER_TO_VAR(gs_menuEnd);\n"
			"	game_scene->callback_screen_change    = ZT_FUNCTION_POINTER_TO_VAR(gs_menuScreenChange);\n"
			"	game_scene->callback_update_frame     = ZT_FUNCTION_POINTER_TO_VAR(gs_menuUpdateFrame);\n"
			"	game_scene->callback_update_tick      = ZT_FUNCTION_POINTER_TO_VAR(gs_menuUpdateTick);\n"
			"	game_scene->callback_render           = ZT_FUNCTION_POINTER_TO_VAR(gs_menuRender);\n\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_FUNC_GAME_SCENE_FREE(gs_menuFree)\n"
			"{\n"
			"	GameSceneMenu *gs = (GameSceneMenu*)game_scene->user_data;\n"
			"	zt_free(gs);\n"
			"	game_scene->user_data = nullptr;\n"
			"}\n\n"
			"// public functions ===============================================================================================================================================================================\n\n"
			"void gs_menuInit(ztGame *game)\n"
			"{\n"
			"	zt_gameSceneManagerAddScene(&game->scene_manager, GAME_SCENE_MENU_GUID, ztGameSceneFlags_HdrScene | ztGameSceneFlags_UnloadOnEnd, ZT_FUNCTION_POINTER_TO_VAR(gs_menuMake), game, ZT_FUNCTION_POINTER_TO_VAR(gs_menuFree), game);\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n");
	}
	else if (zt_strEquals(src_file, "src\\" SRC_FILE_GAME_DLL)) {
		bool draw_list = build_cfg->camera_2d || build_cfg->include_gui;

		zt_fileWritef(&file,
			"// headers ========================================================================================================================================================================================\n\n"
			"#define ZT_HOTSWAP_IMPLEMENTATION\n"
			"#define ZT_HOTSWAP_DLL\n"
			"#define ZT_HOTSWAP_DLL_PROCESS_UNLOAD_FUNC dll_processUnload\n"
			"#define ZT_HOTSWAP_DLL_PROCESS_RELOAD_FUNC dll_processReload\n"
			"#include \"zt_hotswap.h\"\n"
			"#include \"game.h\"\n\n"
			"#include \"game_scene_splash.h\"\n"
			"#include \"game_scene_menu.h\"\n"
			"#include \"game_scene_main.h\"\n\n"
			"// variables ======================================================================================================================================================================================\n\n"
			"// private functions ==============================================================================================================================================================================\n\n"
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
			);

		zt_fileWrite(&file,
			"bool dll_gameSceneManagerMake(ztGame *game)\n"
			"{\n"
			"	//if (!zt_guiDefaultThemeMake(&game->gui_theme, &game->scene_manager.screen_camera, &game->asset_manager, \"textures/gui_new_layout.png\", \"textures/gui_new_layout.spr\")) {\n"
			"	if (!zt_guiDefaultThemeMake(&game->gui_theme, &game->scene_manager.screen_camera)) {\n"
			"		return false;\n"
			"	}\n\n"
			"	game->gui_manager = zt_guiManagerMake(&game->scene_manager.screen_camera, &game->gui_theme, zt_memGetGlobalArena());\n\n"
			"	zt_gameSceneManagerMake(&game->scene_manager, 3, game->details, game->settings, game->settings->screen_w, game->settings->screen_h, game->settings->native_w, game->settings->native_h, &game->asset_manager, &game->input_registry, &game->draw_list, ztGameSceneManagerFlags_UseTargetTexture);\n"
			"	gs_mainInit(game);\n"
			"	gs_splashInit(game);\n"
			"	gs_menuInit(game);\n"
			"	zt_gameSceneManagerTransitionTo(&game->scene_manager, GAME_SCENE_MAIN_GUID, ztGameSceneTransition_Fade, ztGameSceneTransition_Fade);\n\n"
			"	zt_guiInitDebug(game->gui_manager);\n\n"
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"void dll_gameSceneManagerFree(ztGame *game)\n"
			"{\n"
			"	zt_guiDefaultThemeFree(&game->gui_theme);\n"
			"	zt_gameSceneManagerFree(&game->scene_manager, game->gui_manager, false);\n"
			"	zt_guiManagerFree(game->gui_manager);\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_DLLEXPORT bool dll_init(ztGameDetails* details, ztGameSettings* settings, void** game_memory)\n"
			"{\n"
			"	ztGame *game = zt_mallocStruct(ztGame);\n"
			"	*game_memory = game;\n\n"
			"	game->details     = details;\n"
			"	game->settings    = settings;\n\n"
			"	if (!dll_gameSceneManagerMake(game)) {\n"
			"		return false;\n"
			"	}\n\n"
			"	zt_debuggingInit(\"debug.cfg\");\n\n"
			);

		if (build_cfg->include_vr) {
			zt_fileWrite(&file,
				"	game->vr = zt_vrIsHeadsetPresent() ? zt_vrMake() : nullptr;\n\n"
				);
		}

		if (build_cfg->include_pbr) {
			zt_fileWritef(&file,
				"	ztRandom random;\n"
				"	zt_randomInit(&random);\n"
				"	game->texture_random = zt_textureMakeRandom(&random, 4, 4);\n\n"
				"	ztPostProcessingEffect effects[32];\n"
				"	int eidx = 0;\n"
				"	//if (!(game->pp_ssao = zt_postProcessingEffectMakeSSAO(&effects[eidx++], settings->screen_w, settings->screen_h, &game->camera_3d, game->render_target_attach_position, game->render_target_attach_normal, game->texture_random))) {\n"
				"	//	zt_logCritical(\"Could not create ssao post processing effect\");\n"
				"	//	return false;\n"
				"	//}\n"
				"	if (!zt_postProcessingEffectMakeBloom(&effects[eidx++], settings->screen_w, settings->screen_h)) {\n"
				"		zt_logCritical(\"Could not create bloom post processing effect\");\n"
				"		return false;\n"
				"	}\n"
				"	if (!zt_postProcessingEffectMakeTonemap(&effects[eidx++], settings->screen_w, settings->screen_h, 4.4f, 2.f)) {\n"
				"		zt_logCritical(\"Could not create tonemap post processing effect\");\n"
				"		return false;\n"
				"	}\n"
				"	if (!zt_postProcessingEffectMakeVignette(&effects[eidx++], settings->screen_w, settings->screen_h, .55f, .75f, .5f)) {\n"
				"		zt_logCritical(\"Could not create vignette post processing effect\");\n"
				"		return false;\n"
				"	}\n\n"
				"	zt_postProcessingStackMake(&game->post_processing, settings->screen_w, settings->screen_h, effects, eidx);\n"
				"	\n"
				);
		}

		zt_fileWritef(&file,
			"	return true;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_DLLEXPORT void dll_cleanup(void *memory)\n"
			"{\n"
			"	ztGame *game = (ztGame*)memory;\n\n"
			"	dll_gameSceneManagerFree(game);\n\n"
			);

		if (build_cfg->include_pbr) {
			zt_fileWritef(&file,
				"	zt_textureFree(game->texture_random);\n\n"
				"	zt_postProcessingStackFree(&game->post_processing);\n\n"
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

		zt_fileWritef(&file,
			"\n"
			"	zt_free(game);\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_DLLEXPORT void dll_processUnload(void *memory)\n"
			"{\n"
			"	//ztGame *game = (ztGame*)memory;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_DLLEXPORT void dll_processReload(void *memory)\n"
			"{\n"
			"	//ztGame *game = (ztGame*)memory;\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_DLLEXPORT void dll_screenChange(ztGameSettings *settings, void *memory)\n"
			"{\n"
			"	ztGame *game = (ztGame*)memory;\n"
			"	zt_gameSceneManagerScreenUpdate(&game->scene_manager, settings);\n"
			"}\n\n"
			"// ================================================================================================================================================================================================\n\n"
			"ZT_DLLEXPORT bool dll_gameLoop(void *memory, r32 dt)\n"
			"{\n"
			"	ztGame *game = (ztGame*)memory;\n\n"
			"	if (!zt_gameSceneManagerUpdate(&game->scene_manager, game->gui_manager, dt)) {\n"
			"		return false;\n"
			"	}\n"
			"	// post processing\n"
			"	//zt_postProcessingStackRender(&game->post_processing, game->scene_manager.screen_texture, &game->draw_list, &game->scene_manager.screen_camera);\n\n"
			"	zt_drawListAddScreenRenderTexture(&game->draw_list, game->scene_manager.screen_texture, &game->scene_manager.screen_camera);\n"
			"	zt_renderDrawList(&game->scene_manager.screen_camera, &game->draw_list, ztVec4::zero, ztRenderDrawListFlags_NoClear | ztRenderDrawListFlags_NoDepthTest);\n\n"
			"	{\n"
			"		ZT_PROFILE_GAME(\"gameLoop:asset check\");\n"
			"		zt_assetManagerCheckForChanges(&game->asset_manager);\n"
			"	}\n\n"
			"	return true;\n"
			"}\n\n"
			);
	}
	else if (zt_strEquals(src_file, "src\\" SRC_FILE_GAME)) {
	}
	else if (zt_strEquals(src_file, "src\\" SRC_FILE_GUI_THEME_H)) {
	}
	else if (zt_strEquals(src_file, "src\\" SRC_FILE_GUI_THEME)) {
	}

	if (need_impls) {
		if (build_cfg->include_gui) {
			zt_fileWritef(&file,
				"#define ZT_GAME_GUI_IMPLEMENTATION\n"
				"#include \"zt_game_gui.h\"\n\n"
				"#define ZT_GAME_GUI_DEFAULT_THEME_IMPLEMENTATION\n"
				"#include \"zt_game_gui_default_theme.h\"\n\n"
				);
		}
		if (build_cfg->include_vr) {
			zt_fileWritef(&file,
				"#define ZT_VR_IMPLEMENTATION\n"
				"#include \"openvr/openvr.h\"\n"
				"#include \"zt_vr.h\"\n\n"
				);
		}
		if (build_cfg->include_entity) {
			zt_fileWrite(&file,
				"#define ZT_GAME_ENTITY_IMPLEMENTATION\n"
				"#include \"zt_game_entity.h\"\n\n"
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
			sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME_SCENE_SPLASH_H);
			sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME_SCENE_SPLASH);
			sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME_SCENE_MENU_H);
			sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME_SCENE_MENU);
			sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME_SCENE_MAIN_H);
			sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME_SCENE_MAIN);
			sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME_DLL);

			if (build_cfg->include_entity) {
				sln_createSourceFile(build_cfg, proj_dir, "src\\"SRC_FILE_GAME_SCENE_EDITOR);
			}

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
		sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME_SCENE_SPLASH_H);
		sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME_SCENE_SPLASH);
		sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME_SCENE_MENU_H);
		sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME_SCENE_MENU);
		sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME_SCENE_MAIN_H);
		sln_createSourceFile(build_cfg, proj_dir, "src\\" SRC_FILE_GAME_SCENE_MAIN);

		if (build_cfg->include_entity) {
			sln_createSourceFile(build_cfg, proj_dir, "src\\"SRC_FILE_GAME_SCENE_EDITOR);
		}

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
			"run\\data\\scenes",
			"run\\data\\shaders",
			"run\\data\\textures",
			"run\\data\\textures\\materials",
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
		if (build_cfg->include_entity) {
			char *files_src[] = { "textures\\environment.hdr",            "textures\\test_grid.albedo.png",                       "scenes\\test_scene.scene",            "models\\testing_scene.ztm",               "shaders\\default.zts",            "shaders\\default_pbr.zts",            "shaders\\default_pbr_anim.zts" };
			char *files_dst[] = { "run\\data\\textures\\environment.hdr", "run\\data\\textures\\materials\\test_grid.albedo.png", "run\\data\\scenes\\test_scene.scene", "run\\data\\models\\testing_scene.ztm", "run\\data\\shaders\\default.zts", "run\\data\\shaders\\default_pbr.zts", "run\\data\\shaders\\default_pbr_anim.zts" };

			local_copy::copy_files(data_dir, proj_dir, files_src, zt_elementsOf(files_src), files_dst);
		}
		else {
			char *files_src[] = { "textures\\environment.hdr", "models\\pbr_test_statue.obj", "models\\pbr_test_statue_albedo.png", "models\\pbr_test_statue_height.png", "models\\pbr_test_statue_metallic.png", "models\\pbr_test_statue_normal.png", "models\\pbr_test_statue_roughness.png", "models\\floor_panel_albedo.png", "models\\floor_panel_height.png", "models\\floor_panel_metallic.png", "models\\floor_panel_normal.png", "models\\floor_panel_roughness.png" };
			char *files_dst[] = { "run\\data\\textures\\environment.hdr", "run\\data\\models\\pbr_test_statue.obj", "run\\data\\models\\pbr_test_statue_albedo.png", "run\\data\\models\\pbr_test_statue_height.png", "run\\data\\models\\pbr_test_statue_metallic.png", "run\\data\\models\\pbr_test_statue_normal.png", "run\\data\\models\\pbr_test_statue_roughness.png", "run\\data\\models\\floor_panel_albedo.png", "run\\data\\models\\floor_panel_height.png", "run\\data\\models\\floor_panel_metallic.png", "run\\data\\models\\floor_panel_normal.png", "run\\data\\models\\floor_panel_roughness.png" };

			local_copy::copy_files(data_dir, proj_dir, files_src, zt_elementsOf(files_src), files_dst);
		}
	}

	if (build_cfg->dll_project) {
		sln_createBuildBatchFiles(build_cfg, proj_dir);
	}

	return true;
}

// ------------------------------------------------------------------------------------------------
