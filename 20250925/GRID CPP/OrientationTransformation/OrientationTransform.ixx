export module OrientationTransformFunction;

import <concepts>;
import <array>;
import <cmath>;
import <ranges>;
import <algorithm>;

import Quanternion;
import Vec;

inline constexpr double k_2Pi = static_cast<double>(6.2831853071795862);

inline constexpr double k_Pi = static_cast<double>(3.1415926535897932384);

export template <typename T, typename = std::void_t<std::enable_if_t<std::is_convertible_v<T, double>>>>
auto qu2eu(const QuaternionIndex::Quant<T>& q) -> decltype(auto) {
	return q.qu2eu();
}

export template <typename T, typename = std::void_t<std::enable_if_t<std::is_convertible_v<T, double>>>>
auto qu2ax(const QuaternionIndex::Quant<T>& q) -> decltype(auto) {
	return q.qu2ax();
}

export template <typename T, typename = std::void_t<std::enable_if_t<std::is_convertible_v<T, double>>>>
auto qu2ro(const QuaternionIndex::Quant<T>& q) -> decltype(auto) {
	return q.qu2ro();
}


export template <typename T, typename = std::void_t<std::enable_if_t<std::is_convertible_v<T, double>>>>
auto qu2mrp(const QuaternionIndex::Quant<T>& q) -> decltype(auto) {
	return q.qu2mrp();
}

export template <typename T, typename U,
                 typename = std::enable_if_t<std::is_convertible_v<T, double>>>
	requires requires(U u) { u[0]; u[1]; u[2]; }
auto eu2qu(U u) {
	std::array<T, 3> ee{
		static_cast<T>(u[0]) / 2.0, // phi1
		static_cast<T>(u[1]) / 2.0, // PHI
		static_cast<T>(u[2]) / 2.0 // phi2
	};


	const double cPhi = std::cos(ee[1]);
	const double sPhi = std::sin(ee[1]);

	QuaternionIndex::Quant<T> q{
		cPhi * std::cos(ee[0] + ee[2]),
		sPhi * std::cos(ee[0] - ee[2]),
		sPhi * std::sin(ee[0] - ee[2]),
		cPhi * std::sin(ee[0] + ee[2])
	};

	if (q.Data()[0] < 0.0) {
		q = q * static_cast<T>(-1);
	}

	return q;
}
