#include <string.h>
#define ARCHIVO_CONFIG "simulacion/config.txt"
#define ARCHIVO_PUERTO "simulacion/puerto.txt"
#include "menu.h"
#include <stdlib.h>

int menu()
{
    tArbol indiceOperadores;
    tOperador operadorActual;
    long offsetActual;

    tConfiguracion config;
    tPuerto puerto;
    tJornada jornada;

    char nombreIngresado[50];
    int opcion = -1;
    int login = 0;
    int resLogin;
    int camionesPendientes = 0;
    int motivo;


    crearArbol(&indiceOperadores);
    if (levantarIndice(ARCHIVO_INDICE, &indiceOperadores) != EXITO)
    {
        printf("Error al inicializar el archivo de indice\n");
        return ERR_ARCH;
    }



    while (login == 0)
    {
        printf("=========================================\n");
        printf("  PUERTO DE CONTENEDORES: CONTRARRELOJ  \n");
        printf("=========================================\n");
        printf("Ingrese su nombre de operador: ");

        if (fgets(nombreIngresado, sizeof(nombreIngresado), stdin) != NULL)
        {
            limpiarSaltoLinea(nombreIngresado);
            if (*nombreIngresado != '\0')
            {
                resLogin = loginOperador(nombreIngresado, &operadorActual, &offsetActual, &indiceOperadores);
                if (resLogin == EXITO)
                {
                    login = 1;
                }
                else if (resLogin == ERR_ARCH)
                {
                    printf("Error critico al acceder al archivo de operadores\n");
                    eliminarArbol(&indiceOperadores);
                    return ERR_ARCH;
                }

            }
        }
    }
    while (opcion != 0)
    {
        printf("\n=========================================\n");
        printf("             MENU PRINCIPAL              \n");
        printf("  Operador activo: %s\n", operadorActual.nombre);
        printf("=========================================\n");
        printf("1. Iniciar nueva jornada\n");
        printf("2. Ver ranking de operadores\n");
        printf("3. Ver historial de jornadas\n");
        printf("4. Cambiar de operador\n");
        printf("0. Salir\n");
        printf("=========================================\n");
        printf("Seleccione una opcion: ");

        if (scanf("%d", &opcion) != 1)
        {
            opcion = -1;
        }
        while (getchar() != '\n');

        switch (opcion)
        {
        case 1:

        system("cls");
            if (cargarConfiguracion(ARCHIVO_CONFIG, &config) != EXITO)
            {
                printf("Error: No se pudo cargar config.txt\n");
                break;
            }
            if (generarEscenario(ARCHIVO_PUERTO, &config) != EXITO)
            {
                printf("Error: No se pudo generar puerto.txt\n");
                break;
            }
            if (inicializarPuerto(&puerto, ARCHIVO_PUERTO, &config) == EXITO)
            {
                motivo = ejecutarSimulacion(&puerto);
                mostrarResumenFinal(&puerto);

                camionesPendientes = contarCamionesPendientes(&puerto);
                strcpy(jornada.nombreOperador, operadorActual.nombre);
                jornada.buquesDescargados = puerto.buquesDescargados;
                jornada.contenedoresEntregados = puerto.contenedoresEntregados;
                jornada.reubicacionesRealizadas = puerto.reubicacionesRealizadas;
                jornada.camionesPendientes = camionesPendientes;
                jornada.tiempoUtilizado = puerto.relojActual;
                jornada.puntajeFinal = puerto.buquesDescargados * PUNTAJE_BUQUE +
                                       puerto.contenedoresEntregados * PUNTAJE_ENTREGA -
                                       camionesPendientes * PENALIZACION_CAMION;

                if (motivo == SIM_FIN_NORMAL)
                    strcpy(jornada.motivoFinalizacion, "Normal");
                else if (motivo == SIM_FIN_BLOQUEO)
                    strcpy(jornada.motivoFinalizacion, "Bloqueo");
                else if (motivo == SIM_FIN_TIEMPO)
                    strcpy(jornada.motivoFinalizacion, "Tiempo");
                else
                    strcpy(jornada.motivoFinalizacion, "Forzado");

                guardarJornadaBD(&jornada);
                operadorActual.jornadasJugadas++;
                operadorActual.puntajeTotal += jornada.puntajeFinal;
                actualizarOperador(&operadorActual, offsetActual);

                liberarPuerto(&puerto);
            }
            break;

        case 2:
            mostrarRankingOperadores(&indiceOperadores);
            break;

        case 3:
            mostrarHistorialJornadas();
            break;

        case 4:
            login = 0;
            while (login == 0)
            {
                printf("\nIngrese el nuevo nombre de operador: ");
                if (fgets(nombreIngresado, sizeof(nombreIngresado), stdin) != NULL)
                {
                    limpiarSaltoLinea(nombreIngresado);
                    if (*nombreIngresado != '\0')
                    {
                        resLogin = loginOperador(nombreIngresado, &operadorActual, &offsetActual, &indiceOperadores);
                        if (resLogin == EXITO)
                        {
                            login = 1;
                        }
                    }
                }
            }
            break;

        case 0:
            printf("\nGuardando indice de operadores y saliendo del sistema...\n");
            guardarIndiceOrdenado(ARCHIVO_INDICE, &indiceOperadores);
            eliminarArbol(&indiceOperadores);
            break;

        default:
            printf("\nOpcion invalida. Por favor, seleccione un numero del menu.\n");
            break;
        }



        pausarPantalla();
        system("cls");
    }

    return EXITO;
}
