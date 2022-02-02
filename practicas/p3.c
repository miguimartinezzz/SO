#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/resource.h>
#include <time.h>
#include <sys/utsname.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <dirent.h>
#include "lista.h"
#include "lista1.h"
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#define MAXTROZOS 1000
tList L;
tListf memL;
tItem comando;
struct opt{
    int acc;
    int link;
    int longg;
    int hid;
    int reca;
    int recb;
};
char *direccion;
int descriptors;
int key=0;
int exito=0;
int size;
int DoMalloc=0,DoMmap=0,DoShared=0;
int jListfich,kListfich=1;
int jListdir,kListdir=1;
struct opt optfich;
struct opt optdir;
char linea[10000];
char *trozos[MAXTROZOS];
int numtrozos;
int procesarEntrada();
int l=0,r=0;
int TrocearCadena(char * cadena, char * trozos[]){
    int i=1;
    if ((trozos[0]=strtok(cadena," \n\t"))==NULL)
        return 0;
    while ((trozos[i]=strtok(NULL," \n\t"))!=NULL) //trocea el array(cuenta los espacios)
        i++;
    return i;
}
int exists(const char *fname)
{
    FILE *file;
    if ((file = fopen(fname, "r")))
    {
        fclose(file);
        return 1;
    }
    return 0;
}
/*********************************************************/
/*********************************************************/
void * ObtenerMemoriaShmget (key_t clave, size_t tam)
{ /*Obtienen un puntero a una zaona de memoria compartida*/
/*si tam >0 intenta crearla y si tam==0 asume que existe*/
    void * p;
    int aux,id,flags=0777;
    struct shmid_ds s;
    if (tam) /*si tam no es 0 la crea en modo exclusivo
esta funcion vale para shared y shared -create*/
        flags=flags | IPC_CREAT | IPC_EXCL;
/*si tam es 0 intenta acceder a una ya creada*/
    if (clave==IPC_PRIVATE) /*no nos vale*/
    {errno=EINVAL; return NULL;}
    if ((id=shmget(clave, tam, flags))==-1)
        return (NULL);
    if ((p=shmat(id,NULL,0))==(void*) -1){
        aux=errno; /*si se ha creado y no se puede mapear*/
        if (tam) /*se borra */
            shmctl(id,IPC_RMID,NULL);
        errno=aux;
        return (NULL);
    }
    shmctl (id,IPC_STAT,&s);
/* Guardar En Direcciones de Memoria Shared (p, s.shm_segsz, clave.....);*/
    return (p);
}
void SharedCreate (char *arg[]) /*arg[0] is the key
and arg[1] is the size*/
{
    key_t k;
    size_t tam=0;
    void *p;
    if (arg[0]==NULL || arg[1]==NULL)
    {/*Listar Direcciones de Memoria Shared */ return;}
    k=(key_t) atoi(arg[0]);
    if (arg[1]!=NULL)
        tam=(size_t) atoll(arg[1]);
    size=tam;
    if ((p=ObtenerMemoriaShmget(k,tam))==NULL){

        perror ("Cannot locate : File exists\n");
        exito=1;
    }
    else
        printf ("Allocated shared memory (key %d) at %p\n",k,p);
    direccion=p;
    key=k;
}
/************************************************************************/
/************************************************************************/
void * MmapFichero (char * fichero, int protection)
{
    int df, map=MAP_PRIVATE,modo=O_RDONLY;
    struct stat s;
    void *p;
    if (protection&PROT_WRITE) modo=O_RDWR;
    if (stat(fichero,&s)==-1 || (df=open(fichero, modo))==-1)
        return NULL;
    if ((p=mmap (NULL,s.st_size, protection,map,df,0))==MAP_FAILED)
        return NULL;
/*Guardar Direccion de Mmap (p, s.st_size,fichero,df......);*/
    return p;
}
void Mmap (char *arg[]) /*arg[0] is the file name
and arg[1] is the permissions*/
{
    char *perm;
    void *p;
    int protection=0;
    if (arg[0]==NULL)
    {/*Listar Direcciones de Memoria mmap;*/ return;}
    if ((perm=arg[1])!=NULL && strlen(perm)<4) {
        if (strchr(perm,'r')!=NULL) protection|=PROT_READ;
        if (strchr(perm,'w')!=NULL) protection|=PROT_WRITE;
        if (strchr(perm,'x')!=NULL) protection|=PROT_EXEC;
    }
    if ((p=MmapFichero(arg[0],protection))==NULL){
        exito=1;
        perror ("Imposible mapear archivo");
    }
    else
        printf ("archivo %s mapeado en %p\n", arg[0], p);
    direccion=p;
    descriptors=protection;
}
#define LEERCOMPLETO ((ssize_t)-1)
ssize_t LeerFichero (char *fich, void *p, ssize_t n)
{ /* le n bytes del fichero fich en p */
    ssize_t nleidos,tam=n; /*si n==-1 lee el fichero completo*/
    int df, aux;
    struct stat s;
    if (stat (fich,&s)==-1 || (df=open(fich,O_RDONLY))==-1)
        return ((ssize_t)-1);
    if (n==LEERCOMPLETO)
        tam=(ssize_t) s.st_size;
    if ((nleidos=read(df,p, tam))==-1){
        aux=errno;
        close(df);
        errno=aux;
        return ((ssize_t)-1);
    }
    close (df);
    return (nleidos);
}
/*********************************************************************/
/*********************************************************************/
void SharedDelkey (char *args[]) /*arg[0] points to a str containing the key*/
{
    key_t clave;
    int id;
    char *key=args[0];
    if (key==NULL || (clave=(key_t) strtoul(key,NULL,10))==IPC_PRIVATE){
        printf (" shared -delkey clave_valida\n");
        return;
    }
    if ((id=shmget(clave,0,0666))==-1){
        perror ("shmget: imposible obtener memoria compartida");
        return;
    }
    if (shmctl(id,IPC_RMID,NULL)==-1)
        perror ("shmctl: imposible eliminar memoria compartida\n");
}
void dopmap (void) /*no arguments necessary*/
{
    pid_t pid; /*ejecuta el comando externo pmap para */
    char elpid[32]; /*pasandole el pid del proceso actual */
    char *argv[3]={"pmap",elpid,NULL};
    sprintf (elpid,"%d", (int) getpid());
    if ((pid=fork())==-1){
        perror ("Imposible crear proceso");
        return;
    }
    if (pid==0){
        if (execvp(argv[0],argv)==-1)
            perror("cannot execute pmap");
        exit(1);
    }
    waitpid (pid,NULL,0);
}
void options(struct opt *op,int *k){
    op->longg=0;
    op->link=0;
    op->acc=0;
    op->hid=0;
    op->reca=0;
    op->recb=0;
    int i=1;
    while(trozos[i]!=NULL){
        if(strcmp(trozos[i],"-long")==0){
            op->longg=1;
        }
        else if(strcmp(trozos[i],"-link")==0){
            op->link=1;
        }
        else if(strcmp(trozos[i],"-acc")==0){
            op->acc=1;
        }
        else if(strcmp(trozos[i],"-hid")==0){
            op->hid=1;
        }
        else if(strcmp(trozos[i],"-reca")==0){
            op->reca=1;
        }
        else if(strcmp(trozos[i],"-recb")==0){
            op->recb=1;
        }
        else
            break;
        i++;
    }
    *k=i;
}

