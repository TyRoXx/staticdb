#include <boost/test/unit_test.hpp>
#include <staticdb/plan.hpp>
#include <staticdb/expressions.hpp>

BOOST_AUTO_TEST_CASE(plan)
{
	auto argument = Si::make_unique<staticdb::expressions::expression>(staticdb::expressions::argument());
	auto zero = Si::make_unique<staticdb::expressions::expression>(staticdb::expressions::literal(staticdb::values::make_unsigned_integer(0)));
	auto first = staticdb::expressions::expression(staticdb::expressions::tuple_at(std::move(argument), std::move(zero)));

	staticdb::types::type const root_type = staticdb::types::make_tuple(staticdb::types::make_unsigned_integer(8), staticdb::types::make_unsigned_integer(8));

	staticdb::memory_storage storage;
	Si::iterator_range<staticdb::get_function const *> gets;
	Si::iterator_range<staticdb::set_function const *> sets;
	auto const planned = staticdb::make_plan<decltype(storage)>(root_type, gets, sets);

	BOOST_CHECK(planned.gets.empty());
	BOOST_CHECK(planned.sets.empty());
}
