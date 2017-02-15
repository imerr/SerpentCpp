#ifndef SERPENTCPP_SERPENT_HPP
#define SERPENTCPP_SERPENT_HPP

#include <type_traits>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <stdexcept>

class Serpent;

struct SerpentChildren;

class SerpentIterator;

// Define hash so we can use the Serpent as a key to unordered_map
// http://stackoverflow.com/a/17017281/1318435
namespace std {
	template<>
	struct hash<Serpent> {
		std::size_t operator()(const Serpent& s) const;
	};

	template<>
	struct hash<std::reference_wrapper<Serpent>> {
		std::size_t operator()(const std::reference_wrapper<Serpent>& s) const;
	};
}

class Serpent {
public:
	enum ValueType {
		Boolean,
		Number,
		String,
		Null,
		Table
	};

	class Exception : public std::logic_error {
	public:
		Exception(const std::string& reason) : logic_error(reason) {}
	};

	static Serpent NullSerpent;
protected:
	ValueType m_valueType;
	union {
		bool m_boolean;
		double m_number;
		std::string m_string;
	};

	// TODO: we need to keep the order of items for arrays
	// so have a struct which has a vector (for order) and a map (fast lookups)
	SerpentChildren* m_children;
public:
	Serpent();

	~Serpent();

	template<typename T>
	T As() const {
		static_assert(std::is_arithmetic<T>::value && !std::is_same<T, bool>::value,
					  "Type is not supported by Serpent::As<>");
		if (m_valueType != Number) {
			throw Exception("Node is not a number");
		}
		return static_cast<T>(m_number);
	};

	// Null
	bool IsNull() {
		return m_valueType == Null;
	}

	bool IsNil() {
		return IsNull();
	}

	// Bool
	Serpent(bool boolean);

	bool IsBool() {
		return m_valueType == Boolean;
	}

	// Number
	Serpent(double number) : m_valueType(Number), m_number(number), m_children(nullptr) {
	}

	Serpent(float number) : Serpent(static_cast<double>(number)) {}

	Serpent(int64_t number) : Serpent(static_cast<double>(number)) {}

	Serpent(uint64_t number) : Serpent(static_cast<double>(number)) {}

	Serpent(int32_t number) : Serpent(static_cast<double>(number)) {}

	Serpent(uint32_t number) : Serpent(static_cast<double>(number)) {}

	Serpent(int16_t number) : Serpent(static_cast<double>(number)) {}

	Serpent(uint16_t number) : Serpent(static_cast<double>(number)) {}

	Serpent(int8_t number) : Serpent(static_cast<double>(number)) {}

	Serpent(uint8_t number) : Serpent(static_cast<double>(number)) {}


	bool IsNumber() {
		return m_valueType == Number;
	}

	// String
	Serpent(std::string string);

	Serpent(const char*);

	bool IsString() {
		return m_valueType == String;
	}

	// Table
	bool IsTable() {
		return m_valueType == Table;
	}

	template<typename T>
	Serpent& operator[](const T& value);

	std::pair<Serpent, Serpent>* PairAt(size_t index);

	Serpent* At(size_t index);

	void AddChild(const Serpent& key, const Serpent& value);

	SerpentChildren* Children() {
		return m_children;
	}

	const SerpentChildren* Children() const {
		return m_children;
	};

	// Copying
	Serpent(const Serpent& other);

	Serpent& operator=(const Serpent& other);

	ValueType Type() const {
		return m_valueType;
	}

	std::pair<Serpent, Serpent>* begin();

	const std::pair<Serpent, Serpent>* begin() const;

	std::pair<Serpent, Serpent>* end();

	const std::pair<Serpent, Serpent>* end() const;

	size_t size() const;

	static Serpent Load(const std::string& serpentString);

	static Serpent NewTable();

protected:
	// WARNING: These assume the object is empty and will not destroy
	// resources correctly if used on already populated objects
	void LoadSubstring(const std::string& str, size_t& start);

	// WARNING: These assume the object is empty and will not destroy
	// resources correctly if used on already populated objects
	bool LoadAsNumber(const std::string& str, size_t& start);

	// WARNING: These assume the object is empty and will not destroy
	// resources correctly if used on already populated objects
	void LoadAsString(const std::string& str, size_t& start);
};


struct SerpentChildren {
	SerpentChildren() {

	};

	SerpentChildren(const SerpentChildren& other) : children(other.children) {
		for (auto& child : children) {
			map.insert(std::pair<Serpent, Serpent*>(child.first, &child.second));
		}
	}

	std::vector<std::pair<Serpent, Serpent>> children;
	std::unordered_map<Serpent, Serpent*> map;
};

// Bool
template<>
bool Serpent::As<bool>() const;

// String
template<>
std::string Serpent::As<std::string>() const;

bool operator==(const Serpent& lhs, const Serpent& rhs);

template<typename T>
Serpent& Serpent::operator[](const T& value) {
	if (!IsTable()) {
		return NullSerpent;
	}
	Serpent key(value);
	auto it = m_children->map.find(key);
	if (it == m_children->map.end()) {
		return NullSerpent;
	}
	return *it->second;
}


#endif //SERPENTCPP_SERPENT_HPP
