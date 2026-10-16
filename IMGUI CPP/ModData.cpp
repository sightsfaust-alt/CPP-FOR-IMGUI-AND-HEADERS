#include "ModData.h"

#include <mod/amlmod.h>
#include <sys/stat.h>

#include "IniReaderWriter.hpp"
#include "MenuSZK.h"
#include "Localization.h"
#include "imgLoader.h"

ModData::ModData(std::string folderName)
{
    std::string dataRootPath = aml->GetAndroidDataRootPath();

    // Cria pasta principal do mod
    CreateFolder(dataRootPath + "/mods/DexTrainer/" + folderName + "/");

    // Cria pasta de logs
    CreateFolder(dataRootPath + "/mods/DexTrainer/logs/");

    this->modFolderPath = dataRootPath + "/mods/DexTrainer/" + folderName + "/";
}

std::string ModData::GetFileFromAssets(const std::string& localPath)
{
    // Retorna o caminho completo de um arquivo dentro de mods_data/assets/
    return modFolderPath + "/assets/" + localPath;
}

std::string ModData::GetFile(const std::string& localPath)
{
    // Retorna o caminho completo de um arquivo dentro da pasta do mod
    return modFolderPath + "/" + localPath;
}

void ModData::LoadSettings()
{
    auto pathSettings = GetFile("settings.ini");

    if (!file_exists(pathSettings)) return;

    IniReaderWriter ini;
    ini.LoadFromFile(pathSettings);

    menuSZK->debug->enabled = ini.GetBool("settings", "debug_mode", false);
    Localization::currentLanguage = ini.Get("settings", "language", "en");
    g_imgLoaderEnabled = ini.GetBool("settings", "modloader_enabled", false);
    g_useOldTouchSystem = ini.GetBool("settings", "use_old_touch_system", false);
    g_drawTestInfo = ini.GetBool("settings", "draw_test_info", false);

    if (file_exists(modData->GetFile("TOUCH_ERROR"))) { g_useOldTouchSystem = true; }
}

void ModData::CreateFolder(const std::string& path)
{
    // Cria pasta se não existir
    mkdir(path.c_str(), 0777);
}

void ModData::CreateFile(const std::string& path)
{
    // Cria o arquivo vazio se não existir, ou zera se existir
    int fd = open(path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd != -1)
    {
        close(fd); // fecha o arquivo
    }
    else
    {
        printf("Erro ao criar o arquivo: %s\n", path.c_str());
    }
}