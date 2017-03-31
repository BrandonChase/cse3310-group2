#include <ncurses.h>
#include <string>
#include <vector>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <panel.h>

using namespace std;

//Global Data
enum OnlineStatus { Online, Offline };
enum Windows { Login = 0, Chatroom = 1, Settings = 2 };
const int NUM_CHATROOMS = 10;
const int MESSAGE_LENGTH = 144;
const int MAX_CHAT_HISTORY = 15;
const int MAX_USER_NICK_SIZE = 8;
const int MAX_CHATROOM_NAME_SIZE = 20;
string FakeChatroomNames[] = { "Public", "Joe's Jumbo", "PJ's Palace", "Robert Rascals", "Kartik Krazy", "Brandon Bear", "White Lot", "Linux Sucks", "Windows Rocks", "String YoMa" };

class User
{
public:
	string Name;
	OnlineStatus Status;
	string ChatroomName;
	int UserID;
	int ChatroomID;
	int ColorIndex;
};

class Message
{
public:
	string UserName;
	int UserID;
	int ChatroomID;
	string Message;
};

//Global Data
vector <User> Users(0);
vector <Message> ChatMessages(0);
User currentUser;
int chatroom_menu_index;

//- - - - - - - - - - - CALCULATION FUNCTIONS - - - - - - - - - - -
WINDOW* MakeWindow(int Height, int Width, int Yposition, int Xposition, string Title)
{
	//Create the Window
	WINDOW* returnWindow = newwin(Height, Width, Yposition, Xposition);

	//Draw Borders around the window
	box(returnWindow, 0, 0);

	//Print the title if there is one
	if (Title != "")
	{
		mvwchgat(returnWindow, 1, 1, Width - 2, A_NORMAL, 1, NULL);
		wattron(returnWindow, A_BOLD | COLOR_PAIR(1));
		mvwprintw(returnWindow, 1, Width / 2 - Title.length() / 2, Title.c_str());
		wattroff(returnWindow, A_BOLD | COLOR_PAIR(1));
	}

	refresh();

	return returnWindow;
}

void DeleteWindows(vector<WINDOW*> & windows)
{
	//Clear all of the windows
	for (int i = 0; i < windows.size(); i++)
		delwin(windows[i]);
}

void CalculateChatroomNames(string* names)
{
	//Set the array to base Chatroom Names
	names[0] = "Public";
	for (int i = 1; i < 10; i++)
	{
		names[i] = "Chatroom #"; //Chatroom #1 ... #9
	}

	//If a user has a different name than the base names, then update the chatroom name
	for (int i = 0; i < Users.size(); i++)
	{
		if (Users[i].ChatroomID > 0 && Users[i].ChatroomID < 10)
		{
			if (names[Users[i].ChatroomID] != Users[i].ChatroomName)
				names[Users[i].ChatroomID] = Users[i].ChatroomName;
		}
	}
}

void CalculateChatroomStats(int* rooms)
{
	//Count how many users are in the chatroom

	//Set the array to 0
	for (int i = 0; i < 10; i++)
		rooms[i] = 0;

	rooms[currentUser.ChatroomID]++;
	//Total up the amount of users in each chatroom
	for (int i = 1; i < Users.size(); i++)
		rooms[Users[i].ChatroomID]++;
}

void FindOthersinUsersChatroom(vector<User> & usersInSameChatroom)
{
	//Find all of the users in the same chatroom as the current user
	usersInSameChatroom.push_back(currentUser);
	for (int i = 1; i < Users.size(); i++)
		if (Users[i].ChatroomID == currentUser.ChatroomID)
			usersInSameChatroom.push_back(Users[i]);
}

//- - - - - - - - - - - SETTINGS WINDOW - - - - - - - - - - -

WINDOW* Settings_TopBar()
{
	//Make the window
	WINDOW *window = MakeWindow(LINES, COLS, 0, 0, "");

	//Print the text inside top bar window
	mvwprintw(window, 1, 1, "F4 - Cancel\t\tF5 - Save and Return\t\tF6 - Change Username\t\tF7 - Change Chatroom Name");

	//Refresh the Window
	wrefresh(window);
	delwin(window);
	return window;
}

