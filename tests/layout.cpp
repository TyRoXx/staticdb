#include <boost/test/unit_test.hpp>
#include <staticdb/layout.hpp>
#include <silicium/make_unique.hpp>

BOOST_AUTO_TEST_CASE(calculate_layout_array_of_bitset)
{
	namespace types = staticdb::types;
	namespace layouts = staticdb::layouts;
	types::type const root_type = types::array(Si::make_unique<types::type>(types::make_unsigned_integer(8)));

	layouts::layout const root_layout = layouts::calculate(root_type);
	layouts::layout const expected(layouts::array(Si::make_unique<layouts::layout>(layouts::bitset(8))));
	BOOST_CHECK_EQUAL(expected, root_layout);
}

BOOST_AUTO_TEST_CASE(calculate_layout_array_of_bitset_tuple)
{
	namespace types = staticdb::types;
	namespace layouts = staticdb::layouts;
	types::type const root_type = types::array(Si::make_unique<types::type>(
	    types::make_tuple(types::make_unsigned_integer(4), types::make_unsigned_integer(1))));

	layouts::layout const root_layout = layouts::calculate(root_type);
	layouts::layout const expected(layouts::array(Si::make_unique<layouts::layout>(layouts::bitset(5))));
	BOOST_CHECK_EQUAL(expected, root_layout);
}

BOOST_AUTO_TEST_CASE(calculate_layout_array_of_array_of_bitset_tuple)
{
	namespace types = staticdb::types;
	namespace layouts = staticdb::layouts;
	types::type const root_type = types::array(Si::make_unique<types::type>(types::array(Si::make_unique<types::type>(
	    types::make_tuple(types::make_unsigned_integer(4), types::make_unsigned_integer(1))))));

	layouts::layout const root_layout = layouts::calculate(root_type);
	layouts::layout const expected(layouts::array(
	    Si::make_unique<layouts::layout>(layouts::array(Si::make_unique<layouts::layout>(layouts::bitset(5))))));
	BOOST_CHECK_EQUAL(expected, root_layout);
}
