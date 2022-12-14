#include <Command.hpp>

/**
 * @brief set a connection password
 *
 * parameters <password>
 * <in short> [PASS CMD] check whether server should pass the client(user).
 */

void PASS(Client &client, std::string const &server_password){
	int const &fd = client.getFd();
	std::string const &nick = client.getNickname();

	if (client.getParams().size() != 1)
		sendMessage(fd, ERR_NEEDMOREPARAMS(nick, "PASS"), 0);

	std::string const &password = client.getParams()[0];

	if (password == server_password)
		client.setIsAuth();
}
