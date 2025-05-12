#pragma once

#include "rapidjson-macros/shared/macros.hpp"

DECLARE_JSON_STRUCT(AccSaberPlaylist) {
    VALUE(std::string, displayName);
    VALUE(std::string, description);

    VALUE(std::string, downloadLink);
    VALUE(std::string, imageUrl);
};

DECLARE_JSON_STRUCT(AccSaberResponse) {
    NAMED_VECTOR(AccSaberPlaylist, Playlists, SELF_OBJECT_NAME);
};

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