char LetraTF (mode_t m)
{
    switch (m&S_IFMT) { /*and bit a bit con los bits de formato,0170000 */
        case S_IFSOCK: return 's'; /*socket */
        case S_IFLNK: return 'l'; /*symbolic link*/
        case S_IFREG: return '-'; /* fichero normal*/
        case S_IFBLK: return 'b'; /*block device*/
        case S_IFDIR: return 'd'; /*directorio */
        case S_IFCHR: return 'c'; /*char device*/
        case S_IFIFO: return 'p'; /*pipe*/
        default: return '?'; /*desconocido, no deberia aparecer*/
    }
}
char * ConvierteModo2 (mode_t m){
    static char permisos[12];

    strcpy (permisos,"---------- ");
    permisos[0]=LetraTF(m);
    if (m&S_IRUSR) permisos[1]='r'; /*propietario*/
    if (m&S_IWUSR) permisos[2]='w';
    if (m&S_IXUSR) permisos[3]='x';
    if (m&S_IRGRP) permisos[4]='r'; /*grupo*/
    if (m&S_IWGRP) permisos[5]='w';
    if (m&S_IXGRP) permisos[6]='x';
    if (m&S_IROTH) permisos[7]='r'; /*resto*/
    if (m&S_IWOTH) permisos[8]='w';
    if (m&S_IXOTH) permisos[9]='x';
    if (m&S_ISUID) permisos[3]='s'; /*setuid, setgid y stickybit*/
    if (m&S_ISGID) permisos[6]='s';
    if (m&S_ISVTX) permisos[9]='t';
    return (permisos);
}
void MostrarEntorno (char **entorno, char * nombre_entorno)
{
    int i=0;
    while (entorno[i]!=NULL) {
        printf ("%p->%s[%d]=(%p) %s\n", &entorno[i],
                nombre_entorno, i,entorno[i],entorno[i]);
        i++;
    }
}
char * NombreUsuario (uid_t uid)
{
    struct passwd *p;
    if ((p=getpwuid(uid))==NULL)
        return (" ??????");
    return p->pw_name;
}
uid_t UidUsuario (char * nombre)
{
    struct passwd *p;
    if ((p=getpwnam (nombre))==NULL)
        return (uid_t) -1;
    return p->pw_uid;
}
void MostrarUidsProceso (void)
{
    uid_t real=getuid(), efec=geteuid();
    printf ("Credencial real: %d, (%s)\n", real, NombreUsuario (real));
    printf ("Credencial efectiva: %d, (%s)\n", efec, NombreUsuario (efec));
}
void CambiarUidLogin (char * login)
{
    uid_t uid;
    if ((uid=UidUsuario(login))==(uid_t) -1){
        printf("login no valido: %s\n", login);
        return;
    }
    if (setuid(uid)==.1)
        printf ("Imposible cambiar credencial: %s\n", strerror(errno));
}
void cmdAutores() {
    int optlogin=1, optnombre=1;
    if (numtrozos>1 && strcmp(trozos[1],"-l")==0) optnombre=0;
    else if (numtrozos>1 && strcmp(trozos[1],"-n")==0) optlogin=0;

    if(optnombre>0)
        printf("Autor1: Miguel Martinez Montans\nAutor2: Eloy Cores Torres\n");
    if(optlogin>0)
        printf("Login1: miguel.mmontans\nLogin2: eloy.cores\n");

}
void cmdFecha(){
    time_t t = time(NULL);
    struct tm *tiempolocal=localtime(&t);
    char fecha [75];
    char hora [75];
    strftime(fecha, 75, "%d/%m/%Y", tiempolocal);
    strftime(hora, 75, "%H:%M:%S", tiempolocal);
    // tambien se podria todo junto strftime(fechayhora, 100, "%d/%m/%Y %H:%M:%S", tiempolocal);
    if (numtrozos>1 && strcmp(trozos[1],"-d")==0){
        printf ("Today is: %s\n",fecha);
        //otra opcion
        //printf ("Hoy es: %02d/%02d/%d\n", tiempolocal->tm_mday, tiempolocal->tm_mon, 1900+tiempolocal->tm_year);
    }
    else if (numtrozos>1 && strcmp(trozos[1],"-h")==0){
        printf ("Today is: %s\n",hora);
        //otra opcion
        //printf ("Hoy es: %d:%d:%d\n",tiempolocal->tm_hour, tiempolocal->tm_min,tiempolocal->tm_sec);
    }
    else{

        printf ("Today is: %s %s\n",fecha,hora);
    }
}
void cmdPid() {
    if (numtrozos>1 && strcmp(trozos[1],"-p")==0)
        printf("pid: %d\n",getppid() );//getppid consigue el pid del padre.
    else
        printf("pid: %d\n",getpid() );

}
void cmdInfosis(){
    struct utsname unameData;
    uname(&unameData);
    printf("%s\n", unameData.sysname);//Podríamos verificar el valor de retorno(distinto de 0 = falla)
}
void cmdAyuda(){
    if (numtrozos>1 && strcmp(trozos[1],"autores")==0)
        printf("Prints the names and logins of the program authors. authors -l prints only the logins and authors -n prints only the names.\n");//creo que habria que hacer esto con todos los comandos
    else if (numtrozos>1 && strcmp(trozos[1],"pid")==0)
        printf("Prints the pid of the process executing the shell. pid -p rints the pid of the shell’s parent process.\n");
    else if (numtrozos>1 && strcmp(trozos[1],"carpeta")==0)
        printf("Changes the current working directory of the shell to direct (using the chdir system call). When invoked without auguments it prints the current working directory (using the getcwd system call.\n");
    else if (numtrozos>1 && strcmp(trozos[1],"fecha")==0)
        printf("Without arguments it prints both the current date and the current time. fecha -d prints the current date in the format DD/MM/YYYY. fecha -h prints the current time in the format hh:mm:ss.\n");
    else if (numtrozos>1 && strcmp(trozos[1],"hist")==0)
        printf("Shows/clears the historic of commands executed by this shell. In order to do this, a list to store all the commands input to the shell must be implemented. historic -c clears the historic, that’s to say,empties the list.\n– hist Prints all the comands that have been input with their order number.\n– hist -c Clears (empties) the list of historic commands.\n– hist -N Prints the first N comands.\n");
    else if (numtrozos>1 && strcmp(trozos[1],"comando")==0)
        printf("Repeats command number N (from historic list).\n");
    else if (numtrozos>1 && strcmp(trozos[1],"infosis")==0)
        printf("Prints information on the machine running the shell (as obtained via the uname system call/library function).\n");
    else if (numtrozos>1 && strcmp(trozos[1],"ayuda")==0)
        printf("ayuda displays a list of available commands. ayuda cmd gives a brief help on the usage of comand cmd.\n");
    else if (numtrozos>1 && strcmp(trozos[1],"fin")==0)
        printf("Ends the shell.\n");
    else if (numtrozos>1 && strcmp(trozos[1],"salir")==0)
        printf("Ends the shell.\n");
    else if (numtrozos>1 && strcmp(trozos[1],"bye")==0)
        printf("Ends the shell.\n");
    else if (numtrozos>1)
        printf("Not a valid comand\n");
    else
        printf("autores [-l|-n]\npid [-p]\ncarpeta [direct]\nfecha [-d|-h]\nhist [-c|-N]\ncomando N\ninfosis\nayuda [cmd]\nfin\nsalir\nbye\n");


}

