#include "DataBaseAccess.h"
#include "ItemNotFoundException.h"
#include <map>

//get cleared before every query
std::list<Album> albums; 
std::list<User> users;
std::list<Picture> pictures;

int albums_callback(void* data, int argc, char** argv, char** azColName)
{
	Album album;
	for (int i = 0; i < argc; i++) {
		if (std::string(azColName[i]) == "CREATION_DATE") {
			album.setCreationDate(argv[i]);
		}
		else if (std::string(azColName[i]) == "NAME") {
			album.setName(argv[i]);
		}
		else if (std::string(azColName[i]) == "USER_ID") {
			album.setOwner(std::stoi(argv[i]));
		}
		else if (std::string(azColName[i]) == "ID") {
			album.setId(std::stoi(argv[i]));
		}
	}
	albums.push_back(album);
	return 0;
}

int users_callback(void* data, int argc, char** argv, char** azColName)
{
	User user;
	for (int i = 0; i < argc; i++) {
		if (std::string(azColName[i]) == "ID") {
			user.setId(std::stoi(argv[i]));
		}
		else if (std::string(azColName[i]) == "NAME") {
			user.setName(argv[i]);
		}
	}
	users.push_back(user);
	return 0;
}

int pictures_callback(void* data, int argc, char** argv, char** azColName)
{
	Picture pic;
	for (int i = 0; i < argc; i++) {
		if (std::string(azColName[i]) == "ID") {
			pic.setId(std::stoi(argv[i]));
		}
		else if (std::string(azColName[i]) == "CREATION_DATE") {
			pic.setCreationDate(argv[i]);
		}
		else if (std::string(azColName[i]) == "NAME") {
			pic.setName(argv[i]);
		}
		else if (std::string(azColName[i]) == "LOCATION") {
			pic.setPath(argv[i]);
		}
		
	}
	pictures.push_back(pic);
	return 0;
}

int tags_callback(void* data, int argc, char** argv, char** azColName) {
	Picture pic;
	User user;
	for (int i = 0; i < argc; i++) {
		if (std::string(azColName[i]) == "PICTURE_ID") {
			pic.setId(std::stoi(argv[i]));
		}
		else if (std::string(azColName[i]) == "USER_ID") {
			user.setId(std::stoi(argv[i]));
		}
	}
	users.push_back(user);
	pictures.push_back(pic);
	return 0;
}

Picture DatabaseAccess::getTopTaggedPicture()
{
	int currentMax = -1;
	const Picture* mostTaggedPic = nullptr;
	for (const auto& album : getAlbums()) {
		for (const Picture& picture : album.getPictures()) {
			int tagsCount = picture.getTagsCount();
			if (tagsCount == 0) {
				continue;
			}

			if (tagsCount <= currentMax) {
				continue;
			}

			mostTaggedPic = &picture;
			currentMax = tagsCount;
		}
	}
	if (nullptr == mostTaggedPic) {
		throw MyException("There isn't any tagged picture.");
	}

	return *mostTaggedPic;
}

User DatabaseAccess::getTopTaggedUser()
{
	std::map<int, int> userTagsCountMap;
	auto all_albums = getAlbums();
	auto albumsIter = all_albums.begin();
	for (const auto& album : all_albums) {
		for (const auto& picture : album.getPictures()) {

			const std::set<int>& userTags = picture.getUserTags();
			for (const auto& user : userTags) {
				//As map creates default constructed values, 
				//users which we haven't yet encountered will start from 0
				userTagsCountMap[user]++;
			}
		}
	}

	if (userTagsCountMap.size() == 0) {
		throw MyException("There isn't any tagged user.");
	}

	int topTaggedUser = -1;
	int currentMax = -1;
	for (auto entry : userTagsCountMap) {
		if (entry.second < currentMax) {
			continue;
		}

		topTaggedUser = entry.first;
		currentMax = entry.second;
	}

	if (-1 == topTaggedUser) {
		throw MyException("Failed to find most tagged user");
	}

	return getUser(topTaggedUser);
}



