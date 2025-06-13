#include<stdio.h> //Proporciona funciones de entrada/salida como printf, fopen, fprintf
#include<stdlib.h> //Proporciona funciones como malloc, exit, perror
#include<unistd.h> //Proporciona funciones POSIX como fork, write, close
#include<sys/wait.h> //Permite esperar la finalización de procesos hijo con wait
#include<time.h> //Permite medir el tiempo con clock_gettime
#include<fcntl.h> //Permite abrir archivos con flags como O_CREAT y O_TRUNC
#include<string.h> //Permite manipular cadenas de caracteres como strlen, snprintf
#include<sys/stat.h> //Define permisos y atributos de archivos
#include<semaphore.h> //Proporciona operaciones con semáforos POSIX como sem_open, sem_wait
#include<sys/mman.h> //Permite crear y manipular memoria compartida con mmap

#define NUM_GRUPOS 8 //Numero de grupos de transacciones
#define LOG_FILE "TiempoProcesos.txt" //Archivo de salida del log
#define TRANSACCIONES_POR_GRUPO 5 //Cantidad de transacciones por grupo
#define SEM_NAME "/sem_saldo" //Nombre del semáforo compartido
#define TRANSACCIONES_FILE "Transacciones.txt" //Archivo de entrada con transacciones

void procesar_transacciones(char grupo, sem_t* sem, int log_fd, int* saldo_ptr, int transacciones[NUM_GRUPOS][TRANSACCIONES_POR_GRUPO], int grupo_idx) {
    char log[256];
    snprintf(log, sizeof(log), "Grupo %c iniciando transacciones...\n", grupo); //Mensaje de inicio
    write(log_fd, log, strlen(log)); //Escribe mensaje en log

    for (int i = 0; i < TRANSACCIONES_POR_GRUPO; i++) {
        int transaccion = transacciones[grupo_idx][i]; //Lee transacción
        sem_wait(sem); //Bloquea semáforo
        *saldo_ptr += transaccion; //Actualiza saldo compartido
        int saldo_actual = *saldo_ptr; //Obtiene saldo actual
        sem_post(sem); //Libera semáforo

        snprintf(log, sizeof(log), "Grupo %c realizo transaccion %d: %+d=>Saldo parcial: %d\n",
                 grupo, i + 1, transaccion, saldo_actual); //Mensaje de transacción
        write(log_fd, log, strlen(log)); //Escribe mensaje en log
    }

    snprintf(log, sizeof(log), "Grupo %c finalizo.\n\n", grupo); //Mensaje final del grupo
    write(log_fd, log, strlen(log)); //Escribe en log
}

int main() {
    struct timespec start, end; //Tiempos de inicio y fin
    clock_gettime(CLOCK_MONOTONIC, &start); //Marca inicio

    int log_fd = open(LOG_FILE, O_CREAT | O_WRONLY | O_TRUNC, 0644); //Abre archivo de log
    if (log_fd < 0) {
        perror("Error abriendo log");
        return 1;
    }

    int* saldo_ptr = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); //Memoria compartida para el saldo
    if (saldo_ptr == MAP_FAILED) {
        perror("fallo mmap");
        exit(1);
    }
    *saldo_ptr = 1000; //Saldo inicial

    sem_t* sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0600, 1); //Crea semáforo para sincronización
    if (sem == SEM_FAILED) {
        perror("Error creando semaforo");
        exit(1);
    }

    int (*transacciones)[TRANSACCIONES_POR_GRUPO] = mmap(NULL, sizeof(int) * NUM_GRUPOS * TRANSACCIONES_POR_GRUPO, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); //Memoria compartida para transacciones
    if (transacciones == MAP_FAILED) {
        perror("fallo mmap transacciones");
        exit(1);
    }

    FILE* tf = fopen(TRANSACCIONES_FILE, "r"); //Abre archivo con las transacciones
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

    for (int i = 0; i < NUM_GRUPOS; i++) {
        char grupo = 'A' + i; //Nombre del grupo (A-H)
        pid_t pid = fork(); //Crea proceso hijo
        if (pid < 0) {
            perror("ERROR al hacer fork");
            exit(1);
        } else if (pid == 0) {
            procesar_transacciones(grupo, sem, log_fd, saldo_ptr, transacciones, i); //Ejecuta proceso hijo
            close(log_fd); //Cierra log
            sem_close(sem); //Cierra semáforo
            munmap(saldo_ptr, sizeof(int)); //Libera memoria de saldo
            munmap(transacciones, sizeof(int) * NUM_GRUPOS * TRANSACCIONES_POR_GRUPO); //Libera transacciones
            exit(0); //Termina proceso hijo
        }
    }

    for (int i = 0; i < NUM_GRUPOS; i++) {
        wait(NULL); //Espera que todos los hijos terminen
    }

    clock_gettime(CLOCK_MONOTONIC, &end); //Marca fin
    double duracion = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9; //Calcula duración

    char resumen[128];
    snprintf(resumen, sizeof(resumen),
             "\nSaldo final: %d\n Tiempo total de ejecucion: %.4f segundos\n", *saldo_ptr, duracion); //Resumen
    write(log_fd, resumen, strlen(resumen)); //Escribe en log

    close(log_fd); //Cierra log
    sem_close(sem); //Cierra semáforo
    sem_unlink(SEM_NAME); //Elimina semáforo
    munmap(saldo_ptr, sizeof(int)); //Libera memoria saldo
    munmap(transacciones, sizeof(int) * NUM_GRUPOS * TRANSACCIONES_POR_GRUPO); //Libera memoria transacciones

    printf("Ejecucion completada. Los tiempo de ejecución se guardaron en: '%s'\n", LOG_FILE); //Mensaje final
    return 0; //Fin del programa
}
