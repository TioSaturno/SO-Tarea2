#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>    
typedef struct {
    int x;
    int y; 
} Point;
typedef struct{
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
typedef struct{
    int HP;
    int ATCK_DAMAGE;
    int VISION_RANGE;  
    int ATCK_RANGE;
    Point START;
    bool Alerted;
    bool alive;
    pthread_t thread_id;
} Monster;
typedef struct{
    int width;
    int height;
    
} Grid;

Grid tablero;
Heroe *heroes;
int hero_count = 0;
Monster *monsters;
int monster_count = 0;
pthread_mutex_t grid_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t combat_mutex = PTHREAD_MUTEX_INITIALIZER;

int manhattanDistance(Point a, Point b) {
    return abs(a.x - b.x) + abs(a.y - b.y);
}
//esto lo ocupare para buscar los heroes o monstruos mas cercanos en general, siempre para moverse se usara la distacnia de manhattan
double euclideanDistance(Point a, Point b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}
//aqui va lo del heroe
void *Hero(void *arg){
    int hero_id = *((int *)arg);
    free(arg);

    Heroe *h= &heroes[hero_id];
    while (h->isAlive && h->currentPathIndex < h->PathLength) {
        usleep(200000);

        if(!h->isAlive){
            printf("Heroe %d ha muerto.\n", hero_id);
            pthread_exit(NULL);
            break;
        } 

        
        bool monsters_in_range=false;
        
        pthread_mutex_lock(&grid_mutex);
        for(int i=0;i<monster_count;i++){
            if(!monsters[i].alive) continue;
            int dist=manhattanDistance(h->position,monsters[i].START);
            if(dist<=h->ATCK_RANGE){
                monsters_in_range=true;
                break;
          }
         }
    pthread_mutex_unlock(&grid_mutex);

    if(monsters_in_range){
        h->in_combat=true;
        
        for(int i=0;i<monster_count;i++){
            if(!monsters[i].alive) continue;

            pthread_mutex_lock(&grid_mutex);
            int dist=manhattanDistance(h->position,monsters[i].START);
            pthread_mutex_unlock(&grid_mutex);

            if(dist<=h->ATCK_RANGE){
                pthread_mutex_lock(&combat_mutex);
                if(monsters[i].alive){
                monsters[i].HP-=h->ATCK_DAMAGE;
                printf("Heroe %d ataca Monstruo %d, HP Monstruo: %d\n",hero_id,i,monsters[i].HP);
                if(monsters[i].HP<=0){
                    monsters[i].alive=false;
                    printf("Monstruo %d ha sido derrotado por Heroe %d\n",i,hero_id);
                }
            }
             pthread_mutex_unlock(&combat_mutex);
             usleep(150000);
        }
    }
 }else{
        h->in_combat=false;

        if(h->currentPathIndex < h->PathLength){
            pthread_mutex_lock(&grid_mutex);
            h->position = h->Path[h->currentPathIndex];
            h->currentPathIndex++;
            printf("Heroe %d se mueve a (%d, %d)\n", hero_id, h->position.x, h->position.y);
            
            pthread_mutex_unlock(&grid_mutex);
       }
      }  
     } 
     if(h->isAlive){
        printf("Heroe %d ha completado su camino.\n", hero_id);
    }
    return NULL;
  }
//aqui va lo del monstruo
Point get_next_position_to_hero(Point monster_pos, Point hero_pos) {
    Point next_pos = monster_pos;
    int dx = hero_pos.x - monster_pos.x;
    int dy = hero_pos.y - monster_pos.y;
    if(abs(dx > abs(dy))) {
        next_pos.x += (dx > 0) ? 1 : -1;
    } else if(dy != 0) {
        next_pos.y += (dy > 0) ? 1 : -1;
    }

  return next_pos;

}
void alert_nearby_monsters(int monster_id){
    Monster *alerting = &monsters[monster_id];
    for(int i=0; i<monster_count; i++){
        if(i == monster_id || !monsters[i].alive) continue;
        int dist = manhattanDistance(alerting->START, monsters[i].START);
        if(dist <= alerting->VISION_RANGE && !monsters[i].Alerted){
            pthread_mutex_lock(&grid_mutex);
            monsters[i].Alerted = true;
            pthread_mutex_unlock(&grid_mutex);
            printf("Monstruo %d ha sido alertado por Monstruo %d\n", i, monster_id);
    }
  }
}
void *Mounstro(void *arg){
    int monster_id = *((int *)arg);
    free(arg);

    Monster *m = &monsters[monster_id];

    while(m->alive){
     usleep(300000);
     if(!m->alive){
         printf("Monstruo %d ha muerto.\n", monster_id);
         pthread_exit(NULL);
         break;
    }
    Heroe *closest_hero = NULL;
    double min_dist = INFINITY;
    pthread_mutex_lock(&grid_mutex);
    for(int i=0; i<hero_count; i++){
        if(!heroes[i].isAlive) continue;
        double dist = euclideanDistance(m->START, heroes[i].position);
        if(dist < min_dist){
            min_dist = dist;
            closest_hero = &heroes[i];
        }
    }
    pthread_mutex_unlock(&grid_mutex);


    if(!closest_hero){
        continue;
    }
    if(!m->Alerted && min_dist<= m->VISION_RANGE){
        pthread_mutex_lock(&grid_mutex);
        m->Alerted = true;
        pthread_mutex_unlock(&grid_mutex);
        printf("Monstruo %d ha sido alertado al ver al Heroe cercano.\n", monster_id);
        alert_nearby_monsters(monster_id);

    }

    if(m->Alerted){
        int dist_to_hero = manhattanDistance(m->START, closest_hero->position);
          if(dist_to_hero <= m->ATCK_RANGE){
            pthread_mutex_lock(&combat_mutex);
            
            if(closest_hero->isAlive){
                closest_hero->HP -= m->ATCK_DAMAGE;
                printf("Monstruo %d ataca Heroe (-%d HP). HP Heroe: %d\n", monster_id, m->ATCK_DAMAGE, closest_hero->HP);
                
                
                if(closest_hero->HP <= 0){
                    closest_hero->isAlive = false;
                    printf("Heroe ha sido derrotado por Monstruo %d\n", monster_id);
                }
            }
            pthread_mutex_unlock(&combat_mutex);
          }
          else{
            pthread_mutex_lock(&grid_mutex);
            Point next_pos = get_next_position_to_hero(m->START, closest_hero->position);
            m->START = next_pos;
            printf("Monstruo %d se mueve a (%d, %d)\n", monster_id, m->START.x, m->START.y);
            pthread_mutex_unlock(&grid_mutex);
          }
        }
  }
  return NULL;
}
void ParseConfig(const char *filename){
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error al abrir el archivo de configuración");
        exit(EXIT_FAILURE);
    }

    char line_buffer[1024];
    
    // Primera pasada: contar héroes y monstruos
    int temp_hero_count = 0;
    int temp_monster_count = 0;
    
    while(fgets(line_buffer, sizeof(line_buffer), file)) {
        // Contar héroes
        if (strstr(line_buffer, "HERO_") && strstr(line_buffer, "_HP")) {
            int hero_num;
            if (sscanf(line_buffer, "HERO_%d_HP", &hero_num) == 1) {
                if (hero_num > temp_hero_count) {
                    temp_hero_count = hero_num;
                }
            } else if (strncmp(line_buffer, "HERO_HP", 7) == 0) {
                temp_hero_count = 1; // Un solo héroe sin número
            }
        }
        
        // Contar monstruos
        if (strncmp(line_buffer, "MONSTER_COUNT", 13) == 0) {
            sscanf(line_buffer + 14, "%d", &temp_monster_count);
        }
    }
    
    hero_count = temp_hero_count;
    monster_count = temp_monster_count;
    
    // Asignar memoria
    heroes = (Heroe *)malloc(sizeof(Heroe) * hero_count);
    monsters = (Monster *)malloc(sizeof(Monster) * monster_count);
    
    // Inicializar estructuras
    for (int i = 0; i < hero_count; i++) {
        heroes[i].HP = 0;
        heroes[i].ATCK_DAMAGE = 0;
        heroes[i].ATCK_RANGE = 0;
        heroes[i].position.x = 0;
        heroes[i].position.y = 0;
        heroes[i].Path = NULL;
        heroes[i].PathLength = 0;
        heroes[i].currentPathIndex = 0;
        heroes[i].isAlive = true;
        heroes[i].in_combat = false;
    }
    
    for (int i = 0; i < monster_count; i++) {
        monsters[i].HP = 0;
        monsters[i].ATCK_DAMAGE = 0;
        monsters[i].VISION_RANGE = 0;
        monsters[i].ATCK_RANGE = 0;
        monsters[i].START.x = 0;
        monsters[i].START.y = 0;
        monsters[i].Alerted = false;
        monsters[i].alive = true;
    }
    
    // Volver al inicio del archivo
    rewind(file);
    
    // Segunda pasada: leer datos
    while(fgets(line_buffer, sizeof(line_buffer), file)) {
        // GRID_SIZE
        if (strncmp(line_buffer, "GRID_SIZE", 9) == 0) {
            sscanf(line_buffer + 10, "%d %d", &tablero.width, &tablero.height);
        }
        
        // HERO sin número (formato ejemplo1.txt)
        else if (strncmp(line_buffer, "HERO_HP", 7) == 0 && hero_count == 1) {
            sscanf(line_buffer + 8, "%d", &heroes[0].HP);
        }
        else if (strncmp(line_buffer, "HERO_ATTACK_DAMAGE", 18) == 0 && hero_count == 1) {
            sscanf(line_buffer + 19, "%d", &heroes[0].ATCK_DAMAGE);
        }
        else if (strncmp(line_buffer, "HERO_ATTACK_RANGE", 17) == 0 && hero_count == 1) {
            sscanf(line_buffer + 18, "%d", &heroes[0].ATCK_RANGE);
        }
        else if (strncmp(line_buffer, "HERO_START", 10) == 0 && hero_count == 1) {
            sscanf(line_buffer + 11, "%d %d", &heroes[0].position.x, &heroes[0].position.y);
        }
        else if (strncmp(line_buffer, "HERO_PATH", 9) == 0 && hero_count == 1) {
            // Contar y parsear path
            char *ptr = line_buffer;
            int path_count = 0;
            while ((ptr = strchr(ptr, '(')) != NULL) {
                path_count++;
                ptr++;
            }
            
            heroes[0].PathLength = path_count;
            heroes[0].Path = (Point *)malloc(sizeof(Point) * path_count);
            
            ptr = line_buffer;
            for (int j = 0; j < path_count; j++) {
                ptr = strchr(ptr, '(');
                if (ptr) {
                    sscanf(ptr + 1, "%d,%d", &heroes[0].Path[j].x, &heroes[0].Path[j].y);
                    ptr++;
                }
            }
        }
        
        // HERO con número (formato ejemplo2.txt y ejemplo3.txt)
        else if (strstr(line_buffer, "HERO_") && strstr(line_buffer, "_HP")) {
            int hero_num;
            int hp;
            if (sscanf(line_buffer, "HERO_%d_HP %d", &hero_num, &hp) == 2) {
                heroes[hero_num - 1].HP = hp;
            }
        }
        else if (strstr(line_buffer, "HERO_") && strstr(line_buffer, "_ATTACK_DAMAGE")) {
            int hero_num, damage;
            if (sscanf(line_buffer, "HERO_%d_ATTACK_DAMAGE %d", &hero_num, &damage) == 2) {
                heroes[hero_num - 1].ATCK_DAMAGE = damage;
            }
        }
        else if (strstr(line_buffer, "HERO_") && strstr(line_buffer, "_ATTACK_RANGE")) {
            int hero_num, range;
            if (sscanf(line_buffer, "HERO_%d_ATTACK_RANGE %d", &hero_num, &range) == 2) {
                heroes[hero_num - 1].ATCK_RANGE = range;
            }
        }
        else if (strstr(line_buffer, "HERO_") && strstr(line_buffer, "_START")) {
            int hero_num, x, y;
            if (sscanf(line_buffer, "HERO_%d_START %d %d", &hero_num, &x, &y) == 3) {
                heroes[hero_num - 1].position.x = x;
                heroes[hero_num - 1].position.y = y;
            }
        }
        else if (strstr(line_buffer, "HERO_") && strstr(line_buffer, "_PATH")) {
            int hero_num;
            if (sscanf(line_buffer, "HERO_%d_PATH", &hero_num) == 1) {
                char *ptr = line_buffer;
                int path_count = 0;
                while ((ptr = strchr(ptr, '(')) != NULL) {
                    path_count++;
                    ptr++;
                }
                
                heroes[hero_num - 1].PathLength = path_count;
                heroes[hero_num - 1].Path = (Point *)malloc(sizeof(Point) * path_count);
                
                ptr = line_buffer;
                for (int j = 0; j < path_count; j++) {
                    ptr = strchr(ptr, '(');
                    if (ptr) {
                        sscanf(ptr + 1, "%d,%d", 
                               &heroes[hero_num - 1].Path[j].x, 
                               &heroes[hero_num - 1].Path[j].y);
                        ptr++;
                    }
                }
            }
        }
        
        // MONSTERS
        else if (strstr(line_buffer, "MONSTER_") && strstr(line_buffer, "_HP")) {
            int monster_num, hp;
            if (sscanf(line_buffer, "MONSTER_%d_HP %d", &monster_num, &hp) == 2) {
                monsters[monster_num - 1].HP = hp;
            }
        }
        else if (strstr(line_buffer, "MONSTER_") && strstr(line_buffer, "_ATTACK_DAMAGE")) {
            int monster_num, damage;
            if (sscanf(line_buffer, "MONSTER_%d_ATTACK_DAMAGE %d", &monster_num, &damage) == 2) {
                monsters[monster_num - 1].ATCK_DAMAGE = damage;
            }
        }
        else if (strstr(line_buffer, "MONSTER_") && strstr(line_buffer, "_VISION_RANGE")) {
            int monster_num, range;
            if (sscanf(line_buffer, "MONSTER_%d_VISION_RANGE %d", &monster_num, &range) == 2) {
                monsters[monster_num - 1].VISION_RANGE = range;
            }
        }
        else if (strstr(line_buffer, "MONSTER_") && strstr(line_buffer, "_ATTACK_RANGE")) {
            int monster_num, range;
            if (sscanf(line_buffer, "MONSTER_%d_ATTACK_RANGE %d", &monster_num, &range) == 2) {
                monsters[monster_num - 1].ATCK_RANGE = range;
            }
        }
        else if (strstr(line_buffer, "MONSTER_") && strstr(line_buffer, "_COORDS")) {
            int monster_num, x, y;
            if (sscanf(line_buffer, "MONSTER_%d_COORDS %d %d", &monster_num, &x, &y) == 3) {
                monsters[monster_num - 1].START.x = x;
                monsters[monster_num - 1].START.y = y;
            }
        }
    }
    
    fclose(file);
    
    printf("=== Configuración Cargada ===\n");
    printf("Grid: %dx%d\n", tablero.width, tablero.height);
    printf("Héroes: %d\n", hero_count);
    for (int i = 0; i < hero_count; i++) {
        printf("  Héroe %d: HP=%d, Daño=%d, Rango=%d, Inicio=(%d,%d), Path=%d puntos\n",
               i + 1, heroes[i].HP, heroes[i].ATCK_DAMAGE, heroes[i].ATCK_RANGE,
               heroes[i].position.x, heroes[i].position.y, heroes[i].PathLength);
    }
    printf("Monstruos: %d\n", monster_count);
    for (int i = 0; i < monster_count; i++) {
        printf("  Monstruo %d: HP=%d, Daño=%d, VisiónRango=%d, AtaqueRango=%d, Pos=(%d,%d)\n",
               i + 1, monsters[i].HP, monsters[i].ATCK_DAMAGE, 
               monsters[i].VISION_RANGE, monsters[i].ATCK_RANGE,
               monsters[i].START.x, monsters[i].START.y);
    }
    printf("=============================\n\n");
}



