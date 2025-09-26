module;

#include <windows.h>
#include "MemoryMapMacro.h"

export module IO;

import <iostream>;
import <vector>;
import <string>;
import <string_view>;
import <ranges>;
import <memory>;
import <algorithm>;
import <charconv>;

import Mytuple;

using Flat_Tuple::get;
using Flat_Tuple::tuple_elemetn_t;

namespace stdv = std::views;
namespace stdr = std::ranges;

enum class ctf_file_type { Phase, X, Y, Bands, Error, Euler1, Euler2, Euler3, MAD, BC, BS };

#define CTF_TYPE int, double, double, int, int, double, double, double, double, int, int

struct ssl;

export namespace Io_function {
	struct MappedFile {
		std::string_view view; // Non-owning view into the mapped file.
		HANDLE hFile = INVALID_HANDLE_VALUE;
		HANDLE hMapping = nullptr;
		LPVOID pMap = nullptr;
	};

	struct FileFormat {
		int skipLines;
		char delimiter;
	};

	inline constexpr FileFormat csvFormat{1, ' '};
	inline constexpr FileFormat ctfFormat{15, '\t'};
	inline constexpr FileFormat twoPhase{16, '\t'};
	inline constexpr FileFormat threePhase{17, '\t'};
	inline constexpr FileFormat fourPhase{18, '\t'};
	inline constexpr FileFormat fivePhase{19, '\t'};
	inline constexpr FileFormat sixPhase{20, '\t'};
	inline constexpr FileFormat sevenPhase{21, '\t'};
	inline constexpr FileFormat eightPhase{22, '\t'};
	inline constexpr FileFormat ninePhase{23, '\t'};
	inline constexpr FileFormat tenPhase{24, '\t'};


	template <typename T>
	std::ostream& println(std::ostream& os, const T& t) {
		return os << t << '\n';
	}

	template <typename T, std::size_t N>
	std::ostream& println(std::ostream& os, const T (&t)[N]) {
		for (const T& e : t)
			os << e << ' ';
		return os << '\n';
	}

	template <typename T>
	std::ostream& println(std::ostream& os, const std::vector<T>& t) {
		for (const T& e : t)
			os << e << ' ';
		return os << '\n';
	}

	template <typename First, typename... Rest>
	std::ostream& println(std::ostream& os, const First& first, const Rest&... rest) {
		println(os, first);
		return println(os, rest...);
	}

	template <typename T>
	T convert_from_string(const std::string_view& str) {
		T value;
		std::from_chars(str.data(), str.data() + str.size(), value);
		return value;
	}

	template <std::size_t index = 0, typename... Args>
	std::enable_if_t<index == sizeof...(Args), void>
	process_types(const std::vector<std::string_view>&, Flat_Tuple::MyTuple<Args...>&) {
	}

	template <std::size_t index = 0, typename... Args>
	std::enable_if_t<index < sizeof...(Args), void>
	process_types(const std::vector<std::string_view>& parts, Flat_Tuple::MyTuple<Args...>& values) {
		using CurrentType = tuple_elemetn_t<index, Args...>;
		auto value = convert_from_string<CurrentType>(parts[index]);

		Flat_Tuple::get<index>(values) = value;
		process_types<index + 1>(parts, values);
	}

	void read_helper(const char* inputFileName, MappedFile& mfIn) {
		READ_FILE_MEMORY(inputFileName, mfIn);
	}

	template <FileFormat Fmt, typename... Args>
	void read_text(const char* inputFileName, std::vector<Flat_Tuple::MyTuple<Args...>>& data) {
		int skipLines = Fmt.skipLines;
		char delimiter = Fmt.delimiter;

		MappedFile mfIn;
		read_helper(inputFileName, mfIn);
		auto lines = mfIn.view | stdv::split('\n');

		auto Head_message = lines | stdv::take(skipLines);
		auto EBSD_message = lines | stdv::drop(skipLines);

		for (auto&& lineRange : EBSD_message) {
			std::string_view line = std::string_view(stdr::data(lineRange), stdr::size(lineRange));
			if (line.empty()) continue;

			auto tokensRange = line
				| stdv::split(delimiter)
				| stdv::transform([](auto&& tokenRange) -> std::string_view {
					return std::string_view(stdr::data(tokenRange), stdr::size(tokenRange));
				});

			std::vector<std::string_view> tokens;
			tokens.reserve(stdr::distance(tokensRange));
			stdr::copy(tokensRange, std::back_inserter(tokens));

			Flat_Tuple::MyTuple<Args...> values;
			process_types<0>(tokens, values);
			data.push_back(std::move(values));
		}

		FREE_FILE_MEMORY(mfIn);
	}

	void write_Helper(const char* OutputFileName, MappedFile& mfout, unsigned long newFileSize) {
		WRITE_FILE_MEMORY(OutputFileName, mfout, newFileSize);
	}

