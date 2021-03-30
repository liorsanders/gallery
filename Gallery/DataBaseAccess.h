#include "IDataAccess.h"

constexpr auto DB_NAME = "galleryDB.sqlite";

class DatabaseAccess : public IDataAccess {
public:
	bool open() override;
	void close() override {};
	void clear() override;

private:

};