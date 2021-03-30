#include "IDataAccess.h"
#include "sqlite3.h"
#include <exception>
#include <sstream>
#include <io.h>

constexpr auto DB_NAME = "galleryDB.sqlite";

class DatabaseAccess : public IDataAccess {
public:
	bool open() override;
	void close() override;
	void clear() override {};

private:
	void init_db();
	void my_exec(const char* sqlStatement);

	sqlite3* _db;
};