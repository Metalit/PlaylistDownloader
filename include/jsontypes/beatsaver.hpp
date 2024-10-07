#pragma once

#include "rapidjson-macros/shared/macros.hpp"

DECLARE_JSON_CLASS(BeatSaverUser,
    VALUE(std::string, name);
    DISCARD_EXTRA_FIELDS
)

DECLARE_JSON_CLASS(BeatSaverPlaylist,
    VALUE(std::string, name);
    VALUE(BeatSaverUser, owner);
    VALUE(std::string, description);

    VALUE(std::string, downloadURL);
    VALUE(std::string, playlistImage);
    DISCARD_EXTRA_FIELDS
)

DECLARE_JSON_CLASS(BeatSaverResponse,
    NAMED_VECTOR(BeatSaverPlaylist, Playlists, "docs");
    DISCARD_EXTRA_FIELDS
)

#include "apis.hpp"

inline std::unique_ptr<Playlist> FromBeatSaver(BeatSaverPlaylist const& playlist) {
    return std::make_unique<Playlist>(Playlist{
        .Title = playlist.name,
        .Author = playlist.owner.name,
        .Description = playlist.description,
        .PlaylistURL = playlist.downloadURL,
        .ImageURL = playlist.playlistImage,
        .FallbackImage = &IncludedAssets::BeatSaver_png,
    });
}
