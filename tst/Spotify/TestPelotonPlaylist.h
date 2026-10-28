#pragma once

#include <gtest/gtest.h>
#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>

#include "peloton.h"

TEST(PelotonPlaylistTest, FormatsSongsWithSpotifySearchLinks) {
    QJsonArray artists;
    QJsonObject artist;
    artist.insert(QStringLiteral("artist_name"), QStringLiteral("David Bowie"));
    artists.append(artist);

    QJsonObject song;
    song.insert(QStringLiteral("title"), QStringLiteral("Let's Dance"));
    song.insert(QStringLiteral("artists"), artists);

    QJsonArray songs;
    songs.append(song);
    QJsonObject playlist;
    playlist.insert(QStringLiteral("songs"), songs);
    QJsonObject ride;
    ride.insert(QStringLiteral("playlist"), playlist);

    const QStringList tracks = peloton::spotifyTracksFromRide(ride);

    ASSERT_EQ(tracks.size(), 1);
    EXPECT_TRUE(tracks.at(0).startsWith(QStringLiteral("Let's Dance - David Bowie: ")));
    EXPECT_TRUE(tracks.at(0).contains(QStringLiteral("https://open.spotify.com/search/")));
    EXPECT_TRUE(tracks.at(0).contains(QStringLiteral("%20David%20Bowie")));
}

TEST(PelotonPlaylistTest, PreservesArtistOrderAndSkipsSongsWithoutTitles) {
    QJsonArray artists;
    QJsonObject firstArtist;
    firstArtist.insert(QStringLiteral("artist_name"), QStringLiteral("Artist One"));
    artists.append(firstArtist);
    QJsonObject secondArtist;
    secondArtist.insert(QStringLiteral("artist_name"), QStringLiteral("Artist Two"));
    artists.append(secondArtist);

    QJsonObject song;
    song.insert(QStringLiteral("title"), QStringLiteral("Collaboration"));
    song.insert(QStringLiteral("artists"), artists);

    QJsonObject emptySong;
    emptySong.insert(QStringLiteral("artists"), artists);

    QJsonArray songs;
    songs.append(song);
    songs.append(emptySong);
    QJsonObject playlist;
    playlist.insert(QStringLiteral("songs"), songs);
    QJsonObject ride;
    ride.insert(QStringLiteral("playlist"), playlist);

    const QStringList tracks = peloton::spotifyTracksFromRide(ride);

    ASSERT_EQ(tracks.size(), 1);
    EXPECT_TRUE(tracks.at(0).startsWith(QStringLiteral("Collaboration - Artist One, Artist Two: ")));
}

TEST(PelotonPlaylistTest, MissingPlaylistProducesNoTracks) {
    EXPECT_TRUE(peloton::spotifyTracksFromRide(QJsonObject()).isEmpty());
}
