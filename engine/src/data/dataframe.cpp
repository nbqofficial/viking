#include "dataframe.h"

double n_utils::n_math::relative_change(double x, double y)
{
	if (!y) { return 0.0; }
	return ((x - y) / y);
}

std::vector<double> n_utils::n_math::relative_change_vector(const std::vector<double>& x)
{
	if (x.empty()) { return {}; }

	std::vector<double> relative_changes;
	relative_changes.reserve(x.size() - 1);
	for (size_t i = 1; i < x.size(); ++i) { relative_changes.emplace_back(n_utils::n_math::relative_change(x[i], x[i - 1])); }

	return relative_changes;
}

double n_utils::n_math::log_relative_change(double x, double y)
{
	if (!y) { return 0.0; }
	double p = x / y;
	if (p <= 0.0) { return p; }
	return std::log(x / y);
}

std::vector<double> n_utils::n_math::log_relative_change_vector(const std::vector<double>& x)
{
	if (x.empty()) { return {}; }

	std::vector<double> relative_changes;
	relative_changes.reserve(x.size() - 1);
	for (size_t i = 1; i < x.size(); ++i) { relative_changes.emplace_back(n_utils::n_math::log_relative_change(x[i], x[i - 1])); }

	return relative_changes;
}

std::vector<double> n_utils::n_math::first_diff(const std::vector<double>& x)
{
	if (x.empty()) { return {}; }

	std::vector<double> dx(x.size() - 1);
	for (size_t i = 1; i < x.size(); ++i) { dx[i - 1] = x[i] - x[i - 1]; }
	return dx;
}


bool n_utils::n_data::c_dataframe::_is_blank(std::string_view sv)
{
	for (unsigned char c : sv)
	{
		if (!std::isspace(c)) { return false; }
	}

	return true;
}

std::optional<double> n_utils::n_data::c_dataframe::_parse_double(std::string_view sv)
{
	if (sv.empty()) { return std::nullopt; }

	auto eq = [&](std::string_view t) {
		if (sv.size() != t.size()) { return false; }
		for (size_t i = 0; i < sv.size(); ++i)
		{
			if ((char)std::tolower((unsigned char)sv[i]) != t[i]) { return false; }
		}
		return true;
		};

	if (eq("nan")) { return std::numeric_limits<double>::quiet_NaN(); }
	if (eq("inf") || eq("+inf")) { return std::numeric_limits<double>::infinity(); }
	if (eq("-inf")) { return -std::numeric_limits<double>::infinity(); }

	double v = 0.0;
	auto* b = sv.data(); auto* e = b + sv.size();
	auto [p, ec] = std::from_chars(b, e, v);
	if (ec == std::errc() && p == e) { return v; }

	return std::nullopt;
}

std::string n_utils::n_data::c_dataframe::_value_to_string(const value_t& value)
{
	return std::visit([](auto&& val) -> std::string {

		using t = std::decay_t<decltype(val)>;

		if constexpr (std::is_same_v<t, std::string>)
		{
			return val;
		}
		else
		{
			std::ostringstream oss;
			oss.imbue(std::locale::classic());
			oss << val;
			return oss.str();
		}
		}, value);
}

value_t n_utils::n_data::c_dataframe::_to_value(const value_t& v)
{
	return v;
}

value_t n_utils::n_data::c_dataframe::_to_value(value_t&& v)
{
	return std::move(v);
}

value_t n_utils::n_data::c_dataframe::_to_value(double v)
{
	return value_t{ v };
}

value_t n_utils::n_data::c_dataframe::_to_value(size_t v)
{
	return value_t{ static_cast<double>(v) };
}

value_t n_utils::n_data::c_dataframe::_to_value(long long v)
{
	return value_t{ static_cast<double>(v) };
}

value_t n_utils::n_data::c_dataframe::_to_value(int v)
{
	return value_t{ static_cast<double>(v) };
}

value_t n_utils::n_data::c_dataframe::_to_value(bool v)
{
	return value_t{ v ? 1.0 : 0.0 };
}

value_t n_utils::n_data::c_dataframe::_to_value(const std::string& s)
{
	return value_t{ s };
}

value_t n_utils::n_data::c_dataframe::_to_value(std::string&& s)
{
	return value_t{ std::move(s) };
}

value_t n_utils::n_data::c_dataframe::_to_value(std::string_view sv)
{
	return value_t{ std::string(sv) };
}

value_t n_utils::n_data::c_dataframe::_to_value(const char* s)
{
	return value_t{ std::string(s ? s : "") };
}

value_t n_utils::n_data::c_dataframe::_to_value(char* s)
{
	return value_t{ std::string(s ? s : "") };
}

std::vector<value_t> n_utils::n_data::c_dataframe::_to_column(const std::vector<value_t>& v)
{
	return v;
}

std::vector<value_t> n_utils::n_data::c_dataframe::_to_column(std::vector<value_t>&& v)
{
	return std::move(v);
}

std::vector<std::string> n_utils::n_data::c_dataframe::_parse_csv_line(const std::string& line)
{
	std::vector<std::string> fields;

	std::string cur;
	cur.reserve(line.size());

	bool in_quotes = false;
	for (size_t i = 0; i < line.size(); ++i)
	{
		char c = line[i];
		if (in_quotes)
		{
			if (c == '"') {
				if (i + 1 < line.size() && line[i + 1] == '"') { cur.push_back('"'); ++i; }
				else { in_quotes = false; }
			}
			else cur.push_back(c);
		}
		else
		{
			if (c == ',') { fields.emplace_back(std::move(cur)); cur.clear(); }
			else if (c == '"') { in_quotes = true; }
			else { cur.push_back(c); }
		}
	}

	fields.emplace_back(std::move(cur));

	return fields;
}

