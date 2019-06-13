#include <iostream>
#include <vector>
#include <string>
#include <any>
#include <typeinfo>
#include <initializer_list>
#include <functional>
#include <eh.h>
#include <Windows.h>
#include <curses.h>

// this could be done with a map<std::string, std::any>
template<typename T>
class item {
public:
	item(std::string new_title, std::function<void(T)> func, T arg) : title(new_title), function(func), argument(arg) {}
	std::string get_title() {
		return title;
	}
	void set_argument(T arg) {
		argument(arg);
	}

	T get_argument() {
		return argument;
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
	T argument;
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
	menu(std::string title, std::initializer_list<std::any> items) {
		menu_title = title;

		if (items.size() == 0) {
			menu_items.push_back(item<void>("", []{})); // If menu has no items, this will backfire before you know it
		}
		else {
			menu_items = items;
		}
	};
	std::vector<std::any> menu_items;
	std::string menu_title;

};

template<typename T> class homogen_handler {
public:
	homogen_handler(menu m) : handled_menu(m), display_menu(m) {
		menu_trail.push_back(display_menu);
		alive = true;
		initscr();
		noecho();
		curs_set(0);
		keypad(stdscr, TRUE);
	}

	void operator()() {
		std::any selected_item = display_menu.menu_items[current_selection];

		if (selected_item.type() == t_value) {
			auto itm = std::any_cast<item<T>>(selected_item);
			itm.execute(itm.get_argument());
		}
		else if (selected_item.type() == t_function) {
			std::any_cast<item<void>>(selected_item).execute();
		}
		else if (selected_item.type() == t_menu) {
			
			descend();
		}

	}

	void operator[](int selection) {
		set_selection(-1);
		current_selection = selection;
		set_selection(current_selection);
	}

	homogen_handler& operator++(int) {
		if (current_selection == display_menu.menu_items.size()-1) {
			set_selection(-1);
			current_selection = 0;
			set_selection(current_selection);
		}
		else {
			set_selection(-1);
			current_selection++;
			set_selection(current_selection);
		}
		return *this;
	}

	homogen_handler& operator--(int) {
		if (current_selection == 0) {
			set_selection(-1);
			current_selection = display_menu.menu_items.size()-1;
			set_selection(current_selection);
		}
		else {
			set_selection(-1);
			current_selection--;
			set_selection(current_selection);
		}
		return *this;
	}

	homogen_handler& operator+=(std::any _item) {

		if (_item.type() == t_value || _item.type() == t_function || _item.type() == t_menu) {
			menu_trail.back().menu_items.push_back(_item);
			display_menu = menu_trail.back();
		}
		return *this;
	}

	homogen_handler& operator+(std::any _item) {
		operator+=(_item);
		return *this;
	}


	/*
	
	Needs +=/+ operator to add a item at runtime at bottom
	Needs -=/- operator to remove a specifc item at runtime anywhere in the menu recursively 
	Needs implementation with rat !!!!

	
	*/


	bool isalive() {
		return alive;
	}

	int get_key() {
		return getch();
	}

	void show() {
		men = center_box(2, getmaxy(stdscr), getmaxx(stdscr));

		center_title(men, display_menu.menu_title);

		print_menu(men, display_menu, -1);

		set_selection(0);
	}

	~homogen_handler() {
		endwin();
	}

	menu* get_menu() {
		return &display_menu;
	}

	void set_selection(int selection) {
		if (selection < 0) {
			print_item(men, current_selection, false);
			return;
		}
		current_selection = selection;
		print_item(men, selection, true);
		refresh();
		wrefresh(men);
	}

	void descend() {
		next_menu(display_menu, current_selection);
	}

	void ascend() {
		next_menu(display_menu, -1);
	}

	std::string get_tail() {
		std::string sep, output;
		for (auto i_menu : menu_trail) {
			output += sep + i_menu.menu_title;
			sep = " -> ";
		}
		return output;
	}

private:
	menu handled_menu;
	menu display_menu;
	std::vector<menu> menu_trail;

	WINDOW* men;

	bool alive;

	int current_selection = 0;

	const std::type_info& t_value = typeid(item<T>);
	const std::type_info& t_function = typeid(item<void>);
	const std::type_info& t_menu = typeid(menu);

	WINDOW* new_box(int height, int width, int start_y, int start_x) {
		WINDOW* local;
		local = newwin(height, width, start_y, start_x);
		box(local, 0, 0);
		return local;
	}

	WINDOW* center_box(int margin, int y_max, int x_max) {
		return new_box(y_max - margin * 2, x_max - (margin * 4), margin, margin * 2);
	}

	void center_title(WINDOW* win, std::string title) {
		title = " " + title + " ";
		mvwprintw(win, 0, ceil((getmaxx(win) - title.size()) / 2), title.c_str());
	}

	void print_item(WINDOW* win, int index, bool selected) {
		std::any current_item = display_menu.menu_items[index];

		std::string title;

		if (current_item.type() == t_value) {
			title = std::any_cast<item<T>>(current_item).get_title();
		}
		else if (current_item.type() == t_function) {
			title = std::any_cast<item<void>>(current_item).get_title();
		}
		else if (current_item.type() == t_menu) {
			title = std::any_cast<menu>(current_item).menu_title;
		}
		if (selected) {
			
			wattron(win, A_STANDOUT);
			mvwprintw(win, 1 + index, 2, title.c_str());
			wattroff(win, A_STANDOUT);
			current_selection = index;
		}
		else {
			mvwprintw(win, 1 + index, 2, title.c_str());
		}
		
	}

	void print_menu(WINDOW* win, menu men, int selected) {
		for (auto i = 0; (i < men.menu_items.size()); i++) {
			if (i == selected) {
				print_item(win, i, true);
			}
			else {
				print_item(win, i, false);
			}
		}
	}

	void redraw_menu(WINDOW* _menu) {
		wclear(_menu);
		wrefresh(_menu);
		show();
	}

	void next_menu(menu _menu, int selection) {
		if (selection == -1) {
			if (menu_trail.size() >= 2) {
				display_menu = (menu_trail.end()[-2]);
				menu_trail.erase(menu_trail.end() - 1);
				redraw_menu(men);
			}
		}
		else if (selection < _menu.menu_items.size() && _menu.menu_items[selection].type() == t_menu) {
			display_menu = std::any_cast<menu>(_menu.menu_items[selection]);
			menu_trail.push_back(display_menu);
			redraw_menu(men);
		}
	}
};

int main() {
	auto sample = [](int a) {mvprintw(0,0, "%d", rand()*a); };
	auto sample1 = []() {mvprintw(1,0, "%d", rand()*10); };

	menu aa("Sample Title", {
			item<int>("Test Item", sample, 1),
			item<void>("Testing Items", sample1),
			item<int>("Test Item", sample, 1),
			item<int>("Testing Items", sample, 1),
			item<int>("Test Item", sample, 1),
			item<void>("Test Item", sample1),
			menu("Hello World", {
				menu("Hello World", {
					menu("Hello World", {
						item<int>("Test Item", sample, 1),
						item<int>("Testing Items", sample, 1)
					}),
					item<int>("Testing Items", sample, 1),
					item<int>("Testing Items", sample, 1)
				}),
				item<int>("Testing Items", sample, 1),
				item<int>("Testing Items", sample, 1),
				item<int>("Testing Items", sample, 1)
			}),
			menu("Hello World", {
				menu("Hello World", {
					menu("Hello World", {
						menu("Hello World", {
							menu("Hello World", {
								menu("Hello World", {
									item<int>("Test Item", sample, 1),
									item<int>("Testing Items", sample, 1)
								}),
								item<int>("Testing Items", sample, 1)
							}),
							item<int>("Testing Items", sample, 1),
							item<int>("Testing Items", sample, 1),
							item<int>("Testing Items", sample, 1)
						}),
						item<int>("Testing Items", sample, 1),
						item<int>("Testing Items", sample, 1)
					}),
					item<int>("Testing Items", sample, 1)
				}),
				item<int>("Testing Items", sample, 1),
				item<int>("Testing Items", sample, 1),
				item<int>("Testing Items", sample, 1),
				item<int>("Testing Items", sample, 1),
				item<int>("Testing Items", sample, 1)
			}),
		});

	homogen_handler<int> a(aa);

	a += menu("Hello World1", {
			item<int>("Test Item", sample, 1),
			item<int>("Testing Items", sample, 1)
		});

	a + menu("TESTING MENU", {});

	a.show();

	while (a.isalive()) {

		switch (a.get_key()) {

			case KEY_UP:
				a--;
				break;
			case KEY_DOWN:
				a++;
				break;
			case '\n':
				a();
				break;
			case '\b':
				a.ascend();
				break;

		}
	}
}
