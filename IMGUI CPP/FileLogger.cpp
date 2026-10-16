#include "FileLogger.h"

#include <mod/amlmod.h>

#include <fstream>
#include <iostream>
#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>

FileLogger::FileLogger(const std::string& modName)
{
    std::string dataRoot = aml->GetAndroidDataRootPath();
    logFolder = dataRoot + "/mods/DexTrainer/logs/";

    // cria pasta de logs se não existir
    mkdir(logFolder.c_str(), 0777);

    logFile = logFolder + modName + ".log";
}

void FileLogger::Log(const std::string& message) {
    Write("INFO", message);
}

void FileLogger::Warn(const std::string& message) {
    Write("WARN", message);
}

void FileLogger::Error(const std::string& message) {
    Write("ERROR", message);
}

void FileLogger::Debug(const std::string& message) {

    if(debugEnabled)
    {
        Write("DEBUG", message);
    }
}

void FileLogger::Clear()
{
    std::lock_guard<std::mutex> lock(mtx);

    // Abre o arquivo em modo trunc, que apaga o conteúdo
    std::ofstream ofs(logFile, std::ios::trunc);
    ofs.close();
}

void FileLogger::Write(const std::string& level, const std::string& message)
{
    std::lock_guard<std::mutex> lock(mtx);

    std::ofstream ofs(logFile, std::ios::app);
    if (!ofs.is_open()) return;

    // hora atual
    std::time_t t = std::time(nullptr);
    char timeStr[64];
    std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", std::localtime(&t));

    ofs << "[" << timeStr << "] [" << level << "] " << message << "\n";
    ofs.close();
}