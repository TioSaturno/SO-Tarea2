# Simulación de Combate Doom - Sistema Operativos Tarea 2

## Descripción

Este proyecto es una simulación de combate en tiempo real donde múltiples héroes navegan por un tablero siguiendo rutas predefinidas mientras enfrentan monstruos que los detectan y atacan. La simulación utiliza **threads POSIX (pthreads)** para simular el comportamiento concurrente de cada entidad (héroes y monstruos) en el juego.

## Características

### Héroes
- Se mueven siguiendo un **path predefinido** (secuencia de coordenadas)
- Pueden **atacar monstruos** dentro de su rango de ataque
- Tienen **HP** (puntos de vida), **daño de ataque** y **rango de ataque**
- Cada héroe corre en su propio **thread**
- Se detienen cuando completan su path o mueren

### Monstruos
- Permanecen en su posición inicial hasta **detectar** un héroe cercano
- Tienen un **rango de visión** para detectar héroes
- Cuando detectan un héroe, **alertan** a otros monstruos cercanos
- **Persiguen** al héroe más cercano usando distancia Manhattan
- **Atacan** cuando el héroe está dentro de su rango de ataque
- Cada monstruo corre en su propio **thread**
- Se detienen cuando mueren o cuando todos los héroes completan su camino

### Herramientas usadas
- **Distancia Manhattan**: Usada para el movimiento (monstruos se mueven un paso a la vez)
- **Distancia Euclidiana**: Usada para determinar el héroe más cercano
- **Sincronización con Mutex**: Para evitar condiciones de carrera en combate y movimiento
- **Sistema de alerta**: Los monstruos alertan a otros monstruos cercanos cuando detectan un héroe

## Requisitos

- **Sistema Operativo**: macOS, Linux o Unix
- **Compilador**: GCC con soporte para C99 o superior
- **Bibliotecas**: 
  - `pthread` (threads POSIX)
  - `math` (funciones matemáticas)

## Estructura de Archivos

```
Tarea2/
├── Doom.c              # Código fuente principal
├── ejemplo1.txt        # Configuración con 1 héroe y 24 monstruos
├── ejemplo2.txt        # Configuración con 3 héroes y 24 monstruos
├── ejemplo3.txt        # Configuración con 2 héroes y 12 monstruos
└── README.md          # Este archivo
```

## Formato de Archivo de Configuración

El archivo de configuración define el tablero, los héroes y los monstruos. Soporta dos formatos:

### Formato 1: Un solo héroe (ejemplo1.txt)
```
GRID_SIZE <ancho> <alto>
HERO_HP <puntos_vida>
HERO_ATTACK_DAMAGE <daño>
HERO_ATTACK_RANGE <rango>
HERO_START <x> <y>
HERO_PATH (x1,y1) (x2,y2) (x3,y3) ...
(x4,y4) (x5,y5) ...

MONSTER_COUNT <número>
MONSTER_1_HP <puntos_vida>
MONSTER_1_ATTACK_DAMAGE <daño>
MONSTER_1_VISION_RANGE <rango_visión>
MONSTER_1_ATTACK_RANGE <rango_ataque>
MONSTER_1_COORDS <x> <y>
...
```

### Formato 2: Múltiples héroes (ejemplo2.txt, ejemplo3.txt)
```
GRID_SIZE <ancho> <alto>

HERO_1_HP <puntos_vida>
HERO_1_ATTACK_DAMAGE <daño>
HERO_1_ATTACK_RANGE <rango>
HERO_1_START <x> <y>
HERO_1_PATH (x1,y1) (x2,y2) (x3,y3) ...
(x4,y4) (x5,y5) ...

HERO_2_HP <puntos_vida>
HERO_2_ATTACK_DAMAGE <daño>
...

MONSTER_COUNT <número>
MONSTER_1_HP <puntos_vida>
...
```



## Compilación

### Opción 1: Compilación básica
```bash
gcc -o doom Doom.c -lpthread -lm
```

### Opción 2: Compilación con flags de depuración
```bash
gcc -Wall -Wextra -g3 Doom.c -o doom -lpthread -lm
```

### Explicación de flags:
- `-Wall`: Activa todos los warnings comunes
- `-Wextra`: Activa warnings adicionales
- `-g3`: Incluye información de depuración completa
- `-lpthread`: Enlaza la biblioteca de threads POSIX
- `-lm`: Enlaza la biblioteca matemática
- `-o doom`: Especifica el nombre del ejecutable


## Ejecución

### Sintaxis básica
```bash
./doom <archivo_configuracion>
```

### Ejemplos
```bash
# Ejecutar con ejemplo 1 (1 héroe, 24 monstruos)
./doom ejemplo1.txt

# Ejecutar con ejemplo 2 (3 héroes, 24 monstruos)
./doom ejemplo2.txt

# Ejecutar con ejemplo 3 (2 héroes, 12 monstruos)
./doom ejemplo3.txt
```
### Para agregar otras pruebas
si usted desea probar el proyecto con un archivo de configuración distinto debe cumplir lo siguiente:
- El archivo debe tener el mismo formato que tienen los 3 ejemplos dejados
- Debe asegurarse que este archivo esté en el mismo directorio en el cual están todos los demás
- Al momento de realizar la ejecución, debe escribir el nombre del archivo seguido de este como se mostró anteriormente

