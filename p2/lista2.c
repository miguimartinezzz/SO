//
// Created by miguimartinezzz on 2/12/21.
//
//
// Created by miguimartinezzz on 19/11/21.
//

#include "lista2.h"

void createEmptyListj(tListj* L) {
    L->lastPos = 0;
}
bool isEmptyListj(tListj L){
    return L.lastPos==0;
}
void insertItemj(tItemj d,tListj* L){

    L->data[L->lastPos] = d;
    L->lastPos++;
}

tItemj getItemj(tPosj p, tListj L){
    return L.data[p];
}
tPosj firstj(tListj L){
    return 0;
}
tPosj nextj(tPosj p, tListj L){
    if(p==L.lastPos)
        return NULLt;
    else
        return ++p;
}

void deleteAtPositionj(tPosj p, tListj *L){
    tPosj i;

    L->lastPos--;
    for(i=p;i <=L->lastPos; i++)
        L->data[i]= L->data[i+1];
}
