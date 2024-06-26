#include "customtypes/playlistdetail.hpp"

#include "assets.hpp"
#include "bsml/shared/BSML.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"
#include "bsml/shared/Helpers/creation.hpp"
#include "main.hpp"
#include "manager.hpp"
#include "songcore/shared/SongCore.hpp"

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
    UpdateDownloadButtons();
}

void PlaylistDetail::SetupBSMLFields() {
    songData = ListW<BSML::CustomCellInfo*>::New();
}

void PlaylistDetail::PostParse() {
    list->tableView->selectionType = HMUI::TableViewSelectionType::None;
    downloadProgress = BSML::Lite::CreateProgressBar({-1.4, 3.1, 4}, "Downloading Songs...", "", "PlaylistDownloader");
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
    if (!playlist || !list || !noResultsText)
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

        UpdateDownloadButtons();

        songData->Clear();
    }

    int currentPos = songData->Count;

    auto songs = Manager::GetSongs();

    if (!full) {
        bool noSongs = songs.empty();
        list->gameObject->active = !noSongs;
        noResultsText->active = noSongs;
        if (noSongs)
            return;
    }

    for (int i = currentPos; i < songs.size(); i++)
        songData->Add(BSML::CustomCellInfo::construct(songs[i]->GetName(), songs[i]->GetUploader().GetUsername()));

    auto pos = list->tableView->contentTransform->anchoredPosition.y;
    list->tableView->ReloadData();
    list->tableView->scrollView->ScrollTo(std::min(pos, list->tableView->cellSize * list->NumberOfCells()), false);

    for (int i = currentPos; i < songs.size(); i++) {
        Manager::GetSongCover(songs[i], [this, i, playlist](UnityEngine::Sprite* cover) {
            if (playlist != Manager::GetSelectedPlaylist() || this != PlaylistDetail::instance)
                return;

            logger.debug("got song cover {} {}", songData[i]->text, cover != nullptr);

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

void PlaylistDetail::UpdateDownloadButtons() {
    if (!download || !downloadSongs || !update || !updateSongs)
        return;

    bool owned = Manager::SelectedPlaylistOwned();
    download->gameObject->active = !owned;
    downloadSongs->gameObject->active = !owned;
    update->gameObject->active = owned;
    updateSongs->gameObject->active = owned;
    // set uninteractable during download
    bool downloading = downloadProgress->gameObject->active;
    download->interactable = !downloading;
    downloadSongs->interactable = !downloading;
    update->interactable = !downloading;
    updateSongs->interactable = !downloading;
    // explain why uninteractable
    auto hover = downloading ? "Waiting for download..." : "";
    download->GetComponent<HMUI::HoverHint*>()->text = hover;
    downloadSongs->GetComponent<HMUI::HoverHint*>()->text = hover;
    update->GetComponent<HMUI::HoverHint*>()->text = hover;
    updateSongs->GetComponent<HMUI::HoverHint*>()->text = hover;
}

void PlaylistDetail::SetLoading(bool value) {
    if (!list || !loadingIndicator || !noResultsText)
        return;
    if (value)
        noResultsText->active = false;
    list->gameObject->active = !value;
    loadingIndicator->active = value;
}

void PlaylistDetail::SetDownloading(bool value) {
    if (!download || !downloadSongs || !downloadProgress)
        return;
    downloadProgress->gameObject->active = value;
    if (value) {
        downloadProgress->subText1->text = "Waiting for file...";
        downloadProgress->SetProgress(0);
    }
    UpdateDownloadButtons();
}

void PlaylistDetail::downloadClicked() {
    auto playlist = Manager::GetSelectedPlaylist();
    if (!playlist)
        return;

    SetDownloading(true);

    Manager::GetPlaylistFile(playlist, [this](std::optional<PlaylistCore::BPList> file) {
        if (file)
            PlaylistCore::AddPlaylist(*file);
        SetDownloading(false);
    });
}

void PlaylistDetail::downloadSongsClicked() {
    auto playlist = Manager::GetSelectedPlaylist();
    if (!playlist)
        return;

    SetDownloading(true);

    Manager::GetPlaylistFile(playlist, [this](std::optional<PlaylistCore::BPList> file) {
        if (this != PlaylistDetail::instance)
            return;
        if (!file) {
            SetDownloading(false);
            return;
        }
        auto [_, playlist] = PlaylistCore::AddPlaylist(*file);

        DownloadMissingSongs(playlist);
    });
}

void PlaylistDetail::updateClicked() {
    auto playlist = Manager::GetSelectedPlaylist();
    auto target = Manager::SelectedPlaylistOwned();
    if (!playlist || !target)
        return;

    SetDownloading(true);

    Manager::GetPlaylistFile(playlist, [this, target](std::optional<PlaylistCore::BPList> file) {
        if (file) {
            target->playlistJSON = *file;
            target->Save();
            PlaylistCore::MarkPlaylistForReload(target);
            PlaylistCore::ReloadPlaylists();
        }
        SetDownloading(false);
    });
}

void PlaylistDetail::updateSongsClicked() {
    auto playlist = Manager::GetSelectedPlaylist();
    auto target = Manager::SelectedPlaylistOwned();
    if (!playlist || !target)
        return;

    SetDownloading(true);

    Manager::GetPlaylistFile(playlist, [this, target](std::optional<PlaylistCore::BPList> file) {
        if (this != PlaylistDetail::instance)
            return;
        if (!file) {
            SetDownloading(false);
            return;
        }
        target->playlistJSON = *file;
        target->Save();
        PlaylistCore::MarkPlaylistForReload(target);
        PlaylistCore::ReloadPlaylists();

        DownloadMissingSongs(target);
    });
}

void PlaylistDetail::DownloadMissingSongs(PlaylistCore::Playlist* playlist) {
    if (downloadProgress) {
        downloadProgress->subText1->text = "0 / 0";
        downloadProgress->SetProgress(0);
    }

    PlaylistCore::DownloadMissingSongsFromPlaylist(
        playlist,
        [this]() {
            BSML::MainThreadScheduler::Schedule([this]() {
                if (this != PlaylistDetail::instance)
                    return;
                SongCore::API::Loading::RefreshSongs(false);
                SetDownloading(false);
            });
        },
        [this](int total, int progress) {
            BSML::MainThreadScheduler::Schedule([this, progress, total]() {
                if (this != PlaylistDetail::instance)
                    return;
                downloadProgress->subText1->text = fmt::format("{} / {}", progress, total);
                downloadProgress->SetProgress(progress / (float) total);
            });
        });
}
