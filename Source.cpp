#include<iostream>
#include<map>
#include<vector>
#include<string>
#include<algorithm>
#include<utility>
#include<tuple>
#include<optional>
#include<variant>
#include<functional>
#include<set>

template<typename T>
using _tree_data_type = std::tuple<T, size_t, std::vector<size_t>, int>;


template<typename T>
class tree_accsssor {
private:
	bool valid;
	size_t pos;
	std::vector<_tree_data_type<T>>& data;
	std::vector<size_t>reserved;
public:
	tree_accsssor() : valid(false) {}
	tree_accsssor(std::vector<_tree_data_type<T>>& data, size_t pos) :data(data), pos(pos), valid(true) {}

	tree_accsssor<T>& operator=(const tree_accsssor<T>& t) {
		*this = t;
	}

	tree_accsssor get_parent() const {
		return tree_accsssor(data, std::get<1>(data[pos]));
	}

	auto get_children() const {
		std::vector<tree_accsssor<T>> children;
		for (auto& e : std::get<2>(data[pos])) {
			children.push_back(tree_accsssor<T>(data, e));
		}
		return children;
	}

	tree_accsssor<T> back() {
		size_t ret=0;
		bool init = true;
		for (auto& e : std::get<2>(data[pos])) {
			if (init) {
				ret = e;
			}
			else {
				if (std::get<3>(data[e]) > std::get<3>(data[ret])) ret = e;
			}
		}
		return tree_accsssor<T>(data,ret);
	}

	void add(T entity) {
		size_t max = 0;
		for (auto& e : get_children()) {
			auto i = std::get<3>(data[pos]);
			if (max < i) max = i;
		}

		_tree_data_type<T> elem = std::make_tuple(entity, std::get<1>(data[pos]), std::vector<size_t>(0), max+1);
		size_t locate;
		if (!reserved.empty()) {
			data[reserved.back()] = elem;
			locate = reserved.back();
			reserved.pop_back();
		}
		else {
			locate = data.size();
			data.push_back(elem);
		}
		std::get<2>(data[pos]).push_back(locate);
	}

	void remove(tree_accsssor<T> obj) {
		data[obj.pos] = std::make_tuple(T(), 0, std::vector<size_t>(0), 0);
		reserved.push_back(obj.pos);
	}

	void pop() {
		size_t max = 0, rm;
		for (auto& e : get_children()) {
			auto& i = std::get<3>(data[pos]);
			if (max < i) max = i;
			if (max == i) rm = i;
		}
		data[rm] = std::make_tuple(T(), 0, std::vector<size_t>(0), 0);
		reserved.push_back(rm);
	}


};

template<typename T>
class tree {
private:
	//i=0 : entity,i=1 : parent,i=2 : children
	std::vector<_tree_data_type<T>> data;
	std::vector<size_t> reserved;

public:

	tree() {
		data.push_back(std::make_tuple(T(), 0, std::vector<size_t>(0), 0));
	}

	tree_accsssor<T> root() {
		return tree_accsssor<T>(data, 0);
	}
};


namespace parser {

	enum class symbol {
		begin,
		end
	};

	template<typename _State, typename Input>
	class dfa {
		using State = std::variant<symbol, _State>;
	private:
		std::map<State, std::map<Input, State>> data;
		std::variant<State, symbol> state = symbol::begin;

	public:

		auto& get_data() {
			return data;
		}

		virtual ~dfa() {

		}
		inline void add_state(State st) {
			data[st] = std::map<Input, State>();
		}

		inline  void add_seq(const State& begin, const Input& cause, const std::set<State>& end) {
			add_state(begin);
			for (auto& e : end) {
				data[begin][cause].insert(e);
			}
		}

		inline bool is_end() const {
			state == symbol::end;
		}

		inline bool exe(const Input& input) {
			state = data[state][input];
			return state == symbol::end;
		}
	};

	template<typename _State, typename Input>
	class nfa {
		using State = std::variant<symbol, _State>;
	private:
		std::map <State, std::map<Input, std::set<State>>> data;
		std::vector<std::tuple<int, State, State>> free_moves;
	public:
		inline void add_state(State st) {
			if (data.find(st) == data.end())
				data[st] = std::map<Input, std::set<State>>();
		}

		inline  void add_seq(const State& begin, const std::optional<Input>& cause, const std::set<State>& end) {
			add_state(begin);
			if (cause.has_value()) {
				for (auto& e : end) {
					data[begin][cause.value()].insert(e);
				}
			}
			else if (end.size() == 1) {
				free_moves.push_back(std::make_tuple(0, begin, *end.begin()));
			}
		}

