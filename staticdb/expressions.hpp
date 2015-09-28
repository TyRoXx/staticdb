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

			explicit literal(values::value value)
			    : value(std::move(value))
			{
			}
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
		};

		template <class Expression>
		struct basic_tuple_at
		{
			std::unique_ptr<Expression> tuple;
			std::unique_ptr<Expression> index;

			basic_tuple_at(
				std::unique_ptr<Expression> tuple,
				std::unique_ptr<Expression> index)
			    : tuple(std::move(tuple))
			    , index(std::move(index))
			{
			}
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
		};

		template <class Expression>
		struct basic_lambda
		{
			std::unique_ptr<Expression> body;

			explicit basic_lambda(std::unique_ptr<Expression> body)
				: body(std::move(body))
			{
			}
		};

		struct expression : Si::variant<literal, argument, basic_make_tuple<expression>, basic_tuple_at<expression>, basic_branch<expression>, basic_lambda<expression>>
		{
			typedef Si::variant<literal, argument, basic_make_tuple<expression>, basic_tuple_at<expression>, basic_branch<expression>, basic_lambda<expression>> base;

			template <class A0, class ...Args>
			explicit expression(A0 &&a0, Args &&...args)
				: base(std::forward<A0>(a0), std::forward<Args>(args)...)
			{
			}
		};

		typedef basic_make_tuple<expression> make_tuple;
		typedef basic_tuple_at<expression> tuple_at;
		typedef basic_branch<expression> branch;
		typedef basic_lambda<expression> lambda;

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
					Si::optional<boost::uint64_t> const is_index = parse_unsigned_integer(*is_index_tuple);
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
				}
			);
		}
	}
}

#endif
