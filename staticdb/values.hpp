#ifndef STATICDB_VALUES_HPP
#define STATICDB_VALUES_HPP

#include <staticdb/types.hpp>
#include <staticdb/copy.hpp>
#include <silicium/variant.hpp>
#include <silicium/to_unique.hpp>
#include <silicium/sink/append.hpp>
#include <silicium/optional.hpp>
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

		inline std::ostream &operator << (std::ostream &out, unit)
		{
			return out << "unit";
		}

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

		inline bool operator == (bit left, bit right)
		{
			return left.is_set == right.is_set;
		}

		inline std::ostream &operator << (std::ostream &out, bit value)
		{
			return out << value.is_set;
		}

		template <class Value>
		struct basic_tuple
		{
			std::vector<Value> elements;

			basic_tuple()
			{
			}

			explicit basic_tuple(std::vector<Value> elements)
				: elements(std::move(elements))
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
		bool operator == (basic_tuple<Value> const &left, basic_tuple<Value> const &right)
		{
			return left.elements == right.elements;
		}

		template <class Value>
		inline std::ostream &operator << (std::ostream &out, basic_tuple<Value> const &value)
		{
			out << "{";
			for (auto &element : value.elements)
			{
				out << element << ", ";
			}
			return out << "}";
		}

		template <class Value>
		struct basic_variant
		{
			std::unique_ptr<Value> content;

			basic_variant()
			{
			}

			explicit basic_variant(std::unique_ptr<Value> content)
				: content(std::move(content))
			{
				assert(this->content);
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

		template <class Value>
		inline std::ostream &operator << (std::ostream &out, basic_variant<Value> const &value)
		{
			return out << *value.content;
		}

		struct closure_body
		{
			virtual ~closure_body()
			{
			}
			virtual std::unique_ptr<closure_body> clone() const = 0;
		};

		template <class Value>
		struct basic_closure
		{
			std::unique_ptr<closure_body> body;
			std::unique_ptr<Value> bound;

			basic_closure()
			{
			}

			explicit basic_closure(std::unique_ptr<closure_body> body, std::unique_ptr<Value> bound)
				: body(std::move(body))
				, bound(std::move(bound))
			{
				assert(this->body);
				assert(this->bound);
			}

			basic_closure copy() const
			{
				assert(this->body);
				assert(this->bound);
				basic_closure result;
				result.body = body->clone();
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

		template <class Value>
		inline std::ostream &operator << (std::ostream &out, basic_closure<Value> const &value)
		{
			return out << "closure(" << *value.bound << ")";
		}

		template <class Value>
		struct make_value_type
		{
			typedef Si::non_copyable_variant<
				unit,
				bit,
				basic_tuple<Value>,
				basic_variant<Value>,
				basic_closure<Value>
			> type;
		};

		struct value : make_value_type<value>::type
		{
			typedef make_value_type<value>::type base;

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
				return as_variant().apply_visitor(copying_visitor<value>());
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
		typedef basic_closure<value> closure;

		inline bool operator == (value const &first, value const &second)
		{
			return Si::visit<bool>(
				first,
				[&second](unit) -> bool
				{
					return Si::try_get_ptr<unit>(second) != nullptr;
				},
				[&second](bit first_bit) -> bool
				{
					bit const * const second_bit = Si::try_get_ptr<bit>(second);
					return second_bit && (*second_bit == first_bit);
				},
				[&second](tuple const &first_tuple) -> bool
				{
					tuple const * const second_tuple = Si::try_get_ptr<tuple>(second);
					return second_tuple && (*second_tuple == first_tuple);
				},
				[](variant const &) -> bool
				{
					throw std::logic_error("not implemented");
				},
				[](closure const &) -> bool
				{
					throw std::logic_error("not implemented");
				}
			);
		}

		inline std::ostream &operator << (std::ostream &out, value const &v)
		{
			return out << v.as_variant();
		}

		template <class Unsigned>
		inline tuple make_unsigned_integer(Unsigned value)
		{
			tuple result;
			std::size_t const bits = CHAR_BIT * sizeof(value);
			result.elements.resize(bits);
			for (size_t i = 0; i < bits; ++i)
			{
				result.elements[i] = bit(((value >> (bits - 1 - i)) & 1) != 0);
			}
			return result;
		}

		template <class Unsigned>
		inline Si::optional<Unsigned> parse_unsigned_integer(tuple const &big_endian)
		{
			Unsigned result = 0;
			std::size_t parsed_bits = 0;
			for (;;)
			{
				if (parsed_bits >= big_endian.elements.size())
				{
					return result;
				}
				if (parsed_bits == (CHAR_BIT * sizeof(result)))
				{
					return Si::none;
				}
				bit const * const bit_element = Si::try_get_ptr<bit>(big_endian.elements[parsed_bits]);
				if (!bit_element)
				{
					return Si::none;
				}
				result = static_cast<Unsigned>(result << 1u);
				result = static_cast<Unsigned>(result | (bit_element->is_set ? 1u : 0u));
				++parsed_bits;
			}
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
						types::array const * const expected_array = Si::try_get_ptr<types::array>(expected.as_variant());
						if (!expected_array)
						{
							return false;
						}
						for (values::value const &element : value.elements)
						{
							if (!conforms_to_type(element, *expected_array->elements))
							{
								return false;
							}
						}
						return true;
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
					assert(value.content);
					types::variant const * const expected_variant = Si::try_get_ptr<types::variant>(expected.as_variant());
					if (!expected_variant)
					{
						return false;
					}
					for (types::type const &possibility : expected_variant->possibilities)
					{
						if (conforms_to_type(*value.content, possibility))
						{
							return true;
						}
					}
					return false;
				},
				[&expected](closure const &) -> bool
				{
					return Si::try_get_ptr<types::function>(expected.as_variant()) != nullptr;
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
				},
				[](closure const &)
				{
					throw std::logic_error("not implemented");
				}
			);
		}
	}
}

#endif
