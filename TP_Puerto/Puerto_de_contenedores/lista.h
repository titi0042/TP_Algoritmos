#ifndef LISTA_H_INCLUDED
#define LISTA_H_INCLUDED

typedef struct sNodoLista
{
    void *info;
    unsigned tamInfo;
    struct sNodoLista* sig;
}tNodoLista;

typedef tNodoLista* tLista;

void crearLista(tLista* p);

int vaciarLista(tLista* p);
int vaciarListaYMostrar(tLista* p, void (*Mostrar)(const void*, FILE*), FILE* fp);

int listaLlena(const tLista* p, unsigned cantBytes);
int listaVacia(const tLista* p);

int ponerAlComienzo(tLista* p, const void* d, unsigned cantBytes);
int ponerAlFinal(tLista* p, const void* d, unsigned cantBytes);
int ponerEnOrden(tLista* lista, const void* d, unsigned cantBytes,
                 int(*Comparar)(const void*, const void*),
                 int(*Acumular)(void**, unsigned*, const void*, unsigned));

int sacarPrimeroLista(tLista* p, void* d, unsigned cantBytes);
int sacarUltimoLista(tLista* p, void* d, unsigned cantBytes);

int verPrimeroLista(const tLista* p, void* d, unsigned cantBytes);
int verUltimoLista(const tLista* p, void* d, unsigned cantBytes);

int mostrarLista(tLista* p, void (*Mostrar)(const void*, FILE*), FILE* fp);
int mostrarListaAlReves(tLista* p, void (*Mostrar)(const void*, FILE*), FILE* fp);
int mostrarListaAlRevesYVaciar(tLista* p, void (*Mostrar)(const void*, FILE*), FILE* fp);

void ordenarLista(tLista* p, int(*Comparar)(const void*, const void*));

#endif // LISTA_H_INCLUDED
