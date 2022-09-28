#pragma once

uint32_t constexpr MAX_MSG_LENGTH = 1024;
uint32_t constexpr MAX_RUDE_PERSONS = 100;
uint32_t constexpr MAX_NAME_LENGTH = 256;
constexpr char *RUDE_PERSON_FILE_NAME = "rudePersonCollection.txt";
global_variable SOCKET twitchSocket;

struct Message
{
	char *buffer;
	uint32_t length;
};

bool str_cmp( char *a,  char *b);
void send_recieve_and_print_msg(char *msg);
void send_message(char *msg, int msgLength);
Message recieve_message(char *buffer);

struct RudePerson
{
	uint32_t nameLength;
	char name[MAX_NAME_LENGTH];
	uint32_t rudeCount;
};

struct RudePersonCollection
{
	uint32_t rudePersonCount;
	RudePerson rudePersons[100];
};

void add_rude_person(RudePersonCollection *rudePersonCollection,  char *name, uint32_t nameLength)
{
	assert(rudePersonCollection, "No RudePersonCollection supplied: %d", rudePersonCollection);
	assert(name, "No name supplied: %d", name);

	if (rudePersonCollection->rudePersonCount < MAX_RUDE_PERSONS)
	{
		RudePerson *rp = &rudePersonCollection->rudePersons[rudePersonCollection->rudePersonCount++];
		*rp = {};
		rp->nameLength = nameLength;
		memcpy(rp->name, name, nameLength);
		rp->rudeCount = 1;
	}
	else
	{
		assert(0, "Reached maximum amount of rude Persons");
	}

	platform_write_file(RUDE_PERSON_FILE_NAME, ( char *)rudePersonCollection, sizeof(RudePersonCollection), true);
}

void connect_to_chat()
{
	WSAData windowsSocketData;
	WSAStartup(MAKEWORD(1, 2), &windowsSocketData);

	addrinfo socketInfo = {};
	addrinfo *result = 0;

	socketInfo.ai_family = AF_UNSPEC;
	socketInfo.ai_socktype = SOCK_STREAM;
	socketInfo.ai_protocol = IPPROTO_TCP;

	if (!getaddrinfo("irc.chat.twitch.tv", "6667", &socketInfo, &result))
	{
		twitchSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (twitchSocket != INVALID_SOCKET)
		{
			if (!connect(twitchSocket, result->ai_addr, (int)result->ai_addrlen))
			{
				//Connect to twitch using anonymous connection justinfan#
				{
					char authStr[100];
					int length = sprintf_s(authStr, sizeof(authStr), "PASS oauth:%s\r\n", REFRESH_TOKEN);
					send_message(authStr, length);
					// send_recieve_and_print_msg(authStr);
					send_recieve_and_print_msg("NICK tkap_bot\r\n");
					//send_recieve_and_print_msg("CAP REQ :twitch.tv/commands twitch.tv/tags\r\n");
					// send_recieve_and_print_msg(twitchSocket, "NICK lanzelorder\r\n");
				}

				// Join channel of broadcaster to listen
				{
					send_recieve_and_print_msg("JOIN #tkap1");
				}

				std::chrono::steady_clock::time_point lastTimePoint = std::chrono::high_resolution_clock::now();
				float dt = 0.0f;
				float appTime = 0.0f;
				while (true)
				{
					// Take the time it took to update the program
					auto now = std::chrono::high_resolution_clock::now();
					dt = (float)std::chrono::duration<double, std::milli>(now - lastTimePoint).count();
					lastTimePoint = now;

					appTime += dt / 1000.0f;

					char buffer[MAX_MSG_LENGTH] = {};
					Message m = recieve_message(buffer);

					if (m.length > 0)
					{

						if (str_in_str(m.buffer, "PING"))
						{
							char *pong = "PONG tmi.twitch.tv\r\n";
							send_message(pong, (int)strlen(pong));
						}

						CAKEZ_TRACE("%s", m.buffer);

						char *msgBegin = m.buffer + 1;
						char *userBegin = 0;
						int length = 0;
						char userName[200] = {};

						while (char c = *(msgBegin++))
						{
							if (c == '@')
							{
								userBegin = msgBegin - 1;
							}

							if (c == '\r' || c == '\n')
							{
								*(msgBegin - 1) = ' ';
							}

							if (userBegin)
							{
								length++;
							}

							if (userBegin && c == '.' && !userName[0])
							{
								memcpy(userName, userBegin, length - 1);
							}

							if (c == ':')
							{
								break;
							}
						}

						to_lower_case(msgBegin);

						struct s_command_and_reply
						{
							b8 display = true;
							b8 mention_user;
							char* command;
							char* reply;
						};

						constexpr s_command_and_reply commands[] = {
							{
								.display = false,
								.command = "cakez",
							},
							{
								.display = false,
								.command = "commands",
							},
							{
								.display = true,
								.command = "tts",
							},
							// {
							// 	.mention_user = true,
							// 	.command = "discord",
							// 	.reply = "join the discord!",
							// },
						};

						constexpr char* msg_prefix = "PRIVMSG #tkap1 :";

						for(int i = 0; i < array_count(commands); i++)
						{
							if(strstr(msgBegin, format_text("!%s", commands[i].command)))
							{
								s_str_sbuilder<1024> builder;
								builder.append("%s", msg_prefix);
								if(commands[i].mention_user)
								{
									builder.append("%s ", userName);
								}

								switch(i)
								{
									case 0:
									{
										constexpr char* cakez[] = {
											"didsomeonesayClown",
											"PepeClown",
										};
										int r = rand() % 2;
										builder.append("%s", cakez[r]);
									} break;

									case 1:
									{
										for(int command_index = 0; command_index < array_count(commands); command_index++)
										{
											if(commands[command_index].display)
											{
												builder.append("!%s ", commands[command_index].command);
											}
										}
									} break;

									case 2:
									{
										char* tts = NULL;
										char* temp = msgBegin;
										b8 found_space = false;

										while(true)
										{
											if(*temp == 0) { break; }
											else if(*temp == ' ' && !found_space) { found_space = true; }
											else if(*temp != ' ' && found_space)
											{
												tts = temp;
												break;
											}
											temp += 1;
										}

										if(tts)
										{
											// @Fixme(tkap, 28/09/2022):
											// add_tts_message_to_queue(tts);
											handle_tts_request(tts, strlen(tts));

										}

									} break;

									default:
									{
										builder.append("%s", commands[i].reply);
									} break;
								}

								if(builder.len > 0)
								{
									builder.append("\r\n");
									send_message(builder.cstr(), builder.len);
								}

								break;
							}
						}
					}
				}
			}
			else
			{
				CAKEZ_TRACE(0, "Couldn't connect");
			}
		}
		else
		{
			CAKEZ_TRACE(0, "Inavlid Socket");
		}
	}
	else
	{
		CAKEZ_TRACE(0, "Failed getting addressinfo");
	}
}


