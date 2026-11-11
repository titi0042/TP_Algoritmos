#ifndef LISTA_H_INCLUDED
#define LISTA_H_INCLUDED

#include <stdio.h>
#include "../comunes.h"

typedef struct sNodoLista
{
    void *info;
    unsigned tamInfo;
    struct sNodoLista* sig;
} tNodoLista;

typedef tNodoLista* tLista;




typedef void (*tMostrarLista)(const void* info, FILE* out);
typedef void (*tMostrarArchivo)(const void*, FILE*);



void crearLista(tLista* p);

int vaciarLista(tLista* p);
int vaciarListaYMostrar(tLista* p, tMostrarArchivo Mostrar, FILE* fp);

int listaLlena(const tLista* p, unsigned cantBytes);
int listaVacia(const tLista* p);

int ponerAlComienzo(tLista* p, const void* d, unsigned cantBytes);
int ponerAlFinal(tLista* p, const void* d, unsigned cantBytes);
int ponerEnOrden(tLista* lista, const void* d, unsigned cantBytes, tCmp Comparar, tAcumular Acumular);

int sacarPrimeroLista(tLista* p, void* d, unsigned cantBytes);
int sacarUltimoLista(tLista* p, void* d, unsigned cantBytes);

int sacarPosLista(tLista *p, void *info, unsigned tamInfo, unsigned pos);



int busqClaveLista(tLista *p, tCmp comparar, void* clave);


int sacarClaveLista(tLista* p, void* d, unsigned cantBytes, tCmp comparar);


int verPrimeroLista(const tLista* p, void* d, unsigned cantBytes);
int verUltimoLista(const tLista* p, void* d, unsigned cantBytes);

int mostrarLista(tLista* p, tMostrarArchivo Mostrar, FILE* fp);
int mostrarListaAlReves(tLista* p, tMostrarArchivo Mostrar, FILE* fp);
int mostrarListaAlRevesYVaciar(tLista* p, tMostrarArchivo Mostrar, FILE* fp);

void ordenarLista(tLista* p, tCmp Comparar);



void mapLista(tLista* p, tAccion accion, void* parametro);
#endif // LISTA_H_INCLUDED
