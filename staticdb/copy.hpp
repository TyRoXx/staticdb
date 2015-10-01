#ifndef STATICDB_COPY_HPP
#define STATICDB_COPY_HPP

#include <vector>

namespace staticdb
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

	template <class Result>
	struct copying_visitor
	{
		typedef Result result_type;

		template <class T>
		Result operator()(T const &value) const
		{
			return Result(value.copy());
		}
	};
}

#endif