bool n_utils::n_data::c_dataframe::_append_column(const std::string& name, std::vector<value_t> values)
{
	if (name.empty())
	{
		c_logger::get().log("[c_dataframe]: No column name\n", LOG_CLR_RED);
		return false;
	}

	if (values.empty())
	{
		c_logger::get().log("[c_dataframe]: No values\n", LOG_CLR_RED);
		return false;
	}

	if (this->m_rows == 0) { this->m_rows = values.size(); }
	else if (values.size() != this->m_rows)
	{
		c_logger::get().log("[c_dataframe]: Row length mismatch: " + name + "\n", LOG_CLR_RED);
		return false;
	}

	auto [it, inserted] = this->m_data.emplace(name, std::move(values));
	if (!inserted)
	{
		c_logger::get().log("[c_dataframe]: Duplicate column name: " + name + "\n", LOG_CLR_RED);
		return false;
	}

	return true;
}


n_utils::n_data::c_dataframe::c_dataframe(const std::string& csv_path, bool header)
{
	this->from_csv(csv_path, header);
}

n_utils::n_data::c_dataframe::~c_dataframe()
{
}

bool n_utils::n_data::c_dataframe::add_column(const std::string& name, const std::vector<value_t>& values)
{
	return this->_append_column(name, values);
}

bool n_utils::n_data::c_dataframe::remove_column(const std::string& name)
{
	if (name.empty())
	{
		c_logger::get().log("[c_dataframe]: No column name\n", LOG_CLR_RED);
		return false;
	}

	const size_t erased = this->m_data.erase(name);
	if (erased && this->m_data.empty()) { this->m_rows = 0; }

	return erased != 0;
}

bool n_utils::n_data::c_dataframe::remove_column(const std::vector<std::string>& names)
{
	if (names.empty())
	{
		c_logger::get().log("[c_dataframe]: No column names\n", LOG_CLR_RED);
		return false;
	}

	for (const auto& k : names)
	{
		this->m_data.erase(k);
	}

	if (this->m_data.empty()) { this->m_rows = 0; }

	return true;
}

bool n_utils::n_data::c_dataframe::rename_column(const std::string& old_name, const std::string& new_name)
{
	if (old_name.empty() || new_name.empty())
	{
		c_logger::get().log("[c_dataframe]: No column name\n", LOG_CLR_RED);
		return false;
	}

	auto it = m_data.find(old_name);
	if (it == m_data.end())
	{
		c_logger::get().log("[c_dataframe]: No column name\n", LOG_CLR_RED);
		return false;
	}

	if (m_data.find(new_name) != m_data.end())
	{
		c_logger::get().log("[c_dataframe]: Duplicate column name: " + new_name + "\n", LOG_CLR_RED);
		return false;
	}

	m_data.emplace(new_name, std::move(it->second));
	m_data.erase(it);

	return true;
}

bool n_utils::n_data::c_dataframe::dropf()
{
	if (!this->m_rows) { return false; }

	for (auto& [name, col] : this->m_data) { col.erase(col.begin()); }
	this->m_rows--;

	return true;
}

bool n_utils::n_data::c_dataframe::dropb()
{
	if (!this->m_rows) { return false; }

	for (auto& [name, col] : this->m_data) { col.pop_back(); }
	this->m_rows--;

	return true;
}

bool n_utils::n_data::c_dataframe::drop(size_t index)
{
	if (!this->m_rows || index >= this->m_rows) { return false; }

	for (auto& [name, col] : this->m_data) { col.erase(col.begin() + index); }
	this->m_rows--;

	return true;
}

bool n_utils::n_data::c_dataframe::dropna()
{
	if (!this->m_rows) { return false; }

	auto is_str_nan = [](const std::string& s) -> bool {
		if (s.size() != 3) { return false; }
		auto tl = [](unsigned char c) { return static_cast<char>(std::tolower(c)); };
		return tl(s[0]) == 'n' && tl(s[1]) == 'a' && tl(s[2]) == 'n';
		};

	std::vector<size_t> rows_to_drop;
	rows_to_drop.reserve(this->m_rows);

	for (size_t i = 0; i < this->m_rows; ++i)
	{
		bool has_nan = false;
		for (auto& [name, col] : m_data)
		{
			if (std::holds_alternative<double>(col[i]))
			{
				double v = std::get<double>(col[i]);
				if (std::isnan(v))
				{
					has_nan = true;
					break;
				}
			}
			else
			{
				const std::string& v = std::get<std::string>(col[i]);
				if (is_str_nan(v)) { has_nan = true; break; }
			}
		}
		if (has_nan) { rows_to_drop.push_back(i); }
	}

	if (rows_to_drop.empty()) { return false; }

	std::sort(rows_to_drop.begin(), rows_to_drop.end());
	rows_to_drop.erase(std::unique(rows_to_drop.begin(), rows_to_drop.end()), rows_to_drop.end());

	for (auto it = rows_to_drop.rbegin(); it != rows_to_drop.rend(); ++it)
	{
		if (!this->drop(*it)) { return false; }
	}

	return true;
}