int main(int argc, char *argv[]){
    if (argc < 2) {
        printf("Uso: %s <archivo_configuracion>\n", argv[0]);
        printf("Ejemplo: %s ejemplo1.txt\n", argv[0]);
        return 1;
    }
    
    // Cargar configuración
    ParseConfig(argv[1]);
    
    // Crear threads para héroes
    for (int i = 0; i < hero_count; i++) {
        int *hero_id = malloc(sizeof(int));
        *hero_id = i;
        pthread_create(&heroes[i].thread_id, NULL, Hero, hero_id);
    }
    
    // Crear threads para monstruos
    for (int i = 0; i < monster_count; i++) {
        int *monster_id = malloc(sizeof(int));
        *monster_id = i;
        pthread_create(&monsters[i].thread_id, NULL, Mounstro, monster_id);
    }
    
    // Esperar a que terminen los héroes
    for (int i = 0; i < hero_count; i++) {
        pthread_join(heroes[i].thread_id, NULL);
    }
    
    // Esperar a que terminen los monstruos
    for (int i = 0; i < monster_count; i++) {
        pthread_join(monsters[i].thread_id, NULL);
    }
    
    // Liberar memoria
    for (int i = 0; i < hero_count; i++) {
        if (heroes[i].Path) {
            free(heroes[i].Path);
        }
    }
    free(heroes);
    free(monsters);
    
    printf("\n=== Simulación Terminada ===\n");
    
    return 0;
}

 