export module Quanternion;

import <array>;
import <cmath>;
import <iostream>;
import <format>;
import <concepts>;
import <ranges>;
import <numeric>;
import <span>;
import <algorithm>;

import Vec;
inline constexpr double k_2Pi = static_cast<double>(6.2831853071795862);

inline constexpr double k_Pi = static_cast<double>(3.1415926535897932384);


extern "C" {
double __stdcall quantDotDoule_Optimized(const double* m, const double* n);
long __stdcall quantDotlong_Optimized(const long* m, const long* n);

double __stdcall dotProductDouble_Optimized(const double* m, const double* n);
void __stdcall crossProductDouble_Optimized(double* ret, const double* m, const double* n);
}

export namespace QuaternionIndex {
	using std::array;
	using std::cout;
	using std::endl;
	using std::format;
	using std::ostream;
	using std::ofstream;
	using std::abs;
	using std::sqrt;
	using std::atan2;
	using std::fmod;
	using std::sin;
	using std::cos;
	using std::acos;
	using vec_index::vec;
	namespace stdr = std::ranges;

	constexpr double PI = 3.14159265358979323846;
	constexpr double rad2deg = (180.0 / PI);
	constexpr double deg2rad = (PI / 180.0);

	template <typename T>
	concept is_index_operator = requires(T t) { t.operator[](0); };

	template <typename T>
		requires std::is_integral_v<T> || std::is_floating_point_v<T>
	class Quant {
	private:
		std::array<T, 4> q_{};

	public:
		constexpr Quant() noexcept = default;

		constexpr Quant(T w, T x, T y, T z) noexcept : q_{w, x, y, z} {
		}

		template <typename U>
			requires is_index_operator<U>
		constexpr Quant(T w, U&& v) noexcept : q_{w, v[0], v[1], v[2]} {
		}

		template <typename U>
			requires is_index_operator<U>
		constexpr Quant(U&& v) noexcept : q_{v[0], v[1], v[2], v[3]} {
		}

		constexpr Quant(const Quant& other) noexcept = default;
		constexpr Quant(Quant&& other) noexcept = default;
		~Quant() = default;

		template <typename Self, typename U>
		constexpr void swap(this Self&& self, U&& other) noexcept {
			stdr::swap(std::forward<Self>(self).q_, std::forward<U>(other).q_);
		}


		constexpr Quant& operator=(const Quant& other) noexcept {
			if (this != &other) {
				Quant temp(other);
				this->swap(temp);
				return *this;
			}
			return *this;
		}

		constexpr Quant& operator=(Quant&& other) noexcept {
			if (this != &other) {
				Quant temp(std::move(other));
				this->swap(temp);
				return *this;
			}
			return *this;
		}

		template <typename Self>
		constexpr T Length(this Self&& self) noexcept {
			auto&& s = std::forward<Self>(self);
			T sum_squares = std::transform_reduce(
				s.q_.begin(), s.q_.end(), T{}, std::plus<>{}, [](T v) { return v * v; });
			return std::sqrt(sum_squares);
		}

		template <typename Self>
		constexpr auto operator[](this Self&& self, std::size_t i) noexcept -> decltype(auto) {
			return std::forward<Self>(self).q_[i];
		}

		template <typename Self>
		[[nodiscard]] constexpr auto w(this Self&& self) noexcept -> decltype(auto) {
			return std::forward<Self>(self)[0];
		}

		template <typename Self>
		[[nodiscard]] constexpr auto x(this Self&& self) noexcept -> decltype(auto) {
			return std::forward<Self>(self)[1];
		}

		template <typename Self>
		[[nodiscard]] constexpr auto y(this Self&& self) noexcept -> decltype(auto) {
			return std::forward<Self>(self)[2];
		}

		template <typename Self>
		[[nodiscard]] constexpr auto z(this Self&& self) noexcept -> decltype(auto) {
			return std::forward<Self>(self)[3];
		}

		template <typename Self>
		constexpr auto Data(this Self&& self) noexcept -> decltype(auto) {
			return std::span(std::forward<Self>(self).q_);
		}