		inline void solve_freemove() {
			for (auto& mv : free_moves) {
				if (data[std::get<1>(mv)].size() == 1) {
					if (std::get<2>(mv) == symbol::end)

						data[std::get<1>(mv)] = data[std::get<2>(mv)];
					for (auto& _elem : data) {
						for (auto& elem : _elem.second) {
							auto res = elem.second.find(std::get<2>(mv));
							if (res != elem.second.end()) {

							}
						}
					}
				}
			}
			for (auto& mv : free_moves) {
				for (auto& cm : free_moves) {
					if (std::get<1>(cm) == std::get<2>(mv)) {
						std::get<0>(cm) = std::get<0>(mv) - 1;
					}
				}
			}

			auto cmp
				=
				[](const std::tuple<int, State, State>& e1, const std::tuple<int, State, State>& e2) {
				return std::get<0>(e1) > std::get<0>(e2);
			}
			;



			std::sort(free_moves.begin(),
				free_moves.end(),
				cmp
			);


			for (auto& e1 : data) {
				for (auto& e2 : e1.second) {
					for (auto& r : free_moves) {
						if (e1.first == std::get<1>(r)) {
							e2.second.insert(std::get<2>(r));
						}
					}
				}
			}
		}


		void add_seq(const std::vector<std::tuple<State, Input, std::set<State>>>& rules) {
			for (const auto& e : rules) {
				add_seq(std::get<0>(e), std::get<1>(e), std::get<2>(e));
			}
		}

		dfa<int, Input> to_dfa() {
			dfa<int, Input> product;
			std::set<State> f_only = { symbol::end };
			std::map<std::set<State>, int> convert;
			size_t index = 0;


			for (auto& e1 : data) {
				for (auto& e2 : e1.second) {
					if (e2.second.size() != 0 &&
						e2.second != f_only &&
						convert.find(e2.second) == convert.end()) {
						convert[e2.second] = (e2.second.find(symbol::end) == e2.second.end()) ? index : -1;
						index++;
					}

					if (auto res = std::get_if<symbol>(&e1.first)) {
						if (*res == symbol::begin)
							product.get_data()[symbol::begin][e2.first] = convert[e2.second];
					}
				}
			}

			std::map<Input, std::set<State>> tmp;

			for (auto& e1 : convert) {
				tmp.clear();
				for (auto& e2 : e1.first) {
					for (auto& e3 : data[e2]) {

						tmp[e3.first].insert(e3.second.begin(), e3.second.end());
					}
				}

				for (auto& e2 : tmp) {

					if (e2.second.size() != 0 && e2.second != std::set<State>({ symbol::end }) && convert.find(e2.second) == convert.end()) {
						convert[e2.second] = index;
						index++;
						product.add_state(convert[e2.second]);
					}

					std::variant<symbol, int> begin = convert[e1.first];
					if (convert[e1.first] == -1)
						begin = symbol::end;
					std::variant<symbol, int> end = convert[e2.second];
					if (convert[e2.second] == -1)
						end = symbol::end;
					product.get_data()[begin][e2.first] = end;
				}
			}


			return product;
		}
	};

	namespace match {
		using accessor = std::optional<std::reference_wrapper< tree_accsssor<std::pair<std::string, std::string>>>>;
		using matcher = std::function<bool(std::string::iterator&, std::string::iterator, accessor)>;
		bool any(std::string::iterator& it,std::string::iterator end, accessor accessor = std::nullopt) {
			if (it != end)
				it++;
			return true;
		}

		auto character(char c) {
			return [c](std::string::iterator& it,std::string::iterator end, accessor accessor = std::nullopt) ->bool {
				if (it == end) return false;
				bool res = (*it == c);
				it++;
				return res;
			};
		}

		auto link(matcher f1, matcher f2) {
			auto f = [f1, f2](std::string::iterator& it,std::string::iterator end, accessor accessor = std::nullopt)->bool {
				auto res1 = f1(it,end, accessor);
				if (it == end) return res1;
				auto res2 = f2(it,end, accessor);
				return res1 && res2;
			};
			return f;
		}

		auto link(std::vector<matcher> f1) {
			auto f = [f1](std::string::iterator& it,std::string::iterator end, accessor accessor = std::nullopt)->bool {
				bool res = true;
				for (auto ef : f1) {
					res = ef(it,end, accessor);
					if (it == end) return res;
					if (!res) break;
				}
				return res;
			};
			return f;
		}

		auto select(matcher f1, matcher f2) {
			auto f = [f1, f2](std::string::iterator& it,std::string::iterator end, accessor accessor = std::nullopt)->bool {
				auto tmp = it;
				auto res1 = f1(tmp,end, accessor);
				if (it == end) return res1;
				if (!res1) {
					tmp = it;
					res1 = f2(tmp,end, accessor);
				}
				it = tmp;
				return res1;
			};
			return f;
		}

