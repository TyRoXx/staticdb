#ifndef STATICDB_BIT_SINK_HPP
#define STATICDB_BIT_SINK_HPP

#include <silicium/sink/sink.hpp>
#include <staticdb/values.hpp>

namespace staticdb
{
	template <class ByteSink>
	struct bits_to_byte_sink
	{
		typedef values::bit element_type;
		typedef typename ByteSink::error_type error_type;

		bits_to_byte_sink()
		    : m_buffered_bits(0)
		{
		}

		explicit bits_to_byte_sink(ByteSink bytes)
		    : m_bytes(std::move(bytes))
		    , m_buffered_bits(0)
		{
		}

		error_type append(Si::iterator_range<element_type const *> data)
		{
			for (element_type value : data)
			{
				error_type error = append_bit(value);
				if (error)
				{
					return error;
				}
			}
			return error_type();
		}

		std::size_t buffered_bits() const
		{
			return m_buffered_bits;
		}

	private:

		ByteSink m_bytes;
		std::uint8_t m_next_byte;
		std::uint_fast8_t m_buffered_bits;

		enum
		{
			bits_in_byte = 8
		};

		error_type append_bit(values::bit value)
		{
			if (m_buffered_bits == 0)
			{
				m_next_byte = 0;
			}
			m_next_byte = static_cast<std::uint8_t>(m_next_byte | ((value.is_set ? 1u : 0u) << (7u - m_buffered_bits)));
			++m_buffered_bits;
			if (m_buffered_bits != bits_in_byte)
			{
				return error_type();
			}
			error_type error = Si::append(m_bytes, static_cast<typename ByteSink::element_type>(m_next_byte));
			if (!error)
			{
				m_buffered_bits = 0;
			}
			return error;
		}
	};

	template <class ByteSink>
	auto make_bits_to_byte_sink(ByteSink &&bytes)
		-> bits_to_byte_sink<typename std::decay<ByteSink>::type>
	{
		return bits_to_byte_sink<typename std::decay<ByteSink>::type>(std::forward<ByteSink>(bytes));
	}
}

#endif
