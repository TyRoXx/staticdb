#ifndef STATICDB_MULTIPLY_HPP
#define STATICDB_MULTIPLY_HPP

#include <silicium/arithmetic/overflow_or.hpp>

namespace staticdb
{
	template <class Unsigned>
	Si::overflow_or<Unsigned> operator * (Si::overflow_or<Unsigned> const &left, Unsigned right)
	{
		if (left.is_overflow())
		{
			return left;
		}
		if ((right != 0) && (*left.value() > ((std::numeric_limits<Unsigned>::max)() / right)))
		{
			return Si:: overflow;
		}
		return *left.value() * right;
	}
}

#endif