bool n_utils::n_data::c_dataframe::dropinf()
{
	if (!this->m_rows) { return false; }

	auto is_str_inf = [](const std::string& s) -> bool {
		if (s.size() < 3 || s.size() > 4) { return false; }
		auto tl = [](unsigned char c) { return static_cast<char>(std::tolower(c)); };
		size_t start = 0;
		if (s[0] == '+' || s[0] == '-') { start = 1; }
		if (s.size() - start != 3) { return false; }
		return tl(s[start]) == 'i' && tl(s[start + 1]) == 'n' && tl(s[start + 2]) == 'f';
		};

	std::vector<size_t> rows_to_drop;
	rows_to_drop.reserve(this->m_rows);

	for (size_t i = 0; i < this->m_rows; ++i)
	{
		bool has_inf = false;
		for (auto& [name, col] : m_data)
		{
			if (std::holds_alternative<double>(col[i]))
			{
				double v = std::get<double>(col[i]);
				if (std::isinf(v))
				{
					has_inf = true;
					break;
				}
			}
			else
			{
				const std::string& v = std::get<std::string>(col[i]);
				if (is_str_inf(v)) { has_inf = true; break; }
			}
		}
		if (has_inf) { rows_to_drop.push_back(i); }
	}

	if (rows_to_drop.empty()) { return false; }

	std::sort(rows_to_drop.begin(), rows_to_drop.end());
	rows_to_drop.erase(std::unique(rows_to_drop.begin(), rows_to_drop.end()), rows_to_drop.end());

	for (auto it = rows_to_drop.rbegin(); it != rows_to_drop.rend(); ++it)
	{
		if (!this->drop(*it)) { return false; }
	}

	return true;
}

bool n_utils::n_data::c_dataframe::dropemp()
{
	if (!this->m_rows) { return false; }

	std::vector<size_t> rows_to_drop;
	rows_to_drop.reserve(this->m_rows);

	for (size_t i = 0; i < this->m_rows; ++i)
	{
		bool has_empty = false;
		for (auto& [name, col] : m_data)
		{
			if (std::holds_alternative<std::string>(col[i]))
			{
				const std::string& v = std::get<std::string>(col[i]);
				if (_is_blank(v))
				{
					has_empty = true;
					break;
				}
			}
		}
		if (has_empty) { rows_to_drop.push_back(i); }
	}

	if (rows_to_drop.empty()) { return false; }

	std::sort(rows_to_drop.begin(), rows_to_drop.end());
	rows_to_drop.erase(std::unique(rows_to_drop.begin(), rows_to_drop.end()), rows_to_drop.end());

	for (auto it = rows_to_drop.rbegin(); it != rows_to_drop.rend(); ++it)
	{
		if (!this->drop(*it)) { return false; }
	}

	return true;
}

std::vector<value_t>& n_utils::n_data::c_dataframe::at(const std::string& name)
{
	auto it = this->m_data.find(name);
	if (it != this->m_data.end()) { return it->second; }

	std::vector<value_t> v;
	v.resize(this->m_rows, value_t{std::string{}});

	auto [ins, ok] = this->m_data.emplace(name, std::move(v));
	return ins->second;
}

bool n_utils::n_data::c_dataframe::from_csv(const std::string& csv_path, bool header)
{
	if (csv_path.empty()) { return false; }

	if (this->m_data.size() || this->m_rows)
	{
		c_logger::get().log("[c_dataframe]: Already initialized\n", LOG_CLR_RED);
		return false;
	}

	std::ifstream file(csv_path);
	if (!file.is_open()) { return false; }

	std::vector<std::vector<std::string>> values;

	std::string line;
	while (std::getline(file, line))
	{
		std::vector<std::string> fields = _parse_csv_line(line);
		values.resize(fields.size());
		for (int i = 0; i < fields.size(); ++i) { values[i].push_back(fields[i]); }
	}

	if (header)
	{
		this->m_rows = values.back().size() - 1;

		for (size_t i = 0; i < values.size(); ++i)
		{
			const std::string name = values[i][0];

			for (size_t j = 1; j < values[i].size(); ++j)
			{
				this->m_data[name].push_back(_to_value(values[i][j]));
			}
		}
	}
	else
	{
		this->m_rows = values.back().size();

		for (size_t i = 0; i < values.size(); ++i)
		{
			const std::string name = "col_" + std::to_string(i);

			for (size_t j = 0; j < values[i].size(); ++j)
			{
				this->m_data[name].push_back(_to_value(values[i][j]));
			}
		}
	}

	return true;
}

bool n_utils::n_data::c_dataframe::to_csv(const std::string& csv_path, bool header) const
{
	if (csv_path.empty() || this->m_data.empty() || !this->m_rows) { return false; }

	std::ofstream file(csv_path, std::ios::out);
	if (!file.is_open()) { return false; }

	if (header)
	{
		int i = 0;
		for (const auto& [key, value] : this->m_data)
		{
			file << key;
			if (i + 1 < this->m_data.size()) { file << ','; }
			i++;
		}
	}

	for (size_t i = 0; i < this->m_rows; ++i)
	{
		if (header) { file << "\n"; }

		int j = 0;
		for (const auto& [key, value] : this->m_data)
		{
			const value_t& v = value[i];
			file << _value_to_string(v);
			if (j + 1 < this->m_data.size()) { file << ','; }
			j++;
		}

		if (!header) { file << "\n"; }
	}

	return true;
}

std::pair<size_t, size_t> n_utils::n_data::c_dataframe::shape() const
{
	return { this->m_rows, this->m_data.size() };
}

double n_utils::n_data::c_dataframe::sum(const std::string& name, size_t from, size_t length) const
{
	if (name.empty())
	{
		c_logger::get().log("[c_dataframe]: No column name\n", LOG_CLR_RED);
		return std::numeric_limits<double>::quiet_NaN();
	}

	auto it = this->m_data.find(name);
	if (it == this->m_data.end()) { return std::numeric_limits<double>::quiet_NaN(); }

	const auto& col = it->second;

	const size_t n = col.size();
	if (n == 0) { return std::numeric_limits<double>::quiet_NaN(); }

	if (length == 0 || from + length > n) { length = n - from; }
	if (from >= n || length == 0) { return std::numeric_limits<double>::quiet_NaN(); }

	double sum = 0.0;
	for (size_t i = from; i < from + length; ++i)
	{
		double d = std::numeric_limits<double>::quiet_NaN();

		if (std::holds_alternative<double>(col[i]))
		{
			d = std::get<double>(col[i]);
		}
		else
		{
			const std::string& s = std::get<std::string>(col[i]);
			if (_is_blank(s)) { continue; }
			auto parsed = _parse_double(s);
			if (!parsed.has_value()) { continue; }
			d = *parsed;
		}

		if (!std::isnan(d))
		{
			sum += d;
		}
	}

	return sum;
}

