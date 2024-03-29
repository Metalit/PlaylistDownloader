#include "config.hpp"
#include "main.hpp"
#include "manager.hpp"
#include "webutil.hpp"

#include "customtypes/mainmenu.hpp"
#include "customtypes/playlistlist.hpp"
#include "customtypes/playlistdetail.hpp"

#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/CustomTypes/Components/MainThreadScheduler.hpp"
#include "playlistcore/shared/PlaylistCore.hpp"
#include <string>

using namespace PlaylistDownloader;

#define FILE_DOWNLOAD_TIMEOUT 64
#define PLAYLIST_SONGS_PAGE_SIZE 10

// https://gist.github.com/jagt/5797948
template<class K, class V, int MaxSize = -1>
class cache {
private:
    struct Entry {
        K key;
        V value;
        Entry *prev;
        Entry *next;
    };

    std::map<K, Entry*> dict;
    Entry *head, *tail;
    cache(const cache &rhs);
    cache& operator = (const cache &rhs);

    Entry* detach(Entry *entry) {
        entry->prev->next = entry->next;
        entry->next->prev = entry->prev;
        return entry;
    }

    void push_front(Entry *entry) {
        entry->next = head->next;
        entry->next->prev = entry;
        entry->prev = head;
        head->next = entry;
    }
public:
    cache() : dict() {
        head = new Entry();
        tail = new Entry();
        head->next = tail;
        tail->prev = head;
    }

    ~cache() {
        while (head->next != tail) {
            delete detach(head->next);
        }
        delete head;
        delete tail;
    }

    void add(const K &key, const V &value) {
        Entry *entry = new Entry();
        entry->key = key;
        entry->value = value;
        push_front(entry);
        dict[key] = entry;
        if constexpr (MaxSize > 0) {
            if (size() > MaxSize)
                drop();
        }
    }

    V& get(const K &key) {
        Entry *entry = dict[key];
        // move to head
        detach(entry);
        push_front(entry);
        return entry->value;
    }

    bool has(const K& key) const {
        return dict.contains(key);
    }

    void drop() {
        Entry* entry = detach(tail->prev);
        dict.erase(entry->key);
        delete entry;
    }

    size_t size() const {
        size_t size = 0;
        Entry *p = head;
        while (p->next != tail) {
            ++size;
            p = p->next;
        }
        return size;
    }
};

template<class T>
struct dumb_function_copyable {
    dumb_function_copyable(T&& arg) : arg(std::move(arg)) {};
    dumb_function_copyable(const dumb_function_copyable<T>& other)
        : arg(std::move(const_cast<T&>(other.arg))) {};
    T& get() { return arg; }
    private:
    T arg;
};

namespace Manager {
    int source = -1;
    int page = 0;
    int receivedPages = 0;
    bool done = false;
    std::string search = "";

    std::vector<std::unique_ptr<Playlist>> playlists = {};
    int selectedPlaylist = -1;

    int songsPage = 0;
    int receivedSongsPages = 0;
    bool songsDone = false;

    std::vector<std::optional<BeatSaver::Beatmap>> songs = {};

    cache<std::string, UnityEngine::Sprite*, 20> cachedPlaylistCovers = {};
    cache<std::string, PlaylistCore::BPList, 100> cachedPlaylists = {};

    void Refresh() {
        getLogger().debug("Refreshing playlists");
        page = receivedPages = 0;
        done = false;
        playlists.clear();
        MainMenu::SetDetailShown(false);
        PlaylistList::GetInstance()->SetLoading(true);
        RequestMorePlaylists();
    }
    void RefreshSelected() {
        getLogger().debug("Refreshing playlist selection");
        songsPage = receivedSongsPages = 0;
        songsDone = false;
        songs.clear();
        MainMenu::SetDetailShown(GetSelectedPlaylist() != nullptr);
        PlaylistDetail::GetInstance()->SetLoading(true);
        PlaylistDetail::GetInstance()->Refresh(true);
        RequestMoreSongs();
    }
    void Invalidate() {
        playlists.clear();
        while (cachedPlaylistCovers.size() > 0)
            cachedPlaylistCovers.drop();
        while (cachedPlaylists.size() > 0)
            cachedPlaylists.drop();
    }

