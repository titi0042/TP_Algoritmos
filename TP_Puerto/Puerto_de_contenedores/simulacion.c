#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "simulacion.h"

void mostrarBuquesEspera(tCola* colaBuqes, int tiempoActual)
{
    tCola colaAux;
    int i=0;
    tBuque buque;
    crearCola(&colaAux);

    while(sacarDeCola(colaBuqes,&buque,sizeof(tBuque))==EXITO)
    {
        if(buque.tArribo<=tiempoActual)
        {
            i++;
        }
        ponerEnCola(&colaAux,&buque,sizeof(tBuque));
    }
    while(sacarDeCola(&colaAux,&buque,sizeof(tBuque))==EXITO)
    {
        ponerEnCola(colaBuqes,&buque,sizeof(tBuque));
    }

    printf("Buques en espera: %d\n",i);

    vaciarCola(&colaAux);

}
int mostrarTresCamiones(tCola* colaCamiones, int tiempoActual)
{
    tCola colaAux;
    tCamion camion;
    int i=0;

    crearCola(&colaAux);

    printf("Cola camiones: ");

    verPrimeroCola(colaCamiones, &camion, sizeof(tCamion));

    if(camion.tArribo>tiempoActual)
    {
        printf("No llegaron camiones\n");
        vaciarCola(&colaAux);
        return 0;
    }

    while(sacarDeCola(colaCamiones,&camion,sizeof(tCamion))==EXITO)
    {
        ponerEnCola(&colaAux,&camion,sizeof(tCamion));
    }
    while(sacarDeCola(&colaAux,&camion,sizeof(tCamion))==EXITO)
    {
        if(camion.tArribo <= tiempoActual)
        {
            if (i < 3)
            {
                if (i > 0)
                {
                    printf(" -> ");
                }

                printf("%s(%s)", camion.id, camion.contenedorPedido);

            }
            i++;
        }
        ponerEnCola(colaCamiones,&camion,sizeof(tCamion));
    }

    if (i > 0)
    {
        printf("\n");
    }
    else
    {
        printf(" vacia\n");
    }

    vaciarCola(&colaAux);

    return i;

}
int mostrarBuqueAtracado(void* info, const void* parametro)
{
    tMuelle* muelle=(tMuelle*)info;
    tContenedor contenedor;

    if(muelle->ocupado)
    {
        printf("M%d: %s -> Proximo contenedor: ",muelle->idMuelle,muelle->buqueActual.id);
        if(verPrimeroCola(&(muelle->buqueActual.contenedores),&contenedor,sizeof(tContenedor))==EXITO)
            printf("%s\n",contenedor.codigo);
    }

    return EXITO;
}

int mostrarContenedorZona(void* info, const void* parametro)
{
    tZona* zona=(tZona*)info;
    tPila pilaAux;
    tContenedor contenedor;
    crearPila(&pilaAux);

    printf("Z%d:[",zona->idZona);
    while(sacarDePila(&(zona->pilaContenedores),&contenedor,sizeof(tContenedor))==EXITO)
    {
        printf("%s",contenedor.codigo);
        if(!pilaVacia(&(zona->pilaContenedores)))
            printf("|");
        ponerEnPila(&pilaAux,&contenedor,sizeof(tContenedor));
    }

    printf("]\t");

    while(sacarDePila(&pilaAux,&contenedor,sizeof(tContenedor))==EXITO)
    {
        ponerEnPila(&(zona->pilaContenedores),&contenedor,sizeof(tContenedor));
    }

    vaciarPila(&pilaAux);

    return EXITO;

}

int compararEntrega(const void* info, const void* clave)
{
    tZona* aux=(tZona*)info;
    tContenedor contenedorAux;
    if(verTope(&(aux->pilaContenedores),&contenedorAux,sizeof(tContenedor))!=EXITO)
        return PILA_VACIA;
    return strcmpi(contenedorAux.codigo,(char*)clave);
}

int procesarContenedor(void* info, const void* clave)
{
    tZona* zonaAux =(tZona*)info;

    tContenedor contenedorAux;

    if(verTope(&(zonaAux->pilaContenedores),&contenedorAux,sizeof(tContenedor))!= EXITO)
        return PILA_VACIA;

    if(strcmpi((char*)clave,contenedorAux.codigo)==0)
    {
        sacarDePila(&(zonaAux->pilaContenedores),&contenedorAux,sizeof(tContenedor));
        return EXITO;
    }
    return NO_ENCONTRADO;
}

