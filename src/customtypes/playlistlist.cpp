#include "customtypes/playlistlist.hpp"

#include "HMUI/ScrollView.hpp"
#include "System/Action_1.hpp"
#include "assets.hpp"
#include "bsml/shared/BSML.hpp"
#include "bsml/shared/Helpers/creation.hpp"
#include "custom-types/shared/delegate.hpp"
#include "customtypes/mainmenu.hpp"
#include "main.hpp"
#include "manager.hpp"

DEFINE_TYPE(PlaylistDownloader, PlaylistList);

using namespace PlaylistDownloader;

void PlaylistList::OnEnable() {
    name = "PlaylistList";
    rectTransform->anchorMin = {0.5, 0.5};
    rectTransform->anchorMax = {0.5, 0.5};
    rectTransform->sizeDelta = {75, 65};
}

void PlaylistList::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (firstActivation) {
        SetupBSMLFields();
        // AddHotReload(this, "playlistlist");
        BSML::parse_and_construct(IncludedAssets::playlistlist_bsml, transform, this);
    }
    UpdateScrollView();
}

void PlaylistList::SetupBSMLFields() {
    playlistData = ListW<BSML::CustomCellInfo*>::New();
}

void PlaylistList::PostParse() {
    auto delegate = custom_types::MakeDelegate<System::Action_1<float>*>((std::function<void(float)>) [this](float position) {
        auto scrollView = list->tableView->scrollView;
        float pageSize = scrollView->scrollPageSize;
        float remaminingScroll = scrollView->contentSize - pageSize - position;
        if (remaminingScroll < pageSize)
            Manager::RequestMorePlaylists();
    });
    list->tableView->scrollView->add_scrollPositionChangedEvent(delegate);
}

void PlaylistList::OnDestroy() {
    Manager::Invalidate();
    instance = nullptr;
}

PlaylistList* PlaylistList::GetInstance() {
    if (!instance)
        instance = BSML::Helpers::CreateViewController<PlaylistList*>();
    return instance;
}

void PlaylistList::Refresh(bool full) {
    if (!list || !playlistData || !noResultsText)
        return;
    logger.debug("Refreshing playlist list");
    if (full)
        playlistData->Clear();

    int currentPos = playlistData->Count;

    auto playlists = Manager::GetPlaylists();
    auto state = Manager::GetState();

    bool noResults = playlists.empty();
    list->gameObject->active = !noResults;
    noResultsText->active = noResults;
    if (noResults)
        return;

    for (int i = currentPos; i < playlists.size(); i++)
        playlistData->Add(BSML::CustomCellInfo::construct(playlists[i]->Title(), playlists[i]->Author()));

    auto pos = list->tableView->contentTransform->anchoredPosition.y;
    list->tableView->ReloadData();
    list->tableView->scrollView->ScrollTo(std::min(pos, list->tableView->cellSize * list->NumberOfCells()), false);

    for (int i = currentPos; i < playlists.size(); i++) {
        Manager::GetPlaylistCover(playlists[i], [this, i, state](UnityEngine::Sprite* cover) {
            if (state != Manager::GetState() || this != PlaylistList::instance || i >= playlistData->Count)
                return;

            playlistData[i]->icon = cover;
            list->tableView->RefreshCellsContent();
        });
    }
}

void PlaylistList::UpdateScrollView() {
    if (!list)
        return;

    auto table = list->tableView;
    auto pos = table->contentTransform->anchoredPosition.y;
    table->RefreshContentSize();
    table->scrollView->ScrollTo(std::min(pos, table->cellSize * table->numberOfCells), false);
}

void PlaylistList::SetLoading(bool value) {
    if (!list || !loadingIndicator || !noResultsText)
        return;
    if (value) {
        list->tableView->ClearSelection();
        list->tableView->scrollView->ScrollTo(0, false);
        noResultsText->active = false;
    }
    list->gameObject->active = !value;
    loadingIndicator->active = value;
}

void PlaylistList::cellSelected(HMUI::TableView* table, int cellIdx) {
    logger.info("Playlist {} selected", cellIdx);
    Manager::SelectPlaylist(cellIdx);
}