double n_utils::n_data::c_dataframe::prod(const std::string& name, size_t from, size_t length) const
{
	if (name.empty())
	{
		c_logger::get().log("[c_dataframe]: No column name\n", LOG_CLR_RED);
		return std::numeric_limits<double>::quiet_NaN();
	}

	auto it = this->m_data.find(name);
	if (it == this->m_data.end()) { return std::numeric_limits<double>::quiet_NaN(); }

	const auto& col = it->second;

	const size_t n = col.size();
	if (n == 0) { return std::numeric_limits<double>::quiet_NaN(); }

	if (length == 0 || from + length > n) { length = n - from; }
	if (from >= n || length == 0) { return std::numeric_limits<double>::quiet_NaN(); }

	double prod = 1.0;
	for (size_t i = from; i < from + length; ++i)
	{
		double d = std::numeric_limits<double>::quiet_NaN();

		if (std::holds_alternative<double>(col[i]))
		{
			d = std::get<double>(col[i]);
		}
		else
		{
			const std::string& s = std::get<std::string>(col[i]);
			if (_is_blank(s)) { continue; }
			auto parsed = _parse_double(s);
			if (!parsed.has_value()) { continue; }
			d = *parsed;
		}

		if (!std::isnan(d))
		{
			prod *= d;
		}
	}

	return prod;
}

double n_utils::n_data::c_dataframe::mean(const std::string& name, size_t from, size_t length) const
{
	if (name.empty())
	{
		c_logger::get().log("[c_dataframe]: No column name\n", LOG_CLR_RED);
		return std::numeric_limits<double>::quiet_NaN();
	}

	auto it = this->m_data.find(name);
	if (it == this->m_data.end()) { return std::numeric_limits<double>::quiet_NaN(); }

	const auto& col = it->second;

	const size_t n = col.size();
	if (n == 0) { return std::numeric_limits<double>::quiet_NaN(); }

	if (length == 0 || from + length > n) { length = n - from; }
	if (from >= n || length == 0) { return std::numeric_limits<double>::quiet_NaN(); }

	double mean = 0.0;
	for (size_t i = from; i < from + length; ++i)
	{
		double d = std::numeric_limits<double>::quiet_NaN();

		if (std::holds_alternative<double>(col[i]))
		{
			d = std::get<double>(col[i]);
		}
		else
		{
			const std::string& s = std::get<std::string>(col[i]);
			if (_is_blank(s)) { continue; }
			auto parsed = _parse_double(s);
			if (!parsed.has_value()) { continue; }
			d = *parsed;
		}

		if (!std::isnan(d))
		{
			mean += d;
		}
	}

	return mean / static_cast<double>(length);
}

double n_utils::n_data::c_dataframe::var(const std::string& name, size_t from, size_t length) const
{
	if (name.empty())
	{
		c_logger::get().log("[c_dataframe]: No column name\n", LOG_CLR_RED);
		return std::numeric_limits<double>::quiet_NaN();
	}

	auto it = this->m_data.find(name);
	if (it == this->m_data.end()) { return std::numeric_limits<double>::quiet_NaN(); }

	const auto& col = it->second;

	const size_t n = col.size();
	if (n == 0) { return std::numeric_limits<double>::quiet_NaN(); }

	if (length == 0 || from + length > n) { length = n - from; }
	if (from >= n || length == 0) { return std::numeric_limits<double>::quiet_NaN(); }

	if (length < 2) { return 0.0; }

	double mean = 0.0;
	double m2 = 0.0;
	size_t k = 0;
	for (size_t i = from; i < from + length; ++i)
	{
		double d = std::numeric_limits<double>::quiet_NaN();

		if (std::holds_alternative<double>(col[i]))
		{
			d = std::get<double>(col[i]);
		}
		else
		{
			const std::string& s = std::get<std::string>(col[i]);
			if (_is_blank(s)) { continue; }
			auto parsed = _parse_double(s);
			if (!parsed.has_value()) { continue; }
			d = *parsed;
		}

		if (!std::isnan(d))
		{
			++k;
			const double delta = d - mean;
			mean += delta / static_cast<double>(k);
			m2 += delta * (d - mean);
		}
	}

	return m2 / static_cast<double>(length - 1);
}

double n_utils::n_data::c_dataframe::std(const std::string& name, size_t from, size_t length) const
{
	double v = this->var(name, from, length);
	return std::isnan(v) ? v : std::sqrt(v);
}

double n_utils::n_data::c_dataframe::min_(const std::string& name, size_t from, size_t length) const
{
	if (name.empty())
	{
		c_logger::get().log("[c_dataframe]: No column name\n", LOG_CLR_RED);
		return std::numeric_limits<double>::quiet_NaN();
	}

	auto it = this->m_data.find(name);
	if (it == this->m_data.end()) { return std::numeric_limits<double>::quiet_NaN(); }

	const auto& col = it->second;

	const size_t n = col.size();
	if (n == 0) { return std::numeric_limits<double>::quiet_NaN(); }

	if (length == 0 || from + length > n) { length = n - from; }
	if (from >= n || length == 0) { return std::numeric_limits<double>::quiet_NaN(); }

	double min = DBL_MAX;
	for (size_t i = from; i < from + length; ++i)
	{
		double d = std::numeric_limits<double>::quiet_NaN();

		if (std::holds_alternative<double>(col[i]))
		{
			d = std::get<double>(col[i]);
		}
		else
		{
			const std::string& s = std::get<std::string>(col[i]);
			if (_is_blank(s)) { continue; }
			auto parsed = _parse_double(s);
			if (!parsed.has_value()) { continue; }
			d = *parsed;
		}

		if (!std::isnan(d))
		{
			if (d < min) { min = d; }
		}
	}

	return min;
}

