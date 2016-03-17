#include <boost/test/unit_test.hpp>
#include <staticdb/values.hpp>
#include <staticdb/bit_sink.hpp>
#include <staticdb/storage.hpp>
#include <staticdb/expressions.hpp>

BOOST_AUTO_TEST_CASE(tuple_at)
{
	auto argument = Si::make_unique<staticdb::expressions::expression>(staticdb::expressions::argument());
	auto zero = Si::make_unique<staticdb::expressions::expression>(
	    staticdb::expressions::literal(staticdb::values::make_unsigned_integer<std::uint8_t>(0)));
	auto first =
	    staticdb::expressions::expression(staticdb::expressions::tuple_at(std::move(argument), std::move(zero)));

	std::vector<staticdb::values::value> root_elements;
	root_elements.emplace_back(staticdb::values::make_unsigned_integer<std::uint8_t>(23));
	root_elements.emplace_back(staticdb::values::make_unsigned_integer<std::uint8_t>(42));
	staticdb::values::value root = staticdb::values::tuple(std::move(root_elements));

	staticdb::values::value const result =
	    staticdb::expressions::execute(first, root, staticdb::values::value(staticdb::values::unit()));
	staticdb::values::tuple const *const result_tuple = Si::try_get_ptr<staticdb::values::tuple>(result);
	BOOST_REQUIRE(result_tuple);
	Si::optional<std::uint8_t> const parsed_result =
	    staticdb::values::parse_unsigned_integer<std::uint8_t>(*result_tuple);
	BOOST_CHECK_EQUAL(Si::optional<std::uint8_t>(23), parsed_result);
}
