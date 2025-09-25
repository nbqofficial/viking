#include "../logger/logger.h"

using value_t = std::variant<double, std::string>;

namespace n_utils
{
	namespace n_math
	{
		double relative_change(double x, double y);

		std::vector<double> relative_change_vector(const std::vector<double>& x);

		double log_relative_change(double x, double y);

		std::vector<double> log_relative_change_vector(const std::vector<double>& x);

		std::vector<double> first_diff(const std::vector<double>& x);
	}

	namespace n_data
	{
		class c_dataframe
		{
		private:

			std::unordered_map<std::string, std::vector<value_t>> m_data;
			size_t m_rows = 0;


			static bool _is_blank(std::string_view sv);

			static std::optional<double> _parse_double(std::string_view sv);

			static std::string _value_to_string(const value_t& value);

			static value_t _to_value(const value_t& v);

			static value_t _to_value(value_t&& v);

			static value_t _to_value(double v);

			static value_t _to_value(size_t v);

			static value_t _to_value(long long v);

			static value_t _to_value(int v);

			static value_t _to_value(bool v);

			static value_t _to_value(const std::string& s);

			static value_t _to_value(std::string&& s);

			static value_t _to_value(std::string_view sv);

			static value_t _to_value(const char* s);

			static value_t _to_value(char* s);

			template <std::size_t n>
			static value_t _to_value(const char(&lit)[n]);

			template <class t>
			static value_t _to_value(const t&) = delete;

			static std::vector<value_t> _to_column(const std::vector<value_t>& v);

			static std::vector<value_t> _to_column(std::vector<value_t>&& v);

			template<class t>
			static std::vector<value_t> _to_column(const std::vector<t>& v);

			template<class t>
			static std::vector<value_t> _to_column(std::vector<t>&& v);

			static std::vector<std::string> _parse_csv_line(const std::string& line);

			bool _append_column(const std::string& name, std::vector<value_t> values);

		public:

			c_dataframe() = default;

			explicit c_dataframe(const std::string& csv_path, bool header = true);

			~c_dataframe();

			bool add_column(const std::string& name, const std::vector<value_t>& values);

			template<class t>
			bool add_column(const std::string& name, const std::vector<t>& values);

			template<class t>
			bool add_column(const std::string& name, std::vector<t>&& values);

			bool remove_column(const std::string& name);

			bool remove_column(const std::vector<std::string>& names);

			bool rename_column(const std::string& old_name, const std::string& new_name);

			bool dropf();

			bool dropb();

			bool drop(size_t index);

			bool dropna();

			bool dropinf();

			bool dropemp();

			std::vector<value_t>& at(const std::string& name);

			template<typename t>
			std::vector<t> at(const std::string& name);

			template<typename t>
			std::vector<std::vector<t>> at(const std::vector<std::string>& names);

			bool from_csv(const std::string& csv_path, bool header = true);

			bool to_csv(const std::string& csv_path, bool header = true) const;

			std::pair<size_t, size_t> shape() const;

			double sum(const std::string& name, size_t from = 0, size_t length = 0) const;

			double prod(const std::string& name, size_t from = 0, size_t length = 0) const;

			double mean(const std::string& name, size_t from = 0, size_t length = 0) const;

			double var(const std::string& name, size_t from = 0, size_t length = 0) const;

			double std(const std::string& name, size_t from = 0, size_t length = 0) const;

			double min_(const std::string& name, size_t from = 0, size_t length = 0) const;

			double max_(const std::string& name, size_t from = 0, size_t length = 0) const;

			double cov(const std::string& name1, const std::string& name2, size_t from = 0, size_t length = 0) const;

			double corr(const std::string& name1, const std::string& name2, size_t from = 0, size_t length = 0) const;

			double skew(const std::string& name, size_t from = 0, size_t length = 0) const;

			double kurt(const std::string& name, size_t from = 0, size_t length = 0) const;

			std::vector<double> diff(const std::string& name, size_t from = 0, size_t length = 0) const;

			std::vector<double> diff(const std::string& name1, const std::string& name2, size_t from = 0, size_t length = 0) const;

			std::vector<double> pct_change(const std::string& name, size_t from = 0, size_t length = 0) const;

