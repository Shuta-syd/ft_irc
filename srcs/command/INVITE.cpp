#include <Command.hpp>
bool validateMessage(Client &client, const std::vector<std::string> &params, std::map<std::string, Channel> channels, std::map<std::string, int> nick_to_fd);

/**
 * @brief invite a user to a channel
 * INVITE <nickname> <channel>
 */
void INVITE(Client &client, std::map<std::string, int> nick_to_fd, std::map<std::string, Channel> &channels, std::map<int, Client> &users)
{
	const int fd = client.getFd();
	const std::vector<std::string> &params = client.getParams();
	const std::string &nick = client.getNickname();

	if (validateMessage(client, params, channels, nick_to_fd) == false)
		return;

	const int targetFd = nick_to_fd[params[0]];
	Client &targetClient = users[targetFd];
	const std::string channelName = &params[1][1];

	sendMessage(fd, RPL_INVITING(nick, params[0], channelName), 0);
	sendMessage(targetFd, INVITE_MESSAGE(nick, client.getUsername(), client.getHostname(), params[0], channelName), 0);
	targetClient.addInvited(channelName);
}

bool validateMessage(Client &client, const std::vector<std::string> &params, std::map<std::string, Channel> channels, std::map<std::string, int> nick_to_fd)
{
	const std::string nick = client.getNickname();

	if (params.size() != 2)
	{
		sendMessage(client.getFd(), ERR_NEEDMOREPARAMS(nick, "INVITE"), 0);
		return false;
	}
	else if (findChannelForServer(channels, &params[1][1]) == false)
	{
		sendMessage(client.getFd(), ERR_NOSUCHCHANNEL(nick, &params[1][1]), 0);
		return false;
	}

	Channel &channel = channels[&params[1][1]];
	if (nick_to_fd[params[0]] == 0)
	{
		sendMessage(client.getFd(), ERR_NOSUCHNICK(nick, params[0]), 0);
		return false;
	}
	else if (channel.is_inChannel(nick) == false)
	{
		sendMessage(client.getFd(), ERR_NOTONCHANNEL(nick, &params[1][1]), 0);
		return false;
	}
	else if (channel.is_operator(nick) == false)
	{
		std::string channelName = &params[1][1];
		sendMessage(client.getFd(), ERR_CHANOPRIVSNEEDED(channelName), 0);
		return false;
	}
	else if (channel.is_inChannel(params[0]) == true)
	{
		sendMessage(client.getFd(), ERR_USERONCHANNEL(nick, params[0], &params[0][1]), 0);
		return false;
	} else {
		return true;
	}
}
