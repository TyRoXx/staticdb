#include <boost/test/unit_test.hpp>
#include <staticdb/layout.hpp>

BOOST_AUTO_TEST_CASE(simple_layout)
{
	namespace types = staticdb::types;
	types::type const root_type = types::array(Si::make_unique<types::type>(types::make_unsigned_integer(8)));

	staticdb::layouts::layout const root_layout = staticdb::layouts::calculate(root_type);
	BOOST_CHECK(nullptr != Si::try_get_ptr<staticdb::layouts::array>(root_layout));
}