WINDOW* Settings_ChangeUserName(string new_nick)
{
	//Make the window
	int window_width = COLS / 2;

	WINDOW *window = MakeWindow(10, window_width - 1, 4, 1, "Change UserName");

	//Print Columns "Current UserName" and "New Username"
	wattron(window, A_BOLD);
	mvwprintw(window, 2, window_width / 2 - 8, "Current Username");
	mvwprintw(window, 6, window_width / 2 - 6, "New Username");
	wattroff(window, A_BOLD);

	//Print the Current Username
	wattron(window, COLOR_PAIR(9));
	mvwprintw(window, 3, window_width / 2 - currentUser.Name.length() / 2, currentUser.Name.c_str());
	wattroff(window, COLOR_PAIR(9));

	//Input box for New Username
	mvwchgat(window, 7, window_width / 2 - 10, 20, A_NORMAL, 4, NULL);
	mvwprintw(window, 8, window_width / 2 - 8, "%d characters only", MAX_USER_NICK_SIZE);

	wattron(window, COLOR_PAIR(4));
	mvwprintw(window, 7, window_width / 2 - 10, new_nick.c_str());
	wattroff(window, COLOR_PAIR(4));

	//Refresh the Window
	wrefresh(window);
	delwin(window);
	return window;
}

WINDOW* Settings_ChangeChatroomName(string new_chatroom_name)
{
	//Make the window
	int window_width = COLS / 2;

	WINDOW *window = MakeWindow(10, window_width, 4, window_width, "Change Chatroom Name");

	//Print Columns "Current UserName" and "New Username"
	wattron(window, A_BOLD);
	mvwprintw(window, 2, window_width / 2 - 8, "Current Chatroom Name");
	mvwprintw(window, 6, window_width / 2 - 6, "New Chatroom Name");
	wattroff(window, A_BOLD);

	//Print the Current Username
	wattron(window, COLOR_PAIR(9));
	mvwprintw(window, 3, window_width / 2 - currentUser.ChatroomName.length() / 2, currentUser.ChatroomName.c_str());
	wattroff(window, COLOR_PAIR(9));

	//Input box for New Chatroom name
	mvwchgat(window, 7, window_width / 2 - 8, 20, A_NORMAL, 4, NULL);
	mvwprintw(window, 8, window_width / 2 - 7, "%d characters only", MAX_CHATROOM_NAME_SIZE);

	wattron(window, COLOR_PAIR(4));
	mvwprintw(window, 7, window_width / 2 - 8, new_chatroom_name.c_str());
	wattroff(window, COLOR_PAIR(4));

	//Refresh the Window
	wrefresh(window);
	delwin(window);
	return window;
}

WINDOW* Settings_AllUsers()
{
	//Make the window
	WINDOW *window = MakeWindow(20, COLS - 2, 14, 1, "All Users");

	//Create the Columns "Users", "Status", "Chatroom", "Time Online"
	wattron(window, A_BOLD);
	mvwprintw(window, 2, 1, "%-8s\t\t\t%-8s\t\t\t%-10s\t\t\t%s", "User", "Status", "Chatroom", "Time Online");
	wattroff(window, A_BOLD);

	//Print the User Profiles
	for (int i = 0; i < Users.size(); i++)
	{
		//Print User Name
		mvwprintw(window, 3 + i, 1, "%-8s\t\t\t", Users[i].Name.c_str());
		if (Users[i].Status == Online)
		{
			//Print Online Status
			wattron(window, COLOR_PAIR(2));
			wprintw(window, "%-8s\t\t\t", "Online");
			wattroff(window, COLOR_PAIR(2));
			//Print ChatroomName and Time Online
			wprintw(window, "%-10s\t\t\t%s", Users[i].ChatroomName.c_str(), "2 Minutes");
		}
		else
		{
			//Print Offline Status
			wattron(window, COLOR_PAIR(3));
			wprintw(window, "%-8s\t\t\t", "Offline");
			wattroff(window, COLOR_PAIR(3));
			//Print Blanks for ChatroomName and Time Online Fields
			wprintw(window, "%-10s\t\t\t%s", "----------", "----------");
		}
	}

	//Refresh the Window
	wrefresh(window);
	delwin(window);
	return window;
}

void Settings_Draw()
{
	string new_user_nick = currentUser.Name, new_chatroom_name = currentUser.ChatroomName;
	int input_char;

	Settings_TopBar();
	Settings_ChangeUserName("New Nick");
	Settings_ChangeChatroomName("New Chatroom Name");
	Settings_AllUsers();

	int window_char, input;
	input = getch();
	while (window_char = input)
	{
		//Change the User's Name
		if (window_char == KEY_F(6))
		{
			new_user_nick = "";
			Settings_ChangeUserName(new_user_nick);

			do
			{
				input = getch();
				new_user_nick += input;
				Settings_ChangeUserName(new_user_nick);

			} while ((input > 32 && input < 127) && new_user_nick.size() < MAX_USER_NICK_SIZE);
		}
		//Change the Chatroom Name
		else if (window_char == KEY_F(7))
		{
			new_chatroom_name = "";
			Settings_ChangeChatroomName(new_chatroom_name);

			do
			{
				input = getch();
				new_chatroom_name += input;
				Settings_ChangeChatroomName(new_chatroom_name);

			} while ((input > 32 && input < 127) && new_chatroom_name.size() < MAX_CHATROOM_NAME_SIZE);
		}
		else if (window_char == KEY_F(4))
		{
			break;
		}
		else if (window_char == KEY_F(5))
		{
			//Save the results if the user wanted to
			currentUser.Name = new_user_nick;
			currentUser.ChatroomName = new_chatroom_name;
			break;
		}
	}
}

