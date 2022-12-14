#include <Utils.hpp>
#include <Client.hpp>

/**
 * @brief find target from str
 *
 * @param str
 * @param target
 * @return int index
 */
int find(const std::string &str, const std::string &target)
{
	size_t i = 0;
	size_t j = 0;

	while (str[i])
	{
		j = 0;
		if (str[i] == target[0])
		{
			while (str[i] && target[j] && str[i] == target[j])
			{
				i++;
				j++;
			}
			if (j == target.size())
				return i - (target.size() - 1);
		}
		else
			i++;
	}
	return -1;
}

/**
 * @brief
 * @param target (does not include #)
 */
bool findChannel(std::map<std::string, Channel *> channels, const std::string &target)
{
	for (
			std::map<std::string, Channel *>::const_iterator it = channels.begin();
			it != channels.end();
			it++)
	{
		const std::string &channelName = (*it).first;
		if (channelName == target)
			return true;
	}
	return false;
}

bool findChannelForServer(std::map<std::string, Channel> channels, const std::string &target)
{
	for (
			std::map<std::string, Channel>::const_iterator it = channels.begin();
			it != channels.end();
			it++)
	{
		const std::string &channelName = (*it).first;
		if (channelName == target)
			return true;
	}
	return false;
}

/**
 * @brief Function to split channel names by ','
 */
const std::vector<std::string> splitBycomma(const std::string &param)
{
	std::vector<std::string> channels;

	int i = 0;
	while (param[i] && param[i] != '\r' && param[i] != '\n')
	{
		std::string channel;
		for (
				size_t j = i;
				param[j] != ',' && param[j] && param[j] != '\r' && param[j] != '\n';
				j++, i++)
			channel.append(&param[j], 1);
		channels.push_back(channel);
		i++;
	}
	return channels;
}

/**
 * @brief
 */
std::vector<std::string> split(std::string str, std::string del)
{
	uint64_t first = 0;
	uint64_t last = str.find_first_of(del);
	std::vector<std::string> res;
	if (last == std::string::npos)
	{
		res.push_back(str);
		return res;
	}
	while (first < str.size())
	{
		std::string trim_str(str, first, last - first);
		res.push_back(trim_str);
		first = last + 1;
		last = str.find_first_of(del, first);
		if (last == std::string::npos)
		{
			last = str.size();
		}
	}
	return res;
}

/**
 * @brief string is number or not
 */
bool is_number(const std::string &s)
{
	std::string::const_iterator it = s.begin();
	while (it != s.end() && std::isdigit(*it))
		++it;
	return !s.empty() && it == s.end();
}

/**
 * @brief get current time
 */
std::string getTimestamp()
{
	std::time_t timestamp = std::time(0);
	std::string time = std::to_string(timestamp);

	return time;
}

void channelDebug(std::map<std::string, Channel> server, std::map<std::string, Channel *> client)
{
	std::cout << CYN << "------------------------Channel diff------------------------" << RES << std::endl;
	for (
			std::map<std::string, Channel>::iterator it = server.begin();
			it != server.end();
			it++)
	{
		std::cout << "Name: "
							<< "server[" << it->second.getName() << "]  " << std::endl;
		std::cout << "Key: "
							<< "server[" << it->second.getKey() << "]" << std::endl;
		std::cout << "Topic: "
							<< "server[" << it->second.getTopic() << "] " << std::endl;
		const std::vector<std::string> serverOper = it->second.getOper();

		std::cout << "---server channel opers---" << std::endl;
		for (size_t i = 0; i < serverOper.size(); i++)
			std::cout << "[" << serverOper[i] << "] ";
		std::cout << std::endl;
		const std::vector<Client *> serverMember = it->second.getMember();

		std::cout << "---server channel members---" << std::endl;
		for (size_t i = 0; i < serverMember.size(); i++)
			std::cout << "[" << serverMember[i]->getNickname() << "]";
		std::cout << std::endl;
		std::cout << CYN << "------------------------------------------------------------" << RES << std::endl;
	}

	for (
			std::map<std::string, Channel *>::iterator it = client.begin();
			it != client.end();
			it++)
	{
		std::cout << "Name: "
							<< "client [" << it->second->getName() << "]" << std::endl;
		std::cout << "Key: "
							<< "client [" << it->second->getKey() << "]" << std::endl;
		std::cout << "Topic: "
							<< "client [" << it->second->getTopic() << "]" << std::endl;
		const std::vector<std::string> clientOper = it->second->getOper();

		std::cout << "---client channel opers---" << std::endl;
		for (size_t i = 0; i < clientOper.size(); i++)
			std::cout << "[" << clientOper[i] << "] ";
		std::cout << std::endl;

		const std::vector<Client *> clientMember = it->second->getMember();

		std::cout << "---client channel members---" << std::endl;
		for (size_t i = 0; i < clientMember.size(); i++)
			std::cout << "[" << clientMember[i]->getNickname() << "]";
		std::cout << std::endl;

		std::cout << CYN << "------------------------------------------------------------" << RES << std::endl;
	}
}
