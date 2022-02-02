//
// Created by miguimartinezzz on 19/11/21.
//

#include "lista.h"

void createEmptyList(tList* L) {
    L->lastPos = NULLt;
}
bool isEmptyList(tList L){
    return L.lastPos==NULLt;
}
void insertItem(tItem d,tList* L){
    L->lastPos++;
    L->data[L->lastPos] = d;
}

tItem getItem(tPos p, tList L){
    return L.data[p];
}
tPos first(tList L){
    return 0;
}
tPos next(tPos p, tList L){
    if(p==L.lastPos)
        return NULLt;
    else
        return ++p;
}

void deleteAtPosition(tPos p, tList *L){
    tPos i;

    L->lastPos--;
    for(i=p;i <=L->lastPos; i++) /*Se hace el bucle para mover el elemento deseado hasta el final de la lista y tenga una
                                   posicion nula y y asi borrar el elemento*/
        L->data[i]= L->data[i+1];
}
