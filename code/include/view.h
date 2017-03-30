#ifndef VIEW_H
#define VIEW_H 2016

#include <ncurses.h>
#include <string>
#include <vector>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <panel.h>

#include "model.h"

enum class Window { Login = 0, Chatroom = 1, Settings = 2 };

class View
{
public:
	View();
	void initialize();
	void handleInput(char ch);

private:
	Model& chat_building;

	//ncurses stuff
	int chatroom_menu_index;
	WINDOW* settings_top_bar;
	WINDOW* settings_change_user_name;
	WINDOW* settings_change_chat_room_name;
	WINDOW* settings_all_users;
	
	WINDOW* createSettingsTopBar();
	WINDOW* createSettingsChangeUserName();
	WINDOW* createSettingsChangeChatroomName();
	WINDOW* createSettingsAllUsers();

	WINDOW* makeWindow(int Height, int Width, int Yposition, int Xposition, std::string Title);
	void deleteWindows(std::vector<WINDOW*> & windows);
};

#endif
