#pragma once

#include "rapidjson-macros/shared/macros.hpp"

DECLARE_JSON_CLASS(BeatLeaderPlaylist,
    VALUE(std::string, name);

    VALUE(int, playlistId);
    VALUE(std::string, image);
    VALUE(bool, downloadable);
    DISCARD_EXTRA_FIELDS
)

DECLARE_JSON_CLASS(BeatLeaderResponse,
    NAMED_VECTOR(BeatLeaderPlaylist, Playlists, "data");
    DISCARD_EXTRA_FIELDS
)

#include "apis.hpp"

inline std::unique_ptr<Playlist> FromBeatLeader(BeatLeaderPlaylist const& playlist) {
    return std::make_unique<Playlist>(Playlist{
        .Title = playlist.name,
        .Author = "BeatLeader",
        .Description = "",
        .PlaylistURL = "https://api.beatleader.xyz/playlist/" + std::to_string(playlist.playlistId),
        .ImageURL = playlist.image,
    });
}