void cmdCarpeta(){
    char cwd[2048];
    if (numtrozos>1){
        int ch= chdir("/home/eloy/In");
        if(ch<0)
            printf("error: couldn´t switch directory %s\n",trozos[1]);
        else
            printf("succesfully changed\n");
    }
    else {
        if (getcwd(cwd,sizeof(cwd))!=NULL)
            printf("Currently working: %s\n",cwd);
        else printf("error, couldn´t get current directory\n");
    }
}

void cmdHist(){
    tPos pos,posaux;
    int n;
    if (numtrozos>1 && strcmp(trozos[1],"-c")==0) {
        for(pos=first(L);isEmptyList(L)!=true;pos=first(L)){
            posaux=next(pos,L);
            deleteAtPosition(pos,&L);
        }
        printf("history cleared successfully\n");
    }
    else if (numtrozos>1 && (trozos[1]!=NULL)){
        n=(atoi(trozos[1])*-1);
        for(pos=first(L);((pos<=n)&&(pos>=0));pos=next(pos,L)){
            tItem aux=getItem(pos,L);
            printf("%d: %s\n",pos, aux.comando);
        }
    }

    else{

        for(pos=first(L);((pos<=MAX-1)&&(pos>=0));pos=next(pos,L)){
            tItem aux=getItem(pos,L);
            printf("%d: %s\n",pos, aux.comando);
        }
    }
}

void cmdComando(){
    tPos pos;;
    int n,p;
    if (numtrozos>1 && (trozos[1]!=NULL)){
        n=atoi(trozos[1]);
        for(pos=first(L);((pos<=n)&&(pos>=0));pos=next(pos,L)){
            tItem aux=getItem(pos,L);
            if(n==pos){
                if (l==pos)
                    printf("error\n");
                else{
                    printf("%d: %s\n",pos, aux.comando);
                    numtrozos=TrocearCadena(aux.comando,trozos);
                    procesarEntrada();
                }
            }
            if (n>l) printf("invalid command\n");

        }

    }

}

void cmdCrear(){
    char cwd[150];
    FILE *fichero;
    char nombre[2048];

    if (numtrozos>1){
        if((strcmp(trozos[1],"-f")==0)){
            strcpy(nombre,trozos[2]);
            fichero = fopen(nombre, "wb");

        }
        else{
            strcpy(nombre,trozos[1]);
            if(mkdir(nombre, S_IRWXU)==-1)
                printf("cannot create %s: permission denied!\n",nombre);
        }
    }
    else {
        cmdCarpeta();
    }
}


void cmdBorrar(){
    char nombre[2048];
    int i=1;
    if(numtrozos>1){
        while(trozos[i]!=NULL){
            int n = 0;
            FILE *fichero;
            struct dirent *d;
            DIR *dir = opendir(trozos[i]);
            if (dir == NULL){
                strcpy(nombre,trozos[i]);
                remove(nombre);
            }
            else{
                /*   while ((d = readdir(dir)) != NULL) {
                   if(++n > 2)
                    break;
                   } */
                closedir(dir);
                //if (n <= 2) {
                strcpy(nombre,trozos[i]);
                if(rmdir(nombre)==-1)
                    perror("error");

            }
            /*else{
             printf("Not Empty Directory\n");
             d==NULL;
             }*/

            i++;
        }
        /* strcpy(nombre,trozos[i]);
         if(readdir(nombre)<=2)
           rmdir(nombre);
           i++;*/
    }
    else
        cmdCarpeta();
}

