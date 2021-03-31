#include "DataBaseAccess.h"

std::list<Album> albums; //get cleared before every query
std::list<User> users;
std::list<Picture> pictures;

int albums_callback(void* data, int argc, char** argv, char** azColName)
{
	Album album;
	for (int i = 0; i < argc; i++) {
		if (std::string(azColName[i]) == album_field::CREATION_DATE) {
			album.setCreationDate(argv[i]);
		}
		else if (std::string(azColName[i]) == album_field::NAME) {
			album.setName(argv[i]);
		}
		else if (std::string(azColName[i]) == album_field::USER_ID) {
			album.setOwner(std::stoi(argv[i]));
		}
	}
	albums.push_back(album);
	return 0;
}

int users_callback(void* data, int argc, char** argv, char** azColName)
{
	User user;
	for (int i = 0; i < argc; i++) {
		if (std::string(azColName[i]) == user_field::ID) {
			user.setId(std::stoi(argv[i]));
		}
		else if (std::string(azColName[i]) == user_field::NAME) {
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
		if (std::string(azColName[i]) == picture_field::ID) {
			pic.setId(std::stoi(argv[i]));
		}
		else if (std::string(azColName[i]) == picture_field::CREATION_DATE) {
			pic.setCreationDate(argv[i]);
		}
		else if (std::string(azColName[i]) == picture_field::NAME) {
			pic.setName(argv[i]);
		}
	}
	pictures.push_back(pic);
	return 0;
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

void DatabaseAccess::init_db()
{
	std::stringstream statement;
	//create the users table
	statement << "CREATE TABLE IF NOT EXISTS USERS (" <<
		"ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," <<
		"NAME TEXT NOT NULL);";
	my_exec(statement.str().c_str());
	statement.str(std::string());
	//create the albums table
	statement << "CREATE TABLE IF NOT EXISTS ALBUMS (" <<
		"ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," <<
		"NAME TEXT NOT NULL," <<
		"CREATION_DATE DATE," <<
		"USER_ID INTEGER NULL," <<
		"FOREIGN KEY(USER_ID) REFERENCES USERS(ID));";
	my_exec(statement.str().c_str());
	statement.str(std::string());
	//create pictures table
	statement << "CREATE TABLE IF NOT EXISTS PICTURES (" <<
		"ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," <<
		"NAME TEXT NOT NULL," <<
		"LOCATION TEXT NOT NULL," <<
		"CREATION_DATE TEXT NOT NULL," <<
		"ALBUM_ID INTEGER NULL," <<
		"FOREIGN KEY(ALBUM_ID) REFERENCES ALBUMS(ID));";
	my_exec(statement.str().c_str());
	statement.str(std::string());
	//create tags table
	statement << "CREATE TABLE IF NOT EXISTS TAGS (" <<
		"ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," <<
		"USER_ID INTEGER NULL," <<
		"PICTURE_ID INTEGER NULL," <<
		"FOREIGN KEY(USER_ID) REFERENCES USERS(ID)," <<
		"FOREIGN KEY(PICTURE_ID) REFERENCES PICTURES(ID));";
	my_exec(statement.str().c_str());

}

void DatabaseAccess::my_exec(const char* sqlStatement)
{
	char* errMessage = nullptr;
	int res = sqlite3_exec(_db, sqlStatement, nullptr, nullptr, &errMessage);
	if (res != SQLITE_OK) {
		throw std::runtime_error(errMessage);
	}
}

void DatabaseAccess::my_exec(const char* sqlStatement, int(*callback)(void*, int, char**, char**))
{
	char* errMessage = nullptr;
	int res = sqlite3_exec(_db, sqlStatement, callback, nullptr, &errMessage);
	if (res != SQLITE_OK) {
		throw std::runtime_error(errMessage);
	}
}

void DatabaseAccess::addPictureToAlbumByName(const std::string& albumName, const Picture& picture)
{
	//insert into pictures (id, name, location, creation_date, album_id)
	std::stringstream statement;
	statement << "INSERT INTO pictures (id, name, location, creation_date, album_id) VALUES (" <<
		picture.getId() << ", '" << picture.getName() << "', '" << picture.getPath() << "', '" <<
		picture.getCreationDate() << "', " << "(SELECT id FROM albums where NAME = '" << albumName << "'));";
	my_exec(statement.str().c_str());
}


void DatabaseAccess::tagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
	std::stringstream statement;
	statement << "INSERT INTO tags (picture_id, user_id) VALUES((SELECT id FROM pictures WHERE name = '"
		<< pictureName << "'), " << userId << ");";
	my_exec(statement.str().c_str());
}

void DatabaseAccess::untagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
	std::string statement = "DELETE FROM tags WHERE user_id = " + std::to_string(userId) + ';';
	my_exec(statement.c_str());
}