double n_utils::n_data::c_dataframe::max_(const std::string& name, size_t from, size_t length) const
{
	if (name.empty())
	{
		c_logger::get().log("[c_dataframe]: No column name\n", LOG_CLR_RED);
		return std::numeric_limits<double>::quiet_NaN();
	}

	auto it = this->m_data.find(name);
	if (it == this->m_data.end()) { return std::numeric_limits<double>::quiet_NaN(); }

	const auto& col = it->second;

	const size_t n = col.size();
	if (n == 0) { return std::numeric_limits<double>::quiet_NaN(); }

	if (length == 0 || from + length > n) { length = n - from; }
	if (from >= n || length == 0) { return std::numeric_limits<double>::quiet_NaN(); }

	double max = DBL_MIN;
	for (size_t i = from; i < from + length; ++i)
	{
		double d = std::numeric_limits<double>::quiet_NaN();

		if (std::holds_alternative<double>(col[i]))
		{
			d = std::get<double>(col[i]);
		}
		else
		{
			const std::string& s = std::get<std::string>(col[i]);
			if (_is_blank(s)) { continue; }
			auto parsed = _parse_double(s);
			if (!parsed.has_value()) { continue; }
			d = *parsed;
		}

		if (!std::isnan(d))
		{
			if (d > max) { max = d; }
		}
	}

	return max;
}

double n_utils::n_data::c_dataframe::cov(const std::string& name1, const std::string& name2, size_t from, size_t length) const
{
	if (name1.empty() || name2.empty())
	{
		c_logger::get().log("[c_dataframe]: No column name\n", LOG_CLR_RED);
		return std::numeric_limits<double>::quiet_NaN();
	}

	auto it1 = this->m_data.find(name1);
	if (it1 == this->m_data.end()) { return std::numeric_limits<double>::quiet_NaN(); }

	auto it2 = this->m_data.find(name2);
	if (it2 == this->m_data.end()) { return std::numeric_limits<double>::quiet_NaN(); }

	const auto& col1 = it1->second;
	const auto& col2 = it2->second;

	const size_t n = col1.size();
	if (n == 0) { return std::numeric_limits<double>::quiet_NaN(); }

	if (length == 0 || from + length > n) { length = n - from; }
	if (from >= n || length == 0) { return std::numeric_limits<double>::quiet_NaN(); }

	double mean1 = this->mean(name1, from, length);
	double mean2 = this->mean(name2, from, length);
	double covariance = 0.0;
	for (size_t i = from; i < from + length; ++i)
	{
		double d1 = std::numeric_limits<double>::quiet_NaN();
		double d2 = std::numeric_limits<double>::quiet_NaN();

		if (std::holds_alternative<double>(col1[i]))
		{
			d1 = std::get<double>(col1[i]);
		}
		else
		{
			const std::string& s = std::get<std::string>(col1[i]);
			if (_is_blank(s)) { continue; }
			auto parsed = _parse_double(s);
			if (!parsed.has_value()) { continue; }
			d1 = *parsed;
		}

		if (std::holds_alternative<double>(col2[i]))
		{
			d2 = std::get<double>(col2[i]);
		}
		else
		{
			const std::string& s = std::get<std::string>(col2[i]);
			if (_is_blank(s)) { continue; }
			auto parsed = _parse_double(s);
			if (!parsed.has_value()) { continue; }
			d2 = *parsed;
		}

		if (!std::isnan(d1) && !std::isnan(d2))
		{
			covariance += ((d1 - mean1) * (d2 - mean2));
		}
	}

	return covariance / (static_cast<double>(length) - 1);
}

double n_utils::n_data::c_dataframe::corr(const std::string& name1, const std::string& name2, size_t from, size_t length) const
{
	if (name1.empty() || name2.empty())
	{
		c_logger::get().log("[c_dataframe]: No column name\n", LOG_CLR_RED);
		return std::numeric_limits<double>::quiet_NaN();
	}

	double covariance = this->cov(name1, name2, from, length);
	if (std::isnan(covariance)) { return std::numeric_limits<double>::quiet_NaN(); }

	double std1 = this->std(name1, from, length);
	if (std::isnan(std1)) { return std::numeric_limits<double>::quiet_NaN(); }

	double std2 = this->std(name2, from, length);
	if (std::isnan(std2)) { return std::numeric_limits<double>::quiet_NaN(); }

	return covariance / (std1 * std2);
}

double n_utils::n_data::c_dataframe::skew(const std::string& name, size_t from, size_t length) const
{
	if (name.empty())
	{
		c_logger::get().log("[c_dataframe]: No column name\n", LOG_CLR_RED);
		return std::numeric_limits<double>::quiet_NaN();
	}

	auto it = this->m_data.find(name);
	if (it == this->m_data.end()) { return std::numeric_limits<double>::quiet_NaN(); }

	const auto& col = it->second;

	const size_t n = col.size();
	if (n == 0) { return std::numeric_limits<double>::quiet_NaN(); }

	if (length == 0 || from + length > n) { length = n - from; }
	if (from >= n || length == 0) { return std::numeric_limits<double>::quiet_NaN(); }

	double mean = this->mean(name, from, length);
	double std = this->std(name, from, length);
	double skewness = 0.0;
	for (size_t i = from; i < from + length; ++i)
	{
		double d = std::numeric_limits<double>::quiet_NaN();

		if (std::holds_alternative<double>(col[i]))
		{
			d = std::get<double>(col[i]);
		}
		else
		{
			const std::string& s = std::get<std::string>(col[i]);
			if (_is_blank(s)) { continue; }
			auto parsed = _parse_double(s);
			if (!parsed.has_value()) { continue; }
			d = *parsed;
		}

		if (!std::isnan(d))
		{
			skewness += std::pow((d - mean) / std, 3.0);
		}
	}

	return skewness / static_cast<double>(length);
}

