#ifndef STATICDB_EXECUTION_HPP
#define STATICDB_EXECUTION_HPP

#include <staticdb/expressions.hpp>
#include <staticdb/layout.hpp>
#include <staticdb/storage.hpp>

namespace staticdb
{
	namespace execution
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
			layouts::layout element_layout;

			explicit basic_array_accessor(storage_pointer<Storage> begin, layouts::layout element_layout)
				: begin(begin)
				, element_layout(std::move(element_layout))
			{
			}

			basic_array_accessor copy() const
			{
				return basic_array_accessor(begin, element_layout.copy());
			}

#if SILICIUM_COMPILER_GENERATES_MOVES
			SILICIUM_DEFAULT_MOVE(basic_array_accessor)
#else
			basic_array_accessor(basic_array_accessor &&other) BOOST_NOEXCEPT
				: begin(other.begin)
				, element_layout(std::move(other.element_layout))
			{
			}

			basic_array_accessor &operator = (basic_array_accessor &&other) BOOST_NOEXCEPT
			{
				begin = other.begin;
				element_layout = std::move(other.element_layout);
				return *this;
			}
#endif
			SILICIUM_DISABLE_COPY(basic_array_accessor)
		};

		template <class PseudoValue>
		struct basic_tuple
		{
			std::vector<PseudoValue> elements;

			basic_tuple()
			{
			}

			explicit basic_tuple(std::vector<PseudoValue> elements)
				: elements(std::move(elements))
			{
			}

			basic_tuple copy() const
			{
				basic_tuple result;
				result.elements = staticdb::copy(elements);
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

			template <class A0, class = typename std::enable_if<!std::is_same<typename std::decay<A0>::type, pseudo_value<Storage>>::value, void>::type>
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
				return as_variant().apply_visitor(copying_visitor<pseudo_value>());
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
		pseudo_value<Storage> execute_closure(
			pseudo_value<Storage> const &closure,
			pseudo_value<Storage> const &argument_,
			pseudo_value<Storage> const &bound_
		)
		{
			boost::ignore_unused_variable_warning(closure);
			boost::ignore_unused_variable_warning(argument_);
			boost::ignore_unused_variable_warning(bound_);
			throw std::logic_error("not implemented");
		}

		template <class Storage>
		bool extract_bool(pseudo_value<Storage> const &boolean)
		{
			boost::ignore_unused_variable_warning(boolean);
			throw std::logic_error("not implemented");
		}

		const std::size_t address_size_in_bytes = 8;

		template <class Storage>
		address deserialize_address(storage_pointer<Storage> const &begin)
		{
			address result = 0;
			BOOST_STATIC_ASSERT(sizeof(result) == address_size_in_bytes);
			auto byte_source = begin.storage->read_at(begin.where);
			for (std::size_t i = 0; i < address_size_in_bytes; ++i)
			{
				Si::optional<byte> digit = Si::get(byte_source);
				if (!digit)
				{
					throw std::invalid_argument("deserialize_address needs more bytes");
				}
				result |= (*digit << (address_size_in_bytes - 1 - i));
			}
			return result;
		}

		template <class Storage>
		address array_length(storage_pointer<Storage> const &array_begin)
		{
			address length = deserialize_address(array_begin);
			return length;
		}

		template <class Storage>
		pseudo_value<Storage> access_value(storage_pointer<Storage> const &element_begin, layouts::layout const &element_layout)
		{
			boost::ignore_unused_variable_warning(element_begin);
			return Si::visit<pseudo_value<Storage>>(
				element_layout.as_variant(),
				[](layouts::unit) -> pseudo_value<Storage>
				{
					throw std::logic_error("not implemented");
				},
				[](layouts::tuple const &) -> pseudo_value<Storage>
				{
				throw std::logic_error("not implemented");
				},
				[](layouts::array const &) -> pseudo_value<Storage>
				{
					throw std::logic_error("not implemented");
				},
				[](layouts::bitset const &) -> pseudo_value<Storage>
				{
					throw std::logic_error("not implemented");
				},
				[](layouts::variant const &) -> pseudo_value<Storage>
				{
					throw std::logic_error("not implemented");
				}
			);
		}

		template <class Storage>
		pseudo_value<Storage> array_get(storage_pointer<Storage> const &array_begin, address index, layouts::layout const &element)
		{
			address first_element = array_begin.where + (address_size_in_bytes * 8);
			address element_size_in_bits = layout_size_in_bits(element);
			address wanted_element = first_element + (element_size_in_bits * index);
			return access_value(storage_pointer<Storage>(*array_begin.storage, wanted_element), element);
		}

		template <class Storage>
		pseudo_value<Storage> run_filter(pseudo_value<Storage> const &container, pseudo_value<Storage> const &predicate, pseudo_value<Storage> const &bound)
		{
			return Si::visit<pseudo_value<Storage>>(
				container,
				[&predicate, &bound](basic_array_accessor<Storage> const &array) -> pseudo_value<Storage>
				{
					std::vector<pseudo_value<Storage>> results;
					for (address index = 0, length = array_length(array.begin); index < length; ++index)
					{
						pseudo_value<Storage> element = array_get(array.begin, index, array.element_layout);
						pseudo_value<Storage> const is_good = execute_closure(predicate, element, bound);
						if (!extract_bool(is_good))
						{
							continue;
						}
						results.emplace_back(std::move(element));
					}
					return pseudo_value<Storage>(basic_tuple<pseudo_value<Storage>>(std::move(results)));
				},
				[](basic_tuple<pseudo_value<Storage>> const &) -> pseudo_value<Storage>
				{
					throw std::logic_error("not implemented");
				},
				[](basic_closure<pseudo_value<Storage>> const &) -> pseudo_value<Storage>
				{
					throw std::invalid_argument("run_filter called on a closure");
				},
				[](values::value const &) -> pseudo_value<Storage>
				{
					throw std::logic_error("not implemented");
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
					return run_filter(input, predicate, bound_);
				},
				[](expressions::equals const &) -> value_type
				{
					throw std::logic_error("not implemented");
				}
			);
		}
	}
}

#endif