void cmdBorrarrec(char *path){
    char nombre[4096],ruta[2048];
    struct dirent *d;
    struct stat comp;
    if(lstat(path,&comp)!=-1){
        if(S_ISDIR(comp.st_mode)) {
            DIR *dir = opendir(path);
            if (dir == NULL){
                perror("falla\n");
            }
            else{
                while ((d = readdir(dir)) != NULL) {

                    if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
                    {
                        continue;

                    }
                    strcpy(ruta,path);
                    /*  printf("%s\n",d->d_name);
                     strcat(nombre,"/");
                     strcat(nombre,d->d_name);*/
                    sprintf(nombre,"%s/%s",ruta,d->d_name);
                    printf("%s\n",nombre);
                    if(lstat(nombre,&comp)!=-1){
                        if(S_ISDIR(comp.st_mode))
                            cmdBorrarrec(nombre);
                        else
                            remove(nombre);


                    }
                    else perror("Error:");
                }
                closedir(dir);
                rmdir(path);
            }
        }
        else remove(path);
    }
    else perror("Error:");
}


void cmdListfich(char *fich,struct opt op){
    struct stat st;
    struct passwd *pwd;
    struct tm *time;
    struct group *grp;
    int k=1;
    int j;
    char nombre[2048];
    char fecha[2048];
    char path[PATH_MAX];
    //int i=1;
    int size=0;
    if(op.longg>0){

        if(op.link>0 || op.acc>0){

            if(op.link==1 && op.acc==1){
                strcpy(nombre,fich);
                if(exists(nombre)==1){
                    lstat(nombre,&st);
                    grp=getgrgid(st.st_gid);
                    pwd=getpwuid(st.st_uid);
                    strftime(fecha,2048,"%Y/%m/%d-%H:%M:%S",localtime(&st.st_atime));
                    if((LetraTF(st.st_mode) == 'l') && (realpath(nombre,path)!=NULL)){
                        printf("%s   %ld   (%ld)   %s   %s   %s   %ld   %s->%s\n",fecha,st.st_ino,st.st_nlink,pwd->pw_name,grp->gr_name,ConvierteModo2(st.st_mode),st.st_size,nombre,path);
                    }
                    else {
                        printf("%s   %ld   (%ld)   %s   %s   %s   %ld   %s\n",fecha,st.st_ino,st.st_nlink,pwd->pw_name,grp->gr_name,ConvierteModo2(st.st_mode),st.st_size,nombre);
                    }
                }
                else
                    printf("The name: %s doesn´t belong to any file\n",nombre);
                j++;
            }
            else if(op.link==1 && op.acc!=1){
                strcpy(nombre,fich);
                if(exists(nombre)==1){
                    lstat(nombre,&st);
                    grp=getgrgid(st.st_gid);
                    pwd=getpwuid(st.st_uid);
                    strftime(fecha,2048,"%Y/%m/%d-%H:%M:%S",localtime(&st.st_mtime));
                    if((LetraTF(st.st_mode) == 'l') && (realpath(nombre,path)!=NULL)){
                        printf("%s   %ld   (%ld)   %s   %s   %s   %ld   %s->%s\n",fecha,st.st_ino,st.st_nlink,pwd->pw_name,grp->gr_name,ConvierteModo2(st.st_mode),st.st_size,nombre,path);
                    }
                    else {
                        printf("%s   %ld   (%ld)   %s   %s   %s   %ld   %s\n",fecha,st.st_ino,st.st_nlink,pwd->pw_name,grp->gr_name,ConvierteModo2(st.st_mode),st.st_size,nombre);
                    }
                }
                else
                    printf("The name: %s doesn´t belong to any file\n",nombre);
                j++;
            }
            else if(op.link!=1 && op.acc==1){
                strcpy(nombre,fich);
                if(exists(nombre)==1){
                    lstat(nombre,&st);
                    grp=getgrgid(st.st_gid);
                    pwd=getpwuid(st.st_uid);
                    strftime(fecha,2048,"%Y/%m/%d-%H:%M:%S",localtime(&st.st_atime));
                    printf("%s   %ld   (%ld)   %s   %s   %s   %ld   %s\n",fecha,st.st_ino,st.st_nlink,pwd->pw_name,grp->gr_name,ConvierteModo2(st.st_mode),st.st_size,nombre);
                }
                else
                    printf("The name: %s doesn´t belong to any file\n",nombre);
                j++;

            }
        }
        else{
            strcpy(nombre,fich);
            if(exists(nombre)==1){
                lstat(nombre,&st);
                grp=getgrgid(st.st_gid);
                pwd=getpwuid(st.st_uid);
                strftime(fecha,2048,"%Y/%m/%d-%H:%M:%S",localtime(&st.st_mtime));
                printf("%s   %ld   (%ld)   %s   %s   %s   %ld   %s\n",fecha,st.st_ino,st.st_nlink,pwd->pw_name,grp->gr_name,ConvierteModo2(st.st_mode),st.st_size,nombre);
            }
            else
                printf("The name: %s doesn´t belong to any file\n",nombre);
            j++;
        }
    }
    else{
        strcpy(nombre,fich);
        if(exists(nombre)==1){
            lstat(nombre,&st);
            printf("%ld  %s\n",st.st_size,nombre);
        }
        else
            printf("The name: %s doesn´t belong to any file\n",nombre);
        j++;

    }
}
int is_hidden(const char *name)
{
    if(name[0] == '.' && strcmp(name, ".") != 0 && strcmp(name, "..") != 0)
        return 1;
    else
        return 0;
}
void ListarF(char *path, struct opt op){
    char nombre[4096],ruta[2048],nombre2[4096],ruta2[2048];
    struct dirent *d;
    struct stat comp;
    if(lstat(path,&comp)!=-1){
        if(S_ISDIR(comp.st_mode)) {
            DIR *dir = opendir(path);
            if (dir == NULL){
                perror("falla\n");
            }
            else{
                while ((d = readdir(dir)) != NULL) {
                    if(op.hid==1){
                        if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
                            continue;

                    }
                    else {
                        if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, "..")|| !memcmp(d->d_name, ".",1))

                            continue;

                    }


                    strcpy(ruta,path);
                    sprintf(nombre,"%s/%s",ruta,d->d_name);
                    printf("%s\n",nombre);
                    if(lstat(nombre,&comp)!=-1){
                        if(S_ISDIR(comp.st_mode))
                            cmdListfich(nombre,op);
                        else
                            cmdListfich(nombre,op);

                    }
                }

                closedir(dir);
            }
        }
    }

}
void cmdListdir(char *path, struct opt op){
    char nombre[4096],ruta[2048],nombre2[4096],ruta2[2048];
    struct dirent *d;
    struct stat comp;
    if(op.reca==1){
        if(lstat(path,&comp)!=-1){
            if(S_ISDIR(comp.st_mode)) {
                DIR *dir = opendir(path);
                if (dir == NULL){
                    perror("falla\n");
                }
                else{
                    ListarF(path,op);
                    while ((d = readdir(dir)) != NULL) {

                        if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
                        {
                            continue;

                        }


                        strcpy(ruta,path);
                        /*  printf("%s\n",d->d_name);
                         strcat(nombre,"/");
                         strcat(nombre,d->d_name);*/
                        sprintf(nombre,"%s/%s",ruta,d->d_name);
                        if(lstat(nombre,&comp)!=-1){
                            if(S_ISDIR(comp.st_mode)) {
                                cmdListdir(nombre,op);
                            }



                        }
                        else perror("Error:");
                    }
                    closedir(dir);
                }
            }
        }
        else perror("Error:");
    }
    else if(op.recb==1){

        if(lstat(path,&comp)!=-1){
            if(S_ISDIR(comp.st_mode)) {
                DIR *dir = opendir(path);
                if (dir == NULL){
                    perror("falla\n");
                }
                else{
                    while ((d = readdir(dir)) != NULL) {

                        if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
                        {
                            continue;

                        }


                        strcpy(ruta,path);
                        sprintf(nombre,"%s/%s",ruta,d->d_name);
                        if(lstat(nombre,&comp)!=-1){
                            if(S_ISDIR(comp.st_mode)) {
                                cmdListdir(nombre,op);
                            }



                        }
                        else perror("Error:");
                    }
                    closedir(dir);
                }
                ListarF(path,op);
            }
        }
        else perror("Error:");
    }
    else ListarF(path,op);
}
void infoFunction(tListf L,int opcion){
    bool malloc=false,mmap=false,shared=false;
    if(opcion==0)
        malloc=true;
    if(opcion==1)
        mmap=true;
    if(opcion==2)
        shared=true;
    if(opcion==3){
        malloc=true;
        mmap=true;
        shared=true;
    }
    tPosf posf;
    tItemf auxf;
    for(posf=firstf(L);nextf(posf,L)!=NULLt;posf=nextf(posf,L)){
        auxf=getItemf(posf,L);
        if(malloc && strcmp(auxf.comando,"malloc")==0)
            printf("%p:  size:%ld. %s %s\n",auxf.direccion,auxf.size,auxf.comando,auxf.fecha);
        if (mmap && strcmp(auxf.comando,"mmap")==0)
            printf("%p:  size:%ld. %s %s (fd:%d) %s\n",auxf.direccion,auxf.size,auxf.comando,auxf.nombre, auxf.descriptors,auxf.fecha);

        if (shared && strcmp(auxf.comando,"shared")==0)
            printf("%p:  size:%ld. %s memory (key %d) %s\n",auxf.direccion,auxf.size,auxf.comando,auxf.key,auxf.fecha);
    }
}
void cmdMalloc(){
    char *ptr;
    long int tam;
    tItemf itemf,auxf;
    tPosf posf,posauxf;
    int exists=0;
    time_t t = time(NULL);
    struct tm *tiempolocal=localtime(&t);
    char fecha [75];
    strftime(fecha, 75, "%a %b %d  %H:%M:%S %Y", tiempolocal);
    int i=1;
    if(strcmp(trozos[1],"-free")==0){
        tam = atoi(trozos[2]);
        for(posf=firstf(memL);posf!=NULLt;posf=nextf(posf,memL)){
            auxf=getItemf(posf,memL);
            if(auxf.size==tam){
                printf("Deallocated   %ld  at    %p\n",tam, auxf.direccion);
                free(auxf.direccion);
                deleteAtPositionf(posf,&memL);

                break;
            }
        }
    }
    else{
        tam = atoi (trozos[1]);
        itemf.direccion = (char *) malloc(tam);
        printf("Allocated   %ld  at    %p\n",tam, itemf.direccion);
        itemf.size=tam;
        strcpy(itemf.fecha,fecha);
        strcpy(itemf.comando,"malloc");
        insertItemf(itemf,&memL);

    }
}
void cmdMmap(){
    char *args[MAXTROZOS];
    struct stat st;
    char *dir;
    char *ptr;
    long int tam;
    tItemf item,auxf;
    tPosf posf,posaux;
    time_t t = time(NULL);
    struct tm *tiempolocal=localtime(&t);
    char fecha [75];
    strftime(fecha, 75, "%a %b %d  %H:%M:%S %Y", tiempolocal);
    if(strcmp(trozos[1],"-free")==0){
        for(posf=firstf(memL);posf!=NULLt;posf=nextf(posf,memL)){
            auxf=getItemf(posf,memL);
            if(strcmp(auxf.nombre,trozos[2])==0 && strcmp(auxf.comando,"mmap")==0){
                printf("Unmapped   %ld  at    %p\n",tam, auxf.direccion);
                munmap(auxf.nombre,auxf.size);
                deleteAtPositionf(posf,&memL);
                break;

            }
        }
    }
    else{
        lstat(trozos[1],&st);
        tam = st.st_size;
        args[0]=trozos[1];
        if(trozos[2]!=NULL)
            args[1]=trozos[2];
        Mmap(args);
        if(exito==1)
            exit;
        else{
            item.descriptors=descriptors;
            item.size=tam;
            item.direccion = direccion;
            strcpy(item.fecha,fecha);
            strcpy(item.comando,"mmap");
            strcpy(item.nombre,trozos[1]);
            insertItemf(item,&memL);
        }
    }
    exito=0;
}

