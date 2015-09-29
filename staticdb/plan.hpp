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

	inline void analyze_getter(layouts::layout &root, get_function const &get)
	{
		boost::ignore_unused_variable_warning(root);
		boost::ignore_unused_variable_warning(get);
	}

	template <class Storage>
	inline values::value run_getter(Storage &storage, get_function const &get, values::value const &argument, layouts::layout const &root)
	{
		boost::ignore_unused_variable_warning(storage);
		boost::ignore_unused_variable_warning(get);
		boost::ignore_unused_variable_warning(argument);
		boost::ignore_unused_variable_warning(root);
		return values::value(values::unit()); //TODO
	}

	template <class Storage>
	inline basic_plan<Storage> make_plan(types::type const &root, Si::iterator_range<get_function const *> gets, Si::iterator_range<set_function const *> sets)
	{
		typedef Storage storage_type;
		boost::ignore_unused_variable_warning(sets);
		std::shared_ptr<layouts::layout> root_layout = Si::to_shared(layouts::calculate(root));
		basic_plan<Storage> result;
		for (get_function const &get : gets)
		{
			analyze_getter(*root_layout, get);
		}
		for (get_function const &get : gets)
		{
			result.gets.emplace_back([get = get.copy(), root_layout](storage_type &storage, values::value const &argument) -> values::value
			{
				return run_getter(storage, get, argument, *root_layout);
			});
		}
		return result;
	}
}

#endif
