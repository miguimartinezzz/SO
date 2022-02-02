#include "lista1.h"

void createEmptyListf(tListf* L) {
    L->lastPos = 0;
}
bool isEmptyListf(tListf L){
 return L.lastPos==0;
 }
void insertItemf(tItemf d,tListf* L){
       
       L->data[L->lastPos] = d;  
         L->lastPos++;
}

tItemf getItemf(tPosf p, tListf L){
    return L.data[p];
}
tPosf firstf(tListf L){
 return 0;
 }
tPosf nextf(tPosf p, tListf L){
    if(p==L.lastPos)
        return NULLt;
    else
        return ++p;
}

void deleteAtPositionf(tPosf p, tListf *L){
    tPosf i;

    L->lastPos--;
    for(i=p;i <=L->lastPos; i++) 
        L->data[i]= L->data[i+1];
}
