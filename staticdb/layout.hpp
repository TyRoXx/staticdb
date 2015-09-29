#ifndef STATICDB_LAYOUT_HPP
#define STATICDB_LAYOUT_HPP

#include <staticdb/types.hpp>
#include <silicium/to_unique.hpp>

namespace staticdb
{
	namespace layouts
	{
		struct layout;

		struct unit
		{
		};

		struct tuple
		{
			std::vector<layout> elements;

			explicit tuple(std::vector<layout> elements)
				: elements(std::move(elements))
			{
			}

#if SILICIUM_COMPILER_GENERATES_MOVES
			SILICIUM_DEFAULT_MOVE(tuple)
#else
			tuple(tuple &&other) BOOST_NOEXCEPT
				: elements(std::move(other.elements))
			{
			}

			tuple &operator = (tuple &&other) BOOST_NOEXCEPT
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

#if SILICIUM_COMPILER_GENERATES_MOVES
			SILICIUM_DEFAULT_MOVE(array)
#else
			array(array &&other) BOOST_NOEXCEPT
				: element(std::move(other.element))
			{
			}

			array &operator = (array &&other) BOOST_NOEXCEPT
			{
				element = std::move(other.element);
				return *this;
			}
#endif
			SILICIUM_DISABLE_COPY(array)
		};

		struct bitset
		{
			std::size_t length;

			explicit bitset(std::size_t length)
				: length(length)
			{
			}
		};

		struct variant
		{
			std::vector<layout> possibilities;

			explicit variant(std::vector<layout> possibilities)
				: possibilities(std::move(possibilities))
			{
			}

#if SILICIUM_COMPILER_GENERATES_MOVES
			SILICIUM_DEFAULT_MOVE(variant)
#else
			variant(variant &&other) BOOST_NOEXCEPT
				: possibilities(std::move(other.possibilities))
			{
			}

			variant &operator = (variant &&other) BOOST_NOEXCEPT
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

#if SILICIUM_COMPILER_GENERATES_MOVES
			SILICIUM_DEFAULT_MOVE(layout)
#else
			layout(layout &&other) BOOST_NOEXCEPT
				: base(std::move(other.as_variant()))
			{
			}

			layout &operator = (layout &&other) BOOST_NOEXCEPT
			{
				as_variant() = std::move(other.as_variant());
				return *this;
			}
#endif
			SILICIUM_DISABLE_COPY(layout)
		};

		inline layout calculate(types::type const &root)
		{
			return Si::visit<layout>(
				root.as_variant(),
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
					std::size_t bits = 0;
					std::vector<layout> element_layouts;
					for (types::type const &element : tuple_type.elements)
					{
						layout element_layout = calculate(element);
						if (could_be_bitset)
						{
							could_be_bitset = Si::visit<bool>(
								element_layout.as_variant(),
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
									bits += b.length;
									return true;
								},
								[](variant const &)
								{
									return false;
								}
							);
						}
						element_layouts.emplace_back(std::move(element_layout));
					}
					if (could_be_bitset)
					{
						return layout(bitset(bits));
					}
					return layout(tuple(std::move(element_layouts)));
				},
				[](types::variant const &variant_type) -> layout
				{
					//TODO: fold variants
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
				}
			);
		}
	}
}

#endif
