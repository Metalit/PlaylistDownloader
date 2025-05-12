#pragma once

#include "rapidjson-macros/shared/macros.hpp"

DECLARE_JSON_STRUCT(BeatSaverUser) {
    VALUE(std::string, name);
};

DECLARE_JSON_STRUCT(BeatSaverPlaylist) {
    VALUE(std::string, name);
    VALUE(BeatSaverUser, owner);
    VALUE(std::string, description);

    VALUE(std::string, downloadURL);
    VALUE(std::string, playlistImage);
};

DECLARE_JSON_STRUCT(BeatSaverResponse) {
    NAMED_VECTOR(BeatSaverPlaylist, Playlists, "docs");
};

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
