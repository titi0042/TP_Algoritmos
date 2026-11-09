#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pila.h"
#include "cola.h"
#include "lista.h"
#include "simulacion.h"

void limpiarSaltoLinea(char* cadena)
{
    char* ptr = cadena;

    while (*ptr != '\0' && *ptr != '\n' && *ptr != '\r')
    {
        ptr++;
    }
    *ptr = '\0';
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

    /// muelleConCarga && zonaConEspacio = Puede DES
    /// zonaConEspacio && zonaNoVacia = Puede DES y REU
    /// zonaConEspacio && !zonaNoVacia = Puede DES
    /// !zonaConEspacio && zonaNoVacia = No puede DES ni REU
    /// !zonaConEspacio && !zonaNoVacia = Caso no posible
    /// topeCoincide = Puede ENT

    tNodoLista* nMuelle = puerto->muelles;
    tNodoLista* nZona;
    tMuelle* muelle;
    tZona* zona;
    tCamion camionFrente;
    tContenedor contTope;


    while (nMuelle != NULL && muelleConCarga == 0)
    {
        muelle = (tMuelle*)(nMuelle->info);
        if (muelle->ocupado == 1)
        {
            if (colaVacia(&(muelle->buqueActual.contenedores)) == 0)
            {
                muelleConCarga = 1;
            }
        }
        nMuelle = nMuelle->sig;
    }

    nZona = puerto->zonas;
    while (nZona != NULL)
    {
        zona = (tZona*)(nZona->info);
        if (zona->cantidadActual < zona->capacidadMaxima)
        {
            zonaConEspacio = 1;
        }
        if (zona->cantidadActual > 0)
        {
            zonaNoVacia = 1;
        }
        nZona = nZona->sig;
    }

    if (muelleConCarga && zonaConEspacio)
    {
        puedeDES = 1;
    }

    if (zonaNoVacia && zonaConEspacio)
    {
        nZona = puerto->zonas;
        while (nZona != NULL && puedeREU == 0)
        {
            zona = (tZona*)(nZona->info);
            if (zona->cantidadActual > 0)
            {
                tNodoLista* nZonaDest = puerto->zonas;
                while (nZonaDest != NULL && puedeREU == 0)
                {
                    tZona* zonaDest = (tZona*)(nZonaDest->info);
                    if (zonaDest->idZona != zona->idZona && zonaDest->cantidadActual < zonaDest->capacidadMaxima)
                    {
                        puedeREU = 1;
                    }
                    nZonaDest = nZonaDest->sig;
                }
            }
            nZona = nZona->sig;
        }
    }

    if (verPrimeroCola(&(puerto->colaCamiones), &camionFrente, sizeof(tCamion)) == EXITO)
    {
        if (camionFrente.tArribo <= puerto->relojActual)
        {
            nZona = puerto->zonas;
            while (nZona != NULL && topeCoincide == 0)
            {
                zona = (tZona*)(nZona->info);
                if (zona->cantidadActual > 0)
                {
                    if (verTope(&(zona->pilaContenedores), &contTope, sizeof(tContenedor)) == EXITO)
                    {
                        if (strcmp(contTope.codigo, camionFrente.contenedorPedido) == 0)
                        {
                            topeCoincide = 1;
                        }
                    }
                }
                nZona = nZona->sig;
            }
            if (topeCoincide) puedeENT = 1;
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
    tNodoLista* nMuelle;
    tMuelle* muelle;
    tBuque buqueFrente;
    tCamion camionFrente;

    int operacionesValidas = 0;
    int minEventoFuturo = 999999;
    int finNormal = 1;

    if (puerto->relojActual >= puerto->duracionJornada)
    {
        return SIM_FIN_TIEMPO;
    }

    /// Liberar Muelles sin carga
    nMuelle = puerto->muelles;
    while (nMuelle != NULL)
    {
        muelle = (tMuelle*)(nMuelle->info);
        if (muelle->ocupado == 1)
        {
            if (colaVacia(&(muelle->buqueActual.contenedores)) != 0)
            {
                muelle->ocupado = 0;
                puerto->buquesDescargados++;
            }
            else
            {
                finNormal = 0;
            }
        }
        nMuelle = nMuelle->sig;
    }

    /// Asignacion automatico de buques
    nMuelle = puerto->muelles;
    while (nMuelle != NULL)
    {
        muelle = (tMuelle*)(nMuelle->info);
        if (muelle->ocupado == 0)
        {
            if (verPrimeroCola(&(puerto->colaBuques), &buqueFrente, sizeof(tBuque)) == EXITO)
            {
                if (buqueFrente.tArribo <= puerto->relojActual)
                {
                    sacarDeCola(&(puerto->colaBuques), &buqueFrente, sizeof(tBuque));
                    muelle->buqueActual = buqueFrente;
                    muelle->ocupado = 1;

                    if (colaVacia(&(muelle->buqueActual.contenedores)) == 0)
                    {
                        finNormal = 0;
                    }
                }
            }
        }
        nMuelle = nMuelle->sig;
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

    operacionesValidas = evaluarOperacionesValidas(puerto);
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
    if (!idMuelle || !idZona)
    {
        printf("Rechazado: Faltan parametros (DES <muelle> <zona>).\n");
    }
}

void ejecutarREU(tPuerto* puerto, char* idZonaOrigen, char* idZonaDest)
{
    if (!idZonaOrigen || !idZonaDest)
    {
        printf("Rechazado: Faltan parametros (REU <origen> <destino>).\n");
    }
}

void ejecutarENT(tPuerto* puerto)
{

}

void ejecutarVER(tPuerto* puerto)
{

}

void ejecutarESP(tPuerto* puerto)
{
    puerto->relojActual++;
}
