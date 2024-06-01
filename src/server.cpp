#include "server.h"
#include <random>
#include <string>
#include <regex>
#include <memory>

#include "crypto.h"
#include "client.h"

std::vector<std::string> pending_trxs = {};
static std::string generateRandom() {
    std::random_device rd;
    std::mt19937 engine(rd());
    std::uniform_int_distribution<> distrib(0, 9999);
    int random_number = distrib(engine);
    std::string str_random = std::to_string(random_number);
    int base = 1000;
    while (base > random_number) {
        str_random = "0" + str_random;
        base = base / 10;
    }
    return str_random;
}

void show_wallets(const Server& server) {
    std::cout << std::string(20, '*') << std::endl;
    for(const auto& client: server.clients)
        std::cout << client.first->get_id() <<  " : "  << client.second << std::endl;
    std::cout << std::string(20, '*') << std::endl;
}

Server::Server() {}

std::shared_ptr<Client> Server::add_client(const std::string& id) {
    auto new_id = id;
    if (get_client(id) != nullptr) {
        new_id += generateRandom();
    }
    auto client = std::make_shared<Client>(new_id, *this);
    clients.insert({client, 5.0});
    return client;
}

std::shared_ptr<Client> Server::get_client(std::string id) const {
    for (const auto& pair : clients) {
        if (pair.first->get_id() == id) {
            return pair.first;
        }
    }
    return nullptr;
}

double Server::get_wallet(std::string id) {
    auto client = get_client(id);
    if (client) {
        return clients[client];
    }
    return 0;
}

bool Server::parse_trx(std::string trx, std::string& sender, std::string& receiver, double& value) {
    std::regex pattern(R"(([a-zA-Z]+)-([a-zA-Z]+)-([\d.]+))");
    std::smatch matches;
    if (!std::regex_match(trx, matches, pattern)) {
        throw std::runtime_error("parse trx error");
    }
    sender = matches[1];
    receiver = matches[2];
    value = std::stod(matches[3]);
    return true;
}

bool Server::add_pending_trx(std::string trx, std::string signature) {
    std::string sender;
    std::string receiver;
    double value;
    if (!parse_trx(trx, sender, receiver, value)) {
        return false;
    }

    auto pSender = get_client(sender);
    if (!pSender) {
        return false;
    }
    auto pReceiver = get_client(receiver);
    if (!pReceiver) {
        return false;
    }
    double sender_current_value = clients[pSender];
    if (sender_current_value < value) {
        return false;
    }
    bool bVerified = crypto::verifySignature(pSender->get_publickey(), trx, std::move(signature));
    if (!bVerified) {
        return false;
    }
    pending_trxs.push_back(trx);
    return true;
}

size_t Server::mine() {
    size_t count = 0;
    std::string mempool{};
    for(const auto& trx : pending_trxs)
        mempool += trx;
    bool mined = false;
    for (auto& item : clients) {
        auto nouce = item.first->generate_nonce();
        count += nouce;
        const auto& mempool_nonce = mempool + std::to_string(nouce);
        auto sha256 = crypto::sha256(mempool_nonce);
        if (sha256.substr(0, 10).find("000") != std::string::npos) {
            item.second += 6.25;
            mined = true;
            break;
        }
    }
    if (mined) {
        for(const auto& trx : pending_trxs) {
            std::string sender{};
            std::string receiver{};
            double value = 0;
            parse_trx(trx, sender, receiver, value);
            auto pSender = get_client(sender);
            auto pReceiver = get_client(receiver);
            if (!pSender || !pSender) {
                continue;
            }
            if (clients[pSender] < value) {
                continue;
            }
            clients[pSender] -= value;
            clients[pReceiver] += value;
        }
    }
    return count;
}