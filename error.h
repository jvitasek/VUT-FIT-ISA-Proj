//
// Created by Jakub Vitasek (xvitas02) on 13. 11. 2016.
//

#ifndef ISA_ERROR_H
#define ISA_ERROR_H

typedef enum TError {
    ENOP = 0,
    EARG = 1,
    EHOS = 2,
    ECON = 3,
    EAUT = 4,
    EAUF = 5,
    EREC = 6,
    ESOC = 7,
    ESEN = 8,
    EBOX = 9,
    ENUM = 10,
    EFET = 11,
    ESAV = 12,
    EIMF = 13,
    EREG = 14,
    EPAR = 15,
    TLS_ECON = 33,
    TLS_ECER = 34,
    TLS_EDIR = 35,
    TLS_EFIL = 36,
    TLS_EREC = 37,
    TLS_ESEN = 38,
    TLS_EAUT = 39,
    TLS_EBOX = 40,
    TLS_EFET = 41
} TError;

class error {
public:
    error(TError);
};

#endif //ISA_ERROR_H
