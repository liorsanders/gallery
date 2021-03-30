#include "DataBaseAccess.h"

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
		"CREATION_DATE DATE," <<
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
