/*****************************************************************************
 * Media Library
 *****************************************************************************
 * Copyright (C) 2015 Hugo Beauzée-Luyssen, Videolabs
 *
 * Authors: Hugo Beauzée-Luyssen<hugo@beauzee.fr>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifndef MEDIALIBRARY_H
#define MEDIALIBRARY_H

#include "medialibrary/IMediaLibrary.h"
#include "logging/Logger.h"
#include "Settings.h"

#include "medialibrary/IDeviceLister.h"

namespace medialibrary
{

class ModificationNotifier;
class DiscovererWorker;
class Parser;
class ParserService;
class SqliteConnection;

class Album;
class Artist;
class Media;
class Movie;
class Show;
class Device;
class Folder;
class Genre;

namespace factory
{
class IFileSystem;
}
namespace fs
{
class IFile;
class IDirectory;
}

class MediaLibrary : public IMediaLibrary, public IDeviceListerCb
{
    public:
        MediaLibrary();
        ~MediaLibrary();
        virtual bool initialize( const std::string& dbPath, const std::string& thumbnailPath, IMediaLibraryCb* metadataCb ) override;
        virtual void setVerbosity( LogLevel v ) override;

        virtual MediaPtr media( int64_t mediaId ) const override;
        virtual MediaPtr media( const std::string& path ) const override;
        virtual std::vector<MediaPtr> audioFiles( SortingCriteria sort, bool desc) const override;
        virtual std::vector<MediaPtr> videoFiles( SortingCriteria sort, bool desc) const override;

        std::shared_ptr<Media> addFile( const fs::IFile& fileFs, Folder& parentFolder, fs::IDirectory& parentFolderFs );

        bool deleteFolder(const Folder& folder );
        std::shared_ptr<Device> device( const std::string& uuid );

        virtual LabelPtr createLabel( const std::string& label ) override;
        virtual bool deleteLabel( LabelPtr label ) override;

        virtual AlbumPtr album( int64_t id ) const override;
        std::shared_ptr<Album> createAlbum( const std::string& title );
        virtual std::vector<AlbumPtr> albums(SortingCriteria sort, bool desc) const override;

        virtual std::vector<GenrePtr> genres( SortingCriteria sort, bool desc ) const override;
        virtual GenrePtr genre( int64_t id ) const override;

        virtual ShowPtr show( const std::string& name ) const override;
        std::shared_ptr<Show> createShow( const std::string& name );

        virtual MoviePtr movie( const std::string& title ) const override;
        std::shared_ptr<Movie> createMovie( Media& media, const std::string& title );

        virtual ArtistPtr artist( int64_t id ) const override;
        ArtistPtr artist( const std::string& name );
        std::shared_ptr<Artist> createArtist( const std::string& name );
        virtual std::vector<ArtistPtr> artists( SortingCriteria sort, bool desc ) const override;

        virtual PlaylistPtr createPlaylist( const std::string& name ) override;
        virtual std::vector<PlaylistPtr> playlists( SortingCriteria sort, bool desc ) override;
        virtual PlaylistPtr playlist( int64_t id ) const override;
        virtual bool deletePlaylist( int64_t playlistId ) override;

        virtual bool addToHistory( const std::string& mrl ) override;
        virtual std::vector<HistoryPtr> lastStreamsPlayed() const override;
        virtual std::vector<MediaPtr> lastMediaPlayed() const override;
        virtual bool clearHistory() override;

        virtual MediaSearchAggregate searchMedia( const std::string& title ) const override;
        virtual std::vector<PlaylistPtr> searchPlaylists( const std::string& name ) const override;
        virtual std::vector<AlbumPtr> searchAlbums( const std::string& pattern ) const override;
        virtual std::vector<GenrePtr> searchGenre( const std::string& genre ) const override;
        virtual std::vector<ArtistPtr> searchArtists( const std::string& name ) const override;
        virtual SearchAggregate search( const std::string& pattern ) const override;

        virtual void discover( const std::string& entryPoint ) override;
        virtual void setDiscoverNetworkEnabled( bool enabled ) override;
        virtual std::vector<FolderPtr> entryPoints() const override;
        virtual bool removeEntryPoint( const std::string& entryPoint ) override;
        virtual bool banFolder( const std::string& path ) override;
        virtual bool unbanFolder( const std::string& path ) override;

        virtual const std::string& thumbnailPath() const override;
        virtual void setLogger( ILogger* logger ) override;
        //Temporarily public, move back to private as soon as we start monitoring the FS
        virtual void reload() override;
        virtual void reload( const std::string& entryPoint ) override;

        virtual void pauseBackgroundOperations() override;
        virtual void resumeBackgroundOperations() override;

        DBConnection getConn() const;
        IMediaLibraryCb* getCb() const;
        std::shared_ptr<ModificationNotifier> getNotifier() const;

        virtual IDeviceListerCb* setDeviceLister( DeviceListerPtr lister ) override;
        std::shared_ptr<factory::IFileSystem> fsFactoryForPath( const std::string& path ) const;

    public:
        static const uint32_t DbModelVersion;

    private:
        static const std::vector<std::string> supportedVideoExtensions;
        static const std::vector<std::string> supportedAudioExtensions;

    private:
        virtual void startParser();
        virtual void startDiscoverer();
        virtual void startDeletionNotifier();
        bool updateDatabaseModel( unsigned int previousVersion );
        bool createAllTables();
        void registerEntityHooks();
        static bool validateSearchPattern( const std::string& pattern );

    protected:
        virtual void addLocalFsFactory();

        // Mark IDeviceListerCb callbacks as private. They must be invoked through the interface.
    private:
        virtual void onDevicePlugged(const std::string& uuid, const std::string& mountpoint) override;
        virtual void onDeviceUnplugged(const std::string& uuid) override;

    protected:
        std::unique_ptr<SqliteConnection> m_dbConnection;
        std::vector<std::shared_ptr<factory::IFileSystem>> m_fsFactories;
        std::string m_thumbnailPath;
        IMediaLibraryCb* m_callback;
        DeviceListerPtr m_deviceLister;

        // Keep the parser as last field.
        // The parser holds a (raw) pointer to the media library. When MediaLibrary's destructor gets called
        // it might still finish a few operations before exiting the parser thread. Those operations are
        // likely to require a valid MediaLibrary, which would be compromised if some fields have already been
        // deleted/destroyed.
        std::unique_ptr<Parser> m_parser;
        // Same reasoning applies here.
        //FIXME: Having to maintain a specific ordering sucks, let's use shared_ptr or something
        std::unique_ptr<DiscovererWorker> m_discoverer;
        std::shared_ptr<ModificationNotifier> m_modificationNotifier;
        LogLevel m_verbosity;
        Settings m_settings;
};

}

#endif // MEDIALIBRARY_H
