//
// Created by Anton on 14.11.2022.
//

#ifndef P5_INTERPRETER_P5_ERRORS_H
#define P5_INTERPRETER_P5_ERRORS_H
#include <cstdlib>
#include "iostream"
//TODO: throw exception here
#define P5_ERR(...) printf(__VA_ARGS__); exit(1);

#endif //P5_INTERPRETER_P5_ERRORS_H
