#ifndef STATICDB_ADDRESS_HPP
#define STATICDB_ADDRESS_HPP

#include <cstdint>
#include <silicium/safe_arithmetic.hpp>

namespace staticdb
{
	typedef Si::safe_number<std::uint64_t> address;
}

#endif
