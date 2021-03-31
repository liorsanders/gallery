#include "IDataAccess.h"
#include "sqlite3.h"
#include <exception>
#include <sstream>
#include <io.h>


constexpr auto DB_NAME = "galleryDB.sqlite";

namespace album_field {
	auto ID = "ID";
	auto NAME = "NAME";
	auto CREATION_DATE = "CREATION_DATE";
	auto USER_ID = "USER_ID";
};

namespace user_field {
	auto ID = "ID";
	auto NAME = "NAME";
};

namespace tag_field {
	auto ID = "ID";
	auto PICTURE_ID = "PICTURE_ID";
	auto USER_ID = "USER_ID";
};

namespace picture_field {
	auto ID = "ID";
	auto NAME = "NAME";
	auto LOCATION = "LOCATION";
	auto CREATION_DATE = "CREATION_DATE";
	auto ALBUM_ID = "ALBUM_ID";
};

class DatabaseAccess : public IDataAccess {
public:
	// album related
	const std::list<Album> getAlbums() override; //TODO
	const std::list<Album> getAlbumsOfUser(const User& user) override; //TODO
	void createAlbum(const Album& album) override; //TODO

	void deleteAlbum(const std::string& albumName, int userId) override; 

	bool doesAlbumExists(const std::string& albumName, int userId) override; //TODO
	Album openAlbum(const std::string& albumName) override; //TODO

	void closeAlbum(Album& pAlbum) override {}; ///leaving empty for now
	
	void printAlbums() override; //TODO

	// picture related
	void addPictureToAlbumByName(const std::string& albumName, const Picture& picture) override; 
	void removePictureFromAlbumByName(const std::string& albumName, const std::string& pictureName) override; 
	void tagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId) override; 
	void untagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId) override; 
	// user related
	void printUsers() override; //TODO
	void createUser(User& user) override; 

	void deleteUser(const User& user) override; 
	bool doesUserExists(int userId) override; //TODO
	User getUser(int userId) override; //TODO

	// user statistics
	int countAlbumsOwnedOfUser(const User& user) override; //TODO
	int countAlbumsTaggedOfUser(const User& user) override; //TODO
	int countTagsOfUser(const User& user) override; //TODO
	float averageTagsPerAlbumOfUser(const User& user) override; 
	// queries
	User getTopTaggedUser() override; //TODO
	Picture getTopTaggedPicture() override; //TODO
	std::list<Picture> getTaggedPicturesOfUser(const User& user) override; //TODO

	bool open() override;
	void close() override;
	void clear() override {};
	

private:
	void init_db();
	void my_exec(const char* sqlStatement);

	sqlite3* _db;
};