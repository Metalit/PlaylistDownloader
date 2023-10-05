#pragma once

#include "apis.hpp"
#include "songdownloader/shared/BeatSaverAPI.hpp"
#include "playlistcore/shared/Types/BPList.hpp"

#include "UnityEngine/Sprite.hpp"

namespace Manager {

    struct State {
        int source;
        std::string search;
        bool curated;
        bool includeEmpty;
        std::string sort;
        bool operator==(const State& other) const = default;
    };

    void Refresh();
    void Invalidate();

    void SetSearch(std::string value);
    void SetSource(int source);
    void RequestMorePlaylists();

    State GetState();
    std::vector<Playlist*> GetPlaylists();

    void SelectPlaylist(int playlistIdx);
    Playlist* GetSelectedPlaylist();

    void RequestMoreSongs();
    std::vector<BeatSaver::Beatmap*> GetSongs();

    void GetPlaylistCover(Playlist* playlist, std::function<void(UnityEngine::Sprite*)> callback);
    void GetSongCover(BeatSaver::Beatmap* song, std::function<void(UnityEngine::Sprite*)> callback);

    bool SelectedPlaylistOwned();
    void GetPlaylistFile(Playlist* playlist, std::function<void(PlaylistCore::BPList)> callback);
};
