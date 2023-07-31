#pragma once

#include "modloader/shared/modloader.hpp"

#include "beatsaber-hook/shared/utils/hooking.hpp"

Logger& getLogger();

#include "bsml/shared/BSML/Components/HotReloadFileWatcher.hpp"
#include "HMUI/ViewController.hpp"
#include "UnityEngine/GameObject.hpp"

inline void AddHotReload(HMUI::ViewController* host, const char* path) {
    auto watcher = host->get_gameObject()->AddComponent<BSML::HotReloadFileWatcher*>();
    watcher->host = host;
    watcher->filePath = string_format("/sdcard/%s.bsml", path);
    watcher->checkInterval = 5;
}
