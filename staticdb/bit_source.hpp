#ifndef STATICDB_BIT_SOURCE_HPP
#define STATICDB_BIT_SOURCE_HPP

#include <silicium/source/source.hpp>
#include <staticdb/byte.hpp>
#include <staticdb/values.hpp>

namespace staticdb
{
	template <class ByteSource>
	struct byte_to_bit_source
	{
		typedef values::bit element_type;

		explicit byte_to_bit_source(ByteSource next)
		    : m_next(std::move(next))
		    , m_buffered_bits(0)
		{
		}

		Si::iterator_range<element_type const *> map_next(std::size_t size)
		{
			boost::ignore_unused_variable_warning(size);
			return {};
		}

		element_type *copy_next(Si::iterator_range<element_type *> destination)
		{
			element_type *i = destination.begin();
			while (i != destination.end())
			{
				Si::optional<values::bit> bit = read_bit();
				if (!bit)
				{
					break;
				}
				*i = *bit;
				++i;
			}
			return i;
		}

	private:

		ByteSource m_next;
		byte m_buffer;
		std::uint8_t m_buffered_bits;

		Si::optional<values::bit> read_bit()
		{
			if (m_buffered_bits == 0)
			{
				Si::optional<byte> piece = Si::get(m_next);
				if (!piece)
				{
					return Si::none;
				}
				m_buffer = *piece;
				m_buffered_bits = 8;
			}
			values::bit const result(((m_buffer >> (m_buffered_bits - 1)) & 0x01) != 0);
			--m_buffered_bits;
			return result;
		}
	};

	template <class ByteSource>
	auto make_byte_to_bit_source(ByteSource &&next)
		-> byte_to_bit_source<typename std::decay<ByteSource>::type>
	{
		return byte_to_bit_source<typename std::decay<ByteSource>::type>(std::forward<ByteSource>(next));
	}
}

#endif
