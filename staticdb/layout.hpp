#ifndef STATICDB_LAYOUT_HPP
#define STATICDB_LAYOUT_HPP

#include <staticdb/types.hpp>
#include <staticdb/storage.hpp>
#include <staticdb/copy.hpp>
#include <silicium/to_unique.hpp>
#include <silicium/arithmetic/add.hpp>

namespace staticdb
{
	namespace layouts
	{
		struct layout;

		struct unit
		{
			unit copy() const
			{
				return unit();
			}
		};

		struct tuple
		{
			std::vector<layout> elements;

			explicit tuple(std::vector<layout> elements)
			    : elements(std::move(elements))
			{
			}

			tuple copy() const
			{
				return tuple(staticdb::copy(elements));
			}

#if SILICIUM_COMPILER_GENERATES_MOVES
			SILICIUM_DEFAULT_MOVE(tuple)
#else
			tuple(tuple &&other) BOOST_NOEXCEPT : elements(std::move(other.elements))
			{
			}

			tuple &operator=(tuple &&other) BOOST_NOEXCEPT
			{
				elements = std::move(other.elements);
				return *this;
			}
#endif
			SILICIUM_DISABLE_COPY(tuple)
		};

		struct array
		{
			std::unique_ptr<layout> element;

			explicit array(std::unique_ptr<layout> element)
			    : element(std::move(element))
			{
			}

			array copy() const;

#if SILICIUM_COMPILER_GENERATES_MOVES
			SILICIUM_DEFAULT_MOVE(array)
#else
			array(array &&other) BOOST_NOEXCEPT : element(std::move(other.element))
			{
			}

			array &operator=(array &&other) BOOST_NOEXCEPT
			{
				element = std::move(other.element);
				return *this;
			}
#endif
			SILICIUM_DISABLE_COPY(array)
		};

		struct bitset
		{
			address length;

			explicit bitset(address length)
			    : length(length)
			{
			}

			bitset copy() const
			{
				return *this;
			}
		};

		struct variant
		{
			std::vector<layout> possibilities;

			explicit variant(std::vector<layout> possibilities)
			    : possibilities(std::move(possibilities))
			{
			}

			variant copy() const
			{
				return variant(staticdb::copy(possibilities));
			}

#if SILICIUM_COMPILER_GENERATES_MOVES
			SILICIUM_DEFAULT_MOVE(variant)
#else
			variant(variant &&other) BOOST_NOEXCEPT : possibilities(std::move(other.possibilities))
			{
			}

			variant &operator=(variant &&other) BOOST_NOEXCEPT
			{
				possibilities = std::move(other.possibilities);
				return *this;
			}
#endif
			SILICIUM_DISABLE_COPY(variant)
		};

		struct layout : Si::variant<unit, tuple, array, bitset, variant>
		{
			typedef Si::variant<unit, tuple, array, bitset, variant> base;

			template <class A0>
			explicit layout(A0 &&a0)
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

			layout copy() const
			{
				return as_variant().apply_visitor(copying_visitor<layout>());
			}

#if SILICIUM_COMPILER_GENERATES_MOVES
			SILICIUM_DEFAULT_MOVE(layout)
#else
			layout(layout &&other) BOOST_NOEXCEPT : base(std::move(other.as_variant()))
			{
			}

			layout &operator=(layout &&other) BOOST_NOEXCEPT
			{
				as_variant() = std::move(other.as_variant());
				return *this;
			}
#endif
			SILICIUM_DISABLE_COPY(layout)
		};

		inline array array::copy() const
		{
			return array(Si::to_unique(element->copy()));
		}

		bool operator==(layout const &left, layout const &right);

		inline bool operator==(unit, unit)
		{
			return true;
		}

		inline bool operator==(tuple const &left, tuple const &right)
		{
			return left.elements == right.elements;
		}

		inline bool operator==(array const &left, array const &right)
		{
			return *left.element == *right.element;
		}

		inline bool operator==(bitset left, bitset right)
		{
			return left.length == right.length;
		}

		inline bool operator==(variant const &left, variant const &right)
		{
			return left.possibilities == right.possibilities;
		}

		inline bool operator==(layout const &left, layout const &right)
		{
			return left.as_variant() == right.as_variant();
		}

		std::ostream &operator<<(std::ostream &out, layout const &value);