double n_utils::n_data::c_dataframe::kurt(const std::string& name, size_t from, size_t length) const
{
	if (name.empty())
	{
		c_logger::get().log("[c_dataframe]: No column name\n", LOG_CLR_RED);
		return std::numeric_limits<double>::quiet_NaN();
	}

	auto it = this->m_data.find(name);
	if (it == this->m_data.end()) { return std::numeric_limits<double>::quiet_NaN(); }

	const auto& col = it->second;

	const size_t n = col.size();
	if (n == 0) { return std::numeric_limits<double>::quiet_NaN(); }

	if (length == 0 || from + length > n) { length = n - from; }
	if (from >= n || length == 0) { return std::numeric_limits<double>::quiet_NaN(); }

	double mean = this->mean(name, from, length);
	double std = this->std(name, from, length);
	double kurtosis = 0.0;
	for (size_t i = from; i < from + length; ++i)
	{
		double d = std::numeric_limits<double>::quiet_NaN();

		if (std::holds_alternative<double>(col[i]))
		{
			d = std::get<double>(col[i]);
		}
		else
		{
			const std::string& s = std::get<std::string>(col[i]);
			if (_is_blank(s)) { continue; }
			auto parsed = _parse_double(s);
			if (!parsed.has_value()) { continue; }
			d = *parsed;
		}

		if (!std::isnan(d))
		{
			kurtosis += std::pow((d - mean) / std, 4.0);
		}
	}

	return (kurtosis / static_cast<double>(length) - 3.0);
}

std::vector<double> n_utils::n_data::c_dataframe::diff(const std::string& name, size_t from, size_t length) const
{
	if (name.empty())
	{
		c_logger::get().log("[c_dataframe]: No column name\n", LOG_CLR_RED);
		return {};
	}

	auto it = this->m_data.find(name);
	if (it == this->m_data.end()) { return {}; }

	const auto& col = it->second;

	const size_t n = col.size();
	if (n == 0) { return {}; }

	if (length == 0 || from + length > n) { length = n - from; }
	if (from >= n || length == 0) { return {}; }

	std::vector<double> vals;
	vals.reserve(length);
	for (size_t i = from; i < from + length; ++i)
	{
		double d = std::numeric_limits<double>::quiet_NaN();

		if (std::holds_alternative<double>(col[i]))
		{
			d = std::get<double>(col[i]);
		}
		else
		{
			const std::string& s = std::get<std::string>(col[i]);
			if (_is_blank(s)) { continue; }
			auto parsed = _parse_double(s);
			if (!parsed.has_value()) { continue; }
			d = *parsed;
		}

		if (!std::isnan(d))
		{
			vals.emplace_back(d);
		}
	}

	std::vector<double> diffs = n_utils::n_math::first_diff(vals);
	diffs.insert(diffs.begin(), std::numeric_limits<double>::quiet_NaN());

	return diffs;
}

std::vector<double> n_utils::n_data::c_dataframe::diff(const std::string& name1, const std::string& name2, size_t from, size_t length) const
{
	if (name1.empty() || name2.empty())
	{
		c_logger::get().log("[c_dataframe]: No column name\n", LOG_CLR_RED);
		return {};
	}

	auto it1 = this->m_data.find(name1);
	if (it1 == this->m_data.end()) { return {}; }

	auto it2 = this->m_data.find(name2);
	if (it2 == this->m_data.end()) { return {}; }

	const auto& col1 = it1->second;
	const auto& col2 = it2->second;

	const size_t n = col1.size();
	if (n == 0) { return {}; }

	if (length == 0 || from + length > n) { length = n - from; }
	if (from >= n || length == 0) { return {}; }

	std::vector<double> vals;
	vals.reserve(length);
	for (size_t i = from; i < from + length; ++i)
	{
		double d1 = std::numeric_limits<double>::quiet_NaN();
		double d2 = std::numeric_limits<double>::quiet_NaN();

		if (std::holds_alternative<double>(col1[i]))
		{
			d1 = std::get<double>(col1[i]);
		}
		else
		{
			const std::string& s = std::get<std::string>(col1[i]);
			if (_is_blank(s)) { continue; }
			auto parsed = _parse_double(s);
			if (!parsed.has_value()) { continue; }
			d1 = *parsed;
		}

		if (std::holds_alternative<double>(col2[i]))
		{
			d2 = std::get<double>(col2[i]);
		}
		else
		{
			const std::string& s = std::get<std::string>(col2[i]);
			if (_is_blank(s)) { continue; }
			auto parsed = _parse_double(s);
			if (!parsed.has_value()) { continue; }
			d2 = *parsed;
		}

		if (!std::isnan(d1) && !std::isnan(d2))
		{
			vals.emplace_back(d1 - d2);
		}
	}

	return vals;
}

