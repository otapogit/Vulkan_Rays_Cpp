#pragma once

// STL
#include <cstdint>
#include <limits>
#include <ostream>

namespace autis
{

	constexpr uint32_t INVALID_UNSIGNED_INT_ID_VALUE = std::numeric_limits<uint32_t>::max();

	template <typename TAG>
	struct UnsignedIntIdentifier
	{
		// Explicit constructor to prevent implicit conversions
		explicit UnsignedIntIdentifier(uint32_t id = INVALID_UNSIGNED_INT_ID_VALUE) : id(id) {}

		uint32_t get() const { return id; }
		uint32_t& at() { return id; }

		// No upper bound checking is performed
		UnsignedIntIdentifier nextId(uint32_t step = 1) const     { return UnsignedIntIdentifier(id + step); }
		// No lower bound checking is performed
		UnsignedIntIdentifier previousId(uint32_t step = 1) const { return UnsignedIntIdentifier(id - step); }

		std::string toString() const { return std::to_string(id); }

		bool operator<(const UnsignedIntIdentifier& other) const { return id < other.id; }
		bool operator>(const UnsignedIntIdentifier& other) const { return id > other.id; }
		bool operator==(const UnsignedIntIdentifier& other) const { return id == other.id; }
		bool operator!=(const UnsignedIntIdentifier& other) const { return id != other.id; }
		UnsignedIntIdentifier& operator++() { id++; return *this; }
		UnsignedIntIdentifier operator++(int)
		{
			UnsignedIntIdentifier temp = *this;
			++(*this);
			return temp;
		}

		friend inline std::ostream& operator<<(std::ostream& os, const UnsignedIntIdentifier<TAG>& dt)
		{
			os << dt.id;
			return os;
		}

		friend inline std::string operator+(const std::string& lhs, const UnsignedIntIdentifier<TAG>& uid)
		{
			return lhs + uid.toString();
		}

		friend inline std::string operator+(const char* lhs, const UnsignedIntIdentifier<TAG>& uid)
		{
			return std::string(lhs) + uid.toString();
		}

		friend inline std::string operator+(const UnsignedIntIdentifier<TAG>& uid, const std::string& rhs)
		{
			return uid.toString() + rhs;
		}

		friend inline std::string operator+(const UnsignedIntIdentifier<TAG>& uid, const char* rhs)
		{
			return uid.toString() + std::string(rhs);
		}

	private:
		uint32_t id;
	};

}

// Hash function definition for UnsignedIntIdentifier
template <typename TAG>
struct std::hash<autis::UnsignedIntIdentifier<TAG>>
{
	std::size_t operator()(const autis::UnsignedIntIdentifier<TAG>& uid) const
	{
		return hash<int>()(uid.get());
	}
};
