#include <iostream>
#include <vector>
#include <string>
#include <any>
#include <typeinfo>
#include <initializer_list>
#include <functional>
// this could be done with a map<std::string, std::any>
template<typename T>
class item {
public:
	item(std::string new_title, std::function<void(T)> func) : title(new_title), function(func) {}
	std::string get_title() {
		return title;
	}
	void set_title(std::string new_title) {
		title = new_title;
	}
	void execute(T arg) {
		function(arg);
	}
	void operation(std::function<void(T)> func) {
		function = func;
	}
private:
	std::string title;
	std::function<void(T)> function;
};
template<>
class item<void> {
public:
	item(std::string new_title, std::function<void()> func) : title(new_title), function(func) {}
	std::string get_title() {
		return title;
	}
	void set_title(std::string new_title) {
		title = new_title;
	}
	void execute() {
		function();
	}
	void operation(std::function<void()> func) {
		function = func;
	}
private:
	std::string title;
	std::function<void()> function;
};

struct menu {
	menu(std::string title, std::initializer_list<std::any> items) : menu_items(items), menu_title(title) {}
	std::vector<std::any> menu_items;
	std::string menu_title;
};

template<typename T> void print_menu(menu sample, int level) {
	const std::type_info& value_typeinfo = typeid(item<int>);
	const std::type_info& function_typeinfo = typeid(item<void>);
	const std::type_info& menu_typeinfo = typeid(menu);
	T ab;
	std::cout << std::string(level * 4, ' ') << sample.menu_title << std::endl;
	for (auto menu_item : sample.menu_items) {
		if (menu_item.type() == function_typeinfo) {
			std::cout << std::string(level * 4 + 4, ' ') << std::any_cast<item<void>>(menu_item).get_title() << std::endl;
		}
		else if (menu_item.type() == menu_typeinfo) {
			print_menu<T>(c<menu>(menu_item), level + 1);
		}
		else if (menu_item.type() == value_typeinfo) {
			std::cout << std::string(level * 4 + 4, ' ') << std::any_cast<item<decltype(ab)>>(menu_item).get_title() << std::endl;
		}
	}
}

struct handler {
	handler(menu m) : handled_menu(m){}
	menu handled_menu;
};

int main() {

	int ab{};

	auto sample = [](int a) {std::cout << "    " << a << std::endl; };
	auto sample1 = []() {std::cout << "    " << 1 << std::endl; };

	menu aa("Sample Title",
	{
		item<int>("s", sample),
		item<void>("ss", sample1),
		item<int>("s", sample),
		item<int>("ss", sample),
		item<int>("s", sample),
		item<void>("s", sample1),
		menu("Hello World", {
			menu("Hello World", {
				menu("Hello World", {
					item<int>("s", sample),
					item<int>("ss", sample)
				}),
				item<int>("ss", sample)
			}),
			item<int>("ss", sample)
		}),
		menu("Hello World", {
			menu("Hello World", {
				menu("Hello World", {
					menu("Hello World", {
						menu("Hello World", {
							menu("Hello World", {
								item<int>("s", sample),
								item<int>("ss", sample)
							}),
							item<int>("ss", sample)
						}),
						item<int>("ss", sample)
					}),
					item<int>("ss", sample)
				}),
				item<int>("ss", sample)
			}),
			item<int>("ss", sample),
			item<int>("ss", sample),
			item<int>("ss", sample),
			item<int>("ss", sample),
			item<int>("ss", sample)
		}),
	});

	print_menu<int>(aa, 0);
}