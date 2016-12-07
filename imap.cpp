//
// Created by Jakub Vitasek (xvitas02) on 13. 11. 2016.
//

#include <string>
#include <iostream>
#include <regex>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <zconf.h>
#include <fstream>
#include <sstream>
#include "imap.h"
#include "error.h"

/**
 * Constructs the imap object and initializes the class attributes.
 * @return void
 */
imap::imap() {
    this->port = 0;
    this->cmd_count = 0;
    this->server = "";
    this->mailbox = "";
    this->out_dir = "";
    this->new_only = false;
    this->headers_only = false;
    this->sockfd = -1;
}


/**
 * Gets the iterator a1/a2/...
 * @return The prefix string.
 */
string imap::getIteratorPrefix() {
    return string("a").append(std::to_string(this->cmd_count++)).append(" ");
}


/**
 * Connects to a specified host on a specified port
 * and keeps the connection open.
 * @param port
 * @param server
 * @return True on success.
 */
bool imap::connect(int port, const char* server) {
    this->port = port;
    this->server = server;

    if(this->sockfd == -1) {
        /**
         * Creating the socket
         */
        this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if(this->sockfd == - 1) new error(ESOC);
    }

    if(inet_addr(server) == -1) {
        struct hostent *hostent;
        struct in_addr **addresses;

        if((hostent = gethostbyname(server)) == NULL) {
            new error(EHOS);
        }

        addresses = (struct in_addr **) hostent->h_addr_list;

        for(int i = 0; addresses[i] != NULL; i++) {
            this->conn.sin_addr = *addresses[i];
            break;
        }
    } else {
        this->conn.sin_addr.s_addr = inet_addr(server);
    }

    this->conn.sin_family = AF_INET;
    this->conn.sin_port = htons((uint16_t) port);

    if(::connect(this->sockfd, (struct sockaddr *) &this->conn, sizeof(this->conn)) < 0) {
        new error(ECON);
    }
    return this->matchOk(this->receive(), CONNECT) != string::npos;
}

/**
 * Sends the LOGIN command and receives the server response.
 * @param username
 * @param password
 * @return True on success.
 */
bool imap::login(string username, string password) {
    string login_string = this->getIteratorPrefix();
    login_string = login_string.append("LOGIN ").append(username).append(" ").append(password).append("\r\n");
    this->send(login_string);
    return this->matchOk(this->receive(), LOGIN) != string::npos;
}

/**
 * Sends the LOGOUT command.
 * @return True on success.
 */
bool imap::logout() {
    string logout_string = this->getIteratorPrefix();
    logout_string = logout_string.append("LOGOUT\r\n");
    this->send(logout_string);
    return false;
}

/**
 * Sends the SELECT command.
 * @param name The name of the mailbox.
 * @return True on success.
 */
bool imap::setMailbox(string name) {
    this->mailbox = name;
    string mailbox_string = this->getIteratorPrefix();
    mailbox_string = mailbox_string.append("SELECT ").append(name).append("\r\n");
    this->send(mailbox_string);
    string reply = this->receiveWholeResponse(SELECT);
    return true;
}

/**
 * Fetches all the messages and saves them into the specified output directory.
 * @param out_dir Output directory.
 * @param headers_only Only capture headers.
 * @param new_only Only capture unseen emails.
 * @return True on success.
 */
unsigned long imap::fetchAllMessages(string out_dir, bool headers_only, bool new_only) {
    this->out_dir = out_dir;
    this->headers_only = headers_only;
    this->new_only = new_only;

    string message_content;
    string filename;

    this->fetchUids();

    for(int i : this->uids) {
        message_content = this->fetchMessage(i);
        filename = std::string("message").append(std::to_string(i)).append(".txt");
        if(!this->saveMessageToFile(filename, message_content)) {
            new error(ESAV);
        }
    }
    return this->uids.size();
}

/**
 * Fetches all the UIDs for the specified flag.
 * @return True on success.
 */
bool imap::fetchUids() {
    string fetch_string;
    string uids;

    fetch_string = this->getIteratorPrefix();
    fetch_string = fetch_string.append("UID SEARCH ");
    if(this->new_only) fetch_string.append("UNSEEN");
    else fetch_string.append("ALL");
    fetch_string.append("\r\n");

    this->send(fetch_string);
    uids = this->receiveWholeResponse(SEARCH);
    return this->parseUids(uids);
}

/**
 * Fetches the whole message.
 * @param number The number of the message.
 * @return The whole message in the format specified by the assignment.
 */
string imap::fetchMessage(int number) {
    string fetch_string;

    fetch_string = this->getIteratorPrefix();
    fetch_string = fetch_string.append("UID FETCH ").append(std::to_string(number)).append(" BODY[HEADER]\r\n");
    this->send(fetch_string);
    return this->parseMessageByFormat(this->receiveWholeResponse(FETCH), number);
}

/**
 * Fetches the body of the message.
 * @param number The number of the message.
 * @return The body of the message in the format specified by the assignment.
 */
