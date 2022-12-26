#include <Command.hpp>

/**
 * @brief set a connection password
 *
 * parameters <password>
 * <in short> [PASS CMD] check whether server should pass the client(user).
 */
//#issue 10

//#define ERR_ALREADYREGISTRED(nick)	":ft_irc 462 " + nick + " :You may not register the password same as other person!\n"
#define ERR_PASSWDMISMATCH(nick) ":ft_irc 464 " + nick + " :Password incorrect\n"


//bool is_already_used_password(std::string const &password, Server) {
// passwordが既に使用済みのものかどうかを判定する関数をつくる
//↓下の関数もその処理が抜けてる
//}

void PASS(Client &client, std::string const &server_password)
{
	int const &fd = client.getFd();
	std::string const &nick = client.getNickname();
	std::string const &password = client.getParams()[0];


	//defaultでshould_be_capをfalseにしておいて、
	//Errorでない時に、trueにする仕様にした
	client.should_be_cap_pass = false;
	/******* Error handling ********/
	if (client.getParams().size() != 1) {
		sendMessage(fd, ERR_NEEDMOREPARAMS(nick, "PASS"), 0);
	} else if (password != server_password) {
		sendMessage(fd, ERR_PASSWDMISMATCH(nick), 0);
	/*********************************/
	} else {
		client.should_be_cap_pass = true;
		client.setIsAuth(true);
		std::cout << "[PASS Success!]" << " ";
		sendMessage(fd, RPL_NONE((std::string)"Authenticated ..."), 0);
	}
}
