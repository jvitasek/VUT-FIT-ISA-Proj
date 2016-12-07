//
// Created by Jakub Vitasek (xvitas02) on 13. 11. 2016.
//

#include <cstdio>
#include <cstdlib>
#include <string>
#include "error.h"

using namespace std;

/**
 * Gets the error message, prints it and exits with the specific error code.
 * @param err The error code.
 * @return void
 */
error::error(TError err) {
    string err_msg;

    switch(err) {
        case(ENOP):
            err_msg = "No problem";
            break;
        case(EARG):
            err_msg = "Wrong arguments";
            break;
        case(EHOS):
            err_msg = "Host error";
            break;
        case(ECON):
            err_msg = "Connection error";
            break;
        case(EREC):
            err_msg = "Receive error";
            break;
        case(EAUT):
            err_msg = "Authentication error";
            break;
        case(EAUF):
            err_msg = "Authentication file error";
            break;
        case(ESOC):
            err_msg = "Socket error";
            break;
        case(ESEN):
            err_msg = "Send error";
            break;
        case(EBOX):
            err_msg = "Mailbox error";
            break;
        case(ENUM):
            err_msg = "Number of messages error";
            break;
        case(EFET):
            err_msg = "Email fetching error";
            break;
        case(ESAV):
            err_msg = "Message saving error";
            break;
        case(EIMF):
            err_msg = "Message format error";
            break;
        case(EREG):
            err_msg = "Regular expression error";
            break;
        case(EPAR):
            err_msg = "Parsing error";
            break;
        case(TLS_ECON):
            err_msg = "TLS connection error";
            break;
        case(TLS_ECER):
            err_msg = "TLS certificate error";
            break;
        case(TLS_EDIR):
            err_msg = "TLS certificate directory error";
            break;
        case(TLS_EFIL):
            err_msg = "TLS certificate file error";
            break;
        case(TLS_EREC):
            err_msg = "TLS receive error";
            break;
        case(TLS_ESEN):
            err_msg = "TLS send error";
            break;
        case(TLS_EAUT):
            err_msg = "TLS authentication error";
            break;
        case(TLS_EBOX):
            err_msg = "TLS mailbox select error";
            break;
        case(TLS_EFET):
            err_msg = "TLS fetch error";
            break;
        default:
            err_msg = "Unspecified error";
    }

    fprintf(stderr, "%s\n", err_msg.c_str());
    exit(err);
}