string imap::fetchMessageBody(int number) {
    string message;
    string fetch_string;

    fetch_string = this->getIteratorPrefix();
    fetch_string = fetch_string.append("UID FETCH ").append(std::to_string(number)).append(" BODY[TEXT]\r\n");
    this->send(fetch_string);
    message = this->receiveWholeResponse(FETCH);
    return message;
}

/**
 * Saves the message to a specified filename.
 * @param filename
 * @param message
 * @return True on success.
 */
bool imap::saveMessageToFile(string filename, string message) {
    string path = string(this->out_dir).append("/").append(filename);
    std::ofstream file(path);
    file << message;
    return true;
}

/**
 * Parses the UID string into a class attribute vector.
 * @param uids The UIDs to parse.
 * @return True on success.
 */
bool imap::parseUids(string uids) {
    string del;
    try {
        regex re("(\\r\\n.*(?=OK SEARCH completed)[.|\\s\\S]*)");
        smatch match;
        if(regex_search(uids, match, re) && match.size() > 1) {
            del = match.str(1);
        } else {
            del = "";
        }
    } catch(regex_error& e) {
        new error(EPAR);
    }
    uids = uids.substr(0, uids.size() - del.size());

    del = "* SEARCH ";
    size_t found = uids.find(del);
    if(found != string::npos) uids.erase(found, del.length());

    if(uids == "* SEARCH") {
        if(this->new_only) printf("No new messages.");
        else printf("No messages.");
        exit(0);
    }

    istringstream iss(uids);
    for(string token; getline(iss, token, ' '); ) {
        this->uids.push_back(atoi(move(token).c_str()));
    }
    return true;
}

/**
 * Parses the message and gets rid of IMAP info.
 * @param message The message to parse.
 * @param number The message UID.
 * @return The parsed message.
 */
string imap::parseMessageByFormat(string message, int number) {
    string headers, body, result;
    if(!this->headers_only) {
        body = this->fetchMessageBody(number);
        body = this->parseMessageBody(body);
    }

    result = this->parseMessageHeader(message);
    if(!this->headers_only) result = result.append(body);
    return result;
}

/**
 * Parses the message body to follow the format specified in the assignment.
 * @param message The message to parse.
 * @return The parsed body.
 */
string imap::parseMessageBody(string message) {
    string del;
    try {
        regex re("(.*(?=OK FETCH completed)[.|\\s\\S]*)");
        smatch match;
        if(regex_search(message, match, re) && match.size() > 1) {
            del = match.str(1);
        } else {
            del = "";
        }
    } catch(regex_error& e) {
        new error(EREG);
    }

    // delete leftovers
    message = message.substr(0, message.size() - del.size());

    // delete first line
    message.erase(0, message.find("\r\n")+1);

    // delete the leftover parenthesis
    message = message.substr(0, message.size()-5);

    return message;
}

/**
 * Send method to handle all sending through the socket stream.
 * @param data The data to send.
 * @return True on success.
 */
bool imap::send(string data) {
    if(::send(this->sockfd, data.c_str(), data.length(), 0) < 0) {
        new error(ESEN);
    }
    return true;
}

/**
 * Receive method to handle all receiving through the socket stream.
 * @param size The size of the buffer to receive the data to.
 * @return The received data.
 */
string imap::receive(int size) {
    char buf[size];
    string reply;

    if(::recv(this->sockfd, buf, sizeof(buf), 0) < 0) {
        new error(EREC);
    }

    reply = buf;
    return reply;
}

/**
 * Receives until the acknowledgment by the server.
 * @param type Type of acknowledgment.
 * @return The full response.
 */
string imap::receiveWholeResponse(int type) {
    bool ok_ack;
    string message;
    string rec;
    size_t endpos;

    ok_ack = false;
    while(!ok_ack) {
        rec = this->receive();
        endpos = this->matchOk(rec, type);

        if(endpos != string::npos) {
            ok_ack = true;
        }
        message.append(rec);
    }
    return message;
}

/**
 * Matches if the response acknowledges success.
 * @param receive The response string.
 * @param type The type of command.
 * @return The position of the acknowledgment.
 */
size_t imap::matchOk(string receive, int type) {
    if(type == CONNECT) return receive.find("OK");
    if(type == FETCH) return receive.find("OK FETCH completed");
    if(type == SELECT) return receive.find("OK [READ-WRITE] SELECT completed");
    if(type == LOGIN) return receive.find("OK LOGIN Authentication succeeded");
    if(type == SEARCH) return receive.find("OK SEARCH completed");
    return 0;
}

/**
 * Parses the message header and gets rid of IMAP info.
 * @param message The message to parse.
 * @return The parsed string.
 */
string imap::parseMessageHeader(string message) {
    string del;
    try {
        regex re("(\\r\\n\\)[.|\\s\\S]*)");
        smatch match;
        if(regex_search(message, match, re) && match.size() > 1) {
            del = match.str(1);
        } else {
            del = "";
        }
    } catch(regex_error& e) {
        new error(EREG);
    }
    message = message.substr(0, message.size() - del.size());
    message.erase(0, message.find("\r\n")+1);
    message.erase(0, message.find("\n")+1);
    return message;
}