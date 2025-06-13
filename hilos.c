#include<stdio.h> //Proporciona funciones de entrada/salida estandar como printf y fopen
#include<stdlib.h> //Contiene funciones como malloc, free, exit, etc.
#include<pthread.h> //Proporciona funciones para trabajar con hilos POSIX
#include<time.h> //Permite medir el tiempo de ejecucion con clock_gettime
#include<string.h> //Proporciona funciones para manejo de cadenas como strlen y snprintf
#include<unistd.h> //Contiene funciones POSIX como write, close y sleep
#include<fcntl.h> //Permite manipular descriptores de archivo con open y flags
#include<sys/types.h> //Define tipos como pid_t, off_t necesarios para llamadas al sistema
#include<sys/stat.h> //Define estructura de permisos para archivos (usado con open)

#define NUM_GRUPOS 8 //Numero total de grupos de transacciones
#define LOG_FILE "TiempoHilos.txt" //Nombre del archivo de log
#define TRANSACCIONES_POR_GRUPO 5 //Cantidad de transacciones por grupo
#define TRANSACCIONES_FILE "Transacciones.txt" //Nombre del archivo con las transacciones

int saldo = 1000; //Saldo inicial
pthread_mutex_t mutex_saldo; //Mutex para proteger acceso al saldo
int transacciones[NUM_GRUPOS][TRANSACCIONES_POR_GRUPO]; //Matriz de transacciones por grupo

typedef struct {
    char grupo; //Identificador del grupo
    int log_fd; //Descriptor de archivo para el log
    int grupo_idx; //Indice del grupo en la matriz de transacciones
} DatosGrupo;

void* procesar_grupo(void* arg) {
    DatosGrupo* datos = (DatosGrupo*) arg; //Cast de argumento a tipo DatosGrupo
    char log[256];
    snprintf(log, sizeof(log), "Grupo %c iniciando transacciones...\n", datos->grupo); //Mensaje de inicio
    write(datos->log_fd, log, strlen(log)); //Escritura del mensaje en log

    for (int i = 0; i < TRANSACCIONES_POR_GRUPO; i++) {
        int transaccion = transacciones[datos->grupo_idx][i]; //Obtiene transaccion

        pthread_mutex_lock(&mutex_saldo); //Bloquea acceso a saldo
        saldo += transaccion; //Actualiza saldo
        int saldo_actual = saldo; //Copia del saldo actualizado
        pthread_mutex_unlock(&mutex_saldo); //Desbloquea acceso

        snprintf(log, sizeof(log), "Grupo %c realizo transaccion %d: %+d => Saldo parcial: %d\n",
                 datos->grupo, i + 1, transaccion, saldo_actual); //Mensaje de transaccion
        write(datos->log_fd, log, strlen(log)); //Escribe mensaje en log
    }

    snprintf(log, sizeof(log), "Grupo %c finalizo.\n\n", datos->grupo); //Mensaje de finalizacion
    write(datos->log_fd, log, strlen(log)); //Escribe en log

    pthread_exit(NULL); //Termina hilo
}

int main() {
    pthread_t hilos[NUM_GRUPOS]; //Arreglo de hilos
    DatosGrupo datos[NUM_GRUPOS]; //Datos para cada grupo
    pthread_mutex_init(&mutex_saldo, NULL); //Inicializa mutex

    struct timespec start, end; //Tiempos de inicio y fin
    clock_gettime(CLOCK_MONOTONIC, &start); //Marca inicio

    FILE* tf = fopen(TRANSACCIONES_FILE, "r"); //Abre archivo de transacciones
    if (!tf) {
        perror("No se pudo abrir Transacciones.txt");
        exit(1);
    }
    for (int i = 0; i < NUM_GRUPOS; i++) {
        for (int j = 0; j < TRANSACCIONES_POR_GRUPO; j++) {
            if (fscanf(tf, "%d", &transacciones[i][j]) != 1) {
                fprintf(stderr, "Error leyendo transaccion %d,%d\n", i, j);
                fclose(tf);
                exit(1);
            }
        }
    }
    fclose(tf); //Cierra archivo

    int log_fd = open(LOG_FILE, O_CREAT | O_WRONLY | O_TRUNC, 0644); //Abre archivo log
    if (log_fd < 0) {
        perror("Error al abrir log");
        exit(1);
    }

    for (int i = 0; i < NUM_GRUPOS; i++) {
        datos[i].grupo = 'A' + i; //Asigna letra al grupo
        datos[i].log_fd = log_fd; //Asigna descriptor
        datos[i].grupo_idx = i; //Asigna indice
        if (pthread_create(&hilos[i], NULL, procesar_grupo, &datos[i]) != 0) {
            perror("Error al crear hilo");
            exit(1);
        }
    }

    for (int i = 0; i < NUM_GRUPOS; i++) {
        pthread_join(hilos[i], NULL); //Espera finalizacion de hilos
    }

    clock_gettime(CLOCK_MONOTONIC, &end); //Marca fin
    double duracion = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9; //Calcula duracion

    char resumen[128];
    snprintf(resumen, sizeof(resumen),
             "\nSaldo final: %d\n Tiempo total de ejecucion: %.4f segundos\n", saldo, duracion); //Resumen
    write(log_fd, resumen, strlen(resumen)); //Escribe resumen en log

    close(log_fd); //Cierra log
    pthread_mutex_destroy(&mutex_saldo); //Destruye mutex
    printf("Ejecucion completada. Los tiempos de ejecucion se guardaron en: %s\n", LOG_FILE); //Mensaje final
    return 0; //Fin del programa
}




