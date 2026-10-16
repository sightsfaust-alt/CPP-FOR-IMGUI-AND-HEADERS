#include "Localization.h"

#include "Debug.h"

std::unordered_map<std::string, std::unordered_map<std::string, std::string>> Localization::lines;
std::string Localization::currentLanguage = "en"; // padrão

void Localization::RegisterLine(const std::string& language, const std::string& key, const std::string& value)
{
    auto& langMap = lines[language]; // pega o mapa de chaves desse idioma

    // se já existe, apenas loga e sai
    if (langMap.find(key) != langMap.end())
    {
        return;
    }

    // se não existe, registra normalmente
    langMap[key] = value;
    //menuDebug->AddLine("[Localization] Added " + language + " - " + key);
}

void Localization::SetLanguage(const std::string& language) {
    currentLanguage = language;
}

std::string Localization::Get(const std::string& key) {
    auto langIt = lines.find(currentLanguage);
    if (langIt != lines.end()) {
        auto& map = langIt->second;
        auto keyIt = map.find(key);
        if (keyIt != map.end()) {
            return keyIt->second;
        }
    }

    // fallback: tenta inglês
    auto fallbackIt = lines.find("en");
    if (fallbackIt != lines.end()) {
        auto& map = fallbackIt->second;
        auto keyIt = map.find(key);
        if (keyIt != map.end()) {
            return keyIt->second;
        }
    }

    // last resort: hardcoded English defaults.
    // This guarantees the user NEVER sees "[MISSING:key]" even if the
    // .ini files fail to load (wrong path, parse error, etc).
    static const std::unordered_map<std::string, std::string> hardcodedEn = {
        {"test", "Test"},
        {"introduction", "Introduction"},
        {"close", "Close"},
        {"data_author", "Author of this data: CHANGE ME"},
        {"menu_author", "Menu author"},
        {"change_language", "Change language"},
        {"choose_language", "Choose a language below"},
        {"test_window", "Test window"},
        {"test_text1", "This is a test window"},
        {"test_play_audio", "Play audio"},
        {"test_checkbox", "My checkbox"},
        {"test_options", "Test options"},
        {"test_option", "Option"},
        {"debug_mode", "Debug mode"},
        {"loading_dffs", "Loading DFFs..."},
        {"vehicle_model", "Vehicle model: "},
        {"no_vehicle", "No vehicle"},
        {"page_indicator", "Page %d / %d"},
    };
    auto hardIt = hardcodedEn.find(key);
    if (hardIt != hardcodedEn.end()) {
        return hardIt->second;
    }

    return "[MISSING:" + key + "]";
}

bool Localization::HasKey(const std::string& key) {
    auto langIt = lines.find(currentLanguage);
    if (langIt == lines.end()) return false;
    return langIt->second.find(key) != langIt->second.end();
}

std::vector<std::string> Localization::GetLanguages()
{
    std::vector<std::string> result;
    result.reserve(lines.size());

    for (const auto& pair : lines)
    {
        result.push_back(pair.first); // o nome do idioma (ex: "en", "pt", "es")
    }

    return result;
}