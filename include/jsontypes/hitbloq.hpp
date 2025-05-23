#pragma once

#include "rapidjson-macros/shared/macros.hpp"

DECLARE_JSON_STRUCT(HitbloqPlaylist) {
    VALUE(std::string, title);
    VALUE(std::string, author);
    VALUE(std::string, description);
    VALUE(std::string, short_description);

    VALUE(std::string, download_url);
    VALUE(std::string, image);
};

DECLARE_JSON_STRUCT(HitbloqResponse) {
    NAMED_VECTOR(HitbloqPlaylist, Playlists, SELF_OBJECT_NAME);
};

#include "apis.hpp"

inline std::unique_ptr<Playlist> FromHitbloq(HitbloqPlaylist const& playlist) {
    return std::make_unique<Playlist>(Playlist{
        .Title = playlist.title,
        .Author = playlist.author,
        .Description = playlist.description.empty() ? playlist.short_description : playlist.description,
        .PlaylistURL = playlist.download_url,
        .ImageURL = playlist.image,
        .FallbackImage = &IncludedAssets::Hitbloq_png,
    });
}
