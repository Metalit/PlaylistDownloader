#include "main.hpp"
#include "manager.hpp"
#include "assets.hpp"
#include "config.hpp"
#include "customtypes/filtermenu.hpp"

#include "bsml/shared/BSML.hpp"
#include "questui/shared/BeatSaberUI.hpp"

#include "UnityEngine/UI/Toggle.hpp"
#include "HMUI/AnimatedSwitchView.hpp"

DEFINE_TYPE(PlaylistDownloader, FilterMenu);

using namespace PlaylistDownloader;

void InstantSetToggle(UnityEngine::UI::Toggle* toggle, bool value) {
    if(toggle->m_IsOn == value)
        return;
    toggle->m_IsOn = value;
    auto animatedSwitch = toggle->GetComponent<HMUI::AnimatedSwitchView*>();
    animatedSwitch->HandleOnValueChanged(value);
    animatedSwitch->switchAmount = value;
    animatedSwitch->LerpPosition(value);
    animatedSwitch->LerpColors(value, animatedSwitch->highlightAmount, animatedSwitch->disabledAmount);
}

void FilterMenu::OnEnable() {
    set_name("PlaylistFilters");
    get_rectTransform()->set_anchorMin({0.5, 0.5});
    get_rectTransform()->set_anchorMax({0.5, 0.5});
    get_rectTransform()->set_sizeDelta({160, 15});
}

void FilterMenu::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (firstActivation) {
        SetupBSMLFields();
        // AddHotReload(this, "filtermenu");
        BSML::parse_and_construct(IncludedAssets::filtermenu_bsml, get_transform(), this);
    }
}

void FilterMenu::SetupBSMLFields() {
    using Item = HMUI::IconSegmentedControl::DataItem;
    sourceIconData = List<Item*>::New_ctor(3);
    sourceIconData->Add(Item::New_ctor(PNG_SPRITE(BeatSaver), "BeatSaver"));
    sourceIconData->Add(Item::New_ctor(PNG_SPRITE(Hitbloq), "Hitbloq"));
    sourceIconData->Add(Item::New_ctor(PNG_SPRITE(AccSaber), "AccSaber"));
    sourceIconData->Add(Item::New_ctor(PNG_SPRITE(BeatLeader), "BeatLeader"));
    sorts = List<StringW>::New_ctor(4);
    for (auto sort : sortOptions)
        sorts->Add(sort);
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
    filterModal->Hide(false, nullptr);
    InstantSetToggle(curatedToggle->toggle, getConfig().curated.GetValue());
    InstantSetToggle(includeEmptyToggle->toggle, getConfig().includeEmpty.GetValue());
    for (int i = 0; i < sortOptions.size(); i++) {
        if (sortOptions[i] == getConfig().sort.GetValue()) {
            sortEnum->index = i;
            sortEnum->UpdateState();
        }
    }
}

void FilterMenu::OnDestroy() {
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
    filterButton->get_gameObject()->SetActive(SourceHasFilters(cell));
}

void FilterMenu::stringInput(StringW value) {
    getLogger().info("String %s input", value.operator std::string().c_str());
    Manager::SetSearch(value);
}

void FilterMenu::filterClicked() {
    getLogger().info("Filter button clicked");
    filterModal->Show(true, false, nullptr);
}

void FilterMenu::curatedToggled(bool value) {
    if (value == getConfig().curated.GetValue())
        return;
    getConfig().curated.SetValue(value);
    Manager::Refresh();
}

void FilterMenu::includeEmptyToggled(bool value) {
    if (value == getConfig().includeEmpty.GetValue())
        return;
    getConfig().includeEmpty.SetValue(value);
    Manager::Refresh();
}

void FilterMenu::sortPicked(StringW value) {
    if (value == getConfig().sort.GetValue())
        return;
    getConfig().sort.SetValue(value);
    Manager::Refresh();
}
