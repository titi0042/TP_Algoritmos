# Puerto de Contenedores 

**Universidad Nacional de La Matanza (UNLaM)**
**Materia:** Algoritmos y Estructuras de Datos [Com:3300]
**Trabajo Práctico:** Operación Contrarreloj 

##  Equipo de Desarrollo: Tanenbaum
* **Zacarías, Franco Hernán** (DNI: 46.422.064)
* **Luna Gauna, Thiago Gonzalo** (DNI: 45.738.676)

---

## Descripción del Proyecto
Este proyecto es una simulación interactiva desarrollada en lenguaje C que modela la jornada de máxima actividad en un puerto de carga. El sistema gestiona el arribo de buques y camiones mediante un reloj lógico, y el usuario asume el rol de **Operador del Puerto**, tomando decisiones estratégicas para descargar, reubicar y entregar contenedores intentando maximizar la puntuación antes de que finalice el tiempo.

### Características Técnicas y TDA Utilizados
* **Pilas Dinámicas:** Gestión del almacenamiento de contenedores en las zonas con lógica LIFO.
* **Colas Dinámicas:** Administración del orden de llegada (FIFO) para buques esperando muelles y camiones esperando atención.
* **Listas Simplemente Enlazadas:** Manejo dinámico de las zonas de almacenamiento y los muelles.
* **Árbol Binario de Búsqueda (ABB):** Implementación de un índice persistido en disco para la búsqueda optimizada (logarítmica) y login de los operadores del sistema.
* **Archivos Genéricos:** Procesamiento de archivos de texto (`config.txt`, `puerto.txt`) y persistencia binaria de jornadas y perfiles de usuario (`operadores.dat`, `indice_operadores.idx`, `jornadas.dat`).
* **Ubicación de Archivos:** Los archivos `config.txt` y `puerto.txt` se encuentra ubicados en la carpeta simulacion.
---


## Comandos del Operador
Durante la simulación, el tiempo no avanza en tiempo real, sino en función de las operaciones confirmadas. El operador cuenta con la siguiente consola de comandos:

* `DES M<id> Z<id>`: **Descargar.** Mueve el próximo contenedor del buque atracado en un muelle hacia una zona de almacenamiento.
* `REU Z<origen> Z<destino>`: **Reubicar.** Traslada el contenedor ubicado en el tope de una zona hacia otra para despejar los contenedores bloqueados.
* `ENT`: **Entregar.** Atiende al camión ubicado en el frente de la cola, entregándole su contenedor solicitado si este se encuentra en el tope de alguna pila.
* `VER`: **Ver Estado.** Despliega el tablero de muelles, zonas, camiones en espera y puntuación temporal (No consume tiempo lógico).
* `ESP`: **Esperar.** Avanza el reloj lógico 1 minuto de forma manual.
* `FIN`: **Finalización Forzada.** Termina la jornada antes de tiempo.

## Sistema de Puntuación
Al finalizar la jornada (por tiempo, normal, o bloqueo operativo), el sistema calcula el puntaje que se acumulará en el perfil del operador:
* **+10 puntos** por cada contenedor entregado.
* **+5 puntos** por cada buque que completa su manifiesto de descarga.
* **-2 puntos** de penalización por cada camión que quedó esperando atención en la cola.# TP_Algoritmos