std::vector<double> n_utils::n_data::c_dataframe::pct_change(const std::string& name, size_t from, size_t length) const
{
	if (name.empty())
	{
		c_logger::get().log("[c_dataframe]: No column name\n", LOG_CLR_RED);
		return {};
	}

	auto it = this->m_data.find(name);
	if (it == this->m_data.end()) { return {}; }

	const auto& col = it->second;

	const size_t n = col.size();
	if (n == 0) { return {}; }

	if (length == 0 || from + length > n) { length = n - from; }
	if (from >= n || length == 0) { return {}; }

	std::vector<double> vals;
	vals.reserve(length);
	for (size_t i = from; i < from + length; ++i)
	{
		double d = std::numeric_limits<double>::quiet_NaN();

		if (std::holds_alternative<double>(col[i]))
		{
			d = std::get<double>(col[i]);
		}
		else
		{
			const std::string& s = std::get<std::string>(col[i]);
			if (_is_blank(s)) { continue; }
			auto parsed = _parse_double(s);
			if (!parsed.has_value()) { continue; }
			d = *parsed;
		}

		if (!std::isnan(d))
		{
			vals.emplace_back(d);
		}
	}

	std::vector<double> changes = n_utils::n_math::relative_change_vector(vals);
	changes.insert(changes.begin(), std::numeric_limits<double>::quiet_NaN());

	return changes;
}

std::vector<double> n_utils::n_data::c_dataframe::pct_change(const std::string& name1, const std::string& name2, size_t from, size_t length) const
{
	if (name1.empty() || name2.empty())
	{
		c_logger::get().log("[c_dataframe]: No column name\n", LOG_CLR_RED);
		return {};
	}

	auto it1 = this->m_data.find(name1);
	if (it1 == this->m_data.end()) { return {}; }

	auto it2 = this->m_data.find(name2);
	if (it2 == this->m_data.end()) { return {}; }

	const auto& col1 = it1->second;
	const auto& col2 = it2->second;

	const size_t n = col1.size();
	if (n == 0) { return {}; }

	if (length == 0 || from + length > n) { length = n - from; }
	if (from >= n || length == 0) { return {}; }

	std::vector<double> vals;
	vals.reserve(length);
	for (size_t i = from; i < from + length; ++i)
	{
		double d1 = std::numeric_limits<double>::quiet_NaN();
		double d2 = std::numeric_limits<double>::quiet_NaN();

		if (std::holds_alternative<double>(col1[i]))
		{
			d1 = std::get<double>(col1[i]);
		}
		else
		{
			const std::string& s = std::get<std::string>(col1[i]);
			if (_is_blank(s)) { continue; }
			auto parsed = _parse_double(s);
			if (!parsed.has_value()) { continue; }
			d1 = *parsed;
		}

		if (std::holds_alternative<double>(col2[i]))
		{
			d2 = std::get<double>(col2[i]);
		}
		else
		{
			const std::string& s = std::get<std::string>(col2[i]);
			if (_is_blank(s)) { continue; }
			auto parsed = _parse_double(s);
			if (!parsed.has_value()) { continue; }
			d2 = *parsed;
		}

		if (!std::isnan(d1) && !std::isnan(d2))
		{
			vals.emplace_back(n_utils::n_math::relative_change(d1, d2));
		}
	}

	return vals;
}

std::vector<double> n_utils::n_data::c_dataframe::log_change(const std::string& name, size_t from, size_t length) const
{
	if (name.empty())
	{
		c_logger::get().log("[c_dataframe]: No column name\n", LOG_CLR_RED);
		return {};
	}

	auto it = this->m_data.find(name);
	if (it == this->m_data.end()) { return {}; }

	const auto& col = it->second;

	const size_t n = col.size();
	if (n == 0) { return {}; }

	if (length == 0 || from + length > n) { length = n - from; }
	if (from >= n || length == 0) { return {}; }

	std::vector<double> vals;
	vals.reserve(length);
	for (size_t i = from; i < from + length; ++i)
	{
		double d = std::numeric_limits<double>::quiet_NaN();

		if (std::holds_alternative<double>(col[i]))
		{
			d = std::get<double>(col[i]);
		}
		else
		{
			const std::string& s = std::get<std::string>(col[i]);
			if (_is_blank(s)) { continue; }
			auto parsed = _parse_double(s);
			if (!parsed.has_value()) { continue; }
			d = *parsed;
		}

		if (!std::isnan(d))
		{
			vals.emplace_back(d);
		}
	}

	std::vector<double> changes = n_utils::n_math::log_relative_change_vector(vals);
	changes.insert(changes.begin(), std::numeric_limits<double>::quiet_NaN());

	return changes;
}

std::vector<double> n_utils::n_data::c_dataframe::log_change(const std::string& name1, const std::string& name2, size_t from, size_t length) const
{
	if (name1.empty() || name2.empty())
	{
		c_logger::get().log("[c_dataframe]: No column name\n", LOG_CLR_RED);
		return {};
	}

	auto it1 = this->m_data.find(name1);
	if (it1 == this->m_data.end()) { return {}; }

	auto it2 = this->m_data.find(name2);
	if (it2 == this->m_data.end()) { return {}; }

	const auto& col1 = it1->second;
	const auto& col2 = it2->second;

	const size_t n = col1.size();
	if (n == 0) { return {}; }

	if (length == 0 || from + length > n) { length = n - from; }
	if (from >= n || length == 0) { return {}; }

	std::vector<double> vals;
	vals.reserve(length);
	for (size_t i = from; i < from + length; ++i)
	{
		double d1 = std::numeric_limits<double>::quiet_NaN();
		double d2 = std::numeric_limits<double>::quiet_NaN();

		if (std::holds_alternative<double>(col1[i]))
		{
			d1 = std::get<double>(col1[i]);
		}
		else
		{
			const std::string& s = std::get<std::string>(col1[i]);
			if (_is_blank(s)) { continue; }
			auto parsed = _parse_double(s);
			if (!parsed.has_value()) { continue; }
			d1 = *parsed;
		}

		if (std::holds_alternative<double>(col2[i]))
		{
			d2 = std::get<double>(col2[i]);
		}
		else
		{
			const std::string& s = std::get<std::string>(col2[i]);
			if (_is_blank(s)) { continue; }
			auto parsed = _parse_double(s);
			if (!parsed.has_value()) { continue; }
			d2 = *parsed;
		}

		if (!std::isnan(d1) && !std::isnan(d2))
		{
			vals.emplace_back(n_utils::n_math::log_relative_change(d1, d2));
		}
	}

	return vals;
}

