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
	//// Type Identification

	//const std::type_info& value_typeinfo = typeid(item<T>);
	//const std::type_info& function_typeinfo = typeid(item<void>);
	//const std::type_info& menu_typeinfo = typeid(item<menu>);
	//T value_type{};
};

template<typename T> T c(std::any caster) {
	return std::any_cast<T>(caster);
}

int main() {
	const std::type_info& value_typeinfo = typeid(item<int>);
	const std::type_info& function_typeinfo = typeid(item<void>);
	const std::type_info& menu_typeinfo = typeid(menu);
	int ab{};
	//item<std::string> a("Hello");

	auto sample = [](int a) {std::cout << a << std::endl; };
	auto sample1 = []() {std::cout << 1 << std::endl; };

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

	for (auto menu_item : aa.menu_items) {
		if (menu_item.type() == function_typeinfo) {
			std::cout << c<item<void>>(menu_item).get_title() << " is a function" << std::endl;
		}
		else if (menu_item.type() == menu_typeinfo) {
			std::cout << c<menu>(menu_item).menu_title << " is a menu" << std::endl;
			std::cout << "Children: " << std::endl;
			for (auto menu_item : aa.menu_items) {
				if (menu_item.type() == value_typeinfo) {
					std::cout << c<item<decltype(ab)>>(menu_item).get_title() << " is a item" << std::endl;
					std::cout << "    Executing " << c<item<decltype(ab)>>(menu_item).get_title() << std::endl;
					c<item<decltype(ab)>>(menu_item).execute(rand());
				}
			}
		}
		else if (menu_item.type() == value_typeinfo) {
			std::cout << c<item<decltype(ab)>>(menu_item).get_title() << " is a item" << std::endl;
			std::cout << "    Executing " << c<item<decltype(ab)>>(menu_item).get_title() << std::endl;
			c<item<decltype(ab)>>(menu_item).execute(rand());
		}
	}
}