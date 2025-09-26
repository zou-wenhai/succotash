export module Magic;

import <iostream>;

template <std::size_t N>
struct reader {
	friend auto flag(reader);
};

template <std::size_t N>
struct setter {
	friend auto flag(reader<N>) {
	}
};

export template <std::size_t N = 0, auto seed = [] {
                 }>
consteval auto nextCompileIndex() {
	constexpr bool exist = requires { flag(reader<N>{}); };
	if constexpr (!exist) {
		setter<N> setter;
		return N;
	}
	else {
		return nextCompileIndex<N + 1>();
	}
}

export template <typename Key>
struct SMPReader {
	friend static inline constexpr std::size_t getValue(SMPReader<Key>);
};


export template <typename Key, std::size_t Val>
struct SMPWriter {
	friend static inline constexpr std::size_t getValue(SMPReader<Key>) {
		return Val;
	}
};
