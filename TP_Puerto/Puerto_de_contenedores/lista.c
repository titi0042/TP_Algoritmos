#include "lista.h"

void crearLista(tLista* p)
{
    *p = NULL;
}

int vaciarLista(tLista* p)
{
    int cant = 0;
    while(*p)
    {
        tNodoLista* aux = *p;

        cant++;
        *p = aux->sig;
        free(aux->info);
        free(aux);
    }
    return cant;
}

int vaciarListaYMostrar(tLista* p, void (*Mostrar)(const void* elem, FILE* arch), FILE* fp)
{
    int cant = 0;
    while(*p)
    {
        tNodoLista* aux = *p;

        cant++;
        *p = aux->sig;

        if(Mostrar && fp)
            Mostrar(aux->info, fp);

        free(aux->info);
        free(aux);
    }
    return cant;
}

int listaLlena(const tLista* p, unsigned cantBytes)
{
    tNodoLista* aux = malloc(sizeof(tNodoLista));
    void* info = malloc(cantBytes);

    free(aux);
    free(info);

    return aux == NULL || info == NULL;
}

int listaVacia(const tLista* p)
{
    return *p == NULL;
}

int ponerEnOrden(tLista* p, const void* d, unsigned cantBytes,
                 int(*Comparar)(const void*, const void*),
                 int(*Acumular)(void**, unsigned*, const void*, unsigned))
{
    tNodoLista* nuevo;

    while(*p && Comparar((*p)->info, d) < 0)
        p = &(*p)->sig;

    if(*p && Comparar((*p)->info, d) == 0)
    {
        if(Acumular)
            if(!Acumular(&(*p)->info, &(*p)->tamInfo, d, cantBytes))
                return SIN_MEM;
        return CLA_DUP;
    }

    if((nuevo = malloc(sizeof(tNodoLista))) == NULL || (nuevo->info = malloc(cantBytes)) == NULL)
    {
        free(nuevo);
        return SIN_MEM;
    }

    memcpy(nuevo->info, d, cantBytes);
    nuevo->tamInfo = cantBytes;
    nuevo->sig = *p;
    *p = nuevo;

    return TODO_OK;
}

int ponerAlComienzo(tLista* p, const void* d, unsigned cantBytes)
{
    tNodoLista* nuevo;

    if((nuevo = malloc(sizeof(tNodoLista))) == NULL ||
            (nuevo->info = malloc(cantBytes)) ==  NULL)
    {
        free(nuevo);
        return SIN_MEM;
    }

    memcpy(nuevo->info, d, cantBytes);
    nuevo->tamInfo = cantBytes;
    nuevo->sig = *p;
    *p = nuevo;

    return TODO_OK;
}

int ponerAlFinal(tLista* p, const void* d, unsigned cantBytes)
{
    tNodoLista* nuevo;

    while(*p)
        p = &(*p)->sig;

    if((nuevo = malloc(sizeof(tNodoLista))) == NULL ||
            (nuevo->info = malloc(cantBytes)) == NULL)
    {
        free(nuevo);
        return SIN_MEM;
    }

    memcpy(nuevo->info, d, cantBytes);
    nuevo->tamInfo = cantBytes;
    nuevo->sig = NULL;
    *p = nuevo;

    return TODO_OK;
}

int sacarPrimeroLista(tLista* p, void* d, unsigned cantBytes)
{
    tNodoLista* aux = *p;

    if(aux == NULL)
        return SIN_MEM;

    *p = aux->sig;
    memcpy(d, aux->info, MINIMO(cantBytes, aux->tamInfo));
    free(aux->info);
    free(aux);

    return TODO_OK;
}

int sacarUltimoLista(tLista* p, void* d, unsigned cantBytes)
{
    if(*p == NULL)
        return 0;

    while((*p)->sig)
        p = &(*p)->sig;

    memcpy(d, (*p)->info, MINIMO(cantBytes, (*p)->tamInfo));
    free((*p)->info);
    free(*p);
    *p = NULL;

    return 1;
}

int verPrimeroLista(const tLista* p, void* d, unsigned cantBytes)
{
    if(*p == NULL)
        return ESTRUC_VACIA;
    memcpy(d, (*p)->info, MINIMO(cantBytes, (*p)->tamInfo));
    return TODO_OK;
}

int verUltimoLista(const tLista* p, void* d, unsigned cantBytes)
{
    if(*p == NULL)
        return ESTRUC_VACIA;

    while((*p)->sig)
        p = &(*p)->sig;

    memcpy(d, (*p)->info, MINIMO(cantBytes, (*p)->tamInfo));

    return TODO_OK;
}

void ordenarLista(tLista* p, int(*Comparar)(const void*, const void*))
{
    tLista* pri =p;

    if(*p == NULL)
        return;

    while((*p)->sig)
    {
        if(Comparar((*p)->info, (*p)->sig->info) > 0)
        {
            tLista* q = pri;
            tNodoLista* aux = (*p)->sig;

            (*p)->sig = aux->sig;

            while(Comparar((*q)->info, aux->info) <=  0)
                q = &(*q)->sig;

            aux->sig = *q;
            *q = aux;
        }
        else
            p = &(*p)->sig;
    }
}

int mostrarLista(tLista* p, void (*Mostrar)(const void*, FILE*), FILE* fp)
{
    int cant = 0;

    while(*p)
    {
        Mostrar((*p)->info, fp);
        p = &(*p)->sig;
        cant++;
    }
    return cant;
}

int mostrarListaAlReves(tLista* p, void (*Mostrar)(const void*, FILE*), FILE* fp)
{
    if(*p)
    {
        int n = mostrarListaAlReves(&(*p)->sig, Mostrar, fp);

        Mostrar((*p)->info, fp);

        return n + 1;
    }
    return TODO_OK;
}

int mostrarListaAlRevesYVaciar(tLista* p, void (*Mostrar)(const void*, FILE*), FILE* fp)
{
    if(*p)
    {
        int n = mostrarListaAlReves(&(*p)->sig, Mostrar, fp);

        Mostrar((*p)->info, fp);
        free((*p)->info);
        free(*p);
        *p = NULL;

        return n + 1;
    }
    return TODO_OK;
}
