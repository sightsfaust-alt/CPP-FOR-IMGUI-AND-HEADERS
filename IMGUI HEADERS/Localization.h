#pragma once

#include "pch.h"

class Localization {
public:
    // language -> (key -> value)
    static std::unordered_map<std::string, std::unordered_map<std::string, std::string>> lines;

    // idioma atual
    static std::string currentLanguage;

    static void RegisterLine(const std::string& language, const std::string& key, const std::string& value);
    static std::string Get(const std::string& key);
    static void SetLanguage(const std::string& language);
    static bool HasKey(const std::string& key);
    static std::vector<std::string> GetLanguages();
};