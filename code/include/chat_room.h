#ifndef CHATROOM_H
#define CHATROOM_H 2016

#include <deque>
#include <string>

#include "constants.h"
#include "message.h"
#include "user.h"

class ChatRoom
{
public:
	ChatRoom();

	std::deque<Message> message_history; //Using deque because it allows easy push_back, pop_front for history

	//TODO: .toStruct for OpenSplice

	void changeName(std::string desired_name);
	void addMessage(Message message);

	//getters
	std::string getName();
	unsigned long getChatRoomIndex();

	//setters
	void setName(std::string desired_name);

private:
	unsigned long chat_room_index; //DISCUSION: could be int?
	std::string chat_room_name;
};

#endif
