//
// Created by Jakub Vitásek on 20/11/2016.
//

#include <string>
#include <iostream>
#include <sys/socket.h>
#include <regex>
#include <fstream>

#include "imaps.h"
#include "error.h"

using namespace std;

/**
 * Constructs the imaps object and initalizes the basics of OpenSSL.
 * @return void
 */
imaps::imaps() {
    SSL_library_init();
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    OpenSSL_add_all_algorithms();
}

/**
 * Gets the iterator a1/a2/...
 * @return The prefix string.
 */
string imaps::getIteratorPrefix() {
    return string("a").append(std::to_string(this->cmd_count++)).append(" ");
}

/**
 * Connects to the specified server on the specified port using TLS and certs supplied.
 * @param port The port to use.
 * @param server The server to connect to.
 * @param cert_dir The certificate directory to use.
 * @param cert_file The certificate file to use.
 * @return True on success.
 */
bool imaps::connect(int port, const char *server, string cert_dir, string cert_file) {
    this->port = port;
    this->server = server;
    this->ctx = SSL_CTX_new(SSLv23_client_method());

    if(cert_dir != "") {
        if(!SSL_CTX_load_verify_locations(this->ctx, NULL, cert_dir.c_str())) {
            new error(TLS_EDIR);
        }
    } else if(cert_file != "") {
        if(!SSL_CTX_load_verify_locations(this->ctx, cert_file.c_str(), NULL)) {
            new error(TLS_EFIL);
        }
    } else new error(TLS_ECER);

    this->bio = BIO_new_ssl_connect(this->ctx);
    BIO_get_ssl(this->bio, & (this->ssl));
    SSL_set_mode(this->ssl, SSL_MODE_AUTO_RETRY);

    string hostname = string(server).append(":").append(to_string(port));
    BIO_set_conn_hostname(this->bio, hostname.c_str());
    if(BIO_do_connect(this->bio) <= 0) new error(TLS_ECON);

    return this->matchOk(this->receive(), CONNECT) != string::npos;
}

/**
 * Sends the LOGIN command and receives the server response.
 * @param username
 * @param password
 * @return True on success.
 */
bool imaps::login(string username, string password) {
    string login_string = this->getIteratorPrefix();
    login_string = login_string.append("LOGIN ").append(username).append(" ").append(password).append("\r\n");
    this->send(login_string);
    return this->matchOk(this->receive(), LOGIN) != string::npos;
}

/**
 * Sends the LOGOUT command.
 * @return True on success.
 */
bool imaps::logout() {
    string logout_string = this->getIteratorPrefix();
    logout_string = logout_string.append("LOGOUT\r\n");
    this->send(logout_string);
    SSL_CTX_free(this->ctx);
    BIO_free_all(this->bio);
    return true;
}

/**
 * Sends the SELECT command.
 * @param name The name of the mailbox.
 * @return True on success.
 */
bool imaps::setMailbox(string mailbox) {
    this->mailbox = mailbox;
    string mailbox_string = this->getIteratorPrefix();
    mailbox_string = mailbox_string.append("SELECT ").append(mailbox).append("\r\n");
    this->send(mailbox_string);
    return this->matchOk(this->receiveWholeResponse(SELECT), SELECT) != string::npos;
}

/**
 * Fetches all the messages and saves them into the specified output directory.
 * @param out_dir Output directory.
 * @param headers_only Only capture headers.
 * @param new_only Only capture unseen emails.
 * @return True on success.
 */
unsigned long imaps::fetchAllMessages(string out_dir, bool headers_only, bool new_only) {
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
 * Receive method to handle all receiving through the socket stream.
 * @param size The size of the buffer to receive the data to.
 * @return The received data.
 */
string imaps::receive(int size) {
    char buf[size];
    string reply;
    int x;

    x = BIO_read(this->bio, buf, sizeof(buf));
    if(x == 0) new error(TLS_EREC);

    reply = buf;
    return reply;
}

/**
 * Send method to handle all sending through the socket stream.
 * @param data The data to send.
 * @return True on success.
 */
bool imaps::send(string message) {
    if(BIO_write(this->bio, message.c_str(), (int) message.length()) <= 0) {
        new error(TLS_ESEN);
    }
    return true;
}

/**
 * Matches if the response acknowledges success.
 * @param receive The response string.
 * @param type The type of command.
 * @return The position of the acknowledgment.
 */
size_t imaps::matchOk(string receive, int type) {
    if(type == CONNECT) return receive.find("OK");
    if(type == FETCH) return receive.find("OK Fetch completed");
    if(type == SELECT) return receive.find("Select completed.");
    if(type == LOGIN) return receive.find("Logged in");
    if(type == SEARCH) return receive.find("OK Search completed");
    return 0;
}

/**
 * Receives until the acknowledgment by the server.
 * @param type Type of acknowledgment.
 * @return The full response.
 */
string imaps::receiveWholeResponse(int type) {
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
 * Fetches all the UIDs for the specified flag.
 * @return True on success.
 */
bool imaps::fetchUids() {
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
 * Parses the UID string into a class attribute vector.
 * @param uids The UIDs to parse.
 * @return True on success.
 */
bool imaps::parseUids(std::string uids) {
    string del;
    try {
        regex re("(\\r\\n.*(?=OK (Search|SEACH) completed)[.|\\s\\S]*)");
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
 * Fetches the whole message.
 * @param number The number of the message.
 * @return The whole message in the format specified by the assignment.
 */
string imaps::fetchMessage(int number) {
    string fetch_string;

    fetch_string = this->getIteratorPrefix();
    fetch_string = fetch_string.append("UID FETCH ").append(std::to_string(number)).append(" BODY[HEADER]\r\n");
    this->send(fetch_string);
    return this->parseMessageByFormat(this->receiveWholeResponse(FETCH), number);
}

/**
 * Parses the message and gets rid of IMAP info.
 * @param message The message to parse.
 * @param number The message UID.
 * @return The parsed message.
 */
string imaps::parseMessageByFormat(std::string message, int number) {
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
 * Fetches the body of the message.
 * @param number The number of the message.
 * @return The body of the message in the format specified by the assignment.
 */
string imaps::fetchMessageBody(int number) {
    string message;
    string fetch_string;

    fetch_string = this->getIteratorPrefix();
    fetch_string = fetch_string.append("UID FETCH ").append(std::to_string(number)).append(" BODY[TEXT]\r\n");
    this->send(fetch_string);
    message = this->receiveWholeResponse(FETCH);
    return message;
}

/**
 * Parses the message body to follow the format specified in the assignment.
 * @param message The message to parse.
 * @return The parsed body.
 */
string imaps::parseMessageBody(std::string message) {
    string del;
    try {
        regex re("(.*(?=OK (Fetch|FETCH) completed)[.|\\s\\S]*)");
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
 * Parses the message header and gets rid of IMAP info.
 * @param message The message to parse.
 * @return The parsed string.
 */
std::string imaps::parseMessageHeader(std::string message) {
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

/**
 * Saves the message to a specified filename.
 * @param filename
 * @param message
 * @return True on success.
 */
bool imaps::saveMessageToFile(std::string filename, std::string message) {
    string path = string(this->out_dir).append("/").append(filename);
    std::ofstream file(path);
    file << message;
    return true;
}