//- - - - - - - - - - - CHAT MESSAGE WINDOW - - - - - - - - - - -

WINDOW* ChatMessage_TopBar()
{
	//Make the window
	WINDOW *window = MakeWindow(LINES, COLS, 0, 0, "");


	//Print the text inside top bar window
	mvwprintw(window, 1, 1, "F4 - Chatrooms Menu \t\t F5 - Send Message \t\t F6 - Settings \t\t F7 - Logout");

	//Refresh the Window
	wrefresh(window);
	delwin(window);
}

WINDOW* ChatMessage_Users()
{
	//Create the Window
	int userHeight = 21, userWidth = 30;
	string header = currentUser.ChatroomName + " Users";
	WINDOW* window = MakeWindow(userHeight, userWidth, 19, 1, header);

	//Print the text inside the Users Window
	mvwprintw(window, userHeight - 2, userWidth / 2 - 7, "F6 - All Users");


	//Only show the current users in the chatroom
	vector<User> usersInSameChatroom;
	FindOthersinUsersChatroom(usersInSameChatroom);
	for (int i = 0; i < usersInSameChatroom.size(); i++)
	{
		//Print the User's Name
		mvwprintw(window, 2 + i, 2, usersInSameChatroom[i].Name.c_str());
		if (usersInSameChatroom[i].Status == Online)
		{
			//If the user is online, print a green word
			wattron(window, COLOR_PAIR(2));
			mvwprintw(window, 2 + i, 20, "Online");
			wattroff(window, COLOR_PAIR(2));
		}
		else
		{
			wattron(window, COLOR_PAIR(3));
			mvwprintw(window, 2 + i, 20, "Offline");
			wattroff(window, COLOR_PAIR(3));
		}
	}

	//Refresh the Window
	wrefresh(window);
	delwin(window);
}

WINDOW* ChatMessage_Chatrooms(int SelectedIndex)
{
	int chatHeight = 16, chatWidth = 30;
	int roomStats[10];
	string roomNames[10];

	//Create the Window
	WINDOW *window = MakeWindow(chatHeight, chatWidth, 3, 1, "Chatrooms");

	//Display the Chatroom Footer
	mvwprintw(window, chatHeight - 2, chatWidth / 2 - 12, "F5 to Switch to Chatroom");

	//Display the Chatrooms
	CalculateChatroomNames(roomNames);
	CalculateChatroomStats(roomStats);

	for (int i = 0; i < 10; i++)
	{
		mvwprintw(window, 2 + i, 1, "%s", roomNames[i].c_str());

		//if there is more than one user in the chatroom
		if (roomStats[i] > 1)
			mvwprintw(window, 2 + i, 20, "%d users", roomStats[i]);
		//if there is only one user in the chatroom
		else if (roomStats[i] == 1)
			mvwprintw(window, 2 + i, 20, "%d user", roomStats[i]);
		//there are no users in the chatroom
		else
		{
			mvwchgat(window, 2 + i, 1, chatWidth - 2, A_NORMAL, 8, NULL);
			wattron(window, A_NORMAL | COLOR_PAIR(8));
			mvwprintw(window, 2 + i, 20, "0 users");
			wattroff(window, A_NORMAL | COLOR_PAIR(8));
		}

		//highlight the selected user
		if (SelectedIndex == i)
			mvwchgat(window, 2 + i, 1, chatWidth - 2, A_NORMAL, 9, NULL);
	}

	//Refresh the Window
	wrefresh(window);
	delwin(window);
}

