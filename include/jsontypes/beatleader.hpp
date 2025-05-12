#pragma once

#include "rapidjson-macros/shared/macros.hpp"

DECLARE_JSON_STRUCT(BeatLeaderPlaylist) {
    VALUE(std::string, name);

    VALUE(int, playlistId);
    VALUE(std::string, image);
    VALUE(bool, downloadable);
};

DECLARE_JSON_STRUCT(BeatLeaderResponse) {
    NAMED_VECTOR(BeatLeaderPlaylist, Playlists, "data");
};

#include "apis.hpp"

inline std::unique_ptr<Playlist> FromBeatLeader(BeatLeaderPlaylist const& playlist) {
    return std::make_unique<Playlist>(Playlist{
        .Title = playlist.name,
        .Author = "BeatLeader",
        .Description = "",
        .PlaylistURL = "https://api.beatleader.xyz/playlist/" + std::to_string(playlist.playlistId),
        .ImageURL = playlist.image,
        .FallbackImage = &IncludedAssets::BeatLeader_png,
    });
}
