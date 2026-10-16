#include <mod/iaml.h>
#include <mod/amlmod.h>
#include <mod/config.h>

MYMODCFG(com.daniloszk.menuszk, MenuSZK, 1.0.1, DaniloSZK)

#include "pch.h"
#include "MenuSZK.h"
#include "imgLoader.h"
#include "jcrasher.h"

#include <cstring>
#include <string>

ModData* modData = new ModData("dextrainer");
FileLogger* localLogger = new FileLogger("dextrainer");

// Where the crash dumper writes its report. Built once during mod load
// (safe context) so the SIGSEGV handler doesn't need to allocate.
static std::string g_crashLogPath;

// Shim so we can hand AML's mcontext_t* to jcrasher's ucontext_t* API.
// jcrasher reads ((ucontext_t*)ctx)->uc_mcontext, so we place the mcontext
// at the offset it expects. Global so no stack pressure in the signal handler.
static ucontext_t g_crashUCtxShim;

extern "C" void OnModPreLoad()
{
    fileLog->Clear();
    fileLog->Log("Log initialized");
    fileLog->Log("OnModPreLoad");

    ModData::CreateFolder(menuSZK->GetLocalizationsPath());

    RegisterInterface("MenuSZK", (IMenuSZK*)menuSZK);

    modData->LoadSettings();

    // Build the crash log path and make sure the folder exists.
    // Uses the same mod folder as the localization files.
    std::string modRoot = std::string(aml->GetAndroidDataRootPath()) + "/mods/DexTrainer/";
    g_crashLogPath = modRoot + "last_crash.log";
    ModData::CreateFolder(modRoot);
}

extern "C" void OnModLoad()
{
    fileLog->Log("OnModLoad");

#if defined(AML32)
    fileLog->Log("AML 32 defined");
#elif defined(AML64)
    fileLog->Log("AML 64 defined");
#else
    fileLog->Error("Neither AML 32 or 64 defined!");
    return;
#endif

    loadInterface(&sautils, "SAUtils", true);

    loadInterface(&BASS, "BASS", true);

    loadInterface(&scriptCommands, "ScriptCommands", true);

    [[maybe_unused]] void* hGTASA = dlopen("libGTASA.so", RTLD_LAZY);
    [[maybe_unused]] uintptr_t pGTASA = aml->GetLib("libGTASA.so");

    fileLog->Log("Finding addresses and hooking...");

    DoHooks();

    if (BASS)
    {
        fileLog->Log("Initializing BASS...");
        soundsys->Init();
        fileLog->Log("BASS initialized!");
    }

    fileLog->Log("Initializing menuSZK...");

    menuSZK->Initialize();

    // Arm the crash dumper. Must happen from a normal context (this function,
    // during mod load) so jcrasher can install its realtime-signal handler and
    // fork the preinit helper child cleanly.
    {
        int rc = JCrasher::InitAllThreadsDumperPath(g_crashLogPath.c_str());
        if (rc == JCrasher::DUMP_OK)
        {
            fileLog->Log("[Crash] Dumper armed, log path: " + g_crashLogPath);
        }
        else
        {
            fileLog->Error("[Crash] Dumper init failed, code=" + std::to_string(rc));
        }
    }

    fileLog->Log("Mod loaded!");
}

// Crash callback invoked by AML from its SIGSEGV/SIGBUS handler.
// AML gives us a pointer to just the mcontext_t; jcrasher's
// FillRegsFromUctx() reads ((ucontext_t*)ctx)->uc_mcontext. We bridge them by
// copying the mcontext into a global ucontext_t at the offset jcrasher reads.
ON_GAME_CRASH()
{
    if (g_crashLogPath.empty()) return;

    g_crashUCtxShim.uc_mcontext = *mcontext;

    JCrasher::DumpAllThreadsPath(
        g_crashLogPath.c_str(),
        &g_crashUCtxShim,
        JCrasher::DUMP_ALL_THREADS
      | JCrasher::DUMP_SYMBOLS
      | JCrasher::DUMP_CRASHED_FIRST
      | JCrasher::DUMP_THREAD_HEADER,
        64);
}