	template <size_t index = 0, typename... Args>
	std::enable_if_t<index == sizeof...(Args), void>
	write_char(char* buffer, unsigned long& size, Flat_Tuple::MyTuple<Args...>& values) {
		char curr[3] = {'S', 'X', '\n'};
		std::memcpy(buffer, curr, 3);
		size += 3;
	};

	template <size_t index = 0, typename... Args>
	std::enable_if_t<index < sizeof...(Args), void>
	write_char(char* buffer, unsigned long& size, Flat_Tuple::MyTuple<Args...>& values) {
		char curr[32];
		char* curr_last = std::to_chars(curr, curr + 32, Flat_Tuple::get<index>(values)).ptr;
		size_t len = curr_last - curr;
		std::memcpy(buffer, curr, len);
		buffer[len] = '\t';
		size += len + 1;
		write_char<index + 1>(buffer + len + 1, size, values);
	};

	template <std::size_t step = 722, typename... Args>
	struct write_helper {
		static inline void write_text(const char* OutputFileName, std::vector<Flat_Tuple::MyTuple<Args...>>& data) {
			// Allocate buffer
			std::vector<char> buffer(104857600);
			const char* header = "Phase\tX\tY\tBands\tError\tEuler1\tEuler2\tEuler3\tMAD\tBC\tBS\n";
			std::memcpy(buffer.data(), header, strlen(header));
			char* ori_ptr = buffer.data();
			char* ptr = ori_ptr;
			unsigned long newFileSize = strlen(header);
			stdr::for_each(data, [&ptr, &newFileSize, &ori_ptr](Flat_Tuple::MyTuple<Args...>& values) {
				ptr = ori_ptr + newFileSize;
				write_char(ptr, newFileSize, values);
			});
			buffer.resize(newFileSize);

			MappedFile mfout;

			WRITE_FILE_MEMORY(OutputFileName, mfout, newFileSize);

			std::memcpy(mfout.pMap, buffer.data(), newFileSize);
			if (!FlushViewOfFile(mfout.pMap, newFileSize)) {
				std::cerr << "Error flushing output view: " << GetLastError() << std::endl;
			}
			FREE_FILE_MEMORY(mfout);
		}
	};

	template <typename... Args>
	struct write_helper<0, Args...> {
		static inline void write_text(const char* OutputFileName, std::vector<Flat_Tuple::MyTuple<Args...>>& data) {
			std::vector<char> buffer(104857600);
			const char* header = "Euler1\tEuler2\tEuler3\n";
			std::memcpy(buffer.data(), header, strlen(header));
			char* ori_ptr = buffer.data();
			char* ptr = ori_ptr;
			unsigned long newFileSize = strlen(header);
			stdr::for_each(data, [&ptr, &newFileSize, &ori_ptr](Flat_Tuple::MyTuple<Args...>& values) {
				ptr = ori_ptr + newFileSize;
				write_char(ptr, newFileSize, values);
			});
			buffer.resize(newFileSize);

			MappedFile mfout;

			WRITE_FILE_MEMORY(OutputFileName, mfout, newFileSize);

			std::memcpy(mfout.pMap, buffer.data(), newFileSize);
			if (!FlushViewOfFile(mfout.pMap, newFileSize)) {
				std::cerr << "Error flushing output view: " << GetLastError() << std::endl;
			}
			FREE_FILE_MEMORY(mfout);
		}
	};

	template <typename... Args>
	struct write_helper<721, Args...> {
		static inline void write_text(const char* OutputFileName, std::vector<Flat_Tuple::MyTuple<Args...>>& data) {
			// Allocate buffer
			std::vector<char> buffer(104857600); // 100MB
			const char* header =
				"1_pos\t2_pos\t3_pos\tEuler1\tEuler2\tEuler3\t1_qu\t2_qu\t3_qu\t4_qu\tphase\thomogenization\n";
			std::memcpy(buffer.data(), header, strlen(header));
			char* ori_ptr = buffer.data();
			char* ptr = ori_ptr;
			unsigned long newFileSize = strlen(header);
			stdr::for_each(data, [&ptr, &newFileSize, &ori_ptr](Flat_Tuple::MyTuple<Args...>& values) {
				ptr = ori_ptr + newFileSize;
				write_char(ptr, newFileSize, values);
			});
			buffer.resize(newFileSize);

			MappedFile mfout;

			WRITE_FILE_MEMORY(OutputFileName, mfout, newFileSize);

			std::memcpy(mfout.pMap, buffer.data(), newFileSize);
			if (!FlushViewOfFile(mfout.pMap, newFileSize)) {
				std::cerr << "Error flushing output view: " << GetLastError() << std::endl;
			}
			FREE_FILE_MEMORY(mfout);
		}
	};
}
