#include <complex.h>
#include <execution>
import IO;
import Magic;
import Mytuple;
import Quanternion;
import OrientationTransformFunction;

import <iostream>;
import <string_view>;
import <vector>;
import <string>;
import <format>;
import <array>;
import <ranges>;
import <algorithm>;


template <typename... Args>
using tuple = Flat_Tuple::MyTuple<Args...>;
namespace stdv = std::views;
namespace stdr = std::ranges;

struct Ebsd2GridByLYX {
	static inline const char* writeBy = "Liu Yu Xin";
};

struct Sakura {
};

constexpr double PI = 3.14159265358979323846;
constexpr double rad2deg = (180.0 / PI);
constexpr double deg2rad = (PI / 180.0);


void ReadFileName(std::string& str) {
	std::getline(std::cin, str);
}

void OutFileName(std::string& str, std::string& out) {
	std::string_view sv(str);
	auto parts = sv | std::views::split('.');
	auto it = std::ranges::begin(parts);

	std::string head;
	if (it != std::ranges::end(parts)) {
		auto sub = *it;
		head.assign(std::ranges::begin(sub), std::ranges::end(sub)); 
	}
	else {
		head = str;
	}

	out = head + ".csv";
}

#define CTF_TYPE int, double, double, int, int, double, double, double, double, int, int
#define ASCII_TYPE double, double, double, double, double, double, double, double, double, double, int

using Out_Put_Type = std::vector<tuple<ASCII_TYPE>>;
using Ctf_Input_Type = std::vector<tuple<CTF_TYPE>>;

template <typename T>
struct Euler_angle {
	using value_type = T;

	std::array<T, 3> v{};

	constexpr Euler_angle() = default;

	constexpr Euler_angle(T a, T b, T c) : v{a, b, c} {
	}

	// 跨类型拷贝/转换（例如 double -> float）
	template <typename U>
	explicit constexpr Euler_angle(const Euler_angle<U>& other)
		: v{
			static_cast<T>(other.v[0]),
			static_cast<T>(other.v[1]),
			static_cast<T>(other.v[2])
		} {
	}

	template <typename Self>
	Euler_angle(Self&& other) : v{
		static_cast<T>(other[0]),
		static_cast<T>(other[1]),
		static_cast<T>(other[2])
	} {
	}

	explicit constexpr Euler_angle(std::array<T, 3> arr) : v(arr) {
	}

	static constexpr std::size_t size() noexcept { return 3; }

	constexpr T& operator[](std::size_t i) noexcept { return v[i]; }
	constexpr const T& operator[](std::size_t i) const noexcept { return v[i]; }

	constexpr T& phi1() noexcept { return v[0]; }
	constexpr const T& phi1() const noexcept { return v[0]; }
	constexpr T& PHI_() noexcept { return v[1]; } // 避免与宏/名字冲突，命名为 PHI_()
	constexpr const T& PHI_() const noexcept { return v[1]; }
	constexpr T& ph2() noexcept { return v[2]; }
	constexpr const T& ph2() const noexcept { return v[2]; }

	constexpr auto begin() noexcept { return v.begin(); }
	constexpr auto end() noexcept { return v.end(); }
	constexpr auto begin() const noexcept { return v.begin(); }
	constexpr auto end() const noexcept { return v.end(); }
	constexpr auto cbegin() const noexcept { return v.cbegin(); }
	constexpr auto cend() const noexcept { return v.cend(); }

	template <std::size_t I>
	constexpr T& get() noexcept {
		static_assert(I < 3);
		return v[I];
	}

	template <std::size_t I>
	constexpr const T& get() const noexcept {
		static_assert(I < 3);
		return v[I];
	}

	explicit constexpr operator std::array<T, 3>() const { return v; }
};

template <class T1, class T2, class T3>
Euler_angle(T1, T2, T3) -> Euler_angle<std::common_type_t<T1, T2, T3>>;

template <typename Key, size_t i, typename T, typename U>
void Prepare_Data(T& t, U& u) {
};

