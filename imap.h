
#ifndef ISA_IMAP_H
#define ISA_IMAP_H

#include <netinet/in.h>
#include <vector>

#define CONNECT 0
#define LOGIN 1
#define SELECT 2
#define FETCH 3
#define SEARCH 4

using namespace std;

class imap {

public:
    imap();
    string getIteratorPrefix();
    bool saveMessageToFile(string filename, string message);
    bool setMailbox(string name);
    bool login(string username, string password);
    bool fetchUids();
    string fetchMessage(int number);
    unsigned long fetchAllMessages(string out_dir, bool headers_only, bool new_only);
    size_t matchOk(string receive, int type);
    bool parseUids(string uids);
    string parseMessageByFormat(string message, int number);
    string parseMessageBody(string message);
    string fetchMessageBody(int number);
    bool connect(int port, const char *server);
    bool send(string data);
    string receive(int size = 2048);
    string receiveWholeResponse(int type);
    bool logout();
    string parseMessageHeader(string message);

private:
    int port;
    int cmd_count;
    const char* server;
    string mailbox;
    string out_dir;
    bool new_only;
    bool headers_only;
    int sockfd;
    struct sockaddr_in conn;
    vector<int> uids;
};

#endif //ISA_IMAP_H