std::list<Picture> DatabaseAccess::getTaggedPicturesOfUser(const User& user)
{
	pictures.clear();
	std::string statement = "SELECT * FROM pictures WHERE id IN (SELECT picture_id FROM tags WHERE user_id = ";
	statement = statement + std::to_string(user.getId()) + ");";
	my_exec(statement.c_str(), pictures_callback);
	return pictures;
}

bool DatabaseAccess::open()
{
	int doesFileExist = _access(DB_NAME, 0);
	//open the db
	int res = sqlite3_open(DB_NAME, &_db);
	if (res != SQLITE_OK) {
		_db = nullptr;
		throw std::runtime_error(std::string("failed to open: ") + DB_NAME);
	}
	//init the db
	if (doesFileExist != 0) {
		try {
			init_db();
		}
		catch (std::runtime_error& e) {
			std::cout << e.what() << std::endl;
			return false;
		}
	}
	return true;
}

void DatabaseAccess::close()
{
	sqlite3_close(_db);
	_db = nullptr;
}

void DatabaseAccess::clear()
{
	pictures.clear();
	users.clear();
	albums.clear();
}

void DatabaseAccess::init_db()
{
	std::string statement;
	//create the users table
	statement = statement + "CREATE TABLE IF NOT EXISTS USERS (" +
		"ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," +
		"NAME TEXT NOT NULL);";
	my_exec(statement.c_str());
	//create the albums table
	statement = std::string("CREATE TABLE IF NOT EXISTS ALBUMS (") +
		"ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," +
		"NAME TEXT NOT NULL," +
		"CREATION_DATE DATE," +
		"USER_ID INTEGER NULL," +
		"FOREIGN KEY(USER_ID) REFERENCES USERS(ID));";
	my_exec(statement.c_str());
	//create pictures table
	statement = std::string("CREATE TABLE IF NOT EXISTS PICTURES (") +
		"ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," +
		"NAME TEXT NOT NULL," +
		"LOCATION TEXT NOT NULL," +
		"CREATION_DATE TEXT NOT NULL," +
		"ALBUM_ID INTEGER NULL," +
		"FOREIGN KEY(ALBUM_ID) REFERENCES ALBUMS(ID));";
	my_exec(statement.c_str());
	//create tags table
	statement = std::string("CREATE TABLE IF NOT EXISTS TAGS (") +
		"ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," +
		"USER_ID INTEGER NULL," +
		"PICTURE_ID INTEGER NULL," +
		"FOREIGN KEY(USER_ID) REFERENCES USERS(ID)," +
		"FOREIGN KEY(PICTURE_ID) REFERENCES PICTURES(ID));";
	my_exec(statement.c_str());

}

void DatabaseAccess::my_exec(const char* sqlStatement)
{
	std::cout << "executing: " << sqlStatement << std::endl; //for debuging
	char* errMessage = nullptr;
	int res = sqlite3_exec(_db, sqlStatement, nullptr, nullptr, &errMessage);
	if (res != SQLITE_OK) {
		throw std::runtime_error(errMessage);
	}
}

void DatabaseAccess::my_exec(const char* sqlStatement, int(*callback)(void*, int, char**, char**))
{
	std::cout << "executing " << sqlStatement << std::endl; //for debuging
	char* errMessage = nullptr;
	int res = sqlite3_exec(_db, sqlStatement, callback, nullptr, &errMessage);
	if (res != SQLITE_OK) {
		throw std::runtime_error(errMessage);
	}
}

std::list<int> DatabaseAccess::getTagsInPic(const int id)
{
	std::list<int> res;
	pictures.clear();
	users.clear();
	std::string statement("SELECT * FROM tags WHERE picture_id = ");
	statement = statement + std::to_string(id) + ";";
	for (const auto& u : users) {
		res.push_back(u.getId());
	}
	return res;
}

