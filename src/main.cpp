#include "main.hpp"
#include "config.hpp"
#include "customtypes/mainmenu.hpp"

// #include "TMPro/TMP_Settings.hpp"
// #include "TMPro/TMP_FontAsset.hpp"
// #include "System/Collections/Generic/Dictionary_2.hpp"

// bool HasChar(const char c) {
//     auto asset = TMPro::TMP_Settings::get_defaultFontAsset();
//     TMPro::TMP_Character* value;
//     asset->get_characterLookupTable()->TryGetValue(c, byref(value));
//     return value != nullptr;
// }

#include "questui/shared/QuestUI.hpp"

static ModInfo modInfo;

Logger& getLogger() {
    static Logger* logger = new Logger(modInfo, {false, true});
    return *logger;
}

extern "C" void setup(ModInfo& info) {
    info.id = MOD_ID;
    info.version = VERSION;
    modInfo = info;

    getConfig().Init(info);

    getLogger().info("Completed setup!");
}

extern "C" void load() {
    il2cpp_functions::Init();

    QuestUI::Init();
    QuestUI::Register::RegisterMainMenuModSettingsViewController<PlaylistDownloader::MainMenu*>(modInfo, "More Playlists");
}