		// 纯访问器最适合deducing this

		constexpr std::conditional_t<std::floating_point<T>, double, long> Dot(const Quant& other) const noexcept {
			if (*this == other) {
				return Length() * Length();
			}
			if constexpr (std::is_floating_point_v<T>) {
				std::array<double, 4> m{}; // x y z w
				std::array<double, 4> n{};

				stdr::transform(q_.begin(), q_.end(), m.begin(),
				                [](T val) { return static_cast<double>(val); });
				stdr::transform(other.q_.begin(), other.q_.end(), n.begin(),
				                [](T val) { return static_cast<double>(val); });

				return quantDotDoule_Optimized(m.data(), n.data());
			}
			else if constexpr (std::is_integral_v<T>) {
				std::array<long, 4> m{};
				std::array<long, 4> n{};

				stdr::transform(q_.begin(), q_.end(), m.begin(),
				                [](T val) { return static_cast<long>(val); });
				stdr::transform(other.q_.begin(), other.q_.end(), n.begin(),
				                [](T val) { return static_cast<long>(val); });

				return quantDotlong_Optimized(m.data(), n.data());
			}
			else {
				return T(0);
			}
		}


		constexpr Quant Normalize() const noexcept {
			T len = Length();
			if (std::abs(len) <= T(1.0e-6)) {
				cout << "Division by zero!" << endl;
				return Quant(0, 0, 0, 0);
			}
			return Quant(q_[0] / len, q_[1] / len, q_[2] / len, q_[3] / len);
		}

		// 数乘
		constexpr Quant operator*(T scalar) const noexcept {
			return Quant(q_[0] * scalar, q_[1] * scalar, q_[2] * scalar,
			             q_[3] * scalar);
		}

		// 数除
		constexpr Quant operator/(T scalar) const noexcept {
			if (std::abs(scalar) <= T(1.0e-6)) {
				cout << "Division by zero!" << endl;
				return Quant(0, 0, 0, 0);
			}
			T inv_scalar = T(1) / scalar;
			return Quant(q_[0] * inv_scalar, q_[1] * inv_scalar, q_[2] * inv_scalar,
			             q_[3] * inv_scalar);
		}

		// friend operator* 交换顺序
		friend constexpr Quant operator*(T scalar, const Quant& q) {
			return q * scalar;
		}

		friend constexpr Quant operator/(T scalar, const Quant& q) {
			return q / scalar;
		}

		// 四元数乘法
		constexpr Quant operator*(const Quant& q) const noexcept {
			auto temp1 = vec<T>(x(), y(), z());
			auto temp2 = vec<T>(q.x(), q.y(), q.z());
			auto cross = temp1.Cross(temp2);
			return Quant(w() * q.w() - temp1.Dot(temp2), cross + q.w() * temp1 + w() * temp2);
		}

		constexpr bool operator==(const Quant& other) const noexcept {
			return (w() == other.w()) && (x() == other.x()) &&
				(y() == other.y()) && (z() == other.z());
		}

		constexpr bool operator!=(const Quant& other) const noexcept {
			return !(*this == other);
		}

		constexpr Quant Conjugate() const noexcept {
			return Quant(w(), -1 * x(), -1 * y(), -1 * z());
		}

		constexpr Quant Inverse() const noexcept {
			auto len = Length();
			if (std::abs(len) <= T(1.0e-6)) {
				cout << "Division by zero!" << endl;
				return Quant(0, 0, 0, 0);
			}
			return Conjugate() / (len * len);
		}

		constexpr void Standardize() noexcept {
			if (w() < 0.001) {
				q_[0] = -q_[0];
				q_[1] = -q_[1];
				q_[2] = -q_[2];
				q_[3] = -q_[3];
			}
			*this = Normalize();
		}

