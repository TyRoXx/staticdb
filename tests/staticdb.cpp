#include <boost/test/unit_test.hpp>
#include <staticdb/values.hpp>
#include <staticdb/bit_sink.hpp>
#include <staticdb/storage.hpp>
#include <silicium/sink/iterator_sink.hpp>

BOOST_AUTO_TEST_CASE(staticdb_trivial)
{
	staticdb::types::tuple const uint8 = staticdb::types::make_unsigned_integer(8);
	staticdb::types::variant const root = staticdb::types::make_optional(uint8);

	staticdb::values::tuple const my_uint8 = staticdb::values::make_unsigned_integer(static_cast<std::uint8_t>(123));
	staticdb::values::variant const my_root = staticdb::values::make_some(staticdb::values::value(my_uint8.copy()));

	BOOST_CHECK(staticdb::values::conforms_to_type(my_root.copy(), root));

	staticdb::memory_storage storage;
	auto writer = staticdb::make_bits_to_byte_sink(Si::make_container_sink(storage.memory));
	staticdb::values::serialize(writer, my_root.copy());
	assert(writer.buffered_bits() == 0);

	std::vector<std::uint8_t> const expected_storage{123};
	BOOST_CHECK_EQUAL_COLLECTIONS(expected_storage.begin(), expected_storage.end(), storage.memory.begin(), storage.memory.end());
}