int compararMuelle(const void* a, const void* b)
{
    tMuelle* aux1=(tMuelle*)a;
    tMuelle* aux2=(tMuelle*)b;

    return aux1->idMuelle-aux2->idMuelle;
}


int compararZona(const void* a, const void* b)
{
    tZona* aux1=(tZona*)a;
    tZona* aux2=(tZona*)b;

    return aux1->idZona-aux2->idZona;
}

int inicializarPuerto(tPuerto* puerto, const char* rutaEscenario, const tConfiguracion* config)
{
    FILE* arch;
    char linea[256];
    int modoLectura = 0; // 0 = ignorar, 1 = leyendo buques, 2 = leyendo camiones
    int i;

    tMuelle nuevoMuelle;
    tZona nuevaZona;
    tBuque nuevoBuque;
    tCamion nuevoCamion;
    tContenedor cont;

    char* token;
    char* listaContenedores;
    char* tokenContenedor;

    if (!puerto || !rutaEscenario || !config)
    {
        return NO_ENCONTRADO;
    }

    arch = fopen(rutaEscenario, "rt");
    if (!arch)
    {
        return ERR_ARCH;
    }


    puerto->relojActual = 0;
    puerto->buquesDescargados = 0;
    puerto->contenedoresEntregados = 0;
    puerto->reubicacionesRealizadas = 0;

    puerto->duracionJornada = config->duracionJornadaMinutos;
    puerto->cantidadMuelles = config->cantidadMuelles;
    puerto->cantidadZonas = config->cantidadZonasAlmacenamiento;
    puerto->capacidadPila = config->capacidadPila;
    puerto->tiempoDescarga = config->tiempoDescargaContenedor;
    puerto->tiempoReubicacion = config->tiempoReubicacionContenedor;
    puerto->tiempoCarga = config->tiempoCargaCamion;


    crearCola(&(puerto->colaBuques));
    crearCola(&(puerto->colaCamiones));
    crearLista(&(puerto->muelles));
    crearLista(&(puerto->zonas));


    for (i = 1; i <= puerto->cantidadMuelles; i++)
    {
        nuevoMuelle.idMuelle = i;
        nuevoMuelle.ocupado = 0;
        ponerAlFinal(&(puerto->muelles), &nuevoMuelle, sizeof(tMuelle));
    }


    for (i = 1; i <= puerto->cantidadZonas; i++)
    {
        nuevaZona.idZona = i;
        nuevaZona.cantidadActual = 0;
        nuevaZona.capacidadMaxima = puerto->capacidadPila;
        crearPila(&(nuevaZona.pilaContenedores));
        ponerAlFinal(&(puerto->zonas), &nuevaZona, sizeof(tZona));
    }


    while (fgets(linea, sizeof(linea), arch) != NULL)
    {
        limpiarSaltoLinea(linea);


        if (*linea != '\0')
        {
            if (strcmp(linea, "[BUQUES]") == 0)
            {
                modoLectura = 1;
            }
            else if (strcmp(linea, "[CAMIONES]") == 0)
            {
                modoLectura = 2;
            }
            else
            {
                if (modoLectura == 1)
                {
                    crearCola(&(nuevoBuque.contenedores));

                    token = strtok(linea, ";");
                    if (token)
                    {
                        strncpy(nuevoBuque.id, token, sizeof(nuevoBuque.id)-1);
                        *(nuevoBuque.id + sizeof(nuevoBuque.id)-1) = '\0';

                        token = strtok(NULL, ";");
                        if (token)
                        {
                            sscanf(token, "T=%d", &(nuevoBuque.tArribo));

                            token = strtok(NULL, ";");
                            if (token && strncmp(token, "C=", 2) == 0)
                            {
                                listaContenedores = token + 2;
                                tokenContenedor = strtok(listaContenedores, ",");

                                while (tokenContenedor)
                                {
                                    strncpy(cont.codigo, tokenContenedor, sizeof(cont.codigo)-1);
                                    *(cont.codigo + sizeof(cont.codigo)-1) = '\0';
                                    ponerEnCola(&(nuevoBuque.contenedores), &cont, sizeof(tContenedor));

                                    tokenContenedor = strtok(NULL, ",");
                                }
                            }
                        }
                        ponerEnCola(&(puerto->colaBuques), &nuevoBuque, sizeof(tBuque));
                    }

                }
                else if (modoLectura == 2)
                {
                    token = strtok(linea, ";");
                    if (token)
                    {
                        strncpy(nuevoCamion.id, token, sizeof(nuevoCamion.id)-1);
                        *(nuevoCamion.id + sizeof(nuevoCamion.id)-1) = '\0';

                        token = strtok(NULL, ";");
                        if (token)
                        {
                            sscanf(token, "T=%d", &(nuevoCamion.tArribo));

                            token = strtok(NULL, ";");
                            if (token && strncmp(token, "C=", 2) == 0)
                            {
                                strncpy(nuevoCamion.contenedorPedido, token + 2, sizeof(nuevoCamion.contenedorPedido) - 1);
                                *(nuevoCamion.contenedorPedido + sizeof(nuevoCamion.contenedorPedido) - 1) = '\0';
                            }
                        }
                        ponerEnCola(&(puerto->colaCamiones), &nuevoCamion, sizeof(tCamion));
                    }
                }
            }
        }
    }

    fclose(arch);
    return EXITO;
}
int evaluarOperacionesValidas(tPuerto* puerto)
{
    int puedeDES = 0, puedeREU = 0, puedeENT = 0;
    int muelleConCarga = 0;
    int zonaConEspacio = 0;
    int zonaNoVacia = 0;
    int topeCoincide = 0;
    int i, j;

    tMuelle muelleAux;
    tZona zonaAux, zonaDestAux;
    tCamion camionFrente;
    tContenedor contTope;

    /// muelleConCarga && zonaConEspacio = Puede DES
    /// zonaConEspacio && zonaNoVacia = Puede DES y REU
    /// zonaConEspacio && !zonaNoVacia = Puede DES
    /// !zonaConEspacio && zonaNoVacia = No puede DES ni REU
    /// !zonaConEspacio && !zonaNoVacia = Caso no posible
    /// topeCoincide = Puede ENT

    for (i = 1; i <= puerto->cantidadMuelles; i++)
    {
        muelleAux.idMuelle = i;

        if (sacarClaveLista(&(puerto->muelles), &muelleAux, sizeof(tMuelle), compararMuelle) == EXITO)
        {
            if (muelleAux.ocupado == 1 && colaVacia(&(muelleAux.buqueActual.contenedores)) == 0)
            {
                muelleConCarga = 1;
            }
            ponerEnOrden(&(puerto->muelles), &muelleAux, sizeof(tMuelle), compararMuelle, NULL);
        }

    }


    for (i = 1; i <= puerto->cantidadZonas; i++)
    {
        zonaAux.idZona = i;
        if (sacarClaveLista(&(puerto->zonas), &zonaAux, sizeof(tZona), compararZona) == EXITO)
        {
            if (zonaAux.cantidadActual < zonaAux.capacidadMaxima)
                zonaConEspacio = 1;
            if (zonaAux.cantidadActual > 0)
                zonaNoVacia = 1;

            ponerEnOrden(&(puerto->zonas), &zonaAux, sizeof(tZona), compararZona, NULL);
        }
    }

    if (muelleConCarga && zonaConEspacio)
    {
        puedeDES = 1;
    }


    if (zonaNoVacia && zonaConEspacio)
    {
        for (i = 1; i <= puerto->cantidadZonas && puedeREU == 0; i++)
        {
            zonaAux.idZona = i;
            if (sacarClaveLista(&(puerto->zonas), &zonaAux, sizeof(tZona), compararZona) == EXITO)
            {
                if (zonaAux.cantidadActual > 0)
                {
                    for (j = 1; j <= puerto->cantidadZonas && puedeREU == 0; j++)
                    {
                        if (i != j)
                        {
                            zonaDestAux.idZona = j;
                            if (sacarClaveLista(&(puerto->zonas), &zonaDestAux, sizeof(tZona), compararZona) == EXITO)
                            {
                                if (zonaDestAux.cantidadActual < zonaDestAux.capacidadMaxima)
                                {
                                    puedeREU = 1;
                                }
                                ponerEnOrden(&(puerto->zonas), &zonaDestAux, sizeof(tZona), compararZona, NULL);
                            }
                        }
                    }
                }
                ponerEnOrden(&(puerto->zonas), &zonaAux, sizeof(tZona), compararZona, NULL);
            }
        }
    }


    if (verPrimeroCola(&(puerto->colaCamiones), &camionFrente, sizeof(tCamion)) == EXITO)
    {
        if (camionFrente.tArribo <= puerto->relojActual)
        {
            for (i = 1; i <= puerto->cantidadZonas && topeCoincide == 0; i++)
            {
                zonaAux.idZona = i;
                if (sacarClaveLista(&(puerto->zonas), &zonaAux, sizeof(tZona), compararZona) == EXITO)
                {
                    if (zonaAux.cantidadActual > 0)
                    {
                        if (verTope(&(zonaAux.pilaContenedores), &contTope, sizeof(tContenedor)) == EXITO)
                        {
                            if (strcmp(contTope.codigo, camionFrente.contenedorPedido) == 0)
                            {
                                topeCoincide = 1;
                            }
                        }
                    }
                    ponerEnOrden(&(puerto->zonas), &zonaAux, sizeof(tZona), compararZona, NULL);
                }
            }
            if (topeCoincide)
                puedeENT = 1;
        }
    }

    if (puedeDES || puedeREU || puedeENT)
    {
        return EXITO;
    }
    return NO_OP_VALIDAS;
}
int actualizarEstadoPuerto(tPuerto* puerto)
{
    int i;
    tMuelle muelleAux;
    tBuque buqueFrente;
    tCamion camionFrente;

    int minEventoFuturo = 999999;
    int finNormal = 1;

    if (puerto->relojActual >= puerto->duracionJornada)
    {
        return SIM_FIN_TIEMPO;
    }

    for (i = 1; i <= puerto->cantidadMuelles; i++)
    {
        muelleAux.idMuelle = i;
        if (sacarClaveLista(&(puerto->muelles), &muelleAux, sizeof(tMuelle), compararMuelle) == EXITO)
        {
            if (muelleAux.ocupado == 1)
            {
                if (colaVacia(&(muelleAux.buqueActual.contenedores)) != 0)
                {
                    muelleAux.ocupado = 0;
                    puerto->buquesDescargados++;
                }
                else
                {
                    finNormal = 0;
                }
            }
            ponerEnOrden(&(puerto->muelles), &muelleAux, sizeof(tMuelle), compararMuelle, NULL);
        }
    }


    for (i = 1; i <= puerto->cantidadMuelles; i++)
    {
        muelleAux.idMuelle = i;
        if (sacarClaveLista(&(puerto->muelles), &muelleAux, sizeof(tMuelle), compararMuelle) == EXITO)
        {
            if (muelleAux.ocupado == 0)
            {
                if (verPrimeroCola(&(puerto->colaBuques), &buqueFrente, sizeof(tBuque)) == EXITO)
                {
                    if (buqueFrente.tArribo <= puerto->relojActual)
                    {
                        sacarDeCola(&(puerto->colaBuques), &buqueFrente, sizeof(tBuque));
                        muelleAux.buqueActual = buqueFrente;
                        muelleAux.ocupado = 1;
                    }
                }
            }

            if (muelleAux.ocupado == 1 && colaVacia(&(muelleAux.buqueActual.contenedores)) == 0)
            {
                finNormal = 0;
            }

            ponerEnOrden(&(puerto->muelles), &muelleAux, sizeof(tMuelle), compararMuelle, NULL);
        }
    }


    if (verPrimeroCola(&(puerto->colaBuques), &buqueFrente, sizeof(tBuque)) == EXITO)
    {
        finNormal = 0;
        if (buqueFrente.tArribo > puerto->relojActual && buqueFrente.tArribo < minEventoFuturo)
        {
            minEventoFuturo = buqueFrente.tArribo;
        }
    }

    if (verPrimeroCola(&(puerto->colaCamiones), &camionFrente, sizeof(tCamion)) == EXITO)
    {
        finNormal = 0;
        if (camionFrente.tArribo > puerto->relojActual && camionFrente.tArribo < minEventoFuturo)
        {
            minEventoFuturo = camionFrente.tArribo;
        }
    }

    if (finNormal == 1)
    {
        return SIM_FIN_NORMAL;
    }

    int operacionesValidas = evaluarOperacionesValidas(puerto);
    if (operacionesValidas == NO_OP_VALIDAS)
    {
        if (minEventoFuturo != 999999 && minEventoFuturo <= puerto->duracionJornada)
        {
            puerto->relojActual = minEventoFuturo;
            return actualizarEstadoPuerto(puerto);
        }
        else
        {
            return SIM_FIN_BLOQUEO;
        }
    }

    return SIM_EN_CURSO;
}