		template <typename U>
			requires requires(U u) {
				u[0]; u[1]; u[2]; u.Normalize();
			}
		constexpr Quant vecRot(const U& v, T angle) const noexcept {
			U axis = v.Normalize();
			Quant q = axis2Quant(axis, angle);
			if (q.Length() < 0.001) {
				return Quant(0, 0, 0, 0);
			}
			Quant ans = q * (*this) * q.Conjugate(); //不要归一化(inverse)，要用conjugate,因为这里是被旋转的向量，不是旋转轴
			return ans;
		}

		template <typename U>
			requires requires(U u) { u[0]; u[1]; u[2]; }
		constexpr Quant ax2qu(const U& axis, T angle) const noexcept {
			T half_angle = angle / 2.0;
			T sin_half_angle = std::sin(half_angle);
			return Quant(std::cos(half_angle), axis[0] * sin_half_angle,
			             axis[1] * sin_half_angle, axis[2] * sin_half_angle);
		}

		constexpr std::array<T, 3> qu2ax() const noexcept {
			T angle = 2.0 * std::acos(w());
			T sin_half_angle = std::sqrt(1.0 - w() * w());
			if (std::abs(sin_half_angle) < T(1.0e-6)) {
				return {1, 0, 0};
			}
			return {
				x() / sin_half_angle, y() / sin_half_angle,
				z() / sin_half_angle
			};
		}

		constexpr std::array<T, 3> qu2eu() const noexcept {
			double tol = 1.0e-6;
			auto temp = this->Data();
			T a = temp[0]; // w
			T b = temp[3]; // z
			T c = temp[1]; // x
			T d = temp[2]; // y
			T len = static_cast<T>(this->Length());

			T phi1 = std::atan2(b, a);
			T Phi = std::acos(2.0 * (a * a + b * b) / (len * len) - 1.0);
			T phi2 = std::atan2(-1.0 * d, c);

			// sums & diffs
			T sum = phi1 + phi2;
			T diff = phi1 - phi2;

			bool is_zero = std::fabs(Phi) < tol;
			bool is_pi = std::fabs(Phi - k_Pi) < tol;
			bool is_ok = !(is_zero || is_pi);

			T e0 = phi1;
			T e1 = Phi;
			T e2 = phi2;

			if (is_zero) e0 = 2.0 * phi1;
			if (is_pi) e0 = -2.0 * phi2;
			if (!is_ok) e2 = 0.0;
			if (is_ok) {
				e0 = diff;
				e2 = sum;
			}

			// zero out anything extremely close to 0 or 2π
			if (std::fabs(e0) < tol || std::fabs(e0 - k_2Pi) < tol) e0 = T(0);
			if (std::fabs(e1) < tol || std::fabs(e1 - k_2Pi) < tol) e1 = T(0);
			if (std::fabs(e2) < tol || std::fabs(e2 - k_2Pi) < tol) e2 = T(0);

			// bring negatives into [0,mod) exactly as numpy’s x % mod
			if (e0 < 0) {
				e0 = std::fmod(e0, k_2Pi);
				if (e0 < 0) e0 += k_2Pi;
			}
			if (e1 < 0) {
				e1 = std::fmod(e1, k_Pi);
				if (e1 < 0) e1 += k_Pi;
			}
			if (e2 < 0) {
				e2 = std::fmod(e2, k_2Pi);
				if (e2 < 0) e2 += k_2Pi;
			}

			return {e0, e1, e2};
		}

		constexpr std::array<T, 4> qu2ro() const noexcept {
			auto temp = this->Data();
			auto s = std::sqrt(temp[1] * temp[1] + temp[2] * temp[2] + temp[3] * temp[3]);
			return {temp[1] / s, temp[2] / s, temp[3] / s, std::tan(std::asin(s))};
		}

		constexpr std::array<T, 3> qu2mrp() const noexcept {
			// Modified Rodrigues Parameters (MRP) 是
			auto temp = this->Data();
			auto theta = std::acos(temp[0]); // \theta / 2
			auto s = std::tan(theta / 2) / std::sin(theta);
			return {temp[1] * s, temp[2] * s, temp[3] * s};
		}
	};
};
