#include "Serpent.hpp"

namespace std {
	std::size_t hash<Serpent>::operator()(const Serpent& s) const {
		std::size_t valueHash = 0;
		switch (s.Type()) {
			case Serpent::Boolean:
				valueHash = hash<bool>()(s.As<bool>());
				break;
			case Serpent::Number:
				valueHash = hash<double>()(s.As<double>());
				break;
			case Serpent::String:
				valueHash = hash<std::string>()(s.As<std::string>());
				break;
			case Serpent::Null:
				break;
			case Serpent::Table: {
				for (auto& child : s) {
					valueHash = (valueHash << 1) ^ (operator()(child.first));
				}
				break;
			}
		}
		return (static_cast<std::size_t>(s.Type()) << 8) ^ valueHash;
	}

	std::size_t hash<std::reference_wrapper<Serpent>>::operator()(const std::reference_wrapper<Serpent>& s) const {
		return hash<Serpent>()(s.get());
	}
}

bool operator==(const Serpent& lhs, const Serpent& rhs) {
	if (lhs.Type() != rhs.Type()) {
		return false;
	}
	switch (lhs.Type()) {
		case Serpent::Boolean:
			return lhs.As<bool>() == rhs.As<bool>();
		case Serpent::Number:
			return lhs.As<double>() == rhs.As<double>();
		case Serpent::Null:
			return true;
		case Serpent::Table:
			return lhs.As<double>() == rhs.As<double>();
		case Serpent::String:
			return lhs.As<std::string>() == rhs.As<std::string>();
	}
	throw new Serpent::Exception("Invalid type");
}

Serpent::~Serpent() {
	if (m_valueType == String) {
		m_string.~basic_string();
	}
	if (m_children) {
		delete m_children;
	}
}

Serpent::Serpent() : m_valueType(Null), m_children(nullptr) {

}

Serpent::Serpent(const Serpent& other) : m_valueType(Null), m_children(nullptr) {
	this->operator=(other);
}

Serpent& Serpent::operator=(const Serpent& other) {
	if (m_valueType == String) {
		if (other.Type() == String) {
			m_string = other.m_string;
			return *this;
		}
		m_string.~basic_string();
	}
	if (m_valueType == Table) {
		delete m_children;
		m_children = nullptr;
	}
	m_valueType = other.Type();
	switch (m_valueType) {
		case Boolean:
			m_boolean = other.m_boolean;
			break;
		case Number:
			m_number = other.m_number;
			break;
		case String:
			new(&m_string) std::string(other.m_string);
			break;
		case Null:
			break;
		case Table:
			m_children = new SerpentChildren(*other.Children());
			break;
	}
	return *this;
}


std::pair<Serpent, Serpent>* Serpent::PairAt(size_t index) {
	if (m_children && index < m_children->children.size()) {
		return &m_children->children.at(index);
	}
	return nullptr;
}

std::pair<Serpent, Serpent>* Serpent::begin() {
	if (m_children && m_children->children.size()) {
		m_children->children.front();
	}
	return nullptr;
}

std::pair<Serpent, Serpent>* Serpent::end() {
	if (m_children && m_children->children.size()) {
		m_children->children.back();
	}
	return nullptr;
}

Serpent* Serpent::At(size_t index) {
	if (m_children && index < m_children->children.size()) {
		return &m_children->children.at(index).second;
	}
	return nullptr;
}

const std::pair<Serpent, Serpent>* Serpent::end() const {
	if (m_children && m_children->children.size()) {
		m_children->children.back();
	}
	return nullptr;
}

const std::pair<Serpent, Serpent>* Serpent::begin() const {
	if (m_children && m_children->children.size()) {
		m_children->children.front();
	}
	return nullptr;
}

Serpent Serpent::Load(const std::string& str) {
	Serpent base;
	size_t start = 0;
	while (start < str.length() && str[start] != '{') {
		start++;
	}
	if (start == str.length()) {
		throw Exception("Invalid serpent string? Couldn't find opening '{'");
	}
	base.LoadSubstring(str, start);

	return base;
}