void ejecutarSimulacion(tPuerto* puerto)
{
    int estadoSimulacion = SIM_EN_CURSO;
    char entrada[256];

    char* tokenCmd;
    char* tokenArg1;
    char* tokenArg2;

    while (estadoSimulacion == SIM_EN_CURSO)
    {
        estadoSimulacion = actualizarEstadoPuerto(puerto);

        if (estadoSimulacion == SIM_EN_CURSO)
        {
            printf("\n[%02d:%02d] OPERADOR> ", puerto->relojActual / 60, puerto->relojActual % 60);

            if (fgets(entrada, sizeof(entrada), stdin) != NULL)
            {
                limpiarSaltoLinea(entrada);
                tokenCmd = strtok(entrada, " ");

                if (tokenCmd != NULL)
                {
                    if (strcmp(tokenCmd, "DES") == 0)
                    {
                        tokenArg1 = strtok(NULL, " ");
                        tokenArg2 = strtok(NULL, " ");
                        ejecutarDES(puerto, tokenArg1, tokenArg2);
                    }
                    else if (strcmp(tokenCmd, "REU") == 0)
                    {
                        tokenArg1 = strtok(NULL, " ");
                        tokenArg2 = strtok(NULL, " ");
                        ejecutarREU(puerto, tokenArg1, tokenArg2);
                    }
                    else if (strcmp(tokenCmd, "ENT") == 0)
                    {
                        ejecutarENT(puerto);
                    }
                    else if (strcmp(tokenCmd, "VER") == 0)
                    {
                        ejecutarVER(puerto);
                    }
                    else if (strcmp(tokenCmd, "ESP") == 0)
                    {
                        ejecutarESP(puerto);
                    }
                    else if (strcmp(tokenCmd, "FIN") == 0)
                    {
                        estadoSimulacion = SIM_FIN_FORZADO;
                    }
                    else
                    {
                        printf("Comando rechazado: Invalido.\n");
                    }
                }
            }
        }
    }

    if (estadoSimulacion == SIM_FIN_NORMAL)
    {
        printf("\n--- JORNADA FINALIZADA NORMALMENTE ---\n");
    }
    else if (estadoSimulacion == SIM_FIN_BLOQUEO)
    {
        printf("\n--- JORNADA FINALIZADA POR BLOQUEO OPERATIVO ---\n");
    }
    else if (estadoSimulacion == SIM_FIN_TIEMPO)
    {
        printf("\n--- JORNADA FINALIZADA POR TIEMPO ---\n");
    }
    else if (estadoSimulacion == SIM_FIN_FORZADO)
    {
        printf("\n--- JORNADA FINALIZADA FORZOSAMENTE ---\n");
    }
}

