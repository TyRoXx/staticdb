#ifndef STATICDB_VALUES_HPP
#define STATICDB_VALUES_HPP

#include <staticdb/types.hpp>
#include <silicium/variant.hpp>
#include <silicium/to_unique.hpp>
#include <silicium/sink/append.hpp>
#include <cstdint>

namespace staticdb
{
	namespace values
	{
		struct unit
		{
			unit copy() const
			{
				return unit();
			}
		};

		struct bit
		{
			bool is_set;

			bit()
			{
			}

			explicit bit(bool is_set)
				: is_set(is_set)
			{
			}

			bit copy() const
			{
				return *this;
			}
		};

		template <class Value>
		struct basic_tuple
		{
			std::vector<Value> elements;

			basic_tuple()
			{
			}

			basic_tuple copy() const
			{
				basic_tuple result;
				result.elements.resize(elements.size());
				for (size_t i = 0; i < result.elements.size(); ++i)
				{
					result.elements[i] = elements[i].copy();
				}
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

		template <class Value>
		struct basic_variant
		{
			std::unique_ptr<Value> content;

			basic_variant()
			{
			}

			basic_variant copy() const
			{
				assert(content);
				basic_variant result;
				result.content = Si::to_unique(content->copy());
				return result;
			}

#if SILICIUM_COMPILER_GENERATES_MOVES
			SILICIUM_DEFAULT_MOVE(basic_variant)
#else
			basic_variant(basic_variant &&other) BOOST_NOEXCEPT
				: content(std::move(other.content))
			{
			}

			basic_variant &operator = (basic_variant &&other) BOOST_NOEXCEPT
			{
				content = std::move(other.content);
				return *this;
			}
#endif
			SILICIUM_DISABLE_COPY(basic_variant)
		};

		namespace detail
		{
			template <class Result>
			struct copying_visitor
			{
				typedef Result result_type;

				template <class T>
				Result operator()(T const &value) const
				{
					return value.copy();
				}
			};
		}

		struct value : Si::non_copyable_variant<unit, bit, basic_tuple<value>, basic_variant<value>>
		{
			typedef Si::non_copyable_variant<unit, bit, basic_tuple<value>, basic_variant<value>> base;

			value()
			{
			}

			template <class A0, class ...Args>
			value(A0 &&a0, Args &&...args)
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

			value copy() const
			{
				return as_variant().apply_visitor(detail::copying_visitor<value>());
			}

#if SILICIUM_COMPILER_GENERATES_MOVES
			SILICIUM_DEFAULT_MOVE(value)
#else
			value(value &&other) BOOST_NOEXCEPT
				: base(std::move(other.as_variant()))
			{
			}

			value &operator = (value &&other) BOOST_NOEXCEPT
			{
				as_variant() = std::move(other.as_variant());
				return *this;
			}
#endif
			SILICIUM_DISABLE_COPY(value)
		};

		typedef basic_tuple<value> tuple;
		typedef basic_variant<value> variant;

		inline tuple make_unsigned_integer(std::uint8_t value)
		{
			tuple result;
			result.elements.resize(8);
			for (size_t i = 0; i < 8; ++i)
			{
				result.elements[i] = bit(((value >> (7 - i)) & 1) != 0);
			}
			return result;
		}

		inline variant make_some(value content)
		{
			variant result;
			result.content = Si::to_unique(std::move(content));
			return result;
		}

		inline variant make_none()
		{
			variant result;
			result.content = Si::make_unique<value>(unit());
			return result;
		}

		inline bool conforms_to_type(value const &object, types::type const &expected)
		{
			return Si::visit<bool>(
				object,
				[&expected](unit)
				{
					return Si::try_get_ptr<types::unit>(expected.as_variant()) != nullptr;
				},
				[&expected](bit)
				{
					return Si::try_get_ptr<types::bit>(expected.as_variant()) != nullptr;
				},
				[&expected](tuple const &value) -> bool
				{
					types::tuple const * const expected_tuple = Si::try_get_ptr<types::tuple>(expected.as_variant());
					if (!expected_tuple)
					{
						return false;
					}
					if (expected_tuple->elements.size() != value.elements.size())
					{
						return false;
					}
					for (size_t i = 0, c = value.elements.size(); i < c; ++i)
					{
						if (!conforms_to_type(value.elements[i], expected_tuple->elements[i]))
						{
							return false;
						}
					}
					return true;
				},
				[&expected](variant const &value) -> bool
				{
					types::variant const * const expected_variant = Si::try_get_ptr<types::variant>(expected.as_variant());
					if (!expected_variant)
					{
						return false;
					}
					assert(value.content);
					for (types::type const possibility : expected_variant->possibilities)
					{
						if (conforms_to_type(*value.content, possibility))
						{
							return true;
						}
					}
					return false;
				}
			);
		}

		template <class BitSink>
		inline void serialize(BitSink &&destination, value const &object)
		{
			return Si::visit<void>(
				object,
				[](unit)
				{
				},
				[&destination](bit value)
				{
					Si::append(destination, value);
				},
				[&destination](tuple const &t)
				{
					for (value const &element : t.elements)
					{
						serialize(destination, element);
					}
				},
				[&destination](variant const &value)
				{
					assert(value.content);
					return serialize(destination, *value.content);
				}
			);
		}
	}
}

#endif