std::list<Picture> DatabaseAccess::getPicturesInAlbum(const Album& album)
{
	pictures.clear();
	std::string statement("SELECT * FROM pictures WHERE album_id = ");
	statement = statement + std::to_string(album.getId()) + ";";
	my_exec(statement.c_str(), pictures_callback);
	return pictures;
}


void DatabaseAccess::printAlbums()
{
	auto all_albums = getAlbums();
	if (all_albums.empty()) {
		throw MyException("There are no existing albums.");
	}
	std::cout << "Album list:" << std::endl;
	std::cout << "-----------" << std::endl;
	for (const Album& album : all_albums) {
		std::cout << std::setw(5) << "* " << album <<
			"   * created on " << album.getCreationDate() << std::endl;
	}
}

void DatabaseAccess::addPictureToAlbumByName(const std::string& albumName, const Picture& picture)
{
	//insert into pictures (id, name, location, creation_date, album_id)
	std::string statement;
	statement = std::string("INSERT INTO pictures (id, name, location, creation_date, album_id) VALUES (") +
		std::to_string(picture.getId()) + ", '" + picture.getName() + "', '" + picture.getPath() + "', '" +
		picture.getCreationDate() + "', " + "(SELECT id FROM albums where NAME = '" + albumName + "'));";
	my_exec(statement.c_str());
}


void DatabaseAccess::tagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
	std::string statement;
	statement = std::string("INSERT INTO tags (picture_id, user_id) VALUES((SELECT id FROM pictures WHERE name = '") +
		pictureName + "'), " + std::to_string(userId) + ");";
	my_exec(statement.c_str());
}

void DatabaseAccess::untagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
	std::string statement = "DELETE FROM tags WHERE user_id = " + std::to_string(userId) + ';';
	my_exec(statement.c_str());
}

void DatabaseAccess::printUsers()
{
	users.clear();
	my_exec("SELECT * FROM users;", users_callback);
	std::cout << "Users list:" << std::endl;
	std::cout << "-----------" << std::endl;
	for (const auto& user : users) {
		std::cout << user << std::endl;
	}
}

void DatabaseAccess::createUser(User& user)
{
	std::string statement;
	statement = std::string("INSERT INTO users (id, name) VALUES (") +
		std::to_string(user.getId()) + ", '" + user.getName() + "');";
	my_exec(statement.c_str());
}

void DatabaseAccess::deleteUser(const User& user)
{
	//delete tags, albums and then user
	/*first delete user tags*/
	std::string statement;
	statement = std::string("DELETE FROM tags WHERE user_id = ") + std::to_string(user.getId()) + ";";
	my_exec(statement.c_str());
	/* next delete pictures in album of user*/
	statement = std::string("DELETE FROM pictures WHERE album_id IN (SELECT id FROM albums WHERE user_id = ") +
		std::to_string(user.getId()) + ");";
	my_exec(statement.c_str());
	/*next delete albums owned by user*/
	statement = std::string("DELETE FROM albums WHERE user_id = ") + std::to_string(user.getId()) + ";";
	my_exec(statement.c_str());
	/*next delete user from users*/
	statement = std::string("DELETE FROM users WHERE id = ") + std::to_string(user.getId()) + ";";
	my_exec(statement.c_str());
}

bool DatabaseAccess::doesUserExists(int userId)
{
	users.clear();
	std::string statement = "SELECT * FROM users WHERE id = " + std::to_string(userId) + ';';
	my_exec(statement.c_str(), users_callback);
	return !users.empty();
}

User DatabaseAccess::getUser(int userId)
{
	users.clear();
	std::string statement("SELECT id, name FROM users WHERE id = ");
	statement = statement + std::to_string(userId) + ";";
	my_exec(statement.c_str(), users_callback);
	if (users.empty()) {
		throw ItemNotFoundException("User", userId);
	}
	return users.front();
}

int DatabaseAccess::countAlbumsOwnedOfUser(const User& user)
{
	albums.clear();
	std::string statement("SELECT * FROM albums WHERE user_id = ");
	statement = statement + std::to_string(user.getId()) + ";";
	my_exec(statement.c_str(), albums_callback);
	return albums.size();
}

