#include "customtypes/playlistdetail.hpp"

#include "assets.hpp"
#include "bsml/shared/BSML-Lite/Creation/Buttons.hpp"
#include "bsml/shared/BSML.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"
#include "bsml/shared/Helpers/creation.hpp"
#include "main.hpp"
#include "manager.hpp"
#include "playlistcore/shared/PlaylistCore.hpp"
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
        BSML_FILE(playlistdetail);
    }
    UpdateScrollView();
    UpdateDownloadButtons();
}

void PlaylistDetail::SetupBSMLFields() {
    songData = ListW<BSML::CustomCellInfo*>::New();
}

void PlaylistDetail::PostParse() {
    list->tableView->selectionType = HMUI::TableViewSelectionType::None;
    downloadProgress = BSML::Lite::CreateProgressBar({-1.4, 3.1, 4}, "Downloading Playlist...", "", "");
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

        name->text = playlist->Title;
        author->text = std::string("<line-height=75%>") + playlist->Author;
        description->SetText(playlist->Description);
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

    std::string_view selectedUrl = Manager::GetSelectedPlaylist()->PlaylistURL;
    bool currentDownload = currentDownloadUrl == selectedUrl;
    if (!currentDownload) {
        for (auto& download : queue) {
            if (download.playlist.PlaylistURL == selectedUrl) {
                currentDownload = true;
                break;
            }
        }
    }

    bool owned = Manager::SelectedPlaylistOwned();
    download->gameObject->active = !owned;
    downloadSongs->gameObject->active = !owned;
    update->gameObject->active = owned;
    updateSongs->gameObject->active = owned;
    // set uninteractable during download
    download->interactable = !currentDownload;
    downloadSongs->interactable = !currentDownload;
    update->interactable = !currentDownload;
    updateSongs->interactable = !currentDownload;
    // explain why uninteractable
    auto hover = currentDownload ? "Waiting for download..." : "";
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
    if (!downloadProgress)
        return;
    downloadProgress->gameObject->active = value;
    if (value)
        downloadProgress->subText2->text = queue.empty() ? "" : fmt::format("{} in Queue", queue.size());
    UpdateDownloadButtons();
}

void PlaylistDetail::downloadClicked() {
    AddDownload(false, false);
}

void PlaylistDetail::downloadSongsClicked() {
    AddDownload(false, true);
}

void PlaylistDetail::updateClicked() {
    AddDownload(true, false);
}

void PlaylistDetail::updateSongsClicked() {
    AddDownload(true, true);
}

void PlaylistDetail::AddDownload(bool update, bool songs) {
    if (update && !Manager::SelectedPlaylistOwned())
        return;
    auto playlist = Manager::GetSelectedPlaylist();
    if (!playlist)
        return;

    queue.emplace_back(DownloadInfo{.playlist = *playlist, .update = update, .songs = songs});
    if (!downloadProgress || !downloadProgress->gameObject->active)
        PopDownload();

    SetDownloading(true);
}

void PlaylistDetail::PopDownload() {
    auto download = queue[0];
    queue.erase(queue.begin());

    if (downloadProgress) {
        downloadProgress->subText1->text = "Waiting for file...";
        downloadProgress->subText2->text = queue.empty() ? "" : fmt::format("{} in queue", queue.size());
        downloadProgress->SetProgress(0);
    }

    Manager::GetPlaylistFile(&download.playlist, [this, download](std::optional<PlaylistCore::BPList> file) {
        if (this != PlaylistDetail::instance)
            return;
        if (!file) {
            if (!queue.empty())
                PopDownload();
            else
                SetDownloading(false);
            return;
        }
        PlaylistCore::Playlist* target;
        if (!download.update) {
            auto pair = PlaylistCore::AddPlaylist(*file);
            target = pair.second;
        } else {
            target = Manager::PlaylistOwned(&download.playlist);
            // should be owned since we checked before starting, but maybe they deleted it with pm
            if (!target)
                return;
            target->playlistJSON = *file;
            target->Save();
            PlaylistCore::MarkPlaylistForReload(target);
            PlaylistCore::ReloadPlaylists();
        }
        if (download.songs)
            DownloadMissingSongs(target);
        else if (!queue.empty())
            PopDownload();
        else
            SetDownloading(false);
    });
}

void PlaylistDetail::DownloadMissingSongs(PlaylistCore::Playlist* playlist) {
    PlaylistCore::DownloadMissingSongsFromPlaylist(
        playlist,
        [this]() {
            BSML::MainThreadScheduler::Schedule([this]() {
                if (this != PlaylistDetail::instance)
                    return;
                SongCore::API::Loading::RefreshSongs(false);
                if (!queue.empty())
                    PopDownload();
                else
                    SetDownloading(false);
            });
        },
        [this](int total, int progress) {
            BSML::MainThreadScheduler::Schedule([this, progress, total]() {
                if (this != PlaylistDetail::instance || !downloadProgress)
                    return;
                downloadProgress->subText1->text = fmt::format("{} / {} songs", progress, total);
                downloadProgress->SetProgress(progress / (float) total);
            });
        }
    );
}
