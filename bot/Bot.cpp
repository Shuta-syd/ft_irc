#include <Bot.hpp>

Bot::Bot() {}

Bot::Bot(int port, std::string password, std::string ch): isAuth_(false), port_(port), password_(password), nick_("Mr.Bot"), channelName_(ch), auth_counter_(1), isVoted_(false), vote_yes_(0), vote_no_(0) {}

Bot::~Bot(){}

/**
 * @brief bot start func
 */
void Bot::start() {
	this->setupBotSocket();
	std::cout << BLU << "-----------CONNECTED-----------" << RES << std::endl;
	std::cout << "\e[01mYou can see help `bot help` in irssi\e[m" << std::endl;

	sendMessage(bot_sd_, CAP_MESSAGE, 0);
	while (1)	{
		receive();
		execute();
		sleep(2);
	}
}
/**
 * @brief  recv message from irc server
 */
void Bot::receive() {
	char message[MSG_MAX] = {0};

	if (auth_counter_ == 3 && isAuth_ == false) {
		std::cerr << RED << "[ERR] Password Incorrect" << RES << std::endl;
		sendMessage(bot_sd_, "QUIT :leaving\r\n", 0);
	}

	// std::cout << BLU << "--------Waiting Message--------" << RES << std::endl;
	ssize_t bytes = recv(bot_sd_, message, sizeof(message), 0);
	if (bytes < 0)
		throw std::exception();

	// std::cout << "--------RECEIVE Message--------" << std::endl;
	// std::cout << "client : " << message << std::endl;
	// std::cout << "-------------------------------" << std::endl;

	int i = 0;
	int j = 1;
	while (message[i] != ' ' && message[i] != '\r' && message[i] != '\n') {
		if (message[i] == ':')
			while (message[j] != '!' && message[j])
				sender_.push_back(message[j++]);
		i++;
	}
	while (message[i] == ' ' && message[i] != '\r' && message[i] != '\n')
	i++;
	while (message[i] != '\r' && message[i] != '\n') {
		message_.push_back(message[i]);
		i++;
	}
	// std::cout << "ParsedMessage: [" << message_ << "]" << std::endl;
	auth_counter_ += 1;
}

/**
 * @brief
 */
void Bot::execute() {

	if (find(message_, "CAP") == 1)
		authorize();
	else if (find(message_, "NICK") == 1){
		if (isAuth_ == false) {
			sendMessage(bot_sd_, "CAP END\r\n", 0);
			isAuth_ = true;
		}
		sendMessage(bot_sd_, "JOIN #" + channelName_ + "\r\n", 0);
	}
	else if (find(message_, "PRIVMSG") == 1) {
		this->parseMessage();
		if (find(parsedMessage_, "VOTE") == 1 || find(parsedMessage_, "vote") == 1)
			this->vote(&parsedMessage_[4]);
		if (find(parsedMessage_, "NG") == 1 || find(parsedMessage_, "ng") == 1)
			this->addNG_Keyword(&parsedMessage_[2]);
		else if (
			find(parsedMessage_, "BOT help") == 1 || find(parsedMessage_, "Bot help") == 1 ||
			find(parsedMessage_, "BOT HELP") == 1|| find(parsedMessage_, "bot help") == 1
			) {
			this->usage();
		}
			else checkNG_keyword();
	}
	sender_.clear();
	message_.clear();
	parsedMessage_.clear();
}

void Bot::usage() {
	sendMessage(bot_sd_, PRIVMSG(channelName_, "\e[01m1. vote: Function to vote YES or NO on your favorite theme\e[m"), 0);
	sendMessage(bot_sd_, PRIVMSG(channelName_, "\e[01m2. NG  : You can register NG words on this channel, and people who say NG words will be kicked from the channel.\e[m"), 0);
	sendMessage(bot_sd_, PRIVMSG(channelName_, ""), 0);
	sendMessage(bot_sd_, PRIVMSG(channelName_, "vote [topic]: You can begin voting"), 0);
	sendMessage(bot_sd_, PRIVMSG(channelName_, "vote yes: You can vote YES"), 0);
	sendMessage(bot_sd_, PRIVMSG(channelName_, "vote no: You can vote NO\n"), 0);
	sendMessage(bot_sd_, PRIVMSG(channelName_, "vote end: You can end voting\n"), 0);
	sendMessage(bot_sd_, PRIVMSG(channelName_, "ng [word]: You can register NG word"), 0);
	sendMessage(bot_sd_, PRIVMSG(channelName_, "ng del [word]: You can delete NG word"), 0);
}

void Bot::vote(std::string message) {
	int i = 0;
	if (isVoted_) {
		this->voteConter(message);
		return;
	}

	std::cout << "Vote Feature Started" << std::endl;
	std::cout << message << std::endl;
	while ((message[i] == ' ' || message[i] == ':') && message[i] != '\r' && message[i] != '\n')
		i++;

	while (message[i] && message[i] != '\r' && message[i] != '\n') {
		topic_.push_back(message[i]);
		i++;
	}

	sendMessage(bot_sd_, VOTE_WELCOME(channelName_), 0);
	sendMessage(bot_sd_, PRIVMSG(channelName_, "\033[32m[  " + topic_ + "  ]\033[m"), 0);
	sendMessage(bot_sd_, PRIVMSG(channelName_, "------------------------"), 0);
	sendMessage(bot_sd_, PRIVMSG(channelName_, "|           |          |"), 0);
	sendMessage(bot_sd_, PRIVMSG(channelName_, "|  YES [0]  |  NO [0]  |"), 0);
	sendMessage(bot_sd_, PRIVMSG(channelName_, "|           |          |"), 0);
	sendMessage(bot_sd_, PRIVMSG(channelName_, "------------------------"), 0);

	isVoted_ = true;
}


