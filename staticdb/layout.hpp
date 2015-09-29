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
		};

		struct array
		{
			std::unique_ptr<layout> element;

			explicit array(std::unique_ptr<layout> element)
				: element(std::move(element))
			{
			}
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
		};

		struct layout : Si::variant<unit, tuple, array, bitset, variant>
		{
			typedef Si::variant<unit, tuple, array, bitset, variant> base;

			template <class A0>
			explicit layout(A0 &&a0)
				: base(std::forward<A0>(a0))
			{
			}

			base const &as_variant() const
			{
				return *this;
			}
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