		inline std::ostream &operator<<(std::ostream &out, unit)
		{
			return out << "unit";
		}

		inline std::ostream &operator<<(std::ostream &out, tuple const &value)
		{
			out << "tuple{";
			for (layout const &element : value.elements)
			{
				out << element << ", ";
			}
			return out << "}";
		}

		inline std::ostream &operator<<(std::ostream &out, array const &value)
		{
			return out << "array(" << *value.element << ")";
		}

		inline std::ostream &operator<<(std::ostream &out, bitset value)
		{
			return out << "bitset(" << value.length << ")";
		}

		inline std::ostream &operator<<(std::ostream &out, variant const &value)
		{
			out << "variant{";
			for (layout const &element : value.possibilities)
			{
				out << element << ", ";
			}
			return out << "}";
		}

		inline std::ostream &operator<<(std::ostream &out, layout const &value)
		{
			return out << value.as_variant();
		}

		inline Si::overflow_or<address> layout_size_in_bits(layout const &wanted)
		{
			return Si::visit<Si::overflow_or<address>>(wanted.as_variant(),
			                                           [](unit) -> Si::overflow_or<address>
			                                           {
				                                           return address(0);
				                                       },
			                                           [](tuple const &tuple_) -> Si::overflow_or<address>
			                                           {
				                                           Si::overflow_or<address> sum = address(0);
				                                           for (layout const &element : tuple_.elements)
				                                           {
					                                           sum += layout_size_in_bits(element);
				                                           }
				                                           return sum;
				                                       },
			                                           [](array const &) -> Si::overflow_or<address>
			                                           {
				                                           throw std::logic_error("not implemented");
				                                       },
			                                           [](bitset const &bitset_) -> Si::overflow_or<address>
			                                           {
				                                           return bitset_.length;
				                                       },
			                                           [](variant const &) -> Si::overflow_or<address>
			                                           {
				                                           throw std::logic_error("not implemented");
				                                       });
		}

		inline layout calculate(types::type const &root)
		{
			return Si::visit<layout>(root.as_variant(),
			                         [](types::unit) -> layout
			                         {
				                         throw std::invalid_argument("Cannot calculate layout for unit");
				                     },
			                         [](types::bit) -> layout
			                         {
				                         return layout(bitset(1));
				                     },
			                         [](types::function) -> layout
			                         {
				                         throw std::invalid_argument("Cannot calculate layout for a function");
				                     },
			                         [](types::tuple const &tuple_type) -> layout
			                         {
				                         bool could_be_bitset = true;
				                         Si::overflow_or<address> bits(address(0));
				                         std::vector<layout> element_layouts;
				                         for (types::type const &element : tuple_type.elements)
				                         {
					                         layout element_layout = calculate(element);
					                         if (could_be_bitset)
					                         {
						                         could_be_bitset = Si::visit<bool>(element_layout.as_variant(),
						                                                           [](unit const &)
						                                                           {
							                                                           return false;
							                                                       },
						                                                           [](tuple const &)
						                                                           {
							                                                           return false;
							                                                       },
						                                                           [](array const &)
						                                                           {
							                                                           return false;
							                                                       },
						                                                           [&bits](bitset const &b)
						                                                           {
							                                                           bits = bits + b.length;
							                                                           return !bits.is_overflow();
							                                                       },
						                                                           [](variant const &)
						                                                           {
							                                                           return false;
							                                                       });
					                         }
					                         element_layouts.emplace_back(std::move(element_layout));
				                         }
				                         if (could_be_bitset)
				                         {
					                         assert(!bits.is_overflow());
					                         return layout(bitset(*bits.value()));
				                         }
				                         return layout(tuple(std::move(element_layouts)));
				                     },
			                         [](types::variant const &variant_type) -> layout
			                         {
				                         // TODO: fold variants
				                         std::vector<layout> possibilities;
				                         for (types::type const &possible_type : variant_type.possibilities)
				                         {
					                         possibilities.emplace_back(calculate(possible_type));
				                         }
				                         return layout(variant(std::move(possibilities)));
				                     },
			                         [](types::array const &array_type) -> layout
			                         {
				                         layout element = calculate(*array_type.elements);
				                         return layout(array(Si::to_unique(std::move(element))));
				                     });
		}
	}
}

#endif
