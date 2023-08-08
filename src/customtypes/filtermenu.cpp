#include "main.hpp"
#include "manager.hpp"
#include "customtypes/filtermenu.hpp"
#include "assets.hpp"

#include "questui/shared/BeatSaberUI.hpp"
#include "bsml/shared/BSML.hpp"

DEFINE_TYPE(PlaylistDownloader, FilterMenu);

using namespace PlaylistDownloader;

void FilterMenu::OnEnable() {
    set_name("PlaylistFilters");
    get_rectTransform()->set_anchorMin({0.5, 0.5});
    get_rectTransform()->set_anchorMax({0.5, 0.5});
    get_rectTransform()->set_sizeDelta({160, 15});
}

void FilterMenu::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (firstActivation) {
        SetupBSMLFields();
        AddHotReload(this, "filtermenu");
        // BSML::parse_and_construct(IncludedAssets::filtermenu_bsml, get_transform(), this);
    }
}

void FilterMenu::SetupBSMLFields() {
    using Item = HMUI::IconSegmentedControl::DataItem;
    sourceIconData = List<Item*>::New_ctor(3);
    sourceIconData->Add(Item::New_ctor(PNG_SPRITE(BeatSaver), "BeatSaver"));
    sourceIconData->Add(Item::New_ctor(PNG_SPRITE(Hitbloq), "Hitbloq"));
    sourceIconData->Add(Item::New_ctor(PNG_SPRITE(AccSaber), "AccSaber"));
    sourceIconData->Add(Item::New_ctor(PNG_SPRITE(BeatLeader), "BeatLeader"));
}

void FilterMenu::PostParse() {
    auto searchInput = QuestUI::BeatSaberUI::CreateStringSetting(filterBar, "Search", "", {}, {0, -35, 0}, [this](StringW value) { stringInput(value); });
    searchInput->get_transform()->SetSiblingIndex(1);
    sourceIconControl->hideCellBackground = false;
    sourceIconControl->overrideCellSize = true;
    sourceIconControl->iconSize = 6;
    sourceIconControl->padding = 2;
    sourceIconControl->ReloadData();
    Manager::SetSource(sourceIconControl->get_selectedCellNumber());
}

void FilterMenu::dtor() {
    instance = nullptr;
}

FilterMenu* FilterMenu::GetInstance() {
    if (!instance)
        instance = QuestUI::BeatSaberUI::CreateViewController<FilterMenu*>();
    return instance;
}

void FilterMenu::sourceSelected(HMUI::SegmentedControl* control, int cell) {
    getLogger().info("Source %i selected", cell);
    Manager::SetSource(cell);
}

void FilterMenu::stringInput(StringW value) {
    getLogger().info("String %s input", value.operator std::string().c_str());
    Manager::SetSearch(value);
}

void FilterMenu::filterClicked() {
    getLogger().info("Filter button clicked");
}