void Serpent::LoadSubstring(const std::string& str, size_t& start) {
	switch (str[start]) {
		case '{': {
			size_t elementCount = 0;
			m_valueType = Table;
			m_children = new SerpentChildren();
			start++;
			while (start < str.length() && str[start] != '}') {
				Serpent key;
				key.LoadSubstring(str, start);
				if (str[start] == '=') {
					start++;
					Serpent value;
					value.LoadSubstring(str, start);
					AddChild(key, value);
				} else {
					AddChild(Serpent(elementCount), key);
				}
				elementCount++;
				// Next member :)
				if (str[start] == ',') {
					start++;
				}
			}
			if (start == str.length()) {
				throw Exception("Invalid serpent string? Couldn't find closing '}'");
			}
			start++;
			break;
		}
		case '-':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9': {
			LoadAsNumber(str, start);
			break;
		}
		default: {
			if (start + 4 < str.length() &&
				str[start] == 't' &&
				str[start + 1] == 'r' &&
				str[start + 2] == 'u' &&
				str[start + 3] == 'e' &&
				(str[start + 4] == '=' ||
				 str[start + 4] == ',' ||
				 str[start + 4] == '}')) {
				m_valueType = Boolean;
				m_boolean = true;
				start += 4;
				return;
			}
			if (start + 5 < str.length() &&
				str[start] == 'f' &&
				str[start + 1] == 'a' &&
				str[start + 2] == 'l' &&
				str[start + 3] == 's' &&
				str[start + 4] == 'e' &&
				(str[start + 5] == '=' ||
				 str[start + 5] == ',' ||
				 str[start + 5] == '}')) {
				m_valueType = Boolean;
				m_boolean = false;
				start += 5;
				return;
			}
			if (start + 3 < str.length() &&
				str[start] == 'n' &&
				str[start + 1] == 'i' &&
				str[start + 2] == 'l' &&
				(str[start + 3] == '=' ||
				 str[start + 3] == ',' ||
				 str[start + 3] == '}')) {
				m_valueType = Null;
				start += 3;
				return;
			}
			LoadAsString(str, start);
			break;
		}
	}
}

bool Serpent::LoadAsNumber(const std::string& str, size_t& start) {
	size_t oldStart = start;
	LoadAsString(str, start);
	double value = strtod(m_string.c_str(), nullptr);
	// strtod returns 0 when the value was invalid.. so we need to very if the number isnt 0..
	// 0 or 0 with sign in front
	if (value == 0 && str[oldStart] != '0' && str[oldStart + 1] != '0') {
		return false;
	}
	m_valueType = Number;
	m_number = value;
	return true;
}

void Serpent::LoadAsString(const std::string& str, size_t& start) {
	bool enclosed = str[start] == '"';
	if (enclosed) {
		start++;
	}
	std::string result;
	while (start < str.length()) {
		if (enclosed) {
			if (str[start] == '"') {
				break;
			}
		} else {
			if (str[start] == '=' || str[start] == ',' || str[start] == '}') {
				break;
			}
		}
		result += str[start];
		start++;
	}
	if (start == str.length()) {
		if (enclosed) {
			throw Exception("Invalid serpent string? Couldn't find closing \"");
		}
		throw Exception("Invalid serpent string? Couldn't find string end");
	}
	if (enclosed) {
		start++;
	}
	m_valueType = String;
	new(&m_string) std::string(result);
}


void Serpent::AddChild(const Serpent& key, const Serpent& value) {
	if (!m_children) {
		throw Exception("Not a table");
	}
	if (key.Type() == Table) {
		throw Exception(
				"Having a table as a table key doesn't make any sense - is your serpent file malformed or is this a bug?");
	}
	m_children->children.push_back(std::pair<Serpent, Serpent>(key, value));
	auto& it = m_children->children.back();
	m_children->map.insert(std::pair<Serpent, Serpent*>(it.first, &it.second));
}

Serpent Serpent::NewTable() {
	Serpent s;
	s.m_valueType = Table;
	s.m_children = new SerpentChildren();
	return std::move(s);
}


Serpent::Serpent(std::string string) : m_children(nullptr) {
	m_valueType = String;
	new(&m_string) std::string(string);
}

Serpent::Serpent(const char* string) : Serpent(std::string(string)) {

}

Serpent::Serpent(bool boolean) : m_children(nullptr) {
	m_valueType = Boolean;
	m_boolean = boolean;
}

// Bool
template<>
bool Serpent::As<bool>() const {
	if (m_valueType != Boolean) {
		throw Exception("Node is not a boolean");
	}
	return m_boolean;
};


// String
template<>
std::string Serpent::As<std::string>() const {
	if (m_valueType != String) {
		throw Exception("Node is not a string");
	}
	return m_string;
};

Serpent Serpent::NullSerpent = Serpent();