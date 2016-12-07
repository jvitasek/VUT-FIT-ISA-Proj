//
// Created by Jakub Vitásek on 20/11/2016.
//

#ifndef ISA_IMAPS_H
#define ISA_IMAPS_H

#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "openssl/bio.h"
#include "openssl/ssl.h"
#include "openssl/err.h"

#define CONNECT 0
#define LOGIN 1
#define SELECT 2
#define FETCH 3
#define SEARCH 4

class imaps {
public:
    imaps();
    bool connect(int port, const char* server, std::string cert_dir, std::string cert_file);
    bool login(std::string username, std::string password);
    bool setMailbox(std::string mailbox);
    unsigned long fetchAllMessages(std::string out_dir, bool headers_only, bool new_only);
    bool logout();

private:
    std::string mailbox;
    struct sockaddr_in conn;
    BIO* bio;
    int sockfd;
    SSL* ssl;
    SSL_CTX* ctx;
    int port;
    const char* server;
    int cmd_count;
    std::string out_dir;
    bool headers_only;
    bool new_only;
    std::vector<int> uids;
    std::string receive(int size = 2048);
    bool send(std::string message);
    size_t matchOk(std::string message, int type);
    std::string getIteratorPrefix();
    std::string receiveWholeResponse(int type);
    bool fetchUids();
    bool parseUids(std::string uids);
    std::string fetchMessage(int number);
    std::string parseMessageByFormat(std::string message, int number);
    std::string fetchMessageBody(int number);
    std::string parseMessageBody(std::string message);
    std::string parseMessageHeader(std::string message);
    bool saveMessageToFile(std::string filename, std::string message);
};


#endif //ISA_IMAPS_H
