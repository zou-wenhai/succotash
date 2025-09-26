export module Mytuple;

import <iostream>;
import <string>;
import <utility>;

//import std;

export namespace Recursion_Tuple {
	template <typename... Types>
	struct MySimpleTuple;

	template <typename... Types>
	struct MySimpleTuple {
	};

	template <>
	struct MySimpleTuple<> {
		using type = void;
		int value = -1;

		MySimpleTuple() {
			std::cout << "MySimpleTuple<>" << std::endl;
		};
	};

	template <typename Head, typename... Tail>
	struct MySimpleTuple<Head, Tail...> : MySimpleTuple<Tail...> {
		Head head;

		// 构造函数，分别传递Head和Tail...并且递归调用
		MySimpleTuple<Head, Tail...>(Head h, Tail... t) : MySimpleTuple<Tail...>{t...}, head{h} {
		}

		const Head& get_head() const {
			return head;
		}

		Head& get_head() {
			return head;
		}
	};

	template <std::size_t I, typename Tuple>
	struct MySimpleTupleElement;

	template <std::size_t I, typename Head, typename... Tail>
	struct MySimpleTupleElement<I, MySimpleTuple<Head, Tail...>> {
		using type = typename MySimpleTupleElement<I - 1, MySimpleTuple<Tail...>>::type;

		static type& get(MySimpleTuple<Head, Tail...>& t) {
			return MySimpleTupleElement<I - 1, MySimpleTuple<Tail...>>::get(t);
		}

		static const type& get(const MySimpleTuple<Head, Tail...>& t) {
			return MySimpleTupleElement<I - 1, MySimpleTuple<Tail...>>::get(t);
		}
	};

	template <typename Head, typename... Tail>
	struct MySimpleTupleElement<0, MySimpleTuple<Head, Tail...>> {
		using type = Head;

		static type& get(MySimpleTuple<Head, Tail...>& t) {
			return t.get_head();
		}

		static const type& get(const MySimpleTuple<Head, Tail...>& t) {
			return t.get_head();
		}
	};

	template <std::size_t I, typename Head, typename... Tail>
	typename MySimpleTupleElement<I, MySimpleTuple<Head, Tail...>>::type& get(MySimpleTuple<Head, Tail...>& t) {
		return MySimpleTupleElement<I, MySimpleTuple<Head, Tail...>>::get(t);
	}

	void test() {
		MySimpleTuple<int, double, std::string> t{1, 3.14, "hello"};
		auto i1 = get<0>(t);
		auto i2 = get<1>(t);
		auto i3 = get<2>(t);
		std::cout << get<0>(t) << std::endl;
		std::cout << get<1>(t) << std::endl;
		std::cout << get<2>(t) << std::endl;

		MySimpleTuple<> t2{};
	}
}

export namespace Flat_Tuple {
	template <std::size_t I, typename... Ts>
	struct MyTupleElement;

	template <typename T, typename... Ts>
	struct MyTupleElement<0, T, Ts...> {
		using type = T;
	};

	template <std::size_t I, typename T, typename... Ts>
	struct MyTupleElement<I, T, Ts...> {
		using type = typename MyTupleElement<I - 1, Ts...>::type;
	};

	template <std::size_t I, typename... Ts>
	using tuple_elemetn_t = typename MyTupleElement<I, Ts...>::type;

	template <std::size_t I, typename T>
	struct TupleElementHolder {
		T value;

		TupleElementHolder() : value{} {
		}

		TupleElementHolder(const T& v) : value{v} {
		}
	};

	template <>
	struct TupleElementHolder<0, void> {
		using type = void;
		int value = -1;
	};
	template <typename... Args>
	struct MyTupleImpl;

	template <typename IndexSeq, typename... Tys>
	struct MyTupleImpl<IndexSeq, Tys...> {
	};

	template <std::size_t... I, typename... Tys>
	struct MyTupleImpl<std::index_sequence<I...>, Tys...> : TupleElementHolder<I, Tys>... {
		MyTupleImpl() = default;

		MyTupleImpl(const Tys&... args) : TupleElementHolder<I, Tys>{args}... {
		}
	};

	template <>
	struct MyTupleImpl<std::index_sequence<>, void> : TupleElementHolder<0, void> {
	};

	template <typename... Tys>
	struct MyTuple : MyTupleImpl<std::index_sequence_for<Tys...>, Tys...> {
		std::size_t size = sizeof...(Tys);

		MyTuple() = default;

		MyTuple(const Tys&... args) : MyTupleImpl<std::index_sequence_for<Tys...>, Tys...>{args...} {
		}
	};

	template <>
	struct MyTuple<> : MyTupleImpl<std::index_sequence<>, void> {
		std::size_t size = 0;
		MyTuple() = default;
	};

	template <std::size_t I, typename... Args>
	struct MygetHelper {
		static auto& get(MyTuple<Args...>& t) {
			return static_cast<TupleElementHolder<I, typename MyTupleElement<I, Args...>::type>&>(t).value;
		}
	};

	template <std::size_t I, typename... Args>
	constexpr auto& get(MyTuple<Args...>& t) {
		return MygetHelper<I, Args...>::get(t);
	}


	template <std::size_t I, std::size_t index, typename Tuple, typename... ElementType>
	struct MygetRecursion {
	};

	template <std::size_t I, std::size_t index, template <typename ,typename...> typename Tuple, typename R, typename...
	          O,
	          typename Head, typename... Tails>
		requires (I > 0)
	struct MygetRecursion<I, index, Tuple<R, O...>, Head, Tails...> {
		static auto& get(Tuple<R, O...>& t) {
			return MygetRecursion<I - 1, index, Tuple<R, O...>, Tails...>::get(t);
		}
	};

	template <std::size_t index, template <typename, typename...> typename Tuple, typename R, typename... O,
	          typename Head, typename... Tails>
	struct MygetRecursion<0, index, Tuple<R, O...>, Head, Tails...> {
		static auto& get(Tuple<R, O...>& t) {
			using Holder = TupleElementHolder<index, Head>;
			Holder* p = &t;
			return p->value;

		}
	};

	template <std::size_t I, typename... Args>
	auto& get_recursion(MyTuple<Args...>& t) {
		return MygetRecursion<I, I, MyTuple<Args...>, Args...>::get(t);
	}

	void MyTupletest() {
		MyTuple<int, double, std::string, std::string> t{1, 3.14, "hello", " Yesf"};
		auto i1 = get<0>(t);
		auto i2 = get<1>(t);
		auto i3 = get<2>(t);
		auto b1 = get_recursion<0>(t);
		auto b2 = get_recursion<1>(t);
		auto b3 = get_recursion<2>(t);
		auto b4 = get_recursion<3>(t);
		std::cout << "--------------------------" << std::endl;
		std::cout << get<0>(t) << std::endl;
		std::cout << get<1>(t) << std::endl;
		std::cout << get<2>(t) << std::endl;
		std::cout << get<3>(t) << std::endl;
		std::cout << "--------------------------" << std::endl;
		std::cout << get_recursion<0>(t) << std::endl;
		std::cout << get_recursion<1>(t) << std::endl;
		std::cout << get_recursion<2>(t) << std::endl;
		std::cout << get_recursion<3>(t) << std::endl;

		MyTuple t2;
		using void_tuple_ptr = TupleElementHolder<0, void>;
		void_tuple_ptr* p = &t2;
		std::cout << p->value << std::endl;
	}
}