void mostrarResumenFinal(tPuerto* puerto)
{
    tCamion camion;
    tCola colaAux;
    int cant=0, puntajefinal=0;

    printf("RESUMEN DE LA SIMULACION:\n");
    printf("%d min de %d\n",puerto->relojActual,puerto->duracionJornada);
    printf("Buques descargados: %d -> %d puntos\n",puerto->buquesDescargados,puerto->buquesDescargados*PUNTAJE_BUQUE);
    printf("Contenedores entregados: %d -> %d puntos\n",puerto->contenedoresEntregados,puerto->contenedoresEntregados*PUNTAJE_ENTREGA);
    printf("Reorganizaciones: %d\n",puerto->reubicacionesRealizadas);

    crearCola(&colaAux);

    while(sacarDeCola(&(puerto->colaCamiones),&camion,sizeof(tCamion))==EXITO)
    {
        ponerEnCola(&colaAux,&camion,sizeof(tCamion));
    }
    while(sacarDeCola(&colaAux,&camion,sizeof(tCamion))==EXITO)
    {
        if(camion.tArribo <= puerto->relojActual)
        {
            cant++;
        }
        ponerEnCola(&(puerto->colaCamiones),&camion,sizeof(tCamion));
    }
    printf("Camiones pendientes: %d -> %d puntos de penalizacion\n",cant, cant*PENALIZACION_CAMION);

    puntajefinal=puerto->buquesDescargados*PUNTAJE_BUQUE+puerto->contenedoresEntregados*PUNTAJE_ENTREGA-cant*PENALIZACION_CAMION;

    printf("Puntaje Final: %d\n",puntajefinal);

    vaciarCola(&colaAux);
}

