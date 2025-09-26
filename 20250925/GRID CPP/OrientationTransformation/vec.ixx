export module Vec;

// Optimization can be done using assembly language at the bottom of the mile, especially for cross and dot

import <iostream>;
import <concepts>;
import <format>;
import <ranges>;
import <cmath>;
import <array>;
import <numeric>;
import <algorithm>;

namespace stdr = std::ranges;
namespace stdv = std::ranges::views;

extern "C" {
double __stdcall dotProductDouble_Optimized(const double* m, const double* n);
// For crossProduct, because it returns a struct by value,
// the C++ declaration might be:
void __stdcall crossProductDouble_Optimized(double* ret, const double* m, const double* n);

long __stdcall dotProductLong_Optimized(const long* m, const long* n);
void __stdcall crossProductLong_Optimized(long* ret, const long* m, const long* n);
}

template <typename T>
concept is_int_type = std::is_integral_v<T>;

template <typename T>
concept is_double_type = std::is_floating_point_v<T>;

template <typename T>
concept arithmetic_convertible = is_double_type<T> || is_int_type<T>;

export namespace vec_index {
	template <typename T, typename = void>
		requires arithmetic_convertible<T>
	class vec {
	public:
		std::array<T, 3> coords_{};

		template <typename Self, typename U>
		constexpr void swap(this Self&& self, U&& other) noexcept {
			stdr::swap(std::forward<Self>(self).coords_, std::forward<U>(other).coords_);
		}

		// 5 ctor rules
		constexpr vec() noexcept = default;

		constexpr vec(T x, T y, T z) noexcept : coords_{x, y, z} {
		}

		constexpr vec(const vec& other) noexcept = default;

		constexpr vec(vec&& other) noexcept = default;

		virtual ~vec() = default;


		// use swap-copy idiom
		vec& operator=(const vec& other) noexcept {
			if (this != &other) {
				vec tmp{other};
				this->swap(tmp);
				return *this;
			}
			else {
				return *this;
			}
		}

		vec& operator=(vec&& other) noexcept {
			if (this != &other) {
				vec tmp{std::move(other)};
				this->swap(tmp);
				return *this;
			}
			else {
				return *this;
			}
		}

		// Deducing This Accessors

		template <typename Self>
		constexpr auto operator[](this Self&& self, std::size_t i) -> decltype(auto) {
			return std::forward<Self>(self).coords_[i];
		}

		template <typename Self>
		constexpr auto x(this Self&& self) noexcept -> decltype(auto) {
			return std::forward<Self>(self)[0];
		}

		template <typename Self>
		constexpr auto y(this Self&& self) noexcept -> decltype(auto) {
			return std::forward<Self>(self)[1];
		}


		template <typename Self>
		constexpr auto z(this Self&& self) noexcept -> decltype(auto) {
			return std::forward<Self>(self)[2];
		}

		template <typename Self>
		constexpr T Length(this Self&& self) {
			auto&& s = std::forward<Self>(self);
			T sum_squares = std::transform_reduce(
				s.coords_.begin(), s.coords_.end(), T{}, std::plus<>{}, [](T v) { return v * v; });
			return std::sqrt(sum_squares);
		}

		// Element access
		template <typename Self>
		constexpr auto data(this Self&& self) noexcept -> decltype(auto) {
			return std::span(std::forward<Self>(self).coords_);
		}

		// In-Place arithmetic operators

		constexpr vec& operator+=(const vec& other) noexcept {
			stdr::transform(coords_, other.coords_, coords_.begin(), std::plus<>{});
			return *this;
		}

		constexpr vec& operator-=(const vec& other) noexcept {
			stdr::transform(coords_, other.coords_, coords_.begin(), std::minus<>{});
			return *this;
		}

		constexpr vec& operator*=(const T scalar) noexcept {
			stdr::for_each(coords_, [scalar](T& val) { val *= scalar; });
			return *this;
		}

		constexpr vec& operator/=(const T scalar) noexcept {
			if (std::abs(scalar) <= T(1.0e-6)) {
				std::cout << "Division by zero!" << std::endl;
				return *this;
			}
			T inv_scalar = T(1) / scalar;
			stdr::for_each(coords_, [inv_scalar](T& val) { val *= inv_scalar; });
			return *this;
		}

		// Value Return arithmetic operators
		vec operator+(const vec& other) const noexcept {
			vec result(*this);
			result += other;
			return result;
		}

		vec operator-(const vec& other) const noexcept {
			vec result(*this);
			result -= other;
			return result;
		}

		//[[nodiscard]] T dot(const vec& other) const {
		//	auto indices = std::views::iota(static_cast<std::size_t>(0), coords_.size());
		//	return std::accumulate(indices.begin(), indices.end(), T(0),
		//	                       [this, &other](T sum, std::size_t i) {
		//		                       return sum + coords_[i] * other.coords_[i];
		//	                       });
		//}