template <>
void Prepare_Data<Ebsd2GridByLYX, 722>(Ctf_Input_Type& init, Out_Put_Type& out) {

	constexpr double z = 0.05;

	// 把一条输入记录映射成一条输出 MyTuple
	auto to_row = [](auto& in) {
		int phase = Flat_Tuple::get<0>(in);
		double x = Flat_Tuple::get<1>(in);
		double y = Flat_Tuple::get<2>(in);

		double e1_deg = Flat_Tuple::get<5>(in);
		double e2_deg = Flat_Tuple::get<6>(in);
		double e3_deg = Flat_Tuple::get<7>(in);

		Euler_angle<double> eu_rad{
			e1_deg * deg2rad,
			e2_deg * deg2rad,
			e3_deg * deg2rad
		};
		auto q = eu2qu<double>(eu_rad);

		return Flat_Tuple::MyTuple<ASCII_TYPE>{
			x, y, z,
			eu_rad.v[0], eu_rad.v[1], eu_rad.v[2],
			q[0], q[1], q[2], q[3],
			phase
		};
	};

	std::vector<Flat_Tuple::MyTuple<ASCII_TYPE>> tmp(init.size());
	std::transform(std::execution::par_unseq,
	               init.begin(), init.end(),
	               tmp.begin(),
	               to_row);

	out.clear();
	out.reserve(tmp.size());
	out.insert(out.end(),
	           std::make_move_iterator(tmp.begin()),
	           std::make_move_iterator(tmp.end()));
}

template <Io_function::FileFormat Fmt, typename... Args>
void read_text_inst(const char* inputFileName,
                    std::vector<Flat_Tuple::MyTuple<Args...>>& data) {
	Io_function::read_text<Fmt, Args...>(inputFileName, data);
}

template <typename... Args>
using ReaderFn = void(*)(const char*, std::vector<Flat_Tuple::MyTuple<Args...>>&);

template <typename... Args>
consteval auto make_table() {
	return std::array<ReaderFn<Args...>, 10>{
		&read_text_inst<Io_function::ctfFormat, Args...>,
		&read_text_inst<Io_function::twoPhase, Args...>,
		&read_text_inst<Io_function::threePhase, Args...>,
		&read_text_inst<Io_function::fourPhase, Args...>,
		&read_text_inst<Io_function::fivePhase, Args...>,
		&read_text_inst<Io_function::sixPhase, Args...>,
		&read_text_inst<Io_function::sevenPhase, Args...>,
		&read_text_inst<Io_function::eightPhase, Args...>,
		&read_text_inst<Io_function::ninePhase, Args...>,
		&read_text_inst<Io_function::tenPhase, Args...>,
	};
}

template <typename... Args>
void read_text_dispatch(int phases, const char* inputFileName,
                        std::vector<Flat_Tuple::MyTuple<Args...>>& data) {
	if (phases < 1 || phases > 10)
		throw std::out_of_range("phases must be in [1,10]");
	static constexpr auto tbl = make_table<Args...>(); 
	tbl[phases - 1](inputFileName, data);
}

int main() {
	std::string Input_File;
	std::string Output_File;
	int phase = 1;
	std::cout << "Phase number (1-10, default 1): ";
	std::cin >> phase;
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	Ctf_Input_Type ctf_data;
	Out_Put_Type ascii_data;

	{
		ReadFileName(Input_File);
		OutFileName(Input_File, Output_File);
	}

	read_text_dispatch(phase, Input_File.c_str(), ctf_data);

	SMPWriter<Ebsd2GridByLYX, 722>{};
	SMPWriter<Sakura, 721>{};

	Prepare_Data<Ebsd2GridByLYX, getValue(SMPReader<Ebsd2GridByLYX>())>(ctf_data, ascii_data);
	ascii_data[0];

	Io_function::write_helper<getValue(SMPReader<Sakura>()), ASCII_TYPE>::write_text(Output_File.c_str(), ascii_data);

	auto a = 1;
	return 0;
}
