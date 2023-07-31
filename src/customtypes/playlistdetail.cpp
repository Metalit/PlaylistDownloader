#include "main.hpp"
#include "customtypes/playlistdetail.hpp"

#include "questui/shared/BeatSaberUI.hpp"

DEFINE_TYPE(PlaylistDownloader, PlaylistDetail);

using namespace PlaylistDownloader;
using namespace QuestUI;

void PlaylistDetail::OnEnable() {
    set_name("PlaylistDetail");
    get_rectTransform()->set_anchorMin({0.5, 0.5});
    get_rectTransform()->set_anchorMax({0.5, 0.5});
    get_rectTransform()->set_sizeDelta({120, 65});
}

void PlaylistDetail::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (firstActivation) {
        SetupBSMLFields();
        AddHotReload(this, "playlistdetail");
        // BSML::parse_and_construct(IncludedAssets::playlistdetail_bsml, get_transform(), this);
    }
}

void PlaylistDetail::SetupBSMLFields() {
    songData = List<BSML::CustomCellInfo*>::New_ctor(3);
    songData->Add(BSML::CustomCellInfo::construct("Song1", "author"));
    songData->Add(BSML::CustomCellInfo::construct("Song2", "a different guy"));
    songData->Add(BSML::CustomCellInfo::construct("Song3", "a different guy"));
}

void PlaylistDetail::PostParse() {
    list->tableView->set_selectionType(HMUI::TableViewSelectionType::None);
    name->SetText("My Playlist");
    author->SetText("Metalit");
    description->SetText("This is a great playlist with an even greater description. With a catalogue of wonderful maps ranging from easy to midspeed to insane tech, all sorted in an intuitive and friendly way, any user will have loads of fun with just the songs in here. And if you like your songs even more finely categorized, consider downloading my 10 other playlists, each centered around a specific theme! There's truly something for everyone. Follow me on twitter, subscribe to my youtube channel, check out my tiktok, send your job offers to my linkedin, and please donate to my patreon");
}

void PlaylistDetail::dtor() {
    instance = nullptr;
}

PlaylistDetail* PlaylistDetail::GetInstance() {
    if (!instance)
        instance = BeatSaberUI::CreateViewController<PlaylistDetail*>();
    return instance;
}
