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
		};

		template <class Expression>
		struct basic_make_tuple
		{
			std::vector<Expression> elements;

			explicit basic_make_tuple(std::vector<Expression> elements)
			    : elements(std::move(elements))
			{
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

			explicit basic_lambda(std::unique_ptr<Expression> body)
				: body(std::move(body))
			{
			}

#if SILICIUM_COMPILER_GENERATES_MOVES
			SILICIUM_DEFAULT_MOVE(basic_lambda)
#else
			basic_lambda(basic_lambda &&other) BOOST_NOEXCEPT
				: body(std::move(other.body))
			{
			}

			basic_lambda &operator = (basic_lambda &&other) BOOST_NOEXCEPT
			{
				body = std::move(other.body);
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

		struct expression : Si::variant<literal, argument, basic_make_tuple<expression>, basic_tuple_at<expression>, basic_branch<expression>, basic_lambda<expression>, basic_call<expression>>
		{
			typedef Si::variant<literal, argument, basic_make_tuple<expression>, basic_tuple_at<expression>, basic_branch<expression>, basic_lambda<expression>, basic_call<expression>> base;

			template <class A0, class ...Args>
			explicit expression(A0 &&a0, Args &&...args)
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

		inline values::value execute(expression const &program, values::value const &argument)
		{
			return Si::visit<values::value>(
				program,
				[](literal const &literal_)
				{
					return literal_.value.copy();
				},
				[&argument](expressions::argument)
				{
					return argument.copy();
				},
				[&argument](make_tuple const &make_tuple_)
				{
					values::tuple result;
					result.elements.reserve(make_tuple_.elements.size());
					for (expression const &element : make_tuple_.elements)
					{
						result.elements.emplace_back(execute(element, argument));
					}
					return values::value(std::move(result));
				},
				[&argument](tuple_at const &tuple_at_)
				{
					values::value const tuple_ = execute(*tuple_at_.tuple, argument);
					values::value const index = execute(*tuple_at_.index, argument);
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
				[&argument](branch const &) -> values::value
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
				}
			);
		}
	}
}

#endif