void Bot::voteConter(std::string message) {
	int i = 0;
	std::string yes_or_no;

	while ((message[i] == ' ' || message[i] == ':') && message[i] != '\r' && message[i] != '\n')
		i++;

	while (message[i] && message[i] != ' ' && message[i] != '\r' && message[i] != '\n') {
		yes_or_no.push_back(message[i]);
		i++;
	}

	if ((yes_or_no == "YES" || yes_or_no == "yes" )&& this->isVoted() == false) {
		vote_yes_ += 1;
		votedNicks_.push_back(sender_);
	}
	else if ((yes_or_no == "NO" || yes_or_no == "no") && this->isVoted() == false) {
		vote_no_ += 1;
		votedNicks_.push_back(sender_);
	}
	else if (yes_or_no == "END" || yes_or_no == "end") {
		this->voteEnd();
		return ;
	}
	else if (this->isVoted() && (yes_or_no == "YES" || yes_or_no == "NO"|| yes_or_no == "yes"  || yes_or_no == "no")) {
		sendMessage(bot_sd_, PRIVMSG(channelName_, "\033[31m" + sender_ + " already voted" + "\033[m"), 0);
		return;
	}

	sendMessage(bot_sd_, PRIVMSG(channelName_, "\033[32m[  " + topic_ + "  ]\033[m"), 0);
	sendMessage(bot_sd_, PRIVMSG(channelName_, "------------------------"), 0);
	sendMessage(bot_sd_, PRIVMSG(channelName_, "|           |          |"), 0);
	sendMessage(bot_sd_, PRIVMSG(channelName_, "|  YES [" + std::to_string(vote_yes_) + "]  |  NO [" + std::to_string(vote_no_) + "]  |"), 0);
	sendMessage(bot_sd_, PRIVMSG(channelName_, "|           |          |"), 0);
	sendMessage(bot_sd_, PRIVMSG(channelName_, "------------------------"), 0);
}

void Bot::voteEnd() {
	sendMessage(bot_sd_, VOTE_END(channelName_), 0);
	sendMessage(bot_sd_, PRIVMSG(channelName_, "------------------------"), 0);
	sendMessage(bot_sd_, PRIVMSG(channelName_, "|           |          |"), 0);
	sendMessage(bot_sd_, PRIVMSG(channelName_, "|  YES [" + std::to_string(vote_yes_) + "]  |  NO [" + std::to_string(vote_no_) + "]  |"), 0);
	sendMessage(bot_sd_, PRIVMSG(channelName_, "|           |          |"), 0);
	sendMessage(bot_sd_, PRIVMSG(channelName_, "------------------------"), 0);

	isVoted_ = false;
	votedNicks_.clear();
	vote_yes_ = 0;
	vote_no_ = 0;
}

bool Bot::isVoted() {
	for (
		std::vector<std::string>::iterator it = votedNicks_.begin();
		it != votedNicks_.end();
		it++
		) {
		if (*it == sender_)
			return true;
	}
	return false;
}

void Bot::addNG_Keyword(std::string message) {
	int i = 0;
	std::string keyword;

	while ((message[i] == ' ' || message[i] == ':') && message[i] != '\r' && message[i] != '\n')
		i++;

	while (message[i] && message[i] != '\r' && message[i] != '\n') {
		keyword.push_back(message[i]);
		i++;
	}

	if (find(keyword, "DEL") == 1 || find(keyword, "del") == 1) {
		std::string word = &keyword[4];

		for (
				std::vector<std::string>::iterator it = ng_words_.begin();
				it != ng_words_.end();
				it++
				) {
			if (*it == word) {
				ng_words_.erase(it);
				sendMessage(bot_sd_, NG_DELETED(channelName_, word), 0);
				return;
			}
		}
		sendMessage(bot_sd_, NG_NOT_DELETE(channelName_), 0);
		return;
	}

	sendMessage(bot_sd_, NG_REGISTERED(channelName_, keyword), 0);
	ng_words_.push_back(keyword);
}

void Bot::checkNG_keyword() {
	bool isNG = false;

	for (
		std::vector<std::string>::iterator it = ng_words_.begin();
		it != ng_words_.end();
		it++
		) {
		if (find(parsedMessage_, *it) != -1)
			isNG = true;
	}

	if (isNG)
		sendMessage(bot_sd_, KICK_MESSAGE(channelName_, sender_), 0);
}

void Bot::parseMessage() {
	int i = 0;

	while (message_[i] != ':' && message_[i] != '\r' && message_[i] != '\n')
		i++;

	i += 1;

	while (message_[i] && message_[i] != '\r' && message_[i] != '\n') {
		parsedMessage_.push_back(message_[i]);
		i++;
	}
}

/**
 * @brief Pass server authentication
 */
void Bot::authorize() {
	sendMessage(bot_sd_, PASS_MESSAGE(password_), 0);
	sendMessage(bot_sd_, NICK_MESSAGE(nick_), 0);
	sendMessage(bot_sd_, USER_MESSAGE(nick_, "*", "*", nick_), 0);
}

/**
 * @brief set up a bot socket
 */
void Bot::setupBotSocket() {
	bot_sd_ = socket(AF_INET, SOCK_STREAM, 0);
	if (bot_sd_ < 0)
		throw std::exception();

	struct sockaddr_in addr;
	bzero(&addr, sizeof(struct sockaddr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(port_);

	int ret = connect(bot_sd_, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) {
		close(bot_sd_);
		throw std::exception();
	}
}
