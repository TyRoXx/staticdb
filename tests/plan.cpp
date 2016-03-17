#include <boost/test/unit_test.hpp>
#include <staticdb/plan.hpp>
#include <staticdb/expressions.hpp>
#include <staticdb/bit_sink.hpp>
#include <silicium/sink/iterator_sink.hpp>

BOOST_AUTO_TEST_CASE(trivial_plan)
{
	auto argument = Si::make_unique<staticdb::expressions::expression>(staticdb::expressions::argument());
	auto zero = Si::make_unique<staticdb::expressions::expression>(
	    staticdb::expressions::literal(staticdb::values::make_unsigned_integer(0)));
	auto first =
	    staticdb::expressions::expression(staticdb::expressions::tuple_at(std::move(argument), std::move(zero)));

	staticdb::types::type const root_type = staticdb::types::make_tuple(staticdb::types::make_unsigned_integer(8),
	                                                                    staticdb::types::make_unsigned_integer(8));

	staticdb::memory_storage storage;
	Si::iterator_range<staticdb::get_function const *> gets;
	Si::iterator_range<staticdb::set_function const *> sets;
	auto const planned = staticdb::make_plan<decltype(storage)>(root_type, gets, sets);

	BOOST_CHECK(planned.gets.empty());
	BOOST_CHECK(planned.sets.empty());
}

BOOST_AUTO_TEST_CASE(find_uint_in_array_plan)
{
	namespace expr = staticdb::expressions;
	namespace types = staticdb::types;
	types::type const root_type = types::array(Si::make_unique<types::type>(types::make_unsigned_integer(8)));

	staticdb::memory_storage storage;
	{
		// build an array in the storage
		auto writer = staticdb::make_bits_to_byte_sink(Si::make_container_sink(storage.memory));
		staticdb::values::serialize(writer,
		                            staticdb::values::value(staticdb::values::make_unsigned_integer<std::uint64_t>(3)));
		for (std::uint8_t i = 1; i <= 3; ++i)
		{
			staticdb::values::serialize(writer, staticdb::values::value(staticdb::values::make_unsigned_integer(i)));
		}
	}

	expr::lambda element_equals_key(
	    Si::make_unique<expr::expression>(expr::equals(Si::make_unique<expr::expression>(expr::argument()),
	                                                   Si::make_unique<expr::expression>(expr::bound()))),
	    Si::make_unique<expr::expression>(expr::make_tuple_at(expr::expression(expr::argument()), 1)));
	expr::expression const find_equals(expr::filter(
	    Si::make_unique<staticdb::expressions::expression>(expr::make_tuple_at(expr::expression(expr::argument()), 0)),
	    Si::make_unique<staticdb::expressions::expression>(std::move(element_equals_key))));
	Si::iterator_range<staticdb::get_function const *> gets(&find_equals, &find_equals + 1);
	Si::iterator_range<staticdb::set_function const *> sets;
	staticdb::basic_plan<decltype(storage)> const planned =
	    staticdb::make_plan<decltype(storage)>(root_type, gets, sets);

	BOOST_CHECK_EQUAL(1u, planned.gets.size());
	BOOST_CHECK(planned.sets.empty());

	Si::optional<staticdb::values::value> const found =
	    planned.gets[0](storage, staticdb::values::value(staticdb::values::make_unsigned_integer<std::uint8_t>(2)));
	BOOST_REQUIRE(found);
	std::vector<staticdb::values::value> result_set;
	result_set.emplace_back(staticdb::values::make_unsigned_integer<std::uint8_t>(2));
	BOOST_CHECK_EQUAL(staticdb::values::value(staticdb::values::tuple(std::move(result_set))), *found);
}
