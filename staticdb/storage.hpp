#ifndef STATICDB_STORAGE_HPP
#define STATICDB_STORAGE_HPP

#include <vector>
#include <staticdb/address.hpp>
#include <staticdb/byte.hpp>
#include <silicium/trait.hpp>
#include <silicium/source/memory_source.hpp>
#include <silicium/success.hpp>

namespace staticdb
{
	template <class ReadSource, class WriteSink>
	SILICIUM_TRAIT(Storage, ((read_at, (1, (address)), ReadSource &))((write_at, (1, (address)), WriteSink &)))

	template <class Element>
	struct vector_sink
	{
		typedef Element element_type;
		typedef Si::success error_type;

		vector_sink(std::vector<Element> &destination, std::size_t overwrite_at)
		    : m_destination(&destination)
		    , m_overwrite_at(overwrite_at)
		{
		}

		error_type append(Si::iterator_range<element_type const *> data)
		{
			assert(m_destination);
			std::size_t const overwrite_amount =
			    std::min<std::size_t>(data.size(), m_destination->size() - m_overwrite_at);
			std::copy(data.begin(), data.begin() + overwrite_amount, m_destination->begin() + m_overwrite_at);
			m_destination->insert(m_destination->end(), data.begin() + overwrite_amount, data.end());
			return error_type();
		}

	private:
		std::vector<Element> *m_destination;
		std::size_t m_overwrite_at;
	};

	struct memory_storage
	{
		std::vector<byte> memory;

		Si::memory_source<byte> read_at(address where)
		{
			if (where >= (std::numeric_limits<std::size_t>::max)())
			{
				throw std::invalid_argument("read_at where address out of range");
			}
			std::size_t const limited_where = (std::min)(static_cast<std::size_t>(where), memory.size());
			return Si::memory_source<byte>(
			    Si::make_iterator_range(memory.data() + limited_where, memory.data() + memory.size()));
		}

		vector_sink<byte> write_at(address where)
		{
			if (where >= (std::numeric_limits<std::size_t>::max)())
			{
				throw std::invalid_argument("write_at where address out of range");
			}
			memory.resize((std::max)(static_cast<std::size_t>(where), memory.size()));
			return vector_sink<byte>(memory, static_cast<std::size_t>(where));
		}
	};
}

#endif
