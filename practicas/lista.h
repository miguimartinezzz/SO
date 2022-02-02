
#ifndef P1_LISTA_H
#define P1_LISTA_H
#define NULLt -1
#define MAX 1000//4096
#include "stdbool.h"
#include "string.h"
typedef int tPos;
typedef struct tComando{
    char comando [4096];
    tPos P;
}tComando;
typedef tComando tItem;
typedef struct tList{
    tItem data [MAX];
    tPos lastPos;
} tList;
void createEmptyList( tList* L);
void insertItem(tComando d, tList* L);
tPos first(tList L);
tPos next( tPos p, tList L);
void deleteAtPosition(tPos p, tList *L);
tItem getItem(tPos p, tList L);
bool isEmptyList(tList L);
#endif //P2_LISTA_H
