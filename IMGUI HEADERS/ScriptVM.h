// ScriptVM — CheatMenu-style script opcode execution for DexTrainer.
// Ported from plugin-sdk ScriptCommands.h, adapted for ARM64 Android.
//
// Verified offsets from libGTASA.so disassembly (see SCRIPT_VM_RE.md):
//   CRunningScript::Init @ 0x3f2818
//   CRunningScript::ProcessOneCommand @ 0x3f445c
//   m_pCurrentIP @ 0x20, m_bUseMissionCleanup @ 0x116, m_bNotFlag @ 0x122
//   m_bIsMission @ 0x12C, m_aLocalVars @ 0x6C, struct size 0x130
#pragma once

#include <cstdint>
#include <cstring>
#include <vector>

// Script parameter types (from GTA SA CRunningScript)
enum eScriptParamType {
    SCRIPTPARAM_END_OF_ARGUMENTS = 0,
    SCRIPTPARAM_STATIC_INT_32BITS = 1,
    SCRIPTPARAM_GLOBAL_NUMBER_VARIABLE = 2,
    SCRIPTPARAM_LOCAL_NUMBER_VARIABLE = 3,
    SCRIPTPARAM_STATIC_INT_8BITS = 4,
    SCRIPTPARAM_STATIC_INT_16BITS = 5,
    SCRIPTPARAM_STATIC_FLOAT = 6,
};

// CRunningScript layout (ARM64, verified from binary)
struct CRunningScriptLayout {
    uint8_t data[0x130];
};

// ScriptCode — builds script bytecode with proper type tags
class ScriptCode {
public:
    ScriptCode(short commandId = -1);
    ~ScriptCode();

    unsigned char* GetData() { return data_; }

    void Pack(int n);           // static int32
    void Pack(float n);         // static float
    void Pack(int* p);          // local var (result) — registers for SaveResultVariables

    // After ProcessOneCommand, copy local var results to registered pointers
    void SaveResultVariables(CRunningScriptLayout* script);

private:
    void AddByte(unsigned char b);
    void AddBytes(const unsigned char* bytes, unsigned int count);

    unsigned char* data_;
    unsigned int capacity_;
    unsigned int size_;

    struct VarToSet {
        unsigned int varIndex;
        void* pVar;
    };
    std::vector<VarToSet> varsToSet_;
    unsigned short varIndexCounter_;
};

// ScriptVM — singleton for executing script commands
class ScriptVM {
public:
    static void Initialize();  // bind Init and ProcessOneCommand

    // Execute CREATE_CAR (0x00A5): returns vehicle handle, or -1 on failure
    // modelId: vehicle model ID, x/y/z: spawn position
    static int CreateCar(int modelId, float x, float y, float z);

    // Get CVehicle* from handle (via CPools::GetVehicle)
    static void* GetVehicleFromHandle(int handle);

private:
    static void (*s_Init)(void*);
    static void (*s_ProcessOneCommand)(void*);
    static void* (*s_CPools_GetVehicle)(int);
};
