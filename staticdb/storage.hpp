#ifndef STATICDB_STORAGE_HPP
#define STATICDB_STORAGE_HPP

#include <staticdb/values.hpp>
#include <silicium/sink/iterator_sink.hpp>
#include <staticdb/bit_sink.hpp>

namespace staticdb
{
	struct memory_storage
	{
		std::vector<char> memory;
	};

	inline void initialize_storage(memory_storage &storage, values::value const &root)
	{
		auto writer = make_bits_to_byte_sink(Si::make_container_sink(storage.memory));
		values::serialize(writer, root);
		assert(writer.buffered_bits() == 0);
	}
}

#endif
