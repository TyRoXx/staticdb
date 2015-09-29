#ifndef STATICDB_EXPRESSION_HPP
#define STATICDB_EXPRESSION_HPP

#include <staticdb/values.hpp>

namespace staticdb
{
	namespace expressions
	{
		struct literal
		{
			values::value value;

			literal()
			{
			}

			explicit literal(values::value value)
			    : value(std::move(value))
			{
			}

			literal copy() const
			{
				return literal(value.copy());
			}

#if SILICIUM_COMPILER_GENERATES_MOVES
			SILICIUM_DEFAULT_MOVE(literal)
#else
			literal(literal &&other) BOOST_NOEXCEPT
				: value(std::move(other.value))
			{
			}

			literal &operator = (literal &&other) BOOST_NOEXCEPT
			{
				value = std::move(other.value);
				return *this;
			}
#endif
			SILICIUM_DISABLE_COPY(literal)
		};

		struct argument
		{
			argument copy() const
			{
				return argument();
			}
		};

		struct bound
		{
			bound copy() const
			{
				return bound();
			}
		};

		namespace detail
		{
			template <class ExplicitlyCopyable>
			std::vector<ExplicitlyCopyable> copy(std::vector<ExplicitlyCopyable> const &v)
			{
				std::vector<ExplicitlyCopyable> result;
				result.reserve(v.size());
				for (ExplicitlyCopyable const &e : v)
				{
					result.emplace_back(e.copy());
				}
				return result;
			}
		}

		template <class Expression>
		struct basic_make_tuple
		{
			std::vector<Expression> elements;

			explicit basic_make_tuple(std::vector<Expression> elements)
			    : elements(std::move(elements))
			{
			}

			basic_make_tuple copy() const
			{
				return basic_make_tuple(detail::copy(elements));
			}

#if SILICIUM_COMPILER_GENERATES_MOVES
			SILICIUM_DEFAULT_MOVE(basic_make_tuple)
#else
			basic_make_tuple(basic_make_tuple &&other) BOOST_NOEXCEPT
				: elements(std::move(other.elements))
			{
			}

			basic_make_tuple &operator = (basic_make_tuple &&other) BOOST_NOEXCEPT
			{
				elements = std::move(other.elements);
				return *this;
			}
#endif
			SILICIUM_DISABLE_COPY(basic_make_tuple)
		};

		template <class Expression>
		struct basic_tuple_at
		{
			std::unique_ptr<Expression> tuple;
			std::unique_ptr<Expression> index;

			basic_tuple_at()
			{
			}

			basic_tuple_at(
				std::unique_ptr<Expression> tuple,
				std::unique_ptr<Expression> index)
			    : tuple(std::move(tuple))
			    , index(std::move(index))
			{
			}

			basic_tuple_at copy() const
			{
				return basic_tuple_at(Si::to_unique(tuple->copy()), Si::to_unique(index->copy()));
			}

#if SILICIUM_COMPILER_GENERATES_MOVES
			SILICIUM_DEFAULT_MOVE(basic_tuple_at)
#else
			basic_tuple_at(basic_tuple_at &&other) BOOST_NOEXCEPT
				: tuple(std::move(other.tuple))
				, index(std::move(other.index))
			{
			}

			basic_tuple_at &operator = (basic_tuple_at &&other) BOOST_NOEXCEPT
			{
				tuple = std::move(other.tuple);
				index = std::move(other.index);
				return *this;
			}
#endif
			SILICIUM_DISABLE_COPY(basic_tuple_at)
		};

		template <class Expression>
		struct basic_branch
		{
			std::unique_ptr<Expression> condition;
			std::unique_ptr<Expression> positive;
			std::unique_ptr<Expression> negative;

			basic_branch(
				std::unique_ptr<Expression> condition,
				std::unique_ptr<Expression> positive,
				std::unique_ptr<Expression> negative)
			    : condition(std::move(condition))
			    , positive(std::move(positive))
			    , negative(std::move(negative))
			{
			}

			basic_branch copy() const
			{
				return basic_branch(Si::to_unique(condition->copy()), Si::to_unique(positive->copy()), Si::to_unique(negative->copy()));
			}

#if SILICIUM_COMPILER_GENERATES_MOVES
			SILICIUM_DEFAULT_MOVE(basic_branch)
#else
			basic_branch(basic_branch &&other) BOOST_NOEXCEPT
				: condition(std::move(other.condition))
				, positive(std::move(other.positive))
				, negative(std::move(other.negative))
			{
			}

			basic_branch &operator = (basic_branch &&other) BOOST_NOEXCEPT
			{
				condition = std::move(other.condition);
				positive = std::move(other.positive);
				negative = std::move(other.negative);
				return *this;
			}
#endif
			SILICIUM_DISABLE_COPY(basic_branch)
		};

		template <class Expression>
		struct basic_lambda
		{
			std::unique_ptr<Expression> body;
			std::unique_ptr<Expression> bound;

			explicit basic_lambda(std::unique_ptr<Expression> body, std::unique_ptr<Expression> bound)
				: body(std::move(body))
				, bound(std::move(bound))
			{
			}

			basic_lambda copy() const
			{
				return basic_lambda(Si::to_unique(body->copy()), Si::to_unique(bound->copy()));
			}

#if SILICIUM_COMPILER_GENERATES_MOVES
			SILICIUM_DEFAULT_MOVE(basic_lambda)
#else
			basic_lambda(basic_lambda &&other) BOOST_NOEXCEPT
				: body(std::move(other.body))
				, bound(std::move(other.bound))
			{
			}

			basic_lambda &operator = (basic_lambda &&other) BOOST_NOEXCEPT
			{
				body = std::move(other.body);
				bound = std::move(other.bound);
				return *this;
			}
#endif
			SILICIUM_DISABLE_COPY(basic_lambda)
		};

		template <class Expression>
		struct basic_call
		{
			std::unique_ptr<Expression> function;
			std::vector<Expression> arguments;

			explicit basic_call(std::unique_ptr<Expression> function, std::vector<Expression> arguments)
				: function(std::move(function))
				, arguments(std::move(arguments))
			{
			}

			basic_call copy() const
			{
				return basic_call(Si::to_unique(function->copy()), detail::copy(arguments));
			}

#if SILICIUM_COMPILER_GENERATES_MOVES
			SILICIUM_DEFAULT_MOVE(basic_call)
#else
			basic_call(basic_call &&other) BOOST_NOEXCEPT
				: function(std::move(other.function))
				, arguments(std::move(other.arguments))
			{
			}

			basic_call &operator = (basic_call &&other) BOOST_NOEXCEPT
			{
				function = std::move(other.function);
				arguments = std::move(other.arguments);
				return *this;
			}
#endif
			SILICIUM_DISABLE_COPY(basic_call)
		};

		template <class Expression>
		struct basic_filter
		{
			std::unique_ptr<Expression> input;
			std::unique_ptr<Expression> predicate;

			explicit basic_filter(std::unique_ptr<Expression> input, std::unique_ptr<Expression> predicate)
				: input(std::move(input))
				, predicate(std::move(predicate))
			{
			}

			basic_filter copy() const
			{
				return basic_filter(Si::to_unique(input->copy()), Si::to_unique(predicate->copy()));
			}

#if SILICIUM_COMPILER_GENERATES_MOVES
			SILICIUM_DEFAULT_MOVE(basic_filter)
#else
			basic_filter(basic_filter &&other) BOOST_NOEXCEPT
				: input(std::move(other.input))
				, predicate(std::move(other.predicate))
			{
			}

			basic_filter &operator = (basic_filter &&other) BOOST_NOEXCEPT
			{
				input = std::move(other.input);
				predicate = std::move(other.predicate);
				return *this;
			}
#endif
			SILICIUM_DISABLE_COPY(basic_filter)
		};

		template <class Expression>
		struct basic_equals
		{
			std::unique_ptr<Expression> first;
			std::unique_ptr<Expression> second;

			explicit basic_equals(std::unique_ptr<Expression> first, std::unique_ptr<Expression> second)
				: first(std::move(first))
				, second(std::move(second))
			{
			}

			basic_equals copy() const
			{
				return basic_equals(Si::to_unique(first->copy()), Si::to_unique(second->copy()));
			}

#if SILICIUM_COMPILER_GENERATES_MOVES
			SILICIUM_DEFAULT_MOVE(basic_equals)
#else
			basic_equals(basic_equals &&other) BOOST_NOEXCEPT
				: first(std::move(other.first))
				, second(std::move(other.second))
			{
			}

