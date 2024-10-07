#pragma once

#include "rapidjson-macros/shared/macros.hpp"

DECLARE_JSON_CLASS(AccSaberPlaylist,
    VALUE(std::string, displayName);
    VALUE(std::string, description);

    VALUE(std::string, downloadLink);
    VALUE(std::string, imageUrl);
    DISCARD_EXTRA_FIELDS
)

DECLARE_JSON_CLASS(AccSaberResponse,
    NAMED_VECTOR(AccSaberPlaylist, Playlists, rapidjson_macros_types::SelfValueType());
    DISCARD_EXTRA_FIELDS
)

#include "apis.hpp"

inline std::unique_ptr<Playlist> FromAccSaber(AccSaberPlaylist const& playlist) {
    return std::make_unique<Playlist>(Playlist{
        .Title = playlist.displayName,
        .Author = "AccSaber",
        .Description = playlist.description,
        .PlaylistURL = playlist.downloadLink,
        .ImageURL = playlist.imageUrl,
        .FallbackImage = &IncludedAssets::AccSaber_png,
    });
}
