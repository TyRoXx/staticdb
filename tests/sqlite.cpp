#ifdef SQLITE3_FOUND
#include <boost/test/unit_test.hpp>
#include <staticdb/plan.hpp>
#include <staticdb/expressions.hpp>
#include <staticdb/sqlite.hpp>

BOOST_AUTO_TEST_CASE(sqlite_backend)
{
	namespace expr = staticdb::expressions;
	namespace types = staticdb::types;
	types::type const root_type = types::array(Si::make_unique<types::type>(types::make_unsigned_integer(8)));

	expr::lambda element_equals_key(
	    Si::make_unique<expr::expression>(expr::equals(Si::make_unique<expr::expression>(expr::argument()),
	                                                   Si::make_unique<expr::expression>(expr::bound()))),
	    Si::make_unique<expr::expression>(expr::make_tuple_at(expr::expression(expr::argument()), 1)));
	expr::expression const find_equals(expr::filter(
	    Si::make_unique<staticdb::expressions::expression>(expr::make_tuple_at(expr::expression(expr::argument()), 0)),
	    Si::make_unique<staticdb::expressions::expression>(std::move(element_equals_key))));
	Si::iterator_range<staticdb::get_function const *> gets(&find_equals, &find_equals + 1);

	namespace sq = staticdb::sqlite;

	sqlite3pp::database_handle const db = sqlite3pp::open_or_create(Si::c_string(":memory:")).move_value();
	if (false)
	{
		// TODO
		sq::create_tables(*db, root_type, gets);
	}
}
#endif