void send_message(char *msg, int msgLength)
{
	if(msgLength <= 0) { return; }
	send(twitchSocket, msg, msgLength, 0);
}

Message recieve_message(char *buffer)
{
	Message m = {};
	m.buffer = buffer;

	//TODO: twitchSocked coult become NULL I think, maybe reconnect then and log message

	m.length = recv(twitchSocket, buffer, MAX_MSG_LENGTH, 0);
	if (m.length == SOCKET_ERROR)
	{
		int errorCode = WSAGetLastError();

		switch (errorCode)
		{
			case WSANOTINITIALISED:
			{
				assert(0, "");
				break;
			}
			case WSAENETDOWN:
			{
				assert(0, "");
				break;
			}
			case WSAEFAULT:
			{
				assert(0, "");
				break;
			}
			case WSAENOTCONN:
			{
				assert(0, "");
				break;
			}
			case WSAEINTR:
			{
				assert(0, "");
				break;
			}
			case WSAEINPROGRESS:
			{
				assert(0, "");
				break;
			}
			case WSAENETRESET:
			{
				assert(0, "");
				break;
			}
			case WSAENOTSOCK:
			{
				assert(0, "");
				break;
			}
			case WSAEOPNOTSUPP:
			{
				assert(0, "");
				break;
			}
			case WSAESHUTDOWN:
			{
				assert(0, "");
				break;
			}
			case WSAEWOULDBLOCK:
			{
				assert(0, "");
				break;
			}
			case WSAEMSGSIZE:
			{
				assert(0, "");
				break;
			}
			case WSAEINVAL:
			{
				assert(0, "");
				break;
			}
			case WSAECONNABORTED:
			{
				assert(0, "");
				break;
			}
			case WSAETIMEDOUT:
			{
				assert(0, "");
				break;
			}
			case WSAECONNRESET:
			{
				assert(0, "");
				break;
			}
		}
	}

	if (m.length > 0)
	{
		//terminate string
		m.buffer[m.length] = 0;
	}

	return m;
}

void send_recieve_and_print_msg(char *msg)
{
	char buffer[MAX_MSG_LENGTH];
	int i = 0;

	// Copy contents of msg to the buffer
	while (char c = *msg++)
	{
		buffer[i++] = c;
	}

	buffer[i++] = '\r';
	buffer[i++] = '\n';
	buffer[i] = 0;

	send_message(buffer, i);
	Message m = recieve_message(buffer);
	if (m.length > 0)
	{
		CAKEZ_TRACE(m.buffer);
	}
}