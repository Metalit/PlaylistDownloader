#pragma once

#include "rapidjson-macros/shared/macros.hpp"

DECLARE_JSON_CLASS(HitbloqPlaylist,
    VALUE(std::string, title);
    VALUE(std::string, author);
    VALUE(std::string, description);
    VALUE(std::string, short_description);

    VALUE(std::string, download_url);
    VALUE(std::string, image);
    DISCARD_EXTRA_FIELDS
)

DECLARE_JSON_CLASS(HitbloqResponse,
    NAMED_VECTOR(HitbloqPlaylist, Playlists, rapidjson_macros_types::SelfValueType());
    DISCARD_EXTRA_FIELDS
)

#include "apis.hpp"

struct HitbloqPlaylistWrapper : Playlist {
    std::string Title() override {
        return playlist.title;
    }
    std::string Author() override {
        return playlist.author;
    }
    std::string Description() override {
        if (playlist.description.size() > 0)
            return playlist.description;
        return playlist.short_description;
    }
    std::string PlaylistURL() override {
        return playlist.download_url;
    }
    std::string ImageURL() override {
        return playlist.image;
    }
    HitbloqPlaylistWrapper(HitbloqPlaylist playlist) : playlist(playlist) {}
    private:
    HitbloqPlaylist playlist;
};