int liberarPuerto(tPuerto* puerto)
{
    tBuque buqueAux;
    tMuelle muelleAux;
    tZona zonaAux;

    if (!puerto)
    {
        return NO_ENCONTRADO;
    }

    vaciarCola(&(puerto->colaCamiones));

    while (sacarDeCola(&(puerto->colaBuques), &buqueAux, sizeof(tBuque)) == EXITO)
    {
        vaciarCola(&(buqueAux.contenedores));
    }

    while (sacarPrimeroLista(&(puerto->muelles), &muelleAux, sizeof(tMuelle)) == EXITO)
    {
        if (muelleAux.ocupado == 1)
        {
            vaciarCola(&(muelleAux.buqueActual.contenedores));
        }
    }

    while (sacarPrimeroLista(&(puerto->zonas), &zonaAux, sizeof(tZona)) == EXITO)
    {
        vaciarPila(&(zonaAux.pilaContenedores));
    }


    return EXITO;
}

void ejecutarDES(tPuerto* puerto, char* idMuelle, char* idZona)
{
    tMuelle muelleAux;
    tZona zonaAux;
    tContenedor auxContenedor;

    if (!idMuelle || !idZona)
    {
        printf("Rechazado: Faltan parametros (DES <muelle> <zona>).\n");
        return;
    }


    sscanf(idZona,"Z%d",&(zonaAux.idZona));
    sscanf(idMuelle,"M%d",&(muelleAux.idMuelle));


    if(sacarClaveLista(&(puerto->muelles),&muelleAux,sizeof(tMuelle), compararMuelle)!= EXITO)
    {
        printf("No se encontro  muelle: [%s] \n",idMuelle);
        return;
    }

    if(sacarClaveLista(&(puerto->zonas),&zonaAux,sizeof(tZona), compararZona)!=EXITO)
    {
        ponerEnOrden(&(puerto->muelles),&muelleAux,sizeof(tMuelle),compararMuelle,NULL);
        printf("No se encontro  zona: [%s] \n",idZona);
        return;

    }
    if(muelleAux.ocupado==1)
    {
        if(!colaVacia(&(muelleAux.buqueActual.contenedores)) )
        {

            if(zonaAux.cantidadActual<zonaAux.capacidadMaxima)
            {
                sacarDeCola(&(muelleAux.buqueActual.contenedores),&auxContenedor,sizeof(tContenedor));
                ponerEnPila(&(zonaAux.pilaContenedores),&auxContenedor,sizeof(tContenedor));
                zonaAux.cantidadActual++;

                puerto->relojActual+=puerto->tiempoDescarga;
            }
            else
                printf("Z%d esta llena\n",zonaAux.idZona);

        }
        else
            printf("Buque atracado en M%d vacio\n",muelleAux.idMuelle);

    }
    else
        printf("M%d desocupado\n",muelleAux.idMuelle);

    ponerEnOrden(&(puerto->muelles),&muelleAux,sizeof(tMuelle),compararMuelle,NULL);
    ponerEnOrden(&(puerto->zonas),&zonaAux,sizeof(tZona),compararZona,NULL);

}



