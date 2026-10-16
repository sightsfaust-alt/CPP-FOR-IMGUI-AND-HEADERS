#pragma once

#include "FileLogger.h"
#include "ModData.h"

// CLEO support REMOVED (2026-10-14): menu no longer depends on CLEO.
// The single opcode (SET_GLOBAL_INT_VARIABLE) was optional and is dropped.

// SAUtils 1.6
#include "include/isautils.h"
inline ISAUtils* sautils = nullptr;

// Script Commands
#include "include/IScriptCommands.h"
inline IScriptCommands* scriptCommands = nullptr;

// BASS
#include "csoundsystem/csoundsystem.h"
#include "include/ibass.h"

inline IBASS* BASS = nullptr;
inline CSoundSystem* soundsys = new CSoundSystem();

#include "menuSZK/IAudio.h"
inline IAudio* audioChangePage;
inline IAudio* audioSelect;

#include "hooks.h"

#include <algorithm>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Texture.h"

inline CRGBA COLOR_BLACK = CRGBA(0, 0, 0);
inline CRGBA COLOR_WHITE = CRGBA(255, 255, 255);

inline bool g_hasFirstDrawn = false;
inline bool g_hasFirstUpdated = false;
inline int g_currentFrame = 0;

inline bool g_drawTestInfo = false;

inline bool HasScriptCommands()
{
    return scriptCommands != nullptr;
}

template <typename T> inline T* loadInterface(T** out, std::string name, bool optional = false)
{
    fileLog->Log("Looking for " + name + "...");

    void* interface = GetInterface(name.c_str());

    *out = static_cast<T*>(interface);

    if (*out) { fileLog->Log(name + "loaded"); }
    else
    {

        if (optional) { fileLog->Log(name + " was not found, but its optional"); }
        else
        {

            fileLog->Error(name + " was not found");
        }
    }

    return *out;
}

inline double distanceBetweenPoints(CVector point1, CVector point2)
{
    double dx = point1.x - point2.x;
    double dy = point1.y - point2.y;
    double dz = point1.z - point2.z;

    return sqrt(dx * dx + dy * dy + dz * dz);
}

// 01F5: $PLAYER_ACTOR = get_player_actor $PLAYER_CHAR
static DEFOPCODE(01F5, GET_PLAYER_ACTOR, iv);
inline int GET_PLAYER_ACTOR(int player)
{
    int playerActor = 0;
    scriptCommands->ScriptCommand(&scm_GET_PLAYER_ACTOR, player, &playerActor);
    return playerActor;
}

// 0256
static DEFOPCODE(0256, IS_PLAYER_PLAYING, i);
inline int IS_PLAYER_PLAYING(int player)
{
    bool result = scriptCommands->ScriptCommand(&scm_IS_PLAYER_PLAYING, player);
    return result;
}

// 04C4: store_coords_to 4@ 5@ 6@ from_actor $PLAYER_ACTOR with_offset 1.0 2.0
// 0.0
static DEFOPCODE(04C4, STORE_COORDS_FROM_ACTOR_WITH_OFFSET, ifffvvv);
inline void STORE_COORDS_FROM_ACTOR_WITH_OFFSET(int _char, float offsetX, float offsetY, float offsetZ, float* x, float* y, float* z)
{
    scriptCommands->ScriptCommand(&scm_STORE_COORDS_FROM_ACTOR_WITH_OFFSET, _char, offsetX, offsetY, offsetZ, x, y, z);
}

// 0811: $47 = actor $PLAYER_ACTOR used_car
static DEFOPCODE(0811, ACTOR_USED_CAR, iv);
inline int ACTOR_USED_CAR(int _char)
{
    int car = 0;
    scriptCommands->ScriptCommand(&scm_ACTOR_USED_CAR, _char, &car);
    return car;
}

// 0441: 7@ = car $47 model
static DEFOPCODE(0441, GET_CAR_MODEL, iv);
inline int GET_CAR_MODEL(int car)
{
    int modelId = 0;
    scriptCommands->ScriptCommand(&scm_GET_CAR_MODEL, car, &modelId);
    return modelId;
}

inline bool file_exists(const std::string& path)
{
    std::ifstream f(path.c_str());
    return f.good();
}

inline bool delete_file(const std::string& path)
{
    if (std::remove(path.c_str()) == 0)
    {
        return true; // arquivo apagado com sucesso
    }
    else
    {
        return false; // falhou (arquivo não existe ou permissão)
    }
}

inline void rewrite_random_bytes(const std::string& path, size_t count)
{
    // cria buffer de bytes aleatórios
    std::vector<uint8_t> buf(count);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 255);
    for (size_t i = 0; i < count; ++i) buf[i] = static_cast<uint8_t>(dist(gen));

    // abre arquivo via open para poder usar fsync
    int fd = open(path.c_str(), O_RDWR);
    if (fd < 0) return;

    ssize_t written = write(fd, buf.data(), buf.size());
    if (written != (ssize_t)buf.size())
    {
        // erro ao escrever
        close(fd);
        return;
    }

    fsync(fd); // força commit no disco
    close(fd);
}

inline size_t get_file_size(const std::string& path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return 0;
    return static_cast<size_t>(file.tellg());
}