void borrarinfo(char* elemento[MAXTROZOS],bool addr){
    tItemf item,aux;
    tPosf pos,posaux;
    printf("%p\n",elemento[0]);
    for(pos=firstf(memL);nextf(pos,memL)!=NULLt;pos=nextf(pos,memL)){
        aux=getItemf(pos,memL);
        if(addr){
            printf("entro %p\n",aux.direccion);
            if(strcmp(aux.direccion,elemento[0])){
                if(strcmp(aux.comando,"malloc")==0){
                    printf("block at address %p deallocated (%s)\n",aux.direccion,aux.comando);
                    free(aux.direccion);
                    deleteAtPositionf(pos,&memL);
                    break;
                }
                if(strcmp(aux.comando,"mmap")==0){
                    printf("block at address %p deallocated (%s)\n",aux.direccion,aux.comando);
                    munmap(aux.comando,aux.size);
                    deleteAtPositionf(pos,&memL);
                    break;
                }
                if(strcmp(aux.comando,"shared")==0){
                    printf("block at address %p deallocated (%s)\n",aux.direccion,aux.comando);
                    shmdt(aux.direccion);
                    deleteAtPositionf(pos,&memL);
                    break;
                }
            }
        }
        else{
            if(aux.size == atoi(elemento[0]) && strcmp(aux.comando,"malloc")==0){
                printf("block at address %p deallocated (%s)\n",aux.direccion,aux.comando);
                free(aux.direccion);
                deleteAtPositionf(pos,&memL);
                break;
            }
            if(strcmp(aux.nombre,elemento[0])==0 && strcmp(aux.comando,"mmap")==0){
                printf("block at address %p deallocated (%s)\n",aux.direccion,aux.comando);
                munmap(aux.comando,aux.size);
                deleteAtPositionf(pos,&memL);
                break;
            }
            if(aux.key==atoi(elemento[0]) && strcmp(aux.comando,"shared")==0){
                printf("block at address %p deallocated (%s)\n",aux.direccion,aux.comando);
                shmdt(aux.direccion);
                deleteAtPositionf(pos,&memL);
                break;
            }

        }
    }
}
void cmdDealloc(){
    char *borrar[MAXTROZOS];
    if(trozos[1]!=NULL){
        if(strcmp(trozos[1],"-malloc")==0){
            borrar[0]=trozos[2];
            borrarinfo(borrar,false);
        }
        else if(strcmp(trozos[1],"-shared")==0){
            borrar[0]=trozos[2];
            borrarinfo(borrar,false);
        }
        else if(strcmp(trozos[1],"-mmap")==0){
            borrar[0]=trozos[2];
            borrarinfo(borrar,false);
        }
        else{
            borrar[0]=trozos[1];
            borrarinfo(borrar,true);
        }
    }
}
void cmdShared(){
    char *args[MAXTROZOS];
    struct stat st;
    char *dir;
    char *ptr;
    long int tam;
    tItemf item,auxf;
    tPosf posf,posaux;
    time_t t = time(NULL);
    struct tm *tiempolocal=gmtime(&t);
    char fecha [75];
    strftime(fecha, 75, "%a %b %d %H:%M:%S %Y", tiempolocal);
    bool encontrado=false;
    if(strcmp(trozos[1],"-free")==0){
        args[0]=trozos[2];
        /*  for(posf=firstf(mem);posf!=NULLt;posf=nextf(posf,mem)){
             auxf=getItemf(posf,mem);
             if(auxf.key==atoi(trozos[2]) && strcmp(auxf.comando,"shared")==0){
                  printf("Shared memory bloc at %p (key: %d) has been deallocated\n",auxf.direccion,auxf.key);
               shmdt(auxf.direccion);
               deleteAtPositionf(posf,&mem);
               break;

             }
         }*/
        borrarinfo(args,false);
    }
    else if(strcmp(trozos[1],"-create")==0){
        if(trozos[3]==NULL)
            infoFunction(memL,3);
        else{
            args[0]=trozos[2];
            args[1]=trozos[3];
            SharedCreate(args);
            item.key=atoi(trozos[2]);
            item.size=atoi(trozos[3]);
            item.direccion = direccion;
            strcpy(item.fecha,fecha);
            strcpy(item.comando,"shared");
            if(exito)
                insertItemf(item,&memL);
        }
    }
    else if(strcmp(trozos[1],"-delkey")==0){
        printf("Key %d removed from the system\n",atoi(trozos[2]));
        args[0]=trozos[2];
        SharedDelkey(args);
    }
    else {
        for(posf=firstf(memL);nextf(posf,memL)!=NULLt;posf=nextf(posf,memL)){
            auxf=getItemf(posf,memL);
            if(auxf.key == atoi(trozos[1])){
                encontrado=true;
                item.direccion=(char*)ObtenerMemoriaShmget(auxf.key,0);
                item.key=auxf.key;
                strcpy(item.fecha,fecha);
                strcpy(item.comando,"shared");
                item.size=auxf.size;
                insertItemf(item,&memL);
                break;
            }

        }
        if(!encontrado)
            printf("Cannot allocate: No such file or directory");
    }
}
void doRecursiva (int n)
{
    char automatico[4096];
    static char estatico[4096];
    printf ("parametro n:%d en %p\n",n,&n);

    printf ("array estatico en:%p \n",estatico);
    printf ("array automatico en %p\n",automatico);
    n--;
    if (n>0)
        doRecursiva(n);
}
void cmdRecursiva(int n){
    doRecursiva(n);
}
int cmdMemoria(){
    int a,b,c;
    static int d=0,e=0,f=0;
    if(strcmp(trozos[1],"-block")==0){
        infoFunction(memL,3);
        return 0;
    }
    else if(strcmp(trozos[1],"-vars")==0){
        printf("Varaiables locales: %p  %p  %p\n",&a,&b,&c);
        printf("Variables globales: %p  %p  %p\n",&L,&memL,&numtrozos);
        printf("Variables estáticas: %p  %p  %p\n",&e,&f,&d);
        return 0;
    }
    else if(strcmp(trozos[1],"-funcs")==0){
        printf("Funciones Programa : %p,  %p,  %p\n",&cmdCarpeta,&cmdListfich,&cmdCrear);
        printf("Funciones Libreria : %p,  %p,  %p\n",&open,&close,&strftime);
        return 0;
    }
    else if (strcmp(trozos[1],"-all")==0){
        infoFunction(memL,3);
        printf("Varaiables locales: %p  %p  %p\n",&a,&b,&c);
        printf("Variables globales: %p  %p  %p\n",&L,&memL,&numtrozos);
        printf("Variables estáticas: %p  %p  %p\n",&e,&f,&d);
        printf("Funciones Programa : %p,  %p,  %p\n",&cmdCarpeta,&cmdListfich,&cmdCrear);
        printf("Funciones Libreria : %p,  %p,  %p\n",&open,&close,&strftime);
        return 0;
    }
    else if(strcmp(trozos[1],"-pmap")==0){
        dopmap();
        return 0;
    }
}
int cmdVolcarmem(){
    int cont,aux=0;
    unsigned long pos=strtoul(trozos[1],NULL,16);
    unsigned char *pc= (unsigned char *) pos;
    char letras[MAXTROZOS];
    if(trozos[2]!=NULL)
        cont=atoi(trozos[2]);
    else
        cont=25;
    for(int i=0;i<=cont;i++){
        letras[i]=pc[i];
        if(i%25==0) {
            if(i==0)
                continue;
            printf("\n");
            for (int j = aux; j<aux + 25; j++) {
                printf("%02x  ", pc[j]);
            }
            printf("\n");
            aux += 25;
        }
        else{
            printf("%2c  ",pc[i]);
        }
    }
    printf("\n");
    return 0;
}
int cmdLlenarmem(){
    int cont;
    unsigned long pos=strtoul(trozos[1],NULL,16);
    unsigned char *pc= (unsigned char *) pos;
    if(trozos[3]!=NULL) {
        cont = atoi(trozos[2]);
        printf("Memoria llenada con %d bytes de %s byte  en la direccion: %p\n", cont, trozos[3], pc);
        for (int i = 0; i <= cont; i++) {
            pc[i] = strtoul(trozos[3], NULL, 16);
        }
    }
    else {
        cont = 25;
        printf("Memoria llenada con %d bytes de %s byte  en la direccion: %p\n", cont, trozos[2], pc);
        for (int i = 0; i <= cont; i++) {
            pc[i] = strtoul(trozos[2], NULL, 16);
        }
    }
    return 0;

}
int cmdEsread(){
    long int tamano;
    if(trozos[4]!=NULL){
        tamano=LeerFichero(trozos[2],trozos[3],atoi(trozos[4]));
        printf("Leídos %ld bytes del fichero %s en la direccion %p\n",tamano,trozos[2],trozos[3]);
        return 0;
    }
    else{
        tamano=LeerFichero(trozos[2],trozos[3],-1);
        printf("Leídos %ld bytes del fichero %s en la direccion %p\n",tamano,trozos[2],trozos[3]);
        return 0;
    }

}
void cmdEswrite(){
    FILE *fichero;
    struct stat s;
    char nombre[2048];
    long int tamano;
    if(strcmp(trozos[2],"-o")==0){
        strcpy(nombre,trozos[3]);
        int filedesc = open(nombre, O_WRONLY | O_APPEND | O_CREAT);
        stat(nombre,&s);
        tamano=(ssize_t) s.st_size;
        if(trozos[5]!=NULL) {
            tamano = atoi(trozos[5]);
            write(filedesc, trozos[4], atoi(trozos[4]));
        }
        else
            write(filedesc,&trozos[3],atoi(trozos[4]));
        printf("Escritos %d bytes de la direccion %p en el fichero %s\n",atoi(trozos[4]),trozos[3],nombre);
    }
    else{
        strcpy(nombre,trozos[2]);
        if(exists(nombre))
            printf("No se puede sobreescribir, el archivo ya existe\n");
        else{
            if(trozos[4]!=NULL){
                stat(nombre,&s);
                tamano=(ssize_t) s.st_size;
            }
            else
                tamano=atoi(trozos[4]);


            int filedesc = open(nombre, O_WRONLY | O_APPEND | O_CREAT);
            write(filedesc,&trozos[3],atoi(trozos[4]));
            printf("Escritos %d bytes de la direccion %p en el fichero %s\n",atoi(trozos[4]),trozos[3],nombre);

        }
    }
}
void cmdPriority(char* auxiliar[]) {
    int pri;
    int priority;
    id_t pid;
    if (auxiliar[1] != NULL) {
        pid= atoi(auxiliar[1]);
        if (auxiliar[2] != NULL) {
            priority= atoi(auxiliar[2]);
            setpriority(PRIO_PROCESS,pid,priority);
            printf("The priority of the process with pid: %u has been changed to: %d\n",pid,priority);
        } else {
            pri=getpriority(PRIO_PROCESS,pid);
            printf("The priority of the process with pid: %u is: %d\n",pid,pri);
        }
    } else {
        pri = getpriority(PRIO_PROCESS, getpid());
        printf("The priority of the running process is: %d\n", pri);
    }
}
void cmdRederr(){
}
void cmdEntorno(){
    char * env=NULL;
    extern char **environ;
    if(trozos[1]!=NULL){
        if(strcmp(trozos[2],"-environ")==0){
            MostrarEntorno(trozos,environ);
        }
        else if(strcmp(trozos[2],"-adrr")==0){

        }
    }
    else{
        MostrarEntorno(trozos,env);
    }
}
void cmdMostrarvar(){

}
void cmdUid() {
    if (trozos[1] != NULL) {
        if (strcmp(trozos[1], "-get") == 0) {
           MostrarUidsProceso();
        }
        if (strcmp(trozos[1], "-set") == 0) {
            if (trozos[2] != NULL) {
                if (strcmp(trozos[2], "-l") == 0) {
                    CambiarUidLogin(trozos[3]);
                } else if (trozos[3] != NULL) {
                    CambiarUidLogin(trozos[2]);
                }
            } else
               MostrarUidsProceso();
        }
    }else
          MostrarUidsProceso();
}
void cmdFork(){

}
void cmdEjec(){
 execvp(trozos[1],&trozos[1]);
}
int cmdEjecpri(){
    char pid[100];
    sprintf(pid,"%d",getpid());
    char *aux[100]={"priority",pid,trozos[1]};
    cmdPriority(aux);
    execvp(trozos[2],&trozos[2]);
    return 0;
}
void imprimirPrompt(){
    printf("@>");
}

