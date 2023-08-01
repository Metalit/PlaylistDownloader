#include "main.hpp"
#include "manager.hpp"
#include "customtypes/playlistlist.hpp"
#include "customtypes/mainmenu.hpp"

#include "questui/shared/BeatSaberUI.hpp"

#include "HMUI/ScrollView.hpp"
#include "System/Action_1.hpp"
#include "custom-types/shared/delegate.hpp"

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
    UpdateScrollView();
}

void PlaylistList::SetupBSMLFields() {
    playlistData = List<BSML::CustomCellInfo*>::New_ctor();
}

void PlaylistList::PostParse() {
    auto delegate = custom_types::MakeDelegate<System::Action_1<float>*>((std::function<void(float)>) [this](float position) {
        auto scrollView = list->tableView->scrollView;
        float pageSize = scrollView->get_scrollPageSize();
        float remaminingScroll = scrollView->get_contentSize() - pageSize - position;
        if (remaminingScroll < pageSize)
            Manager::RequestMorePlaylists();
    });
    list->tableView->scrollView->add_scrollPositionChangedEvent(delegate);
}

void PlaylistList::dtor() {
    instance = nullptr;
}

PlaylistList* PlaylistList::GetInstance() {
    if (!instance)
        instance = BeatSaberUI::CreateViewController<PlaylistList*>();
    return instance;
}

void PlaylistList::Refresh() {
    getLogger().debug("Refreshing playlist list");
    auto playlists = Manager::GetPlaylists();
    auto state = Manager::GetState();

    playlistData->Clear();
    for (auto& playlist : playlists)
        playlistData->Add(BSML::CustomCellInfo::construct(playlist->Title(), playlist->Author()));

    auto pos = list->tableView->contentTransform->get_anchoredPosition().y;
    list->tableView->ReloadData();
    list->tableView->scrollView->ScrollTo(std::min(pos, list->tableView->cellSize * list->NumberOfCells()), false);

    for (int i = 0; i < playlists.size(); i++) {
        Manager::GetPlaylistCover(playlists[i], [this, i, state](UnityEngine::Sprite* cover) {
            if (state != Manager::GetState())
                return;

            playlistData->get_Item(i)->icon = cover;
            list->tableView->RefreshCellsContent();
        });
    }
}

void PlaylistList::UpdateScrollView() {
    if (!list)
        return;

    auto table = list->tableView;
    auto pos = table->contentTransform->get_anchoredPosition().y;
    table->RefreshContentSize();
    table->scrollView->ScrollTo(std::min(pos, table->cellSize * table->numberOfCells), false);
}

void PlaylistList::SetLoading(bool value) {
    if (!list || !loadingIndicator)
        return;
    if (value) {
        list->tableView->ClearSelection();
        list->tableView->scrollView->ScrollTo(0, false);
    }
    list->get_gameObject()->SetActive(!value);
    loadingIndicator->SetActive(value);
}

void PlaylistList::cellSelected(HMUI::TableView* table, int cellIdx) {
    getLogger().info("Playlist %i selected", cellIdx);
    Manager::SelectPlaylist(cellIdx);
}
