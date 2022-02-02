//
// Created by miguimartinezzz on 19/11/21.
//

#ifndef P2_LISTA1_H
#define P2_LISTA1_H
#define NULLt -1
#define MAXt 100
#include "stdbool.h"
#include "string.h"
typedef int tPosf;
typedef struct tComandof{
    char *direccion;
    long int size;
    char comando [8192];
    char fecha[8192];
    char nombre[4096];
    int descriptors;
    int key;
    tPosf P;
}tComandof;
typedef tComandof tItemf;
typedef struct tListf{
    tItemf data [MAXt];
    tPosf lastPos;
} tListf;
void createEmptyListf( tListf* L);
void insertItemf(tComandof d, tListf* L);
tPosf firstf(tListf L);
tPosf nextf( tPosf p, tListf L);
void deleteAtPositionf(tPosf p, tListf *L);
tItemf getItemf(tPosf p, tListf L);
bool isEmptyListf(tListf L);
#endif //P2_LISTA1_H
