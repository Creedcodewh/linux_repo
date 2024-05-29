#include "head.h"
CmdType getCommandType(const char *cmd) {
    if (strcmp(cmd, "register1") == 0) {
        return REGISTERONE;
    } else if (strcmp(cmd, "register2") == 0) {
        return REGISTERTWO;
    } else if (strcmp(cmd, "login1") == 0) {
        return LOGINONE;
    } else if (strcmp(cmd, "login2") == 0) {
        return LOGINTWO;
    } else if (strcmp(cmd, "ls") == 0) {
        return LS;
    } else if (strcmp(cmd, "cd") == 0) {
        return CD;
    } else if (strcmp(cmd, "pwd") == 0) {
        return PWD;
    } else if (strcmp(cmd, "rm") == 0) {
        return REMOVE;
    } else if (strcmp(cmd, "mkdir") == 0) {
        return MKDIR;
    } else if (strcmp(cmd, "gets") == 0) {
        return GETS;
    } else if (strcmp(cmd, "puts") == 0) {
        return PUTS;
    } else if (strcmp(cmd, "token") == 0) {
        return PUTSTOKEN;
    } else {
        return INVALID;
    }
}