void ejecutarREU(tPuerto* puerto, char* idZonaOrigen, char* idZonaDestino)
{

    tZona zonaOrigen, zonaDestino;
    tContenedor auxContenedor;

    if (!idZonaOrigen || !idZonaDestino)
    {
        printf("Rechazado: Faltan parametros (REU <origen> <destino>).\n");
        return;
    }

    sscanf(idZonaOrigen,"Z%d",&(zonaOrigen.idZona));
    sscanf(idZonaDestino,"Z%d",&(zonaDestino.idZona));


    if( sacarClaveLista(&(puerto->zonas),&zonaOrigen,sizeof(tZona), compararZona) != EXITO)
    {
        printf("No se encontro  zona origen: [%s] \n",idZonaOrigen);
        return;
    }

    if(sacarClaveLista(&(puerto->zonas),&zonaDestino,sizeof(tZona), compararZona)!=EXITO)
    {
        ponerEnOrden(&(puerto->zonas),&zonaOrigen,sizeof(tZona),compararZona,NULL);
        printf("No se encontro  zona destino: [%s] \n",idZonaDestino);
        return;
    }

    if(zonaOrigen.cantidadActual > 0)
    {
        if(zonaDestino.cantidadActual<zonaDestino.capacidadMaxima)
        {
            sacarDePila(&zonaOrigen.pilaContenedores,&auxContenedor,sizeof(tContenedor));
            zonaOrigen.cantidadActual--;

            ponerEnPila(&(zonaDestino.pilaContenedores),&auxContenedor,sizeof(tContenedor));
            zonaDestino.cantidadActual++;



            puerto->relojActual+=puerto->tiempoReubicacion;
            puerto->reubicacionesRealizadas++;
        }
        else
            printf("Rechazado: Z%d destino llena",zonaDestino.idZona);


    }
    else
        printf("Rechazado: Z%d origen vacia",zonaOrigen.idZona);

    ponerEnOrden(&(puerto->zonas),&zonaOrigen,sizeof(tZona),compararZona,NULL);
    ponerEnOrden(&(puerto->zonas),&zonaDestino,sizeof(tZona),compararZona,NULL);




}

