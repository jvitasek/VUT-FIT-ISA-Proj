//
// Created by Jakub Vitasek (xvitas02) on 13. 11. 2016.
//

#include <iostream>
#include <map>
#include "input_parser.h"
#include "error.h"
#include "imap.h"
#include "imaps.h"

int main(int argc, char* argv[]) {
    bool succ;
    input_parser parser(argc, argv);
    unsigned long fetched;

    if(parser.isSecured()) {
        imaps imaps;

        /**
         * Connects to the specified server on the specified port using certificates and TLS.
         */
        succ = imaps.connect(parser.getPort(), parser.getServer(), parser.getCert_dir(), parser.getCert_file());
        if(!succ) {
            new error(TLS_ECON);
        }

        /**
         * Gets the authentication data and send the login command.
         */
        map<std::string, std::string> credentials = parser.extractAuthData();
        succ = imaps.login(credentials["username"], credentials["password"]);
        if(!succ) {
            new error(TLS_EAUT);
        }

        /**
         * Sets the user-specified mailbox.
         */
        succ = imaps.setMailbox(parser.getMailbox());
        if(!succ) {
            new error(TLS_EBOX);
        }

        /**
         * Fetches all messages.
         */
        fetched = imaps.fetchAllMessages(parser.getOut_dir(), parser.isHeaders_only(), parser.isNew_only());
        if(fetched <= 0) new error(TLS_EFET);
        else {
            imaps.logout();
            if(parser.isNew_only()) printf("Received %lu new messages", fetched);
            else printf("Received %lu messages", fetched);
        }
    } else {
        imap imap;

        /**
         * Connects to the specified server on the specified port
         */
        succ = imap.connect(parser.getPort(), parser.getServer());
        if(!succ) {
            new error(ECON);
        }

        /**
         * Gets the authentication data and sends the login command.
         */
        map<std::string, std::string> credentials = parser.extractAuthData();
        succ = imap.login(credentials["username"], credentials["password"]);
        if(!succ) {
            new error(EAUT);
        }

        /**
         * Sets the user-specified mailbox.
         */
        succ = imap.setMailbox(parser.getMailbox());
        if(!succ) {
            new error(EBOX);
        }

        /**
         * Fetches all messages.
         */
        fetched = imap.fetchAllMessages(parser.getOut_dir(), parser.isHeaders_only(), parser.isNew_only());
        if(fetched <= 0) {
            new error(EFET);
        } else imap.logout();
    }

    if(parser.isNew_only()) printf("Received %lu new messages", fetched);
    else printf("Received %lu messages", fetched);
    return 0;
}