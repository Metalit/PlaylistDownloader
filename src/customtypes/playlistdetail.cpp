#include "customtypes/playlistdetail.hpp"

#include "assets.hpp"
#include "bsml/shared/BSML.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"
#include "bsml/shared/Helpers/creation.hpp"
#include "main.hpp"
#include "manager.hpp"
#include "playlistcore/shared/PlaylistCore.hpp"
#include "songcore/shared/SongCore.hpp"
#include "songdownloader/shared/BeatSaverAPI.hpp"

DEFINE_TYPE(PlaylistDownloader, PlaylistDetail);

using namespace PlaylistDownloader;

void PlaylistDetail::OnEnable() {
    set_name("PlaylistDetail");
    rectTransform->anchorMin = {0.5, 0.5};
    rectTransform->anchorMax = {0.5, 0.5};
    rectTransform->sizeDelta = {140, 65};
}

void PlaylistDetail::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (firstActivation) {
        SetupBSMLFields();
        // AddHotReload(this, "playlistdetail");
        BSML::parse_and_construct(IncludedAssets::playlistdetail_bsml, transform, this);
    }
    UpdateScrollView();
}

void PlaylistDetail::SetupBSMLFields() {
    songData = ListW<BSML::CustomCellInfo*>::New();
}

void PlaylistDetail::PostParse() {
    list->tableView->selectionType = HMUI::TableViewSelectionType::None;
    downloadProgress = BSML::Lite::CreateProgressBar({-1.4, 3.1, 4}, "Downloading Songs...", "0 / 0", "PlaylistDownloader");
    downloadProgress->gameObject->active = false;

    auto delegate = custom_types::MakeDelegate<System::Action_1<float>*>((std::function<void(float)>) [this](float position) {
        auto scrollView = list->tableView->scrollView;
        float pageSize = scrollView->scrollPageSize;
        float remaminingScroll = scrollView->contentSize - pageSize - position;
        if (remaminingScroll < pageSize)
            Manager::RequestMoreSongs();
    });
    list->tableView->scrollView->add_scrollPositionChangedEvent(delegate);
}

void PlaylistDetail::OnDestroy() {
    Manager::Invalidate();
    instance = nullptr;
}

PlaylistDetail* PlaylistDetail::GetInstance() {
    if (!instance)
        instance = BSML::Helpers::CreateViewController<PlaylistDetail*>();
    return instance;
}

void PlaylistDetail::Refresh(bool full) {
    auto playlist = Manager::GetSelectedPlaylist();
    if (!playlist || !list)
        return;

    logger.debug("Refreshing playlist detail");

    if (full) {
        Manager::GetPlaylistCover(playlist, [this, playlist](UnityEngine::Sprite* sprite) {
            if (playlist != Manager::GetSelectedPlaylist() || this != PlaylistDetail::instance)
                return;

            cover->sprite = sprite;
        });

        name->text = playlist->Title();
        author->text = std::string("<line-height=75%>") + playlist->Author();
        description->SetText(playlist->Description());
        description->ScrollTo(0, false);

        bool owned = Manager::SelectedPlaylistOwned();
        download->interactable = !owned;
        downloadSongs->interactable = !owned;

        songData->Clear();
    }

    int currentPos = songData->Count;

    auto songs = Manager::GetSongs();

    for (int i = currentPos; i < songs.size(); i++)
        songData->Add(BSML::CustomCellInfo::construct(songs[i]->GetName(), songs[i]->GetUploader().GetUsername()));

    auto pos = list->tableView->contentTransform->anchoredPosition.y;
    list->tableView->ReloadData();
    list->tableView->scrollView->ScrollTo(std::min(pos, list->tableView->cellSize * list->NumberOfCells()), false);

    for (int i = currentPos; i < songs.size(); i++) {
        Manager::GetSongCover(songs[i], [this, i, playlist](UnityEngine::Sprite* cover) {
            if (playlist != Manager::GetSelectedPlaylist() || this != PlaylistDetail::instance)
                return;

            songData[i]->icon = cover;
            list->tableView->RefreshCellsContent();
        });
    }
}

void PlaylistDetail::UpdateScrollView() {
    if (!list)
        return;

    auto table = list->tableView;
    auto pos = table->contentTransform->anchoredPosition.y;
    table->RefreshContentSize();
    table->scrollView->ScrollTo(std::min(pos, table->cellSize * table->numberOfCells), false);
}

void PlaylistDetail::SetLoading(bool value) {
    if (!list || !loadingIndicator)
        return;
    list->gameObject->active = !value;
    loadingIndicator->active = value;
}

void PlaylistDetail::downloadClicked() {
    auto playlist = Manager::GetSelectedPlaylist();
    if (!playlist)
        return;

    download->interactable = false;
    downloadSongs->interactable = false;

    Manager::GetPlaylistFile(playlist, [](PlaylistCore::BPList file) { PlaylistCore::AddPlaylist(file); });
}

void PlaylistDetail::downloadSongsClicked() {
    auto playlist = Manager::GetSelectedPlaylist();
    if (!playlist)
        return;

    download->interactable = false;
    downloadSongs->interactable = false;

    Manager::GetPlaylistFile(playlist, [this](PlaylistCore::BPList file) {
        if (this != PlaylistDetail::instance)
            return;
        auto [_, playlist] = PlaylistCore::AddPlaylist(file);

        downloadProgress->subText1->text = "0 / 0";
        downloadProgress->SetProgress(0);

        BeatSaver::API::DownloadMissingSongsFromPlaylist(
            playlist,
            [this]() {
                BSML::MainThreadScheduler::Schedule([this]() {
                    if (this != PlaylistDetail::instance)
                        return;
                    SongCore::API::Loading::RefreshSongs(false);
                    downloadProgress->gameObject->active = false;
                });
            },
            [this](int progress, int total) {
                BSML::MainThreadScheduler::Schedule([this, progress, total]() {
                    if (this != PlaylistDetail::instance)
                        return;
                    downloadProgress->subText1->text = fmt::format("{} / {}", progress, total);
                    downloadProgress->SetProgress(progress / (float) total);
                });
            });
    });
}