		constexpr std::conditional_t<std::floating_point<T>, double, long> Dot(
			const vec& other) const noexcept {
			if (*this == other) {
				return Length() * Length();
			}
			if constexpr (is_double_type<T>) {
				std::array<double, 3> m{};
				std::array<double, 3> n{};
				// use transform as ranges and lambda to change T to double
				stdr::transform(coords_.begin(), coords_.end(), m.begin(),
				                [](T val) { return static_cast<double>(val); });
				stdr::transform(other.coords_.begin(), other.coords_.end(), n.begin(),
				                [](T val) { return static_cast<double>(val); });
				return dotProductDouble_Optimized(m.data(), n.data());
			}
			else if constexpr (is_int_type<T>) {
				std::array<long, 3> m{};
				std::array<long, 3> n{};
				stdr::transform(coords_.begin(), coords_.end(), m.begin(),
				                [](T val) { return static_cast<long>(val); });
				stdr::transform(other.coords_.begin(), other.coords_.end(), n.begin(),
				                [](T val) { return static_cast<long>(val); });
				return dotProductLong_Optimized(m.data(), n.data());
			}
			else {
				return T(0);
			}
		}

		constexpr bool operator==(const vec& other) const {
			if constexpr (std::integral<T>) {
				return coords_ == other.coords_;
			}
			else {
				return (*this - other).Length() <= static_cast<T>(1e-6);
			}
		}

		// Non-member friend arithmetic operators.
		friend constexpr vec operator+(vec lhs, const vec& rhs) noexcept { return lhs += rhs; }
		friend constexpr vec operator-(vec lhs, const vec& rhs) noexcept { return lhs -= rhs; }
		friend constexpr vec operator*(vec v, T scalar) noexcept { return v *= scalar; }
		friend constexpr vec operator*(T scalar, vec v) noexcept { return v *= scalar; }
		friend constexpr vec operator/(vec v, T scalar) noexcept { return v /= scalar; }

		//[[nodiscard]] constexpr vec cross(const vec& other) const {
		//	if (*this == other) {
		//		return vec(0, 0, 0);
		//	}
		//	return vec(coords_[1] * other.coords_[2] - coords_[2] * other.coords_[1],
		//	                 coords_[2] * other.coords_[0] - coords_[0] * other.coords_[2],
		//	                 coords_[0] * other.coords_[1] - coords_[1] * other.coords_[0]);
		//}
		[[nodiscard]] constexpr vec Cross(const vec& other) const {
			if (*this == other) {
				return vec(0, 0, 0);
			}
			if constexpr (is_double_type<T>) {
				std::array<double, 3> m = {};
				std::array<double, 3> n = {};
				stdr::transform(coords_.begin(), coords_.end(), m.begin(),
				                [](T val) { return static_cast<double>(val); });
				stdr::transform(other.coords_.begin(), other.coords_.end(), n.begin(),
				                [](T val) { return static_cast<double>(val); });
				std::array<double, 3> cross{};
				crossProductDouble_Optimized(cross.data(), m.data(), n.data());
				return vec{cross[0], cross[1], cross[2]};
			}
			else if constexpr (is_int_type<T>) {
				std::array<long, 3> m{};
				std::array<long, 3> n{};
				stdr::transform(coords_.begin(), coords_.end(), m.begin(),
				                [](T val) { return static_cast<long>(val); });
				stdr::transform(other.coords_.begin(), other.coords_.end(), n.begin(),
				                [](T val) { return static_cast<long>(val); });
				std::array<long, 3> cross{};
				crossProductLong_Optimized(cross.data(), m.data(), n.data());
				return vec{cross[0], cross[1], cross[2]};
			}
			else {
				return vec(0, 0, 0);
			}
		}

		[[nodiscard]] constexpr vec Normalize() const {
			T len = Length();
			if (std::abs(len) <= T(1.0e-6)) {
				std::cout << "Division by zero!" << std::endl;
				return vec{0, 0, 0};
			}
			return vec{coords_[0] / len, coords_[1] / len, coords_[2] / len};
		}

		virtual void Standardize() {
			T len = Length();
			stdr::for_each(coords_.begin(), coords_.end(), [len](T& val) { val /= len; });
		}

		std::ostream& print(std::ostream& os) const {
			os << std::format("({}, {}, {})\n", x(), y(), z());
			return os;
		}

		friend std::ostream& operator<<(std::ostream& os, const vec& mi) {
			return mi.print(os);
		}
	};

	// User-Inferred Custom Types Practice
	vec(int, int, int) -> vec<double>;
}

export namespace std {
	using vec_index::vec;

	template <typename T>
	struct formatter<vec<T>, char> {
		char type = 'd';

		constexpr auto parse(std::format_parse_context& ctx) {
			auto it = ctx.begin();
			auto end = ctx.end();
			if (it == end || *it == '}') {
				return it;
			}
			type = *it;
			if (type != 'd' && type != 'x' && type != 'b') {
				throw format_error("Invalid format type for MyClass");
			}
			return it;
		}

		auto format(const vec<T>& obj, auto& context) const {
			if (type == 'd' || type == 'f') {
				return std::format_to(context.out(), "({}, {}, {})", obj.x(), obj.y(), obj.z());
			}
			if (type == 'x') {
				return std::format_to(context.out(), "({:x}, {:x}, {:x})", obj.x(), obj.y(), obj.z());
			}
			if (type == 'b') {
				return std::format_to(context.out(), "({:b}, {:b}, {:b})", obj.x(), obj.y(), obj.z());
			}
			return std::format_to(context.out(), "({}, {}, {})", obj.x(), obj.y(), obj.z());
		}
	};
}