int DatabaseAccess::countAlbumsTaggedOfUser(const User& user)
{
	int res = 0; bool flag = false;
	pictures.clear();
	std::string statement("SELECT picture_id FROM tags WHERE user_id = ");
	statement = statement + std::to_string(user.getId()) + ";";
	my_exec(statement.c_str(), tags_callback);
	for (const auto& album : getAlbums()) {
		//go over pictures that user is tagged in
		for (const auto& pic : pictures) {
			if (album.doesPictureExists(pic.getName()) && !flag) {
				flag = true;
				res++;
			}
		}
		flag = false;
	}
	return res;
}

int DatabaseAccess::countTagsOfUser(const User& user)
{
	users.clear();
	std::string statement("SELECT user_id FROM tags WHERE user_id = ");
	statement = statement + std::to_string(user.getId()) + ";";
	return users.size();
}

const std::list<Album> DatabaseAccess::getAlbums()
{
	albums.clear();
	std::string statement("SELECT * FROM albums;");
	my_exec(statement.c_str(), albums_callback);
	for (auto it = albums.begin(); it != albums.end(); ++it) {
		auto pics = getPicturesInAlbum(*it);
		for (auto& p : pics) {
			for (int val : getTagsInPic(p.getId())) {
				p.tagUser(val);
			}
		}
		it->setPictures(pics);
	}
	return albums;
}

const std::list<Album> DatabaseAccess::getAlbumsOfUser(const User& user)
{
	albums.clear();
	std::string statement("SELECT creation_date, name, user_id FROM albums WHERE user_id = ");
	statement = statement + std::to_string(user.getId()) + ";";
	my_exec(statement.c_str(), albums_callback);
	return albums;
}

void DatabaseAccess::createAlbum(const Album& album)
{
	std::string statement = "INSERT INTO albums (name, creation_date, user_id) VALUES('";
	statement = statement + album.getName() + "', '" + album.getCreationDate() + "', " + std::to_string(album.getOwnerId()) + ");";
	my_exec(statement.c_str());
}

void DatabaseAccess::deleteAlbum(const std::string& albumName, int userId)
{
	//delete all the pictures in the album 
	std::string statement = "DELETE FROM pictures WHERE album_id = (SELECT id FROM albums where NAME = '" + albumName + "');";
	my_exec(statement.c_str());
	//now delete the album
	statement = "DELETE FROM albums WHERE name = '" + albumName + "';";
	my_exec(statement.c_str());
}

bool DatabaseAccess::doesAlbumExists(const std::string& albumName, int userId)
{
	albums.clear();
	std::string statement = "SELECT name, user_id FROM albums WHERE name = '";
	statement = statement + albumName + "' AND user_id = " + std::to_string(userId) + ';';
	my_exec(statement.c_str(), albums_callback);
	return !albums.empty();
}

Album DatabaseAccess::openAlbum(const std::string& albumName)
{
	albums.clear();
	std::string statement("SELECT * FROM albums WHERE name = '");
	statement = statement + albumName + "';";
	my_exec(statement.c_str(), albums_callback);
	auto pics = getPicturesInAlbum(albums.front());
	for (auto& p : pics) {
		for (int val : getTagsInPic(p.getId())) {
			p.tagUser(val);
		}
	}
	albums.front().setPictures(pics);
	return albums.front();
}

void DatabaseAccess::removePictureFromAlbumByName(const std::string& albumName, const std::string& pictureName)
{
	//first remove tags
	std::string statement;
	statement = statement + "DELETE FROM tags WHERE picture_id IN (SELECT id FROM pictures WHERE name = '" +
		pictureName + "');";
	my_exec(statement.c_str());
	//then pictures
	statement = std::string("DELETE FROM pictures WHERE name = '") + pictureName + "';";
	my_exec(statement.c_str());
}

