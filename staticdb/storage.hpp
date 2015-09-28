#ifndef STATICDB_STORAGE_HPP
#define STATICDB_STORAGE_HPP

#include <vector>
#include <staticdb/byte.hpp>

namespace staticdb
{
	struct memory_storage
	{
		std::vector<char> memory;
	};
}

#endif
