#ifndef SERVER_H
#define SERVER_H
#include <memory>
#include <map>
#include <string>
#include <vector>



class Client;
class Server
{
public:
	Server();
	std::shared_ptr<Client> add_client(const std::string& id);
	[[nodiscard]] std::shared_ptr<Client> get_client(std::string id) const;
	double get_wallet(std::string id);
	static bool parse_trx(std::string trx, std::string& sender, std::string& receiver, double& value);
	bool add_pending_trx(std::string trx, std::string signature);
	size_t mine();
private:
	std::map<std::shared_ptr<Client>, double> clients;
    friend void show_wallets(const Server& server);
};

extern std::vector<std::string> pending_trxs;
#endif //SERVER_H