			basic_equals &operator = (basic_equals &&other) BOOST_NOEXCEPT
			{
				first = std::move(other.first);
				second = std::move(other.second);
				return *this;
			}
#endif
			SILICIUM_DISABLE_COPY(basic_equals)
		};

		template <class Expression>
		struct make_expression_type
		{
			typedef Si::variant<
				literal,
				argument,
				bound,
				basic_make_tuple<Expression>,
				basic_tuple_at<Expression>,
				basic_branch<Expression>,
				basic_lambda<Expression>,
				basic_call<Expression>,
				basic_filter<Expression>,
				basic_equals<Expression>
			> type;
		};

		struct expression : make_expression_type<expression>::type
		{
			typedef make_expression_type<expression>::type base;

			template <class A0, class ...Args>
			expression(A0 &&a0, Args &&...args)
				: base(std::forward<A0>(a0), std::forward<Args>(args)...)
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

			expression copy() const
			{
				return as_variant().apply_visitor(values::detail::copying_visitor<expression>());
			}
			
#if SILICIUM_COMPILER_GENERATES_MOVES
			SILICIUM_DEFAULT_MOVE(expression)
#else
			expression(expression &&other) BOOST_NOEXCEPT
				: base(std::move(other.as_variant()))
			{
			}

			expression &operator = (expression &&other) BOOST_NOEXCEPT
			{
				as_variant() = std::move(other.as_variant());
				return *this;
			}
#endif
			SILICIUM_DISABLE_COPY(expression)
		};

		typedef basic_make_tuple<expression> make_tuple;
		typedef basic_tuple_at<expression> tuple_at;
		typedef basic_branch<expression> branch;
		typedef basic_lambda<expression> lambda;
		typedef basic_call<expression> call;
		typedef basic_filter<expression> filter;
		typedef basic_equals<expression> equals;

		inline tuple_at make_tuple_at(expression tuple, std::size_t index)
		{
			return tuple_at(Si::to_unique(std::move(tuple)), Si::make_unique<expression>(literal(values::make_unsigned_integer(index))));
		}

		inline values::value execute(expression const &program, values::value const &argument_, values::value const &bound_)
		{
			return Si::visit<values::value>(
				program,
				[](literal const &literal_)
				{
					return literal_.value.copy();
				},
				[&argument_](argument)
				{
					return argument_.copy();
				},
				[&bound_](bound)
				{
					return bound_.copy();
				},
				[&argument_, &bound_](make_tuple const &make_tuple_)
				{
					values::tuple result;
					result.elements.reserve(make_tuple_.elements.size());
					for (expression const &element : make_tuple_.elements)
					{
						result.elements.emplace_back(execute(element, argument_, bound_));
					}
					return values::value(std::move(result));
				},
				[&argument_, &bound_](tuple_at const &tuple_at_)
				{
					values::value const tuple_ = execute(*tuple_at_.tuple, argument_, bound_);
					values::value const index = execute(*tuple_at_.index, argument_, bound_);
					values::tuple const * const is_tuple = Si::try_get_ptr<values::tuple>(tuple_.as_variant());
					if (!is_tuple)
					{
						throw std::invalid_argument("tuple_at was called with a non-tuple first argument");
					}
					values::tuple const * const is_index_tuple = Si::try_get_ptr<values::tuple>(index.as_variant());
					if (!is_index_tuple)
					{
						throw std::invalid_argument("tuple_at was called with a non-tuple index (second) argument");
					}
					Si::optional<std::size_t> const is_index = values::parse_unsigned_integer<std::size_t>(*is_index_tuple);
					if (!is_index)
					{
						throw std::invalid_argument("tuple_at was called with a non-integer index (second) argument");
					}
					if (*is_index >= is_tuple->elements.size())
					{
						throw std::invalid_argument("tuple_at was called with an out-of-range index (second) argument");
					}
					return is_tuple->elements[*is_index].copy();
				},
				[&argument_](branch const &) -> values::value
				{
					throw std::logic_error("not implemented");
				},
				[](lambda const &) -> values::value
				{
					throw std::logic_error("not implemented");
				},
				[](call const &) -> values::value
				{
					throw std::logic_error("not implemented");
				},
				[](filter const &) -> values::value
				{
					throw std::logic_error("not implemented");
				},
				[](equals const &) -> values::value
				{
					throw std::logic_error("not implemented");
				}
			);
		}
	}
}

#endif