WINDOW* ChatMessage_ChatHistory()
{
	int winWidth = COLS - 32;
	int winHeight = LINES - 13;
	//Initialize the Window
	WINDOW *window = MakeWindow(winHeight, winWidth, 3, 31, currentUser.ChatroomName);

	//delete a message if necessary to keep it at a max of MAX_CHAT_HISTORY
	if (ChatMessages.size() > MAX_CHAT_HISTORY)
		ChatMessages.erase(ChatMessages.begin());

	//Print the Chat History
	for (int i = 0; i < ChatMessages.size() && i < MAX_CHAT_HISTORY; i++)
	{
		mvwprintw(window, 2 * i + 2, 2, "%s:", ChatMessages[i].UserName.c_str());
		mvwprintw(window, 2 * i + 3, 5, "%s", ChatMessages[i].Message.c_str());
	}

	//Refresh the Window
	wrefresh(window);
	delwin(window);
}

WINDOW* ChatMessage_SendMessage(string Message)
{
	int winHeight = 9;
	int winWidth = COLS - 32;
	int charCount = 0;

	//Initialize the Window
	WINDOW *window = MakeWindow(winHeight, winWidth, LINES - winHeight - 1, 31, "Send a Message");

	//Print the "Send a Message" Footer
	mvwprintw(window, winHeight - 2, 1, "Press 'Enter' To Send Message");
	mvwprintw(window, winHeight - 2, winWidth - 26, "%d Characters Remaining", MESSAGE_LENGTH - Message.length());

	//Print the message
	mvwprintw(window, 2, 1, Message.c_str());

	//Refresh the Window
	wrefresh(window);
	delwin(window);
}

int ChatMessage_Draw(int chatroom_index)
{
	WINDOW *mainWin = newwin(LINES, COLS, 0, 0);

	//Assign the user to the passed chatroomIndex
	currentUser.ChatroomID = chatroom_index;
	//Do Lookup below for chatroomName
	currentUser.ChatroomName = FakeChatroomNames[chatroom_index];

	ChatMessage_TopBar();
	ChatMessage_Users();
	ChatMessage_Chatrooms(-1);
	ChatMessage_ChatHistory();
	ChatMessage_SendMessage("");

	//Navigation
	int window_char, sub_char;
	sub_char = getch();
	while (window_char = sub_char)
	{
		if (window_char == KEY_F(4))
		{
			int index = 0;

			//Draw initial
			ChatMessage_Chatrooms(index);

			//Get keyboard for up, down, or other function keys
			do
			{
				sub_char = getch();
				switch (sub_char)
				{
				case KEY_UP:
					ChatMessage_Chatrooms(index >= 1 ? --index : 0);
					break;
				case KEY_DOWN:
					ChatMessage_Chatrooms(index == NUM_CHATROOMS - 1 ? NUM_CHATROOMS - 1 : ++index);
					break;
				case KEY_F(5):
					//Change user to this index of chatroom
					return index;
				default:
					break;
				}

			} while (sub_char == KEY_UP || sub_char == KEY_DOWN);
		}

		//Go to the 'Send Message' Window
		else if (window_char == KEY_F(5))
		{
			//Clear the screen again
			string spaces, user_Message;
			for (int i = 0; i < MESSAGE_LENGTH; i++) spaces += " ";

			//Clear the message and initialize the characters
			user_Message = "";
			do
			{
				ChatMessage_SendMessage(user_Message);
				sub_char = getch();

				//Check what input was
				if (sub_char == 10) //ENTER key
				{
					//Send the message
					Message newMessage;
					newMessage.UserName = currentUser.Name;
					newMessage.UserID = currentUser.UserID;
					newMessage.ChatroomID = currentUser.ChatroomID;
					newMessage.Message = user_Message;

					ChatMessages.push_back(newMessage);
					user_Message = "";

					//Redraw the Chatmessage History
					ChatMessage_ChatHistory();
				}
				else if (sub_char == 127) //Backspace key
				{
					ChatMessage_SendMessage(user_Message);
				}

				//add the character if the message is not longer than MESSAGE_LENGTH and the character is not 'enter'
				if (user_Message.length() < MESSAGE_LENGTH && sub_char != 10)
					user_Message += sub_char;

			} while ((sub_char >= 32 && sub_char <= 127) || sub_char == 10);

			ChatMessage_SendMessage(spaces);
		}

		//Go to Settings Window
		else if (window_char == KEY_F(6))
		{
			Settings_Draw();
			break;
		}

		//Go to the Logout Window
		else if (window_char == KEY_F(7))
		{
			//Show the Login Screen
			return -1;
			//StartScreen_Draw();
		}
	}
}

//- - - - - - - - - - - START WINDOW - - - - - - - - - - -

WINDOW* StartScreen_TopBorder()
{
	//Make the window
	WINDOW* window = MakeWindow(3, COLS, 0, 0, "");

	//Print the text inside the window
	mvwprintw(window, 1, 1, "F4 - Set User Nick \t Enter Key - Login \t F6 - EXIT ");

	//Refresh the window
	wrefresh(window);
	delwin(window);
}

