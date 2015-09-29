#include <boost/test/unit_test.hpp>
#include <staticdb/values.hpp>
#include <staticdb/bit_sink.hpp>
#include <staticdb/storage.hpp>
#include <silicium/sink/iterator_sink.hpp>

BOOST_AUTO_TEST_CASE(serialize_unsigned_integer)
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

namespace
{
	staticdb::types::type const unit_type = staticdb::types::unit();
	staticdb::types::type const bit_type = staticdb::types::bit();
	staticdb::types::type const empty_tuple_type = staticdb::types::tuple(std::vector<staticdb::types::type>());
	staticdb::types::type const tuple_of_bit = staticdb::types::tuple([]{
		std::vector<staticdb::types::type> types;
		types.emplace_back(staticdb::types::bit());
		return types;
	}());
	staticdb::types::type const variant_bit_unit = staticdb::types::variant([]{
		std::vector<staticdb::types::type> types;
		types.emplace_back(staticdb::types::bit());
		types.emplace_back(staticdb::types::unit());
		return types;
	}());
	staticdb::types::type const array_of_bits = staticdb::types::array(Si::make_unique<staticdb::types::type>(staticdb::types::bit()));
}

BOOST_AUTO_TEST_CASE(unit_conforms_to_type)
{
	staticdb::values::value value = staticdb::values::unit();
	BOOST_CHECK(staticdb::values::conforms_to_type(value, unit_type));
	BOOST_CHECK(!staticdb::values::conforms_to_type(value, bit_type));
	BOOST_CHECK(!staticdb::values::conforms_to_type(value, empty_tuple_type));
	BOOST_CHECK(!staticdb::values::conforms_to_type(value, tuple_of_bit));
	BOOST_CHECK(!staticdb::values::conforms_to_type(value, variant_bit_unit));
	BOOST_CHECK(!staticdb::values::conforms_to_type(value, array_of_bits));
}

BOOST_AUTO_TEST_CASE(bit_conforms_to_type)
{
	staticdb::values::value value = staticdb::values::bit(true);
	BOOST_CHECK(staticdb::values::conforms_to_type(value, bit_type));
	BOOST_CHECK(!staticdb::values::conforms_to_type(value, unit_type));
	BOOST_CHECK(!staticdb::values::conforms_to_type(value, empty_tuple_type));
	BOOST_CHECK(!staticdb::values::conforms_to_type(value, tuple_of_bit));
	BOOST_CHECK(!staticdb::values::conforms_to_type(value, variant_bit_unit));
	BOOST_CHECK(!staticdb::values::conforms_to_type(value, array_of_bits));
}

BOOST_AUTO_TEST_CASE(empty_tuple_conforms_to_type)
{
	staticdb::values::value value = staticdb::values::tuple(std::vector<staticdb::values::value>());
	BOOST_CHECK(staticdb::values::conforms_to_type(value, empty_tuple_type));
	BOOST_CHECK(!staticdb::values::conforms_to_type(value, unit_type));
	BOOST_CHECK(!staticdb::values::conforms_to_type(value, bit_type));
	BOOST_CHECK(!staticdb::values::conforms_to_type(value, tuple_of_bit));
	BOOST_CHECK(!staticdb::values::conforms_to_type(value, variant_bit_unit));
	BOOST_CHECK(staticdb::values::conforms_to_type(value, array_of_bits));
}

BOOST_AUTO_TEST_CASE(one_tuple_conforms_to_type)
{
	std::vector<staticdb::values::value> values;
	values.emplace_back(staticdb::values::bit(false));
	staticdb::values::value value = staticdb::values::tuple(std::move(values));
	BOOST_CHECK(staticdb::values::conforms_to_type(value, tuple_of_bit));
	BOOST_CHECK(!staticdb::values::conforms_to_type(value, unit_type));
	BOOST_CHECK(!staticdb::values::conforms_to_type(value, bit_type));
	BOOST_CHECK(!staticdb::values::conforms_to_type(value, empty_tuple_type));
	BOOST_CHECK(!staticdb::values::conforms_to_type(value, variant_bit_unit));
	BOOST_CHECK(staticdb::values::conforms_to_type(value, array_of_bits));
}

BOOST_AUTO_TEST_CASE(variant_conforms_to_type)
{
	staticdb::values::value value = staticdb::values::variant(Si::to_unique(staticdb::values::value(staticdb::values::unit())));
	BOOST_CHECK(staticdb::values::conforms_to_type(value, variant_bit_unit));
	BOOST_CHECK(!staticdb::values::conforms_to_type(value, unit_type));
	BOOST_CHECK(!staticdb::values::conforms_to_type(value, bit_type));
	BOOST_CHECK(!staticdb::values::conforms_to_type(value, empty_tuple_type));
	BOOST_CHECK(!staticdb::values::conforms_to_type(value, tuple_of_bit));
	BOOST_CHECK(!staticdb::values::conforms_to_type(value, array_of_bits));
}

BOOST_AUTO_TEST_CASE(array_conforms_to_type)
{
	std::vector<staticdb::values::value> values;
	values.emplace_back(staticdb::values::bit(false));
	values.emplace_back(staticdb::values::bit(false));
	values.emplace_back(staticdb::values::bit(false));
	values.emplace_back(staticdb::values::bit(false));
	staticdb::values::value value = staticdb::values::tuple(std::move(values));
	BOOST_CHECK(!staticdb::values::conforms_to_type(value, variant_bit_unit));
	BOOST_CHECK(!staticdb::values::conforms_to_type(value, unit_type));
	BOOST_CHECK(!staticdb::values::conforms_to_type(value, bit_type));
	BOOST_CHECK(!staticdb::values::conforms_to_type(value, empty_tuple_type));
	BOOST_CHECK(!staticdb::values::conforms_to_type(value, tuple_of_bit));
	BOOST_CHECK(staticdb::values::conforms_to_type(value, array_of_bits));
}