void ejecutarENT(tPuerto* puerto)
{
    tCamion camionAuxiliar;
    tZona zonaAux;
    tContenedor entregado;
    int posicion=NO_ENCONTRADO;
    if(verPrimeroCola(&puerto->colaCamiones,&camionAuxiliar,sizeof(tCamion))==COLA_VACIA)
        printf("No hay camiones\n");


    if( (posicion= busqClaveLista(&(puerto->zonas),compararEntrega,&camionAuxiliar.contenedorPedido)) == NO_ENCONTRADO)
    {
        printf("No hay entrega Valida para el Camion: [%s] \n",camionAuxiliar.id);
        return;
    }
    sacarPosLista(&(puerto->zonas),&zonaAux,sizeof(tZona),posicion);
    sacarDePila(&(zonaAux.pilaContenedores),&entregado,sizeof(tContenedor));
    zonaAux.cantidadActual--;
    ponerEnOrden(&(puerto->zonas),&zonaAux,sizeof(tZona),compararZona,NULL);
    sacarDeCola(&(puerto->colaCamiones),&camionAuxiliar,sizeof(tCamion));

    puerto->relojActual+=puerto->tiempoCarga;
    puerto->contenedoresEntregados++;

}

void ejecutarVER(tPuerto* puerto)
{

    int puntuacion=0, cantidadCamiones;
    printf("Tiempo Actual: %d\n",puerto->relojActual);
    mapLista(&(puerto->muelles),mostrarBuqueAtracado,NULL);
    mapLista(&(puerto->zonas),mostrarContenedorZona,NULL);
    puts("");
    cantidadCamiones = mostrarTresCamiones(&(puerto->colaCamiones), puerto->relojActual);
    mostrarBuquesEspera(&(puerto->colaBuques), puerto->relojActual);
    puntuacion=puerto->buquesDescargados*PUNTAJE_BUQUE+puerto->contenedoresEntregados*PUNTAJE_ENTREGA-cantidadCamiones*PENALIZACION_CAMION;
    printf("Puntaje Actual: %d",puntuacion);

}

void ejecutarESP(tPuerto* puerto)
{
    puerto->relojActual++;
}



