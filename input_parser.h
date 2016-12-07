//
// Created by Jakub Vitasek (xvitas02) on 13. 11. 2016.
//

#ifndef ISA_INPUT_PARSER_H
#define ISA_INPUT_PARSER_H

using namespace std;

class input_parser {
public:
    input_parser(int arg_count, char** arg_vect);
    const char* getServer();
    int getPort();
    bool isSecured();
    string getCert_file();
    string getCert_dir();
    bool isNew_only();
    bool isHeaders_only();
    string getAuth_file();
    string getMailbox();
    string getOut_dir();
    map<string, string> extractAuthData();

private:
    void parse();
    int argc;
    char** argv;
    string server;
    string port; // -p port
    bool secured; // -T
    string cert_file; // -c cert_file
    string cert_dir; //  -C cert_dir
    bool new_only; // -n
    bool headers_only; // -h
    string auth_file; // -a auth_file
    string mailbox; // -b mailbox
    string out_dir; // -o out_dir
};


#endif //ISA_INPUT_PARSER_H
