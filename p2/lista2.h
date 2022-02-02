//
// Created by miguimartinezzz on 2/12/21.
//

#ifndef P2_LISTA2_H
#define P2_LISTA2_H
#define NULLt -1
#define MAXt 100
#include "stdbool.h"
#include "string.h"
typedef int tPosj;
typedef struct tComandoj{
    char comando[1000];
    int pid;
    int priority;
    char user [8192];
    char fecha[8192];
    int state;
    char termination[4096];
    tPosj P;
}tComandoj;
typedef tComandoj tItemj;
typedef struct tListj{
    tItemj data [MAXt];
    tPosj lastPos;
} tListj;
void createEmptyListj( tListj* L);
void insertItemj(tComandoj d, tListj* L);
tPosj firstj(tListj L);
tPosj nextj( tPosj p, tListj L);
void deleteAtPositionj(tPosj p, tListj *L);
tItemj getItemj(tPosj p, tListj L);
bool isEmptyListj(tListj L);

#endif //P2_LISTA2_H
