#include "IDataAccess.h"
#include "sqlite3.h"
#include <exception>
#include <sstream>
#include <io.h>


constexpr auto DB_NAME = "galleryDB.sqlite";

class DatabaseAccess : public IDataAccess {
public:
	// album related
	const std::list<Album> getAlbums() override;
	const std::list<Album> getAlbumsOfUser(const User& user) override;
	void createAlbum(const Album& album) override;

	void deleteAlbum(const std::string& albumName, int userId) override; //TODO

	bool doesAlbumExists(const std::string& albumName, int userId) override;
	Album openAlbum(const std::string& albumName) override;

	void closeAlbum(Album& pAlbum) override {}; ///leaving empty for now
	
	void printAlbums() override;

	// picture related
	void addPictureToAlbumByName(const std::string& albumName, const Picture& picture) override; //TODO
	void removePictureFromAlbumByName(const std::string& albumName, const std::string& pictureName) override; //TODO
	void tagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId) override; //TODO
	void untagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId) override; //TODO
	// user related
	void printUsers() override;
	void createUser(User& user) override; //done

	void deleteUser(const User& user) override; //TODO
	bool doesUserExists(int userId) override;
	User getUser(int userId) override;

	// user statistics
	int countAlbumsOwnedOfUser(const User& user) override;
	int countAlbumsTaggedOfUser(const User& user) override;
	int countTagsOfUser(const User& user) override;
	float averageTagsPerAlbumOfUser(const User& user) override; //TODO
	// queries
	User getTopTaggedUser() override;
	Picture getTopTaggedPicture() override;
	std::list<Picture> getTaggedPicturesOfUser(const User& user) override;

	bool open() override;
	void close() override;
	void clear() override {};
	

private:
	void init_db();
	void my_exec(const char* sqlStatement);

	sqlite3* _db;
};