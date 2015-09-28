#ifndef STATICDB_TYPES_HPP
#define STATICDB_TYPES_HPP

#include <silicium/variant.hpp>

namespace staticdb
{
	namespace types
	{
		struct unit
		{
		};

		struct bit
		{
		};

		template <class Type>
		struct basic_tuple
		{
			std::vector<Type> elements;

			basic_tuple()
			{
			}

			explicit basic_tuple(std::vector<Type> elements)
				: elements(std::move(elements))
			{
			}
		};

		template <class Type>
		struct basic_variant
		{
			std::vector<Type> possibilities;

			basic_variant()
			{
			}

			explicit basic_variant(std::vector<Type> possibilities)
				: possibilities(std::move(possibilities))
			{
			}
		};

		struct type : Si::variant<unit, bit, basic_tuple<type>, basic_variant<type>>
		{
			typedef Si::variant<unit, bit, basic_tuple<type>, basic_variant<type>> base;

			type()
			{
			}

			template <class A0, class ...Args>
			type(A0 &&a0, Args &&...args)
				: base(std::forward<A0>(a0), std::forward<Args>(args)...)
			{
			}

			Si::variant<unit, bit, basic_tuple<type>, basic_variant<type>> const &as_variant() const
			{
				return *this;
			}
		};

		typedef basic_tuple<type> tuple;
		typedef basic_variant<type> variant;

		template <class ...Types>
		tuple make_tuple(Types &&...elements)
		{
			std::vector<type> elements_vector;
			elements_vector.reserve(sizeof...(Types));
			int dummy[] = {(elements_vector.emplace_back(std::forward<Types>(elements)), 0)...};
			(void)dummy;
			return tuple(std::move(elements_vector));
		}

		inline tuple make_unsigned_integer(std::size_t bits)
		{
			tuple integer;
			integer.elements.resize(bits);
			std::generate_n(integer.elements.begin(), bits, []{ return bit(); });
			return integer;
		}

		inline variant make_optional(type possibility)
		{
			variant optional;
			optional.possibilities.reserve(2);
			optional.possibilities.emplace_back(unit());
			optional.possibilities.emplace_back(std::move(possibility));
			return optional;
		}
	}
}

#endif