    void SetSearch(std::string value) {
        getLogger().debug("Searching %s", value.c_str());
        if (value == search)
            return;
        search = value;
        Refresh();
    }
    void SetSource(int newSource) {
        getLogger().debug("Source set to %i", newSource);
        if (newSource == source)
            return;
        source = newSource;
        Refresh();
    }
    void RequestMorePlaylists() {
        if (page > receivedPages || done)
            return;
        bool firstRequest = page == 0;
        getLogger().info("Requesting playlists from %i search %s", source, search.c_str());
        auto callback = [pastState = GetState(), firstRequest](std::vector<std::unique_ptr<Playlist>> newPlaylists) mutable {

            using capture_fix = dumb_function_copyable<std::vector<std::unique_ptr<Playlist>>>;
            auto arg = capture_fix(std::move(newPlaylists));

            QuestUI::MainThreadScheduler::Schedule([pastState, firstRequest, arg]() mutable {
                if (pastState != GetState())
                    return;
                if (!arg.get().empty()) {
                    playlists.insert(playlists.end(),
                        std::make_move_iterator(arg.get().begin()),
                        std::make_move_iterator(arg.get().end())
                    );
                } else
                    done = true;
                receivedPages++;
                if (!done) {
                    PlaylistList::GetInstance()->SetLoading(false);
                    PlaylistList::GetInstance()->Refresh(firstRequest);
                }
            });
        };
        switch (source) {
            case 0:
                GetBeatSaverPlaylists(callback, page, search);
                break;
            case 1:
                GetHitbloqPlaylists(callback, page, search);
                break;
            case 2:
                GetAccSaberPlaylists(callback, page, search);
                break;
            case 3:
                GetBeatLeaderPlaylists(callback, page, search);
                break;
        }
        page++;
    }

    State GetState() {
        // no change to state if filters are changed on a source that doesn't use them
        bool curated = SourceHasFilters(source) ? getConfig().curated.GetValue() : false;
        bool includeEmpty = SourceHasFilters(source) ? getConfig().includeEmpty.GetValue() : false;
        std::string sort = SourceHasFilters(source) ? getConfig().sort.GetValue() : "";
        return State{
            .source = source,
            .search = search,
            .curated = curated,
            .includeEmpty = includeEmpty,
            .sort = sort,
        };
    }
    std::vector<Playlist*> GetPlaylists() {
        std::vector<Playlist*> ret = {};
        ret.reserve(playlists.size());

        for (auto& playlist : playlists)
            ret.push_back(playlist.get());

        return ret;
    }

    void SelectPlaylist(int playlistIdx) {
        getLogger().debug("Playlist set to %i", playlistIdx);
        selectedPlaylist = playlistIdx;
        RefreshSelected();
    }
    Playlist* GetSelectedPlaylist() {
        if (selectedPlaylist < 0 || selectedPlaylist > playlists.size())
            return nullptr;
        return playlists[selectedPlaylist].get();
    }

