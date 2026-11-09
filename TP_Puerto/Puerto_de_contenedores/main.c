#include <stdio.h>
#include <stdlib.h>
#include "comunes.h"
#include "archivos.h"

int main()
{
    tConfiguracion config;
    int estado;

    printf("Iniciando simulacion\n");
    estado = cargarConfiguracion("config.txt", &config);
    if (estado != EXITO)
    {
        printf("no existe archivo config.txt\n");
        return ERR_ARCH;
    }
    printf("config.txt cargado exitosamente.\n");


    estado = generarEscenario("puerto.txt", &config);
    if (estado != EXITO)
    {
        return ERR_ARCH;
    }
    printf("puerto.txt generado exitosamente\n");



    return EXITO;
}
