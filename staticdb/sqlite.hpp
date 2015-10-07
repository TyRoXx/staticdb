#ifndef STATICDB_SQLITE_HPP
#define STATICDB_SQLITE_HPP

#include <silicium/sqlite3.hpp>
#include <staticdb/plan.hpp>
#include <silicium/sink/iterator_sink.hpp>
#include <silicium/source/memory_source.hpp>

namespace staticdb
{
	namespace sqlite
	{
		template <class CharSink>
		void escape_query_char(char c, CharSink &&escaped)
		{
			switch (c)
			{
			case '"':
				Si::append(escaped, "\"\"");
				break;

			default:
				Si::append(escaped, c);
				break;
			}
		}

		template <class CharSource, class CharSink>
		void escape_identifier(CharSource &&raw, CharSink &&escaped)
		{
			Si::append(escaped, '"');
			for (;;)
			{
				Si::optional<char> c = Si::get(raw);
				if (!c)
				{
					break;
				}
				escape_query_char(*c, escaped);
			}
			Si::append(escaped, '"');
		}

		inline void create_tables(sqlite3 &database, staticdb::types::type const &root, Si::iterator_range<staticdb::get_function const *> gets)
		{
			boost::ignore_unused_variable_warning(database);
			boost::ignore_unused_variable_warning(gets);
			Si::visit<void>(
				root.as_variant(),
				[](types::unit) -> void
				{
					throw std::logic_error("not implemented");
				},
				[](types::bit) -> void
				{
					throw std::logic_error("not implemented");
				},
				[](types::function) -> void
				{
					throw std::logic_error("not implemented");
				},
				[](types::tuple const &) -> void
				{
					throw std::logic_error("not implemented");
				},
				[](types::variant const &) -> void
				{
					throw std::logic_error("not implemented");
				},
				[&database](types::array const &array_type) -> void
				{
					std::vector<char> query;
					auto query_writer = Si::make_container_sink(query);
					Si::append(query_writer, "CREATE TABLE ");
					escape_identifier(Si::make_c_str_source("root"), query_writer);
					Si::append(query_writer, "( ");

					boost::ignore_unused_variable_warning(array_type);
					Si::append(query_writer, "element_0 INTEGER");

					Si::append(query_writer, ")");
					query.emplace_back('\0');
					Si::SQLite3::statement_handle const create_table = Si::SQLite3::prepare(database, Si::c_string(query.data())).move_value();
					throw std::logic_error("not implemented");
				}
			);
		}
	}
}

#endif
