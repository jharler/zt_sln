/**************************************************************************************************
** file: loader_main.cpp
**
** This library is in the public domain.  Do with it what you will.
**
**************************************************************************************************
**
** implementation notes:
**
**************************************************************************************************/

// headers ========================================================================================

#define ZT_GAME_NAME                "ZeroTolerance Project Generator"
#define ZT_GAME_LOCAL_ONLY
#define ZT_GAME_FUNC_SETTINGS       ldr_gameSettings
#define ZT_GAME_FUNC_INIT           ldr_gameInit
#define ZT_GAME_FUNC_SCREENCHANGE   ldr_gameScreenChange
#define ZT_GAME_FUNC_CLEANUP        ldr_gameCleanup
#define ZT_GAME_FUNC_LOOP           ldr_gameLoop

#include "zt_tools.h"
#include "zt_game.h"
#include "zt_game_gui.h"


// types/enums/defines ============================================================================

#define LDR_DLL_NAME	"dll.dll"

typedef bool (zt_dllSettings_Func    ) (ztGameDetails *, ztGameSettings *);
typedef bool (zt_dllInit_Func        ) (ztGameDetails *, ztGameSettings *, void **game_memory);
typedef bool (zt_dllReload_Func      ) (void *);
typedef bool (zt_dllUnload_Func      ) (void *);
typedef void (zt_dllCleanup_Func     ) (void *);
typedef void (zt_dllScreenChange_Func) (ztGameSettings *, void *);
typedef bool (zt_dllGameLoop_Func    ) (void *, r32);


// structs/classes ================================================================================

struct ztDll
{
	void *game_memory;

	HMODULE game_dll;
	char    game_dll_name[ztFileMaxPath];

	zt_dllSettings_Func     *zt_dllSettings;
	zt_dllInit_Func         *zt_dllInit;
	zt_dllReload_Func       *zt_dllReload;
	zt_dllUnload_Func       *zt_dllUnload;
	zt_dllCleanup_Func      *zt_dllCleanup;
	zt_dllScreenChange_Func *zt_dllScreenChange;
	zt_dllGameLoop_Func     *zt_dllGameLoop;

	ztDirectoryMonitor dir_mon;

	ztGameDetails *details;
};


// variables ======================================================================================

ztDll g_dll;
ztFile g_log;


// private functions ==============================================================================

// ------------------------------------------------------------------------------------------------

ZT_FUNC_LOG_CALLBACK(ldr_logCallback)
{
	zt_fileWrite(&g_log, message, zt_strSize(message) - 1);
	zt_fileWrite(&g_log, "\n", 1);
}

// ------------------------------------------------------------------------------------------------

void ldr_buildDllCleanup()
{
	ztBlockProfiler bp("ldr_buildDllCleanup");

	char cleanup_file[ztFileMaxPath];
	zt_fileConcatFileToPath(cleanup_file, zt_elementsOf(cleanup_file), g_dll.details->app_path, "..\\cleanup.bat");

	if (zt_fileExists(cleanup_file)) {
		char cleanup_path[ztFileMaxPath];
		zt_fileGetFullPath(cleanup_file, cleanup_path, zt_elementsOf(cleanup_path));

		char current_path[ztFileMaxPath];
		zt_fileGetCurrentPath(current_path, zt_elementsOf(current_path));
		zt_fileSetCurrentPath(cleanup_path);

		zt_strMakePrintf(cleanup_cmd, ztFileMaxPath, "cmd.exe /c \"%s\"", cleanup_file);
		zt_processRun(cleanup_cmd);

		zt_fileSetCurrentPath(current_path);
	}
	else {
		zt_logCritical("cleanup file not found: %s", cleanup_file);
	}
}

// ------------------------------------------------------------------------------------------------

