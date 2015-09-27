#include <boost/test/unit_test.hpp>
#include <staticdb/staticdb.hpp>

BOOST_AUTO_TEST_CASE(staticdb_trivial)
{
	staticdb::types::tuple const uint8 = staticdb::types::make_unsigned_integer(8);
	staticdb::types::variant const root = staticdb::types::make_optional(uint8);

	staticdb::values::tuple const my_uint8 = staticdb::values::make_unsigned_integer(static_cast<std::uint8_t>(123));
	staticdb::values::variant const my_root = staticdb::values::make_some(staticdb::values::value(my_uint8.copy()));

	BOOST_CHECK(staticdb::values::conforms_to_type(my_root.copy(), root));

	staticdb::memory_storage storage;
	staticdb::initialize_storage(storage, my_root.copy());

	std::string const expected_storage = "nyyyynyy";
	BOOST_CHECK_EQUAL_COLLECTIONS(expected_storage.begin(), expected_storage.end(), storage.memory.begin(), storage.memory.end());
}
