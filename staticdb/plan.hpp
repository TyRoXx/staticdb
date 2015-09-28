#ifndef STATICDB_PLAN_HPP
#define STATICDB_PLAN_HPP

#include <staticdb/expressions.hpp>
#include <staticdb/storage.hpp>
#include <silicium/function.hpp>

namespace staticdb
{
	template <class Storage>
	struct basic_plan
	{
		typedef Storage storage_type;
		typedef Si::function<values::value (Storage &, values::value const &)> planned_get_function;
		typedef Si::function<values::value (Storage &, values::value const &)> planned_set_function;

		std::vector<planned_get_function> gets;
		std::vector<planned_set_function> sets;
	};

	typedef expressions::expression get_function;
	typedef expressions::expression set_function;

	template <class Storage>
	inline basic_plan<Storage> make_plan(types::type const &root, Si::iterator_range<get_function const *> gets, Si::iterator_range<set_function const *> sets)
	{
		boost::ignore_unused_variable_warning(root);
		boost::ignore_unused_variable_warning(gets);
		boost::ignore_unused_variable_warning(sets);
		return basic_plan<Storage>(); //TODO
	}
}

#endif