WINDOW* StartScreen_Username(string userName)
{
	//Make the window
	WINDOW* window = MakeWindow(LINES - 3, COLS, 3, 0, "Username");

	//Print the limits
	mvwprintw(window, 3, COLS / 2 - 8, "8 Character Limit");
	mvwchgat(window, 2, COLS / 2 - 10, 20, A_NORMAL, 4, NULL);

	wattron(window, COLOR_PAIR(4));
	mvwprintw(window, 2, COLS / 2 - 10, userName.c_str());
	wattroff(window, COLOR_PAIR(4));

	//Refresh the window
	wrefresh(window);
	delwin(window);
}

void StartScreen_Draw()
{
	StartScreen_TopBorder();
	StartScreen_Username("usernick");

	string user_nick;
	int input_char;
	int chat_room_index;

	user_nick = "";
	chat_room_index = 0;

	do
	{
		input_char = getch();
		if (input_char == KEY_F(4))
		{
			do // login 
			{
				StartScreen_TopBorder();
				StartScreen_Username(user_nick.c_str());
				input_char = getch();
				if (input_char == 10) // enter key
				{
					do
					{
						chat_room_index = ChatMessage_Draw(chat_room_index);
						if (chat_room_index<-1 && chat_room_index>9)
						{
							chat_room_index = 0; // public
						}
					} while (chat_room_index != -1);
					StartScreen_Username("usernick");
				}
				else
				{
					user_nick += input_char;
				}
			} while ((input_char>32 && input_char<128) || input_char == 10);
		}
	} while (input_char != KEY_F(6));

}

//- - - - - - - - - - - FAKE DATA CREATION - - - - - - - - - - -
void CreateFakeData()
{
	string userNames[] = { "Joe", "Robert", "Rivka", "Ramon", "Stephani", "Jewel", "Isaias", "Murray", "Darell", "Alyce", "Carylon", "Dona", "George", "Doug", "Hannah" };
	srand(time(NULL));

	//Create the Fake Users
	User tempUser;
	for (int i = 0, chatID = rand() % 10; i < 15; i++, chatID = rand() % 10)
	{
		tempUser.Name = userNames[rand() % 15];
		if (i % 2 == 0)
			tempUser.Status = Online;
		else tempUser.Status = Offline;
		tempUser.ChatroomName = FakeChatroomNames[chatID];
		tempUser.UserID = i;
		tempUser.ColorIndex = -1;
		tempUser.ChatroomID = chatID;
		Users.push_back(tempUser);
	}

	//Assign the current User as the first user in Users
	currentUser = Users[0];
	vector<User> usersInSameChatroom(0);
	FindOthersinUsersChatroom(usersInSameChatroom);

	//Create the Fake Chat Messages
	Message tempMessage;
	for (int i = 0; i < 3; i++)
	{
		tempMessage.UserName = usersInSameChatroom[rand() % usersInSameChatroom.size()].Name;
		tempMessage.UserID = usersInSameChatroom[rand() % usersInSameChatroom.size()].UserID;
		tempMessage.Message = "";
		for (int j = 0, randomNum = rand() % 144; j < randomNum; j++)
		{
			tempMessage.Message += rand() % 26 + 65;
		}

		ChatMessages.push_back(tempMessage);
	}
}

//- - - - - - - - - - - MAIN - - - - - - - - - - -
void StartGUI()
{
	//initialize ncurses
	initscr();
	start_color();
	cbreak();
	keypad(stdscr, TRUE);
	noecho();

	//Create the Color Pairs - Global Throughout Program
	init_pair(1, COLOR_BLACK, COLOR_WHITE);	 //Inverse
	init_pair(2, COLOR_GREEN, COLOR_BLACK); //Online
	init_pair(3, COLOR_RED, COLOR_BLACK);   //Offline
	init_pair(4, COLOR_WHITE, COLOR_BLUE);	//Textbox
	init_pair(5, COLOR_WHITE, COLOR_BLACK); //Normal
	init_pair(7, COLOR_WHITE, COLOR_YELLOW); //Element of Interest
	init_pair(8, COLOR_YELLOW, COLOR_BLACK);
	init_pair(9, COLOR_BLUE, COLOR_BLACK);
	init_pair(10, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(11, COLOR_CYAN, COLOR_BLACK);
	init_pair(12, COLOR_BLUE, COLOR_WHITE); //selected Index

											//Create Fake Data to similate program
	CreateFakeData();

	//Show the Login Screen
	StartScreen_Draw();

	endwin();
}

int main()
{

	StartGUI();

	return 0;
}