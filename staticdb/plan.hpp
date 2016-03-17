#ifndef STATICDB_PLAN_HPP
#define STATICDB_PLAN_HPP

#include <staticdb/execution.hpp>
#include <staticdb/storage.hpp>
#include <staticdb/layout.hpp>
#include <silicium/to_shared.hpp>
#include <silicium/function.hpp>

namespace staticdb
{
	template <class Storage>
	struct basic_plan
	{
		typedef Storage storage_type;
		typedef Si::function<Si::optional<values::value>(storage_type &, values::value const &)> planned_get_function;
		typedef Si::function<values::value(storage_type &, values::value const &)> planned_set_function;

		std::vector<planned_get_function> gets;
		std::vector<planned_set_function> sets;
		Si::function<void(storage_type &)> initialize_storage;
	};

	typedef expressions::expression get_function;
	typedef expressions::expression set_function;

	inline void analyze_getter(layouts::layout &root, get_function const &get)
	{
		boost::ignore_unused_variable_warning(root);
		boost::ignore_unused_variable_warning(get);
	}

	template <class Storage>
	inline Si::optional<values::value> run_getter(Storage &storage, get_function const &get,
	                                              values::value const &argument, layouts::layout const &root)
	{
		typedef execution::pseudo_value<Storage> pseudo_value;
		return Si::visit<Si::optional<values::value>>(
		    root.as_variant(),
		    [](layouts::unit) -> Si::optional<values::value>
		    {
			    throw std::logic_error("not implemented");
			},
		    [](layouts::tuple const &) -> Si::optional<values::value>
		    {
			    throw std::logic_error("not implemented");
			},
		    [&storage, &get, &argument](layouts::array const &array_layout) -> Si::optional<values::value>
		    {
			    execution::basic_array_accessor<Storage> root_array(execution::storage_pointer<Storage>(storage, 0),
			                                                        array_layout.element->copy());
			    execution::basic_tuple<pseudo_value> get_argument;
			    get_argument.elements.emplace_back(std::move(root_array));
			    get_argument.elements.emplace_back(argument.copy());
			    Si::optional<pseudo_value> const complex_result = execution::execute(
			        get, pseudo_value(std::move(get_argument)), pseudo_value(values::value(values::unit())));
			    if (!complex_result)
			    {
				    return Si::none;
			    }
			    values::value simple_result = execution::reduce_value(*complex_result);
			    return std::move(simple_result);
			},
		    [](layouts::bitset const &) -> Si::optional<values::value>
		    {
			    throw std::logic_error("not implemented");
			},
		    [](layouts::variant const &) -> Si::optional<values::value>
		    {
			    throw std::logic_error("not implemented");
			});
	}

	template <class Storage>
	inline basic_plan<Storage> make_plan(types::type const &root, Si::iterator_range<get_function const *> gets,
	                                     Si::iterator_range<set_function const *> sets)
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
#if !SILICIUM_COMPILER_HAS_EXTENDED_CAPTURE
			auto get_ptr = Si::to_shared(get.copy());
#endif
			result.gets.emplace_back(
			    [
#if SILICIUM_COMPILER_HAS_EXTENDED_CAPTURE
			        get = get.copy()
#else
			        get_ptr
#endif
			            ,
			        root_layout](storage_type &storage, values::value const &argument) -> Si::optional<values::value>
			    {
				    return run_getter(storage,
#if SILICIUM_COMPILER_HAS_EXTENDED_CAPTURE
				                      get,
#else
				                      *get_ptr,
#endif
				                      argument, *root_layout);
				});
		}
		return result;
	}
}

#endif
