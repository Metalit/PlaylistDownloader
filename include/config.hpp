#pragma once

#include "config-utils/shared/config-utils.hpp"

inline std::vector<std::string> const sortOptions = {
    "Latest", "Relevance", "Curated",
    // doesn't apply to playlists
    // "Rating",
};

DECLARE_CONFIG(Config) {
    CONFIG_VALUE(curated, bool, "Curated", false);
    CONFIG_VALUE(includeEmpty, bool, "Include Empty", false);
    CONFIG_VALUE(sort, std::string, "Sort Order", sortOptions[0]);
};

inline bool SourceHasFilters(int source) {
    return source == 0;
}
