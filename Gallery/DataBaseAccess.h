#include "IDataAccess.h"
#include "MyException.h"
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
	const std::list<Album> getAlbums() override; 
	const std::list<Album> getAlbumsOfUser(const User& user) override; 
	void createAlbum(const Album& album) override; 

	void deleteAlbum(const std::string& albumName, int userId) override; 

	bool doesAlbumExists(const std::string& albumName, int userId) override; 
	Album openAlbum(const std::string& albumName) override; 

	void closeAlbum(Album& pAlbum) override {}; ///leaving empty for now
	
	void printAlbums() override; 

	// picture related
	void addPictureToAlbumByName(const std::string& albumName, const Picture& picture) override; 
	void removePictureFromAlbumByName(const std::string& albumName, const std::string& pictureName) override; 
	void tagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId) override; 
	void untagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId) override; 
	// user related
	void printUsers() override; 
	void createUser(User& user) override; 

	void deleteUser(const User& user) override; 
	bool doesUserExists(int userId) override; 
	User getUser(int userId) override; 

	// user statistics
	int countAlbumsOwnedOfUser(const User& user) override; 
	int countAlbumsTaggedOfUser(const User& user) override; 
	int countTagsOfUser(const User& user) override; 
	float averageTagsPerAlbumOfUser(const User& user) override; 
	// queries
	User getTopTaggedUser() override; 
	Picture getTopTaggedPicture() override; 
	std::list<Picture> getTaggedPicturesOfUser(const User& user) override; 

	bool open() override;
	void close() override;
	void clear() override;
	

private:
	void init_db();
	void my_exec(const char* sqlStatement);
	void my_exec(const char* sqlStatement, int (*callback)(void*, int, char**, char**));
	int getNumOfTagsInPic(const int id);
	std::list<Picture> getPicturesInAlbum(const Album& album);

	sqlite3* _db;
};