#ifndef STATICDB_PLAN_HPP
#define STATICDB_PLAN_HPP

#include <staticdb/expressions.hpp>
#include <staticdb/storage.hpp>
#include <staticdb/layout.hpp>
#include <silicium/function.hpp>

namespace staticdb
{
	template <class Storage>
	struct basic_plan
	{
		typedef Storage storage_type;
		typedef Si::function<values::value (storage_type &, values::value const &)> planned_get_function;
		typedef Si::function<values::value (storage_type &, values::value const &)> planned_set_function;

		std::vector<planned_get_function> gets;
		std::vector<planned_set_function> sets;
		Si::function<void (storage_type &)> initialize_storage;
	};

	typedef expressions::expression get_function;
	typedef expressions::expression set_function;

	template <class Storage>
	inline basic_plan<Storage> make_plan(types::type const &root, Si::iterator_range<get_function const *> gets, Si::iterator_range<set_function const *> sets)
	{
		typedef Storage storage_type;
		boost::ignore_unused_variable_warning(sets);
		layouts::layout root_layout = layouts::calculate(root);
		basic_plan<Storage> result;
		for (get_function const &get : gets)
		{
			boost::ignore_unused_variable_warning(get);
			result.gets.emplace_back([](storage_type &, values::value const &) -> values::value
			{
				throw std::logic_error("not implemented");
			});
		}
		return result;
	}
}

#endif