void leerEntrada(){
    if (fgets(linea,10000,stdin)==NULL) {exit(0); }  //siempre arrancar con if
    strcpy(comando.comando,linea);
    insertItem(comando,&L);
    numtrozos=TrocearCadena(linea,trozos);
    /*for(i=0;i<numtrozos;i++)
    printf("trozo %d= [%s]\n",i,trozos[i]);*/  //cuenta los trozos
}

int procesarEntrada(){
    if(trozos[0]==NULL){
        printf("error: you have not entered a command\n");
        return 0;

    }
    else{
        if(strncmp(trozos[0],"autores",8)==0){
            cmdAutores();


        }
        else if(strncmp(trozos[0],"fecha",6)==0){
            cmdFecha();

        }
        else  if(strncmp(trozos[0],"pid",4)==0){
            cmdPid();

        }
        else  if(strncmp(trozos[0],"infosis",8)==0){
            cmdInfosis();

        }
        else if(strncmp(trozos[0],"fin",4)==0){
            printf("Ends the shell.\n");
            return 1;
        }
        else if(strncmp(trozos[0],"salir",6)==0){
            printf("Ends the shell.\n");
            return 1;
        }
        else if(strncmp(trozos[0],"bye",4)==0){
            printf("Ends the shell.\n");
            return 1;
        }
        else if(strncmp(trozos[0],"ayuda",6)==0){
            cmdAyuda();
        }
        else if(strncmp(trozos[0],"carpeta",8)==0){
            cmdCarpeta();

        }
        else if(strncmp(trozos[0],"hist",5)==0){
            cmdHist();

        }
        else if(strncmp(trozos[0],"comando",8)==0){
            cmdComando();

        }
        else if(strncmp(trozos[0],"crear",6)==0){
            cmdCrear();
        }
        else if(strncmp(trozos[0],"borrar",7)==0){
            cmdBorrar();
        }
        else if(strncmp(trozos[0],"borrarrec",10)==0){
            if(numtrozos>1){
                for(int i = 1;trozos[i]!=NULL;i++){
                    cmdBorrarrec(trozos[i]);
                }
            }
            else cmdCarpeta();
        }
        else if(strncmp(trozos[0],"listfich",9)==0){
            options(&optfich,&kListfich);
            jListfich=kListfich;
            if(numtrozos>1){
                while(trozos[jListfich]!=NULL){
                    cmdListfich(trozos[jListfich],optfich);
                    jListfich ++;
                }
            }
            else
                cmdCarpeta();
        }
        else if(strncmp(trozos[0],"listdir",8)==0){
            options(&optdir,&kListdir);
            jListdir=kListdir;
            if(numtrozos>1){
                while(trozos[jListdir]!=NULL){
                    cmdListdir(trozos[jListdir],optdir);
                    jListdir ++;
                }
            }
            else
                cmdCarpeta();
        }
        else if(strncmp(trozos[0],"malloc",6)==0){
            if(trozos[1]==NULL){
                infoFunction(memL,0);

            }
            else
                cmdMalloc();
        }
        else if(strncmp(trozos[0],"mmap",4)==0){
            if(trozos[1]==NULL){
                infoFunction(memL,1);
            }
            else
                cmdMmap();
        }
        else if(strncmp(trozos[0],"shared",6)==0){
            if(trozos[1]==NULL){
                infoFunction(memL,2);
            }
            else
                cmdShared();
        }
        else  if(strncmp(trozos[0],"recursiva",9)==0){
            cmdRecursiva(atoi(trozos[1]));

        }
        else if(strncmp(trozos[0],"dealloc",7)==0){
            cmdDealloc();
        }
        else  if(strncmp(trozos[0],"memoria",7)==0){
            cmdMemoria();
        }
        else  if(strncmp(trozos[0],"llenarmem",8)==0){
            cmdLlenarmem();
        }
        else  if(strncmp(trozos[0],"volcarmem",8)==0){
            cmdVolcarmem();
        }
        else  if(strncmp(trozos[0],"e-s",3)==0){
            if(strncmp(trozos[1],"read",4)==0)
                cmdEsread();
            else if(strncmp(trozos[1],"write",5)==0)
                cmdEswrite();
        }
        else if(strncmp(trozos[0],"priority",8)==0){
            cmdPriority(trozos);
        }
        else if(strncmp(trozos[0],"rederr",6)==0){
            cmdRederr();
        }
        else if(strncmp(trozos[0],"entorno",7)==0){
            cmdEntorno();
        }
        else if(strncmp(trozos[0],"uid",3)==0){
            cmdUid();
        }
        else if(strcmp(trozos[0],"ejec")==0){
            cmdEjec();
        }
        else if(strncmp(trozos[0],"ejecpri",7)==0){
            cmdEjecpri();
        }
        else
            printf("Not a valid comand\n");
            l++;
            return 0;

    }
}

int main(){
    int terminar=0;
    createEmptyListf(&memL);
    createEmptyList(&L);
    while(terminar==0){
        imprimirPrompt();
        leerEntrada();
        terminar=procesarEntrada();
    }
}
