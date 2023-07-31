#include "main.hpp"
#include "customtypes/playlistlist.hpp"

#include "questui/shared/BeatSaberUI.hpp"

DEFINE_TYPE(PlaylistDownloader, PlaylistList);

using namespace PlaylistDownloader;
using namespace QuestUI;

void PlaylistList::OnEnable() {
    set_name("PlaylistList");
    get_rectTransform()->set_anchorMin({0.5, 0.5});
    get_rectTransform()->set_anchorMax({0.5, 0.5});
    get_rectTransform()->set_sizeDelta({75, 65});
}

void PlaylistList::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (firstActivation) {
        SetupBSMLFields();
        AddHotReload(this, "playlistlist");
        // BSML::parse_and_construct(IncludedAssets::playlistlist_bsml, get_transform(), this);
    }
}

void PlaylistList::SetupBSMLFields() {
    playlistData = List<BSML::CustomCellInfo*>::New_ctor(3);
    playlistData->Add(BSML::CustomCellInfo::construct("Playlist1", "author"));
    playlistData->Add(BSML::CustomCellInfo::construct("Playlist2", "a different guy"));
    playlistData->Add(BSML::CustomCellInfo::construct("Playlist3", "author"));
}

void PlaylistList::dtor() {
    instance = nullptr;
}

PlaylistList* PlaylistList::GetInstance() {
    if (!instance)
        instance = BeatSaberUI::CreateViewController<PlaylistList*>();
    return instance;
}

void PlaylistList::cellSelected(HMUI::TableView* table, int cellIdx) {
    getLogger().info("Playlist %i selected", cellIdx);
}
