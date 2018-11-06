// ↓↓↓ XXX
inline std::ostream&
cout_pack()
{
	return std::cout;
}


template<class T, class ...Ts>
	inline std::ostream&
	cout_pack (T&& arg, Ts&& ...args)
	{
		std::cout << arg;
		if constexpr (sizeof...(args) > 0)
			std::cout << ", ";
		return cout_pack (args...);
	}


template<class Tuple, size_t Pos = 0>
	inline std::ostream&
	cout_tuple (Tuple const& tuple)
	{
		if (Pos == 0)
			std::cout << "{ ";
		std::cout << std::get<Pos> (tuple);
		if constexpr (Pos < std::tuple_size_v<Tuple> - 1)
		{
			std::cout << ", ";
			cout_tuple<Tuple, Pos + 1> (tuple);
		}
		else
			std::cout << " }";
		return std::cout;
	}


inline size_t _depth = 0;

inline std::ostream&
_cout()
{
	return std::cout << "\n" + std::string (_depth, ' ');
}
// ↑↑↑ XXX