		auto select(std::vector<matcher> f) {
			auto fr = [f](std::string::iterator& it,std::string::iterator end, accessor accessor = std::nullopt)->bool {
				auto tmp = it;
				bool res;
				for (auto ef : f) {
					res = ef(it,end, accessor);
					if (it == end) return res;
					if (res) break;
					it = tmp;
				}
				return res;
			};
			return fr;
		}

		auto zmore(matcher f1) {
			auto f = [f1](std::string::iterator& it,std::string::iterator end, accessor accessor = std::nullopt)->bool {

				auto tmp = it;
				while (f1(tmp,end, accessor)) {
					if (it == end) break;
					it = tmp;
				}
				it = tmp;
				return true;
			};
			return f;
		}

		auto omore(matcher f1) {
			auto f = [f1](std::string::iterator& it,std::string::iterator end, accessor accessor = std::nullopt)->bool {
				auto tmp = it;
				bool res = f1(tmp,end, accessor);
				if (it == end) return res;
				if (!res) {
					return res;
				}
				it = tmp;
				while (f1(it,end, accessor)) {
					if (it == end) break;
					it = tmp;
				}
				return true;
			};
			return f;
		}

		auto optional(matcher f1) {
			auto f = [f1](std::string::iterator& it,std::string::iterator end, accessor accessor = std::nullopt)->bool {
				auto tmp = it;
				bool res = f1(tmp,end, accessor);
				if (res)
					it = tmp;
				return true;
			};
			return f;
		}

		auto follow(matcher f1) {
			auto f = [f1](std::string::iterator& it,std::string::iterator end, accessor accessor = std::nullopt)->bool {
				auto tmp = it;
				bool res = f1(tmp,end, accessor);
				return res;
			};
			return f;
		}

		auto not_follow(matcher f1) {
			auto f = [f1](std::string::iterator& it,std::string::iterator end, accessor accessor = std::nullopt)->bool {
				auto tmp = it;
				bool res = f1(tmp,end, accessor);
				return !res;
			};
			return f;
		}

		auto elem(matcher f, std::string id) {
			auto fr = [f, id](std::string::iterator& it,std::string::iterator end, accessor accessor = std::nullopt)->bool {

				auto from = it;
				auto branch = accessor.value().get().back();
				bool res = f(it,end, std::ref(branch));
				if (!res) {
					accessor.value().get().remove(branch);
				}
				std::string str = "";
				std::for_each(from, it, [&str, id](char ch) {str.push_back(ch); });

				if (accessor.has_value())accessor.value().get().add(std::make_pair(id, str));
				return res;
			};
			return fr;
		}

		auto character_class(std::string str) {
			std::vector<matcher> f;
			for (auto c : str) {
				f.push_back(character(c));
			}
			return select(f);
		}

		auto string(std::string str) {
			std::vector<matcher> f;
			for (auto ch : str) {
				f.push_back(character(ch));
			}
			return link(f);
		}

		auto filter(std::function<int(char)> f) {
			return [f](std::string::iterator& it,std::string::iterator end, accessor accessor = std::nullopt)->bool {
				if (it == end) return false;
				bool res = f(*it);
				if (res) it++;
				return res;
			};
		}

		auto alpha = character_class("abcdefghijklmnopqrstuvwxyz");
		auto num = character_class("0123456789");
		auto digit = omore(link(not_follow(character('0')), num));
		auto alnum = select(alpha, num);
		auto id = link(alpha, zmore(alnum));
		
		auto blank = filter(std::isblank);
		auto blanks = zmore(blank);
	}

	namespace PEG {
		auto el = match::select(match::character('.'), match::alpha);
		auto body = match::link(match::elem(el, "id"), match::character('+'));


	}

	auto parserer(match::matcher f) {

		return [f](std::string str)->tree<std::pair<std::string, std::string>> {

			tree<std::pair<std::string, std::string>> ast;
			auto root = ast.root();

			f(str.begin(),str.end(), std::ref(root));
			return ast;
		};
	}
}

namespace myasm {
	auto statement = parser::match::link(
		{ parser::match::elem(parser::match::id,"opecode"),
		parser::match::blanks,
		parser::match::elem(parser::match::id,"operand1"),
		parser::match::blanks,
		parser::match::elem(parser::match::id,"operand2") 
		});
	auto body = parser::match::omore(parser::match::link(parser::match::elem(statement, "statement"),parser::match::blanks));
}

int main() {
	auto t = parser::parserer(myasm::body)("move ax cx");

	int i = 0;
}