			std::vector<double> pct_change(const std::string& name1, const std::string& name2, size_t from = 0, size_t length = 0) const;

			std::vector<double> log_change(const std::string& name, size_t from = 0, size_t length = 0) const;

			std::vector<double> log_change(const std::string& name1, const std::string& name2, size_t from = 0, size_t length = 0) const;

			std::vector<double> cumsum(const std::string& name, size_t from = 0, size_t length = 0) const;

			std::vector<double> cumprod(const std::string& name, size_t from = 0, size_t length = 0) const;

			void reset();

			c_dataframe head(size_t n = 5) const;

			c_dataframe tail(size_t n = 5) const;

			void print(size_t n = 5) const;
		};

		template<size_t n>
		inline value_t c_dataframe::_to_value(const char(&lit)[n])
		{
			return value_t{ std::string(lit) };
		}

		template<class t>
		std::vector<value_t> c_dataframe::_to_column(const std::vector<t>& v)
		{
			std::vector<value_t> out;
			out.reserve(v.size());
			for (const auto& x : v) { out.push_back(_to_value(x)); }
			return out;
		}

		template<class t>
		std::vector<value_t> c_dataframe::_to_column(std::vector<t>&& v)
		{
			std::vector<value_t> out;
			out.reserve(v.size());
			for (auto& x : v) { out.push_back(_to_value(std::move(x))); }
			return out;
		}


		template<class t>
		bool c_dataframe::add_column(const std::string& name, const std::vector<t>& values)
		{
			return this->_append_column(name, _to_column(values));
		}

		template<class t>
		bool c_dataframe::add_column(const std::string& name, std::vector<t>&& values)
		{
			return this->_append_column(name, _to_column(std::move(values)));
		}

		template<typename t>
		std::vector<t> c_dataframe::at(const std::string& name)
		{
			const auto& col = this->m_data.at(name);
			std::vector<t> out;
			out.reserve(col.size());

			if (std::holds_alternative<double>(col[0]))
			{
				if constexpr (std::is_same_v<t, double>)
				{
					for (const auto& v : col) { out.push_back(std::get<double>(v)); }
				}
				else if constexpr (std::is_integral_v<t> || std::is_same_v<t, bool>)
				{
					for (const auto& v : col) { out.push_back(static_cast<t>(std::get<double>(v))); }
				}
				else
				{
					throw std::runtime_error("c_dataframe::at<T>: requested non-numeric T from numeric column");
				}

				return out;
			}

			if constexpr (std::is_same_v<t, std::string>)
			{
				for (const auto& v : col) { out.push_back(std::get<std::string>(v)); }
				return out;
			}
			else if constexpr (std::is_same_v<t, double>)
			{
				for (const auto& v : col)
				{
					const auto& s = std::get<std::string>(v);
					if (_is_blank(s))
					{
						out.push_back(std::numeric_limits<double>::quiet_NaN());
						continue;
					}
					auto parsed = _parse_double(s);
					out.push_back(parsed.value_or(std::numeric_limits<double>::quiet_NaN()));
				}
				return out;
			}
			else if constexpr (std::is_integral_v<t> || std::is_same_v<t, bool>)
			{
				for (const auto& v : col)
				{
					const auto& s = std::get<std::string>(v);
					auto parsed = _parse_double(s);
					if (parsed.has_value() && std::isfinite(*parsed)) { out.push_back(static_cast<t>(*parsed)); }
					else { throw std::runtime_error("c_dataframe::at<T>: non-parsable string to integer"); }
				}
				return out;
			}
			else { throw std::runtime_error("c_dataframe::at<T>: unsupported T for string column"); }
		}

		template<typename t>
		inline std::vector<std::vector<t>> c_dataframe::at(const std::vector<std::string>& names)
		{
			if (names.empty())
			{
				c_logger::get().log("[c_dataframe]: No column names\n", LOG_CLR_RED);
				return {};
			}

			std::vector<std::vector<t>> out;
			out.reserve(names.size());

			for (const std::string& name : names)
			{
				out.emplace_back(this->at<t>(name));
			}

			return out;
		}
	}
}