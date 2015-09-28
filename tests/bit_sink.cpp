#include <boost/test/unit_test.hpp>
#include <staticdb/bit_sink.hpp>
#include <silicium/sink/iterator_sink.hpp>

BOOST_AUTO_TEST_CASE(bit_sink_append)
{
	std::vector<std::uint8_t> buffer;
	auto writer = staticdb::make_bits_to_byte_sink(Si::make_container_sink(buffer));
	std::vector<std::uint8_t> expected;

	BOOST_CHECK_EQUAL(0u, writer.buffered_bits());
	BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), buffer.begin(), buffer.end());

	for (std::size_t i = 0; i < 3; ++i)
	{
		Si::append(writer, staticdb::values::bit(true));
		BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), buffer.begin(), buffer.end());
		BOOST_CHECK_EQUAL(1u, writer.buffered_bits());

		Si::append(writer, staticdb::values::bit(false));
		BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), buffer.begin(), buffer.end());
		BOOST_CHECK_EQUAL(2u, writer.buffered_bits());

		Si::append(writer, staticdb::values::bit(true));
		BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), buffer.begin(), buffer.end());
		BOOST_CHECK_EQUAL(3u, writer.buffered_bits());

		Si::append(writer, staticdb::values::bit(false));
		BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), buffer.begin(), buffer.end());
		BOOST_CHECK_EQUAL(4u, writer.buffered_bits());

		Si::append(writer, staticdb::values::bit(true));
		BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), buffer.begin(), buffer.end());
		BOOST_CHECK_EQUAL(5u, writer.buffered_bits());

		Si::append(writer, staticdb::values::bit(false));
		BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), buffer.begin(), buffer.end());
		BOOST_CHECK_EQUAL(6u, writer.buffered_bits());

		Si::append(writer, staticdb::values::bit(true));
		BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), buffer.begin(), buffer.end());
		BOOST_CHECK_EQUAL(7u, writer.buffered_bits());

		Si::append(writer, staticdb::values::bit(false));
		expected.push_back(128 + 32 + 8 + 2);
		BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), buffer.begin(), buffer.end());
		BOOST_CHECK_EQUAL(0u, writer.buffered_bits());
	}
}
