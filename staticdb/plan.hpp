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

	namespace detail
	{
		template <class Storage>
		struct storage_pointer
		{
			Storage *storage;
			address where;

			explicit storage_pointer(Storage &storage, address where)
				: storage(&storage)
				, where(where)
			{
			}
		};

		template <class Storage>
		struct basic_array_accessor
		{
			storage_pointer<Storage> begin;

			explicit basic_array_accessor(storage_pointer<Storage> begin)
				: begin(begin)
			{
			}

			basic_array_accessor copy() const
			{
				return *this;
			}
		};

		template <class PseudoValue>
		struct basic_tuple
		{
			std::vector<PseudoValue> elements;

			basic_tuple()
			{
			}

			basic_tuple copy() const
			{
				basic_tuple result;
				result.elements = expressions::detail::copy(elements);
				return result;
			}

#if SILICIUM_COMPILER_GENERATES_MOVES
			SILICIUM_DEFAULT_MOVE(basic_tuple)
#else
			basic_tuple(basic_tuple &&other) BOOST_NOEXCEPT
				: elements(std::move(other.elements))
			{
			}

			basic_tuple &operator = (basic_tuple &&other) BOOST_NOEXCEPT
			{
				elements = std::move(other.elements);
				return *this;
			}
#endif
				SILICIUM_DISABLE_COPY(basic_tuple)
		};

		template <class PseudoValue>
		struct basic_closure
		{
			std::unique_ptr<expressions::expression> body;
			std::unique_ptr<PseudoValue> bound;

			basic_closure()
			{
			}

			basic_closure copy() const
			{
				basic_closure result;
				result.body = Si::to_unique(body->copy());
				result.bound = Si::to_unique(bound->copy());
				return result;
			}

#if SILICIUM_COMPILER_GENERATES_MOVES
			SILICIUM_DEFAULT_MOVE(basic_closure)
#else
			basic_closure(basic_closure &&other) BOOST_NOEXCEPT
				: body(std::move(other.body))
				, bound(std::move(other.bound))
			{
			}

			basic_closure &operator = (basic_closure &&other) BOOST_NOEXCEPT
			{
				body = std::move(other.body);
				bound = std::move(other.bound);
				return *this;
			}
#endif
			SILICIUM_DISABLE_COPY(basic_closure)
		};

		template <class Storage>
		struct pseudo_value : Si::non_copyable_variant<
			values::value,
			basic_array_accessor<Storage>,
			basic_tuple<pseudo_value<Storage>>,
			basic_closure<pseudo_value<Storage>>
		>
		{
			typedef Si::non_copyable_variant<
				values::value,
				basic_array_accessor<Storage>,
				basic_tuple<pseudo_value<Storage>>,
				basic_closure<pseudo_value<Storage>>
			> base;

			template <class A0, class = std::enable_if<!std::is_same<std::decay<A0>::type, pseudo_value<Storage>>::value, void>::type>
			pseudo_value(A0 &&a0)
				: base(std::forward<A0>(a0))
			{
			}

			base &as_variant()
			{
				return *this;
			}

			base const &as_variant() const
			{
				return *this;
			}

			pseudo_value copy() const
			{
				return as_variant().apply_visitor(values::detail::copying_visitor<pseudo_value>());
			}

#if SILICIUM_COMPILER_GENERATES_MOVES
			SILICIUM_DEFAULT_MOVE(pseudo_value)
#else
			pseudo_value(pseudo_value &&other) BOOST_NOEXCEPT
				: base(std::move(other.as_variant()))
			{
			}

			pseudo_value &operator = (pseudo_value &&other) BOOST_NOEXCEPT
			{
				as_variant() = std::move(other.as_variant());
				return *this;
			}
#endif
			SILICIUM_DISABLE_COPY(pseudo_value)
		};

		template <class Storage>
		address extract_address(pseudo_value<Storage> const &address_value)
		{
			return Si::visit<address>(
				address_value,
				[](basic_array_accessor<Storage> const &) -> address
				{
					throw std::invalid_argument("extract_address called on an array");
				},
				[](basic_tuple<pseudo_value<Storage>> const &indirect_tuple) -> address
				{
					boost::ignore_unused_variable_warning(indirect_tuple);
					throw std::logic_error("not implemented");
				},
				[](basic_closure<pseudo_value<Storage>> const &) -> address
				{
					throw std::invalid_argument("extract_address called on a closure");
				},
				[](values::value const &direct_value) -> address
				{
					values::tuple const * const direct_tuple = Si::try_get_ptr<values::tuple>(direct_value.as_variant());
					if (!direct_tuple)
					{
						throw std::invalid_argument("extract_address called on not-a-tuple");
					}
					Si::optional<address> parsed = values::parse_unsigned_integer<address>(*direct_tuple);
					if (!parsed)
					{
						throw std::invalid_argument("extract_address called with non-bitset or too long tuple");
					}
					return *parsed;
				}
			);
		}

		template <class Storage>
		pseudo_value<Storage> tuple_at(pseudo_value<Storage> const &tuple, pseudo_value<Storage> const &index)
		{
			address index_int = extract_address(index);
			return Si::visit<pseudo_value<Storage>>(
				tuple,
				[](basic_array_accessor<Storage> const &) -> pseudo_value<Storage>
				{
					throw std::invalid_argument("tuple_at called on an array");
				},
				[index_int](basic_tuple<pseudo_value<Storage>> const &indirect_tuple) -> pseudo_value<Storage>
				{
					if (index_int >= indirect_tuple.elements.size())
					{
						throw std::invalid_argument("tuple_at called with index out of range");
					}
					return indirect_tuple.elements[static_cast<size_t>(index_int)].copy();
				},
				[](basic_closure<pseudo_value<Storage>> const &) -> pseudo_value<Storage>
				{
					throw std::invalid_argument("tuple_at called on a closure");
				},
				[index_int](values::value const &direct_value) -> pseudo_value<Storage>
				{
					values::tuple const * const direct_tuple = Si::try_get_ptr<values::tuple>(direct_value.as_variant());
					if (!direct_tuple)
					{
						throw std::invalid_argument("tuple_at called on not-a-tuple");
					}
					if (index_int >= direct_tuple->elements.size())
					{
						throw std::invalid_argument("tuple_at called with index out of range");
					}
					return pseudo_value<Storage>(direct_tuple->elements[static_cast<size_t>(index_int)].copy());
				}
			);
		}

		template <class Storage>
		pseudo_value<Storage> execute(
			expressions::expression const &program,
			pseudo_value<Storage> const &argument_,
			pseudo_value<Storage> const &bound_)
		{
			typedef pseudo_value<Storage> value_type;
			return Si::visit<value_type>(
				program,
				[](expressions::literal const &literal_) -> value_type
				{
					return value_type(literal_.value.copy());
				},
				[&argument_](expressions::argument) -> value_type
				{
					return argument_.copy();
				},
				[&bound_](expressions::bound) -> value_type
				{
					return bound_.copy();
				},
				[&argument_, &bound_](expressions::make_tuple const &make_tuple_) -> value_type
				{
					basic_tuple<value_type> result;
					result.elements.reserve(make_tuple_.elements.size());
					for (expressions::expression const &element : make_tuple_.elements)
					{
						result.elements.emplace_back(execute(element, argument_, bound_));
					}
					return value_type(std::move(result));
				},
				[&argument_, &bound_](expressions::tuple_at const &tuple_at_) -> value_type
				{
					value_type const tuple_ = execute(*tuple_at_.tuple, argument_, bound_);
					value_type const index = execute(*tuple_at_.index, argument_, bound_);
					value_type element = tuple_at(tuple_, index);
					return element;
				},
				[&argument_](expressions::branch const &) -> value_type
				{
					throw std::logic_error("not implemented");
				},
				[&argument_, &bound_](expressions::lambda const &lambda_) -> value_type
				{
					basic_closure<value_type> closure;
					closure.body = Si::to_unique(lambda_.body->copy());
					closure.bound = Si::to_unique(execute(*lambda_.bound, argument_, bound_));
					return value_type(std::move(closure));
				},
				[](expressions::call const &) -> value_type
				{
					throw std::logic_error("not implemented");
				},
				[&argument_, &bound_](expressions::filter const &filter_) -> value_type
				{
					value_type const input = execute(*filter_.input, argument_, bound_);
					value_type const predicate = execute(*filter_.predicate, argument_, bound_);
					throw std::logic_error("not implemented");
				},
				[](expressions::equals const &) -> value_type
				{
					throw std::logic_error("not implemented");
				}
			);
		}
	}

	template <class Storage>
	inline values::value run_getter(Storage &storage, get_function const &get, values::value const &argument, layouts::layout const &root)
	{
		boost::ignore_unused_variable_warning(root);
		typedef detail::pseudo_value<Storage> pseudo_value;
		detail::basic_array_accessor<Storage> root_array(detail::storage_pointer<Storage>(storage, 0));
		detail::basic_tuple<pseudo_value> get_argument;
		get_argument.elements.emplace_back(root_array);
		get_argument.elements.emplace_back(argument.copy());
		pseudo_value result = detail::execute(get, pseudo_value(std::move(get_argument)), pseudo_value(values::value(values::unit())));
		values::value * const finite_result = Si::try_get_ptr<values::value>(result);
		if (!finite_result)
		{
			throw std::runtime_error("not implemented");
		}
		return std::move(*finite_result);
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
#if !SILICIUM_COMPILER_HAS_EXTENDED_CAPTURE
			auto get_ptr = Si::to_shared(get.copy());
#endif
			result.gets.emplace_back([
#if SILICIUM_COMPILER_HAS_EXTENDED_CAPTURE
				get = get.copy()
#else
				get_ptr
#endif
				, root_layout
			](storage_type &storage, values::value const &argument) -> values::value
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