    void RequestMoreSongs() {
        auto playlist = GetSelectedPlaylist();
        if (!playlist || songsPage > receivedSongsPages || songsDone)
            return;
        getLogger().debug("Getting playlist songs %s page %i", playlist->Title().c_str(), songsPage);
        auto callback = [currentPage = songsPage, playlist](PlaylistCore::BPList downloaded) {
            if (playlist != GetSelectedPlaylist())
                return;
            auto& bpSongs = downloaded.Songs;
            int start = PLAYLIST_SONGS_PAGE_SIZE * currentPage;
            int end = PLAYLIST_SONGS_PAGE_SIZE * (currentPage + 1);
            end = std::min(end, (int) bpSongs.size());
            int count = end - start;
            if (count <= 0) {
                receivedSongsPages++;
                songsDone = true;
                return;
            }
            auto foundCount = new std::atomic_int(0);
            auto foundMaps = new std::vector<std::optional<BeatSaver::Beatmap>>(count);
            for (int i = start; i < end; i++) {
                getLogger().debug("Getting playlist song %i / %li", i, bpSongs.size());
                BeatSaver::API::GetBeatmapByHashAsync(bpSongs[i].Hash, [foundCount, foundMaps, i, count, start, playlist](std::optional<BeatSaver::Beatmap> map) {
                    foundMaps->at(i - start) = map;
                    int newCount = foundCount->fetch_add(1) + 1;
                    if (map.has_value())
                        getLogger().debug("Got playlist song %i total %i / %i", i, newCount, count);
                    else
                        getLogger().debug("Failed to get playlist song %i total %i / %i", i, newCount, count);
                    if (newCount == count) {
                        if (playlist == GetSelectedPlaylist()) {
                            QuestUI::MainThreadScheduler::Schedule([maps = *foundMaps]() {
                                for (auto& map : maps)
                                    songs.push_back(map);
                                receivedSongsPages++;
                                PlaylistDetail::GetInstance()->SetLoading(false);
                                PlaylistDetail::GetInstance()->Refresh(false);
                            });
                        }
                        delete foundCount;
                        delete foundMaps;
                    }
                });
            }
        };
        GetPlaylistFile(playlist, callback);
        songsPage++;
    }
    std::vector<BeatSaver::Beatmap*> GetSongs() {
        std::vector<BeatSaver::Beatmap*> ret = {};
        ret.reserve(songs.size());

        for (auto& song : songs) {
            if (song.has_value())
                ret.push_back(&*song);
        }
        return ret;
    }

    void GetPlaylistCover(Playlist* playlist, std::function<void(UnityEngine::Sprite*)> callback) {
        getLogger().debug("Getting playlist cover %s", playlist->Title().c_str());
        auto url = playlist->ImageURL();
        if (cachedPlaylistCovers.has(url)) {
            callback(cachedPlaylistCovers.get(url));
            return;
        }
        WebUtils::GetAsync(url, FILE_DOWNLOAD_TIMEOUT, [url, callback](long httpCode, std::string data) {
            QuestUI::MainThreadScheduler::Schedule([bytes = std::vector<uint8_t>(data.begin(), data.end()), url, callback]() {
                auto sprite = QuestUI::BeatSaberUI::VectorToSprite(bytes);
                cachedPlaylistCovers.add(url, sprite);
                callback(sprite);
            });
        });
    }
    void GetSongCover(BeatSaver::Beatmap* song, std::function<void(UnityEngine::Sprite*)> callback) {
        getLogger().debug("Getting song cover %s", song->GetName().c_str());
        song->GetLatestCoverImageAsync([callback](std::vector<uint8_t> bytes) {
            QuestUI::MainThreadScheduler::Schedule([bytes, callback]() {
                auto sprite = QuestUI::BeatSaberUI::VectorToSprite(bytes);
                callback(sprite);
            });
        }, nullptr);
    }

    bool SelectedPlaylistOwned() {
        auto playlist = GetSelectedPlaylist();
        if (!playlist)
            return false;

        auto allLists = PlaylistCore::GetLoadedPlaylists();
        for (auto& loaded : allLists) {
            auto& cdata = loaded->playlistJSON.CustomData;
            if (cdata.has_value() && cdata->SyncURL == playlist->PlaylistURL())
                return true;
        }
        return false;
    }
    void GetPlaylistFile(Playlist* playlist, std::function<void(PlaylistCore::BPList)> callback) {
        getLogger().debug("Getting playlist file %s", playlist->Title().c_str());
        auto url = playlist->PlaylistURL();
        if (cachedPlaylists.has(url)) {
            callback(cachedPlaylists.get(url));
            return;
        }
        WebUtils::GetAsync(url, FILE_DOWNLOAD_TIMEOUT, [url, callback](long httpCode, std::string data) {
            QuestUI::MainThreadScheduler::Schedule([data, url, callback]() {
                PlaylistCore::BPList list;
                try {
                    ReadFromString(data, list);
                    if (!list.CustomData.has_value())
                        list.CustomData.emplace();
                    auto& syncUrl = list.CustomData->SyncURL;
                    if (!syncUrl.has_value() || syncUrl->empty())
                        syncUrl = url;
                } catch (const JSONException& exc) {
                    getLogger().error("Failed to deserialize playlist %s: %s", data.c_str(), exc.what());
                    return;
                }
                cachedPlaylists.add(url, list);
                callback(list);
            });
        });
    }
};
