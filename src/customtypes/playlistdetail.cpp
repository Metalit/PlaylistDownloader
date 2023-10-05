#include "main.hpp"
#include "manager.hpp"
#include "assets.hpp"
#include "customtypes/playlistdetail.hpp"

#include "bsml/shared/BSML.hpp"
#include "questui/shared/BeatSaberUI.hpp"

#include "playlistcore/shared/PlaylistCore.hpp"
#include "songloader/shared/API.hpp"

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
        // AddHotReload(this, "playlistdetail");
        BSML::parse_and_construct(IncludedAssets::playlistdetail_bsml, get_transform(), this);
    }
    UpdateScrollView();
}

void PlaylistDetail::SetupBSMLFields() {
    songData = List<BSML::CustomCellInfo*>::New_ctor();
}

void PlaylistDetail::PostParse() {
    list->tableView->set_selectionType(HMUI::TableViewSelectionType::None);
    downloadProgress = QuestUI::BeatSaberUI::CreateProgressBar({-1.4, 3.1, 4}, "Downloading Songs...", "0 / 0", "PlaylistDownloader");
    downloadProgress->get_gameObject()->SetActive(false);

    auto delegate = custom_types::MakeDelegate<System::Action_1<float>*>((std::function<void(float)>) [this](float position) {
        auto scrollView = list->tableView->scrollView;
        float pageSize = scrollView->get_scrollPageSize();
        float remaminingScroll = scrollView->get_contentSize() - pageSize - position;
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
        instance = BeatSaberUI::CreateViewController<PlaylistDetail*>();
    return instance;
}

void PlaylistDetail::Refresh(bool full) {
    auto playlist = Manager::GetSelectedPlaylist();
    if (!playlist || !list)
        return;

    getLogger().debug("Refreshing playlist detail");

    if (full) {
        Manager::GetPlaylistCover(playlist, [this, playlist](UnityEngine::Sprite* sprite) {
            if (playlist != Manager::GetSelectedPlaylist() || this != PlaylistDetail::instance)
                return;

            cover->set_sprite(sprite);
        });

        name->set_text(playlist->Title());
        author->set_text(std::string("<line-height=75%>") + playlist->Author());
        description->SetText(playlist->Description());
        description->ScrollTo(0, false);

        bool owned = Manager::SelectedPlaylistOwned();
        download->set_interactable(!owned);
        downloadSongs->set_interactable(!owned);

        songData->Clear();
    }

    int currentPos = songData->get_Count();

    auto songs = Manager::GetSongs();

    for (int i = currentPos; i < songs.size(); i++)
        songData->Add(BSML::CustomCellInfo::construct(songs[i]->GetName(), songs[i]->GetUploader().GetUsername()));

    auto pos = list->tableView->contentTransform->get_anchoredPosition().y;
    list->tableView->ReloadData();
    list->tableView->scrollView->ScrollTo(std::min(pos, list->tableView->cellSize * list->NumberOfCells()), false);

    for (int i = currentPos; i < songs.size(); i++) {
        Manager::GetSongCover(songs[i], [this, i, playlist](UnityEngine::Sprite* cover) {
            if (playlist != Manager::GetSelectedPlaylist() || this != PlaylistDetail::instance)
                return;

            songData->get_Item(i)->icon = cover;
            list->tableView->RefreshCellsContent();
        });
    }
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

void PlaylistDetail::downloadClicked() {
    auto playlist = Manager::GetSelectedPlaylist();
    if (!playlist)
        return;

    download->set_interactable(false);
    downloadSongs->set_interactable(false);

    Manager::GetPlaylistFile(playlist, [](PlaylistCore::BPList file) {
        PlaylistCore::AddPlaylist(file);
    });
}

void PlaylistDetail::downloadSongsClicked() {
    auto playlist = Manager::GetSelectedPlaylist();
    if (!playlist)
        return;

    download->set_interactable(false);
    downloadSongs->set_interactable(false);

    Manager::GetPlaylistFile(playlist, [this](PlaylistCore::BPList file) {
        if (this != PlaylistDetail::instance)
            return;
        auto [_, playlist] = PlaylistCore::AddPlaylist(file);

        downloadProgress->subText1->SetText("0 / 0");
        downloadProgress->SetProgress(0);

        PlaylistCore::DownloadMissingSongsFromPlaylist(playlist, [this]() {
            if (this != PlaylistDetail::instance)
                return;
            RuntimeSongLoader::API::RefreshSongs(false);
            downloadProgress->get_gameObject()->SetActive(false);
        },
        [this](int progress, int total) {
            if (this != PlaylistDetail::instance)
                return;
            downloadProgress->subText1->SetText(std::to_string(progress) + " / " + std::to_string(total));
            downloadProgress->SetProgress(progress / (float) total);
        });
    });
}
