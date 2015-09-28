#ifndef STATICDB_STORAGE_HPP
#define STATICDB_STORAGE_HPP

#include <vector>
#include <staticdb/byte.hpp>
#include <silicium/trait.hpp>
#include <silicium/source/memory_source.hpp>
#include <silicium/success.hpp>

namespace staticdb
{
	typedef std::uint64_t address;

	template <class ReadSource, class WriteSink>
	SILICIUM_TRAIT(
		Storage,
		((read_at, (1, (address)), ReadSource &))
		((write_at, (1, (address)), WriteSink &))
	)

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
			std::size_t const overwrite_amount = std::min<std::size_t>(data.size(), m_destination->size() - m_overwrite_at);
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
			return Si::memory_source<byte>(Si::make_iterator_range(memory.data() + std::min(where, memory.size()), memory.data() + memory.size()));
		}

		vector_sink<byte> write_at(address where)
		{
			memory.resize(std::max(where, memory.size()));
			return vector_sink<byte>(memory, where);
		}
	};

}

#endif
