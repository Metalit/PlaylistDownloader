#include "main.hpp"
#include "manager.hpp"
#include "customtypes/playlistdetail.hpp"

#include "questui/shared/BeatSaberUI.hpp"

DEFINE_TYPE(PlaylistDownloader, PlaylistDetail);

using namespace PlaylistDownloader;
using namespace QuestUI;

void PlaylistDetail::OnEnable() {
    set_name("PlaylistDetail");
    get_rectTransform()->set_anchorMin({0.5, 0.5});
    get_rectTransform()->set_anchorMax({0.5, 0.5});
    get_rectTransform()->set_sizeDelta({140, 65});
}

void PlaylistDetail::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (firstActivation) {
        SetupBSMLFields();
        AddHotReload(this, "playlistdetail");
        // BSML::parse_and_construct(IncludedAssets::playlistdetail_bsml, get_transform(), this);
    }
    UpdateScrollView();
}

void PlaylistDetail::SetupBSMLFields() {
    songData = List<BSML::CustomCellInfo*>::New_ctor();
}

void PlaylistDetail::PostParse() {
    list->tableView->set_selectionType(HMUI::TableViewSelectionType::None);
    Refresh();
}

void PlaylistDetail::dtor() {
    instance = nullptr;
}

PlaylistDetail* PlaylistDetail::GetInstance() {
    if (!instance)
        instance = BeatSaberUI::CreateViewController<PlaylistDetail*>();
    return instance;
}

void PlaylistDetail::Refresh() {
    auto playlist = Manager::GetSelectedPlaylist();
    if (!playlist || !list)
        return;

    getLogger().debug("Refreshing playlist detail");

    Manager::GetPlaylistCover(playlist, [this, playlist](UnityEngine::Sprite* sprite) {
        if (playlist != Manager::GetSelectedPlaylist())
            return;

        cover->set_sprite(sprite);
    });
    name->SetText(playlist->Title());
    author->SetText(std::string("<line-height=75%>") + playlist->Author());
    description->SetText(playlist->Description());
    description->ScrollTo(0, false);

    songData->Clear();
    SetLoading(true);

    Manager::GetPlaylistSongs(playlist, [this, playlist](std::vector<std::optional<BeatSaver::Beatmap>> songs) {
        if (playlist != Manager::GetSelectedPlaylist())
            return;

        for (auto& song : songs) {
            if (!song.has_value())
                continue;
            songData->Add(BSML::CustomCellInfo::construct(song->GetName(), song->GetUploader().GetUsername()));
            Manager::GetSongCover(&*song, [this, idx = songData->get_Count() - 1, playlist](UnityEngine::Sprite* cover) {
                if (playlist != Manager::GetSelectedPlaylist())
                    return;

                songData->get_Item(idx)->icon = cover;
                list->tableView->RefreshCellsContent();
            });
        }

        SetLoading(false);
        list->tableView->ReloadData();
    });
}

void PlaylistDetail::UpdateScrollView() {
    if (!list)
        return;

    auto table = list->tableView;
    auto pos = table->contentTransform->get_anchoredPosition().y;
    table->RefreshContentSize();
    table->scrollView->ScrollTo(std::min(pos, table->cellSize * table->numberOfCells), false);
}

void PlaylistDetail::SetLoading(bool value) {
    if (!list || !loadingIndicator)
        return;
    list->get_gameObject()->SetActive(!value);
    loadingIndicator->SetActive(value);
}
