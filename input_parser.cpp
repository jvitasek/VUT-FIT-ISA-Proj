//
// Created by Jakub Vitasek (xvitas02) on 13. 11. 2016.
//

#include <string>
#include <stdlib.h>
#include <getopt.h>
#include <fstream>
#include <regex>
#include <map>
#include "input_parser.h"
#include "error.h"

using namespace std;

/**
 * Constructs the parser object and initalizes the default values.
 * @param arg_count Argument count to inject.
 * @param arg_vect Argument vector to inject.
 * @return void
 */
input_parser::input_parser(int arg_count, char **arg_vect) {
    this->argc = 0;
    this->server = "";
    this->port = "143";
    this->secured = false;
    this->cert_file = "";
    this->cert_dir = "/etc/ssl/certs";
    this->new_only = false;
    this->headers_only = false;
    this->auth_file = "";
    this->mailbox = "INBOX";
    this->out_dir = "";

    this->argc = arg_count;
    this->argv = arg_vect;
    this->parse();
}

/**
 * The basic parse functionality using getopt.
 */
void input_parser::parse() {
    if(this->argc < 6 || this->argc > 16) new error(EARG);
    this->server = this->argv[1];
    int c = 0;
    while((c = getopt(this->argc, this->argv, "p:c:C:a:b:o:Tnh") != -1)) {
        switch(c) {
            case 'p':
                this->port = optarg;
                break;
            case 'c':
                this->cert_file = optarg;
                break;
            case 'C':
                this->cert_dir = optarg;
                break;
            case 'a':
                this->auth_file = optarg;
                break;
            case 'b':
                this->mailbox = optarg;
                break;
            case 'o':
                this->out_dir = optarg;
                break;
            case 'T':
                this->secured = true;
                break;
            case 'n':
                this->new_only = true;
                break;
            case 'h':
                this->headers_only = true;
                break;
            case '?':
            default:
                new error(EARG);
        }
    }
}

/**
 * Getter function for the server.
 * @return The server.
 */
const char* input_parser::getServer() {
    return this->server.c_str();
}

/**
 * Getter function for the port.
 * @return The port.
 */
int input_parser::getPort() {
    return atoi(this->port.c_str());
}

/**
 * Getter function for secure connection.
 * @return True if secure.
 */
bool input_parser::isSecured() {
    return this->secured;
}

/**
 * Getter function for the certificate file.
 * @return The certificate file.
 */
string input_parser::getCert_file() {
    return this->cert_file;
}

/**
 * Getter function for the certificate directory.
 * @return The certificate directory.
 */
string input_parser::getCert_dir() {
return this->cert_dir;
}

/**
 * Getter function for only new messages.
 * @return True if only new messages should be fetched.
 */
bool input_parser::isNew_only() {
    return this->new_only;
}

/**
 * Getter function for only headers.
 * @return True if only headers should be fetched.
 */
bool input_parser::isHeaders_only() {
    return this->headers_only;
}

/**
 * Getter function for the authentification file.
 * @return The authentification file.
 */
string input_parser::getAuth_file() {
    return this->auth_file;
}

/**
 * Getter function for the mailbox name.
 * @return The mailbox name.
 */
string input_parser::getMailbox() {
    return this->mailbox;
}

/**
 * Getter function for the output directory.
 * @return The output directory.
 */
string input_parser::getOut_dir() {
    return this->out_dir;
}

/**
 * Extracts credentials out of the authentication file.
 * @return The map with the credentials.
 */
map<string, string> input_parser::extractAuthData() {
    ifstream in(this->getAuth_file());
    string contents((istreambuf_iterator<char>(in)),
                    istreambuf_iterator<char>());
    map<string, string> credentials;
    try {
        regex re("username\\s\\=\\s(.*)\\npassword\\s\\=\\s(.*)");
        smatch match;
        if(regex_search(contents, match, re) && match.size() > 1) {
            credentials["username"] = match.str(1);
            credentials["password"] = match.str(2);
        } else {
            new error(EAUF);
        }
    } catch(regex_error& e) {
        new error(EAUF);
    }
    return credentials;
}