bool ldr_loadDll(const char *dll_name, bool initial_load)
{
	if (initial_load) {
		ldr_buildDllCleanup();
	}

	zt_logInfo("Loading game DLL: %s", dll_name);

	if (g_dll.game_dll != NULL) {
		if (g_dll.zt_dllUnload) {
			g_dll.zt_dllUnload(g_dll.game_memory);
		}
		FreeLibrary(g_dll.game_dll);
	}

	if (!initial_load) {
		zt_sleep(.2f);
	}

	HMODULE dll = LoadLibraryA(dll_name);

	if (dll == NULL) {
		for (int i = 0; i < 100 && dll == NULL && GetLastError() == 32; ++i) {
			zt_sleep(.1f);
			dll = LoadLibraryA(dll_name);
		}
		if (dll == NULL) {
			zt_logCritical("Unable to load DLL: %s (%d)", dll_name, GetLastError());
			return false;
		}
	}

	g_dll.game_dll = dll;

	zt_dllSetGlobals_Func *zt_dllSetGlobals = (zt_dllSetGlobals_Func *)GetProcAddress(g_dll.game_dll, "zt_dllSetGlobals");
	if (!zt_dllSetGlobals) {
		zt_logCritical("Game DLL does not contain a SetGlobals function");
		return false;
	}
	zt_dllSendGlobals(zt_dllSetGlobals);

	zt_dllSetGameGlobals_Func *zt_dllSetGameGlobals = (zt_dllSetGameGlobals_Func *)GetProcAddress(g_dll.game_dll, "zt_dllSetGameGlobals");
	if (!zt_dllSetGameGlobals) {
		zt_logCritical("Game DLL does not contain a SetGameGlobals function");
		return false;
	}

#	if defined(ZT_OPENGL)
	zt_dllSetOpenGLGlobals_Func *zt_dllSetOpenGLGlobals = (zt_dllSetOpenGLGlobals_Func *)GetProcAddress(g_dll.game_dll, "zt_dllSetOpenGLGlobals");
	if (!zt_dllSetOpenGLGlobals) {
		zt_logCritical("Game DLL does not contain a SetOpenGLGlobals function");
		return false;
	}
	zt_dllSendGameGlobals(zt_dllSetGameGlobals, zt_dllSetOpenGLGlobals);

	if(!initial_load) {
		zt_dllSendOpenGLGlobals(zt_dllSetOpenGLGlobals);
	}
#	else
	zt_dllSendGameGlobals(zt_dllSetGameGlobals);
#	endif // ZT_OPENGL

	zt_dllSetGameGuiGlobals_Func *zt_dllSetGameGuiGlobals = (zt_dllSetGameGuiGlobals_Func *)GetProcAddress(g_dll.game_dll, "zt_dllSetGameGuiGlobals");
	if (zt_dllSetGameGuiGlobals) {
		zt_dllSendGameGuiGlobals(zt_dllSetGameGuiGlobals);
	}

	g_dll.zt_dllSettings     = (zt_dllSettings_Func     *)GetProcAddress(g_dll.game_dll, "zt_dllSettings");
	g_dll.zt_dllInit         = (zt_dllInit_Func         *)GetProcAddress(g_dll.game_dll, "zt_dllInit");
	g_dll.zt_dllReload       = (zt_dllReload_Func       *)GetProcAddress(g_dll.game_dll, "zt_dllReload");
	g_dll.zt_dllUnload       = (zt_dllUnload_Func       *)GetProcAddress(g_dll.game_dll, "zt_dllUnload");
	g_dll.zt_dllCleanup      = (zt_dllCleanup_Func      *)GetProcAddress(g_dll.game_dll, "zt_dllCleanup");
	g_dll.zt_dllScreenChange = (zt_dllScreenChange_Func *)GetProcAddress(g_dll.game_dll, "zt_dllScreenChange");
	g_dll.zt_dllGameLoop     = (zt_dllGameLoop_Func     *)GetProcAddress(g_dll.game_dll, "zt_dllGameLoop");

	if (!initial_load && g_dll.zt_dllReload) {
		g_dll.zt_dllReload(g_dll.game_memory);
	}

	zt_strCpy(g_dll.game_dll_name, zt_elementsOf(g_dll.game_dll_name), dll_name);

	zt_logInfo("Game DLL successfully loaded.");

	ldr_buildDllCleanup();

	return true;
}

// ------------------------------------------------------------------------------------------------

void ldr_buildDll()
{
	zt_logInfo("Building game DLL");

	char *build_file_name = zt_debugOnly("..\\build_d.bat") zt_releaseOnly("..\\build_r.bat");

	char build_file[ztFileMaxPath];
	zt_fileConcatFileToPath(build_file, zt_elementsOf(build_file), g_dll.details->app_path, build_file_name);

	if (zt_fileExists(build_file)) {
		char build_path[ztFileMaxPath];
		zt_fileGetFullPath(build_file, build_path, zt_elementsOf(build_path));

		char current_path[ztFileMaxPath];
		zt_fileGetCurrentPath(current_path, zt_elementsOf(current_path));
		zt_fileSetCurrentPath(build_path);

		int size = 1024 * 32;
		char *build_results = zt_mallocStructArrayArena(char, size, zt_memGetGlobalArena());

		zt_strMakePrintf(build_cmd, ztFileMaxPath, "cmd.exe /c \"%s\"", build_file);

		zt_processRun(build_cmd, build_results, size);

		ztToken lines[100];
		int lines_count = zt_strTokenize(build_results, "\r\n", lines, zt_elementsOf(lines));

		zt_fiz(zt_min(lines_count, zt_elementsOf(lines))) {
			char line[1024];
			zt_strCpy(line, zt_elementsOf(line), zt_strMoveForward(build_results, lines[i].beg), lines[i].len);

			if (zt_strFindPos(line, ") : error ", 0) != ztStrPosNotFound) {
				zt_logCritical(line);
			}
			else {
				zt_logInfo(line);
			}
		}

		if (lines_count >= zt_elementsOf(lines)) {
			zt_logInfo("%d more lines...", lines_count - zt_elementsOf(lines));
		}

		zt_freeArena(build_results, zt_memGetGlobalArena());

		zt_fileSetCurrentPath(current_path);
	}
	else {
		zt_logCritical("build file not found: %s", build_file);
	}
}

// ------------------------------------------------------------------------------------------------

ZT_FUNC_CONSOLE_COMMAND(ldr_buildDllConsoleCommand)
{
	ldr_buildDll();
}


// functions ======================================================================================

bool ldr_gameSettings(ztGameDetails* details, ztGameSettings* settings)
{
	zt_fileOpen(&g_log, "log.txt", ztFileOpenMethod_WriteOver);
	zt_logAddCallback(ldr_logCallback, ztLogMessageLevel_Verbose);

	g_dll.details = details;

	zt_strMakePrintf(dll_name, ztFileMaxPath, "%s\\" LDR_DLL_NAME, details->app_path);
	if (!ldr_loadDll(dll_name, true)) {
		return false;
	}

	if (g_dll.zt_dllSettings == nullptr || g_dll.zt_dllInit == nullptr) {
		zt_logCritical("Game DLL does not contain a startup or init function");
		return false;
	}

	if (!g_dll.zt_dllSettings(details, settings)) {
		zt_logCritical("Game DLL failed to startup");
		return false;
	}
	else {
		char bytes[128];
		zt_strBytesToString(bytes, zt_elementsOf(bytes), settings->memory);
		zt_logInfo("Game DLL requesting %s of memory", bytes);
	}

	zt_directoryMonitor(&g_dll.dir_mon, details->app_path, false, ztDirectoryMonitorFlags_All);

	return true;
}

// ------------------------------------------------------------------------------------------------

bool ldr_gameInit(ztGameDetails* details, ztGameSettings* settings)
{
	zt_guiInitGlobalMemory(zt_memGetGlobalArena());

	if (!g_dll.zt_dllInit(details, settings, &g_dll.game_memory)) {
		zt_logCritical("Game DLL failed to initialize");
		return false;
	}

#	if defined(ZT_LOADER)
	zt_consoleAddCommand("build", "Compiles the DLL", ZT_FUNCTION_POINTER_TO_VAR(ldr_buildDllConsoleCommand), ZT_FUNCTION_POINTER_TO_VAR_NULL);
#	endif

	return true;
}

// ------------------------------------------------------------------------------------------------

void ldr_gameCleanup()
{
	if (g_dll.zt_dllCleanup) {
		g_dll.zt_dllCleanup(g_dll.game_memory);
	}
	if (g_dll.zt_dllUnload) {
		g_dll.zt_dllUnload(g_dll.game_memory);
	}

	FreeLibrary(g_dll.game_dll);

	ldr_buildDllCleanup();

	g_dll.zt_dllSettings     = nullptr;
	g_dll.zt_dllInit         = nullptr;
	g_dll.zt_dllReload       = nullptr;
	g_dll.zt_dllUnload       = nullptr;
	g_dll.zt_dllCleanup      = nullptr;
	g_dll.zt_dllScreenChange = nullptr;
	g_dll.zt_dllGameLoop     = nullptr;

	zt_logRemoveCallback(ldr_logCallback);
	zt_fileClose(&g_log);
}

// ------------------------------------------------------------------------------------------------

void ldr_gameScreenChange(ztGameSettings *settings)
{
	if (g_dll.zt_dllScreenChange) {
		g_dll.zt_dllScreenChange(settings, g_dll.game_memory);
	}
}

// ------------------------------------------------------------------------------------------------

bool ldr_gameLoop(r32 dt)
{
	ztInputKeys *keys = zt_inputKeysAccessState();
	if (keys[ztInputKeys_Control].pressed() && keys[ztInputKeys_Menu].pressed() && keys[ztInputKeys_Shift].pressed()) {
		if (keys[ztInputKeys_B].justPressed()) {
			ldr_buildDll();
		}
		if (keys[ztInputKeys_L].justPressed()) {
			zt_logInfo("This is a log message");
			zt_logDebug("This is a debug message");
		}
	}

	if (zt_directoryMonitorHasChanged(&g_dll.dir_mon)) {
		while (zt_directoryMonitorHasChanged(&g_dll.dir_mon)) {
			;
		}

		char file_buffer[1024 * 16];
		zt_getDirectoryFiles(g_dll.details->app_path, file_buffer, zt_elementsOf(file_buffer), false);

		const char *files = file_buffer;
		while (files) {
			int end = zt_strFindPos(files, "\n", 0);
			if (end == ztStrPosNotFound) end = zt_strLen(files);
			if (end == 0) break;

			if (zt_striEndsWith(files, end, ".hotload." LDR_DLL_NAME, zt_strLen(".hotload." LDR_DLL_NAME))) {
				char file[ztFileMaxPath];
				zt_strCpy(file, ztFileMaxPath, files, end);

				if (!zt_strEquals(file, g_dll.game_dll_name)) {
					if (!ldr_loadDll(file, false)) {
						zt_logCritical("Unable to reload game DLL");
						return false;
					}

					break;
				}
			}

			files = zt_strMoveForward(files, end + 1);
		}
	}

	return g_dll.zt_dllGameLoop(g_dll.game_memory, dt);
}

// ------------------------------------------------------------------------------------------------
