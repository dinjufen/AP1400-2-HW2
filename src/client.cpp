#include "client.h"
#include "crypto.h"
#include <utility>
#include "server.h"
#include <random>
#include <iomanip>
#include <sstream>

template <typename T>
std::string floatToStringWithOneDecimal(T number, int fixed) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(fixed) << number;
    return out.str();
}

Client::Client(std::string id, Server& server) : id(std::move(id)), server(&server) {
    crypto::generate_key(public_key, private_key);
}

std::string Client::get_id() {
    return id;
}

const std::string& Client::get_publickey() const {
    return public_key;
}

double Client::get_wallet() {
    return server->get_wallet(id);
}

std::string Client::sign(std::string txt) const {
    return crypto::signMessage(private_key, txt);
}

bool Client::transfer_money(std::string receiver, double value) {
    if (server->get_client(receiver) == nullptr) {
        return false;
    }
    auto trx = id + "-" + receiver + "-" + floatToStringWithOneDecimal(value, 1);
    return server->add_pending_trx(trx, sign(trx));
}

size_t Client::generate_nonce() {
    std::random_device rd;
    std::mt19937 engine(rd());
    std::uniform_int_distribution<size_t> distrib(0, 99);
    auto nonce = distrib(engine);
    return nonce;
}