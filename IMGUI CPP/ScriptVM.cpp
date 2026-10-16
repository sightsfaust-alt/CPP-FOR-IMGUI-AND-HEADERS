// ScriptVM — CheatMenu-style script opcode execution.
// See ScriptVM.h for verified offsets.

#include "ScriptVM.h"
#include <dlfcn.h>
#include <cstdio>

// ---------------------------------------------------------------------------
// ScriptCode implementation (ported from plugin-sdk)
// ---------------------------------------------------------------------------

ScriptCode::ScriptCode(short commandId) {
    capacity_ = 32;
    data_ = new unsigned char[capacity_];
    size_ = 0;
    varIndexCounter_ = 0;
    if (commandId != -1) {
        AddBytes(reinterpret_cast<unsigned char*>(&commandId), 2);
    }
}

ScriptCode::~ScriptCode() {
    delete[] data_;
}

void ScriptCode::AddByte(unsigned char b) {
    AddBytes(&b, 1);
}

void ScriptCode::AddBytes(const unsigned char* bytes, unsigned int count) {
    unsigned int newSize = size_ + count;
    if (newSize > capacity_) {
        while (newSize > capacity_) capacity_ += 32;
        unsigned char* newData = new unsigned char[capacity_];
        memcpy(newData, data_, size_);
        delete[] data_;
        data_ = newData;
    }
    memcpy(&data_[size_], bytes, count);
    size_ = newSize;
}

void ScriptCode::Pack(int n) {
    AddByte(SCRIPTPARAM_STATIC_INT_32BITS);
    AddBytes(reinterpret_cast<unsigned char*>(&n), 4);
}

void ScriptCode::Pack(float n) {
    AddByte(SCRIPTPARAM_STATIC_FLOAT);
    AddBytes(reinterpret_cast<unsigned char*>(&n), 4);
}

void ScriptCode::Pack(int* p) {
    AddByte(SCRIPTPARAM_LOCAL_NUMBER_VARIABLE);
    // GTASA allows 32 local vars (0-31)
    if (varIndexCounter_ >= 31) return;
    varsToSet_.push_back({varIndexCounter_, p});
    unsigned short idx = varIndexCounter_;
    AddBytes(reinterpret_cast<unsigned char*>(&idx), 2);
    ++varIndexCounter_;
}

void ScriptCode::SaveResultVariables(CRunningScriptLayout* script) {
    // m_aLocalVars @ 0x6C, each var is 4 bytes (tScriptParam union)
    for (auto& v : varsToSet_) {
        int32_t* localVar = reinterpret_cast<int32_t*>(script->data + 0x6C + v.varIndex * 4);
        *reinterpret_cast<int*>(v.pVar) = *localVar;
    }
}

// ---------------------------------------------------------------------------
// ScriptVM implementation
// ---------------------------------------------------------------------------

void (*ScriptVM::s_Init)(void*) = nullptr;
void (*ScriptVM::s_ProcessOneCommand)(void*) = nullptr;
void* (*ScriptVM::s_CPools_GetVehicle)(int) = nullptr;

void ScriptVM::Initialize() {
    void* h = dlopen("libGTASA.so", RTLD_NOLOAD | RTLD_LAZY);
    if (!h) h = dlopen("libGTASA.so", RTLD_LAZY);
    if (!h) return;
    s_Init = (void(*)(void*))dlsym(h, "_ZN14CRunningScript4InitEv");
    s_ProcessOneCommand = (void(*)(void*))dlsym(h, "_ZN14CRunningScript17ProcessOneCommandEv");
    s_CPools_GetVehicle = (void*(*)(int))dlsym(h, "_ZN6CPools10GetVehicleEi");
}

int ScriptVM::CreateCar(int modelId, float x, float y, float z) {
    if (!s_Init || !s_ProcessOneCommand) return -1;

    // Create dummy script (exactly like CheatMenu/plugin-sdk)
    static CRunningScriptLayout script;
    memset(&script, 0, sizeof(script));

    s_Init(&script);

    // Init() already sets these, but be explicit (harmless, matches plugin-sdk):
    // m_bIsMission @ 0x12C = false
    script.data[0x12C] = 0;
    // m_bUseMissionCleanup @ 0x116 = false → param=0 → vehicle PERSISTS
    script.data[0x116] = 0;
    // m_bNotFlag @ 0x122 = (commandId >> 15) & 1 = 0 for 0x00A5
    script.data[0x122] = 0;

    // Build bytecode: 00A5 <model:int> <x:float> <y:float> <z:float> <&hveh:localvar>
    int hveh = -1;
    ScriptCode code(0x00A5);
    code.Pack(modelId);
    code.Pack(x);
    code.Pack(y);
    code.Pack(z);
    code.Pack(&hveh);

    // m_pBaseIP @ 0x18, m_pCurrentIP @ 0x20
    void* bytecode = code.GetData();
    memcpy(script.data + 0x18, &bytecode, 8);
    memcpy(script.data + 0x20, &bytecode, 8);

    s_ProcessOneCommand(&script);
    code.SaveResultVariables(&script);

    return hveh;
}

void* ScriptVM::GetVehicleFromHandle(int handle) {
    if (!s_CPools_GetVehicle || handle < 0) return nullptr;
    return s_CPools_GetVehicle(handle);
}