void DatabaseAccess::createUser(User& user)
{
	std::stringstream statement;
	statement << "INSERT INTO users (id, name) VALUES (" <<
		std::to_string(user.getId()) << ", '" << user.getName() << "');";
	my_exec(statement.str().c_str());
}

void DatabaseAccess::deleteUser(const User& user)
{
	//delete tags, albums and then user
	/*first delete user tags*/
	std::stringstream statement;
	statement << "DELETE FROM tags WHERE user_id = " << user.getId() << ";";
	my_exec(statement.str().c_str());
	statement.str(std::string());
	/* next delete pictures in album of user*/
	statement << "DELETE FROM pictures WHERE album_id IN (SELECT id FROM albums WHERE user_id = " << 
		user.getId() << ");";
	my_exec(statement.str().c_str());
	statement.str(std::string());
	/*next delete albums owned by user*/
	statement << "DELETE FROM albums WHERE user_id = " << user.getId() << ";";
	my_exec(statement.str().c_str());
	statement.str(std::string());
	/*next delete user from users*/
	statement << "DELETE FROM users WHERE id = " << user.getId() << ";";
	my_exec(statement.str().c_str());
}

bool DatabaseAccess::doesUserExists(int userId)
{
	users.clear();
	std::stringstream statement("SELECT * FROM users WHERE id = ");
	statement << userId << ";";
	my_exec(statement.str().c_str(), users_callback);
	return !users.empty();
}

User DatabaseAccess::getUser(int userId)
{
	users.clear();
	std::stringstream statement("SELECT id, name FROM users WHERE id = ");
	statement << userId << ";";
	my_exec(statement.str().c_str(), users_callback);
	return users.front();
}

int DatabaseAccess::countAlbumsOwnedOfUser(const User& user)
{
	albums.clear();
	std::stringstream statement("SELECT * FROM albums WHERE user_id = ");
	statement << user.getId() << ";";
	my_exec(statement.str().c_str(), albums_callback);
	return albums.size();
}

int DatabaseAccess::countAlbumsTaggedOfUser(const User& user)
{
	//TODO
}

int DatabaseAccess::countTagsOfUser(const User& user)
{
	//TODO
}

const std::list<Album> DatabaseAccess::getAlbums()
{
	//TODO
}

const std::list<Album> DatabaseAccess::getAlbumsOfUser(const User& user)
{
	albums.clear();
	std::stringstream statement("SELECT creation_date, name, user_id FROM albums WHERE user_id = ");
	statement << user.getId() << ";";
	my_exec(statement.str().c_str(), albums_callback);
	return albums;
}

void DatabaseAccess::createAlbum(const Album& album)
{
	std::stringstream statement("INSERT INTO albums (name, creation_date, user_id) VALUES('");
	statement << album.getName() << "', '" << album.getCreationDate() << "', " << album.getOwnerId() << ");";
	my_exec(statement.str().c_str());
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
	std::stringstream statement("SELECT name, user_id FROM albums WHERE name = '");
	statement << albumName << "' AND user_id = " << userId << ";";
	my_exec(statement.str().c_str(), albums_callback);
	return !albums.empty();
}

Album DatabaseAccess::openAlbum(const std::string& albumName)
{
	albums.clear();
	std::stringstream statement("SELECT * FROM albums WHERE name = '");
	statement << albumName << "';";
	my_exec(statement.str().c_str(), albums_callback);
	return albums.front();
}

void DatabaseAccess::removePictureFromAlbumByName(const std::string& albumName, const std::string& pictureName)
{
	//first remove tags
	std::stringstream statement;
	statement << "DELETE FROM tags WHERE picture_id IN (SELECT id FROM pictures WHERE name = '" <<
		pictureName << "');";
	my_exec(statement.str().c_str());
	//then pictures
	statement.str(std::string());
	statement << "DELETE FROM pictures WHERE name = '" << pictureName << "';";
	my_exec(statement.str().c_str());
}