std::vector<double> n_utils::n_data::c_dataframe::cumsum(const std::string& name, size_t from, size_t length) const
{
	if (name.empty())
	{
		c_logger::get().log("[c_dataframe]: No column name\n", LOG_CLR_RED);
		return {};
	}

	auto it = this->m_data.find(name);
	if (it == this->m_data.end()) { return {}; }

	const auto& col = it->second;

	const size_t n = col.size();
	if (n == 0) { return {}; }

	if (length == 0 || from + length > n) { length = n - from; }
	if (from >= n || length == 0) { return {}; }

	std::vector<double> vals;
	vals.reserve(length);
	for (size_t i = from; i < from + length; ++i)
	{
		double d = std::numeric_limits<double>::quiet_NaN();

		if (std::holds_alternative<double>(col[i]))
		{
			d = std::get<double>(col[i]);
		}
		else
		{
			const std::string& s = std::get<std::string>(col[i]);
			if (_is_blank(s)) { continue; }
			auto parsed = _parse_double(s);
			if (!parsed.has_value()) { continue; }
			d = *parsed;
		}

		if (!std::isnan(d))
		{
			vals.emplace_back(d);
		}
	}

	std::vector<double> cumsum;
	cumsum.reserve(length);
	cumsum.emplace_back(vals[0]);
	for (size_t i = 1; i < vals.size(); ++i) { cumsum.emplace_back(cumsum.back() + vals[i]); }

	return cumsum;
}

std::vector<double> n_utils::n_data::c_dataframe::cumprod(const std::string& name, size_t from, size_t length) const
{
	if (name.empty())
	{
		c_logger::get().log("[c_dataframe]: No column name\n", LOG_CLR_RED);
		return {};
	}

	auto it = this->m_data.find(name);
	if (it == this->m_data.end()) { return {}; }

	const auto& col = it->second;

	const size_t n = col.size();
	if (n == 0) { return {}; }

	if (length == 0 || from + length > n) { length = n - from; }
	if (from >= n || length == 0) { return {}; }

	std::vector<double> vals;
	vals.reserve(length);
	for (size_t i = from; i < from + length; ++i)
	{
		double d = std::numeric_limits<double>::quiet_NaN();

		if (std::holds_alternative<double>(col[i]))
		{
			d = std::get<double>(col[i]);
		}
		else
		{
			const std::string& s = std::get<std::string>(col[i]);
			if (_is_blank(s)) { continue; }
			auto parsed = _parse_double(s);
			if (!parsed.has_value()) { continue; }
			d = *parsed;
		}

		if (!std::isnan(d))
		{
			vals.emplace_back(d);
		}
	}

	std::vector<double> cumprod;
	cumprod.reserve(length);
	cumprod.emplace_back(vals[0]);
	for (size_t i = 1; i < vals.size(); ++i) { cumprod.emplace_back(cumprod.back() * vals[i]); }

	return cumprod;
}

void n_utils::n_data::c_dataframe::reset()
{
	this->m_data.clear();
	this->m_rows = 0;
}

n_utils::n_data::c_dataframe n_utils::n_data::c_dataframe::head(size_t n) const
{
	if (this->m_data.empty() || !this->m_rows || !n)
	{
		c_logger::get().log("[c_dataframe]: No data\n", LOG_CLR_RED);
		return c_dataframe();
	}

	c_dataframe df;
	size_t num = min(n, this->m_rows);

	for (const auto& [name, col] : this->m_data)
	{
		std::vector<value_t> sub(col.begin(), col.begin() + num);
		df.m_data.emplace(name, std::move(sub));
	}

	df.m_rows = num;
	return df;
}

n_utils::n_data::c_dataframe n_utils::n_data::c_dataframe::tail(size_t n) const
{
	if (this->m_data.empty() || !this->m_rows || !n)
	{
		c_logger::get().log("[c_dataframe]: No data\n", LOG_CLR_RED);
		return c_dataframe();
	}

	c_dataframe df;
	size_t num = min(n, this->m_rows);

	for (const auto& [name, col] : this->m_data)
	{
		std::vector<value_t> sub(col.end() - num, col.end());
		df.m_data.emplace(name, std::move(sub));
	}

	df.m_rows = num;
	return df;
}

void n_utils::n_data::c_dataframe::print(size_t n) const
{
	if (this->m_data.empty() || !this->m_rows || !n)
	{
		c_logger::get().log("[c_dataframe]: No data\n", LOG_CLR_RED);
		return;
	}

	c_logger::get().log_formatted("\t", LOG_CLR_NORMAL, 30);
	for (const auto& [key, value] : this->m_data) { c_logger::get().log_formatted(key, LOG_CLR_YELLOW, 30); }
	std::cout << std::endl << std::endl;

	size_t num = min(n, this->m_rows);
	for (size_t i = 0; i < num; ++i)
	{
		c_logger::get().log_formatted("\t" + std::to_string(i), LOG_CLR_YELLOW, 30);
		for (const auto& [key, value] : this->m_data)
		{
			c_logger::get().log_formatted(_value_to_string(value[i]), LOG_CLR_NORMAL, 30);
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}