## Salida de la Simulación

### Al inicio:
```
=== Configuración Cargada ===
Grid: 60x40
Héroes: 3
  Héroe 1: HP=220, Daño=25, Rango=3, Inicio=(3,3), Path=65 puntos
  Héroe 2: HP=200, Daño=22, Rango=2, Inicio=(10,5), Path=49 puntos
  Héroe 3: HP=250, Daño=28, Rango=4, Inicio=(5,10), Path=40 puntos
Monstruos: 24
  Monstruo 1: HP=60, Daño=10, VisiónRango=6, AtaqueRango=1, Pos=(12,6)
  ...
=============================
```

### Durante la simulación:
```
Heroe 0 se mueve a (4, 3)
Monstruo 0 ha sido alertado al ver al Heroe 0 cercano.
Monstruo 5 ha sido alertado por Monstruo 0
Heroe 0 ataca Monstruo 0, HP Monstruo: 35
Monstruo 0 se mueve a (11, 5)
Monstruo 0 ataca Heroe 0 (-10 HP). HP Heroe: 210
Monstruo 0 ha sido derrotado por Heroe 0
```

### Al finalizar:
```
Heroe 0 ha completado su camino.
Monstruo 5 se detiene (no hay héroes activos).
...
=== Simulación Terminada ===
```

## Arquitectura del Código

### Estructuras de Datos

#### `Point`
```c
typedef struct {
    int x;
    int y;
} Point;
```

#### `Heroe`
```c
typedef struct {
    int HP;
    int ATCK_DAMAGE;
    int ATCK_RANGE;
    Point position;
    Point *Path;
    int PathLength;
    int currentPathIndex;
    bool isAlive;
    bool in_combat;
    pthread_t thread_id;
} Heroe;
```

#### `Monster`
```c
typedef struct {
    int HP;
    int ATCK_DAMAGE;
    int VISION_RANGE;
    int ATCK_RANGE;
    Point START;
    bool Alerted;
    bool alive;
    pthread_t thread_id;
} Monster;
```

### Funciones Principales

#### `void *Hero(void *arg)`
Thread principal de cada héroe. Controla:
- Movimiento a lo largo del path
- Detección de monstruos en rango
- Ataque a monstruos
- Gestión del estado de vida

#### `void *Mounstro(void *arg)`
Thread principal de cada monstruo. Controla:
- Detección de héroes cercanos
- Sistema de alerta a otros monstruos
- Persecución del héroe más cercano
- Ataque cuando está en rango

#### `void ParseConfig(const char *filename)`
Lee y procesa el archivo de configuración:
- Parsea el tamaño del grid
- Carga datos de héroes (HP, daño, paths, etc.)
- Carga datos de monstruos
- Soporta paths multi-línea

#### `int manhattanDistance(Point a, Point b)`
Calcula la distancia Manhattan entre dos puntos (usada para movimiento).

#### `double euclideanDistance(Point a, Point b)`
Calcula la distancia Euclidiana entre dos puntos (usada para encontrar el más cercano).

#### `void alert_nearby_monsters(int monster_id)`
Alerta a los monstruos cercanos cuando uno detecta un héroe.

### Sincronización

El programa usa dos mutex para evitar condiciones de carrera:

```c
pthread_mutex_t grid_mutex;      // Para acceso al tablero y posiciones
pthread_mutex_t combat_mutex;    // Para operaciones de combate
```

## Flujo de Ejecución

1. **Inicialización**
   - Lee argumentos de línea de comandos
   - Carga configuración del archivo
   - Asigna memoria para héroes y monstruos

2. **Creación de Threads**
   - Crea un thread por cada héroe
   - Crea un thread por cada monstruo

3. **Simulación Concurrente**
   - Héroes se mueven por su path
   - Monstruos patrullan/persiguen
   - Se ejecutan combates
   - Sincronización con mutex

4. **Finalización**
   - Espera a que terminen todos los threads de héroes
   - Espera a que terminen todos los threads de monstruos
   - Libera memoria
   - Imprime resumen final

## Condiciones de Terminación

La simulación termina cuando:
- **Todos los héroes** completan su path o mueren
- Los monstruos detectan que no hay héroes activos y se detienen
- Todos los threads finalizan correctamente

## Solución de Problemas

### Error: "command not found"
Asegúrate de estar en el directorio correcto:
```bash
cd /Users/tiosaturno/Desktop/Universidad/SO/Tarea2
```

### Error: "Failed to open config file"
Verifica que el archivo de configuración existe:
```bash
ls ejemplo*.txt
```

### Error de compilación con pthread
En algunos sistemas Linux, asegúrate de incluir `-pthread`:
```bash
gcc -o doom Doom.c -pthread -lm
```

### Deadlock o programa no termina
Esto puede ocurrir si hay problemas de sincronización. Ejecuta con `Ctrl+C` para detener y revisa los logs.


