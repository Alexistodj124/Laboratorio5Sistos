#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define THREAD_COUNT 5
#define ITERATIONS 5
#define RESOURCE_COUNT 3

int available_resources = RESOURCE_COUNT;
sem_t resource_sem;
pthread_mutex_t log_mutex;

// Escribir bitácora
void log_message(const char *msg, int thread_id) {
    pthread_mutex_lock(&log_mutex);
    FILE *log = fopen("bitacora_semaforos.txt", "a");
    if (log) {
        fprintf(log, "Thread %d: %s\n", thread_id, msg);
        fclose(log);
    }
    pthread_mutex_unlock(&log_mutex);
}

// Ejecucion de cada thread
void *thread_func(void *arg) {
    int id = *(int *)arg;
    char msg[100];

    for (int i = 0; i < ITERATIONS; ++i) {
        snprintf(msg, sizeof(msg), "Esperando recurso... (iteración %d)", i + 1);
        log_message(msg, id);

        sem_wait(&resource_sem);  // Espera recurso

        snprintf(msg, sizeof(msg), "Recurso obtenido. Recursos restantes: %d", --available_resources);
        log_message(msg, id);

        int uso = rand() % 3 + 1; // Simula recurso
        sleep(uso);

        ++available_resources;
        sem_post(&resource_sem); // Libera recurso

        snprintf(msg, sizeof(msg), "Recurso liberado. Recursos disponibles: %d", available_resources);
        log_message(msg, id);
    }

    pthread_exit(NULL);
}

int main() {
    srand(time(NULL));
    pthread_t threads[THREAD_COUNT];
    int ids[THREAD_COUNT];

    sem_init(&resource_sem, 0, RESOURCE_COUNT);
    pthread_mutex_init(&log_mutex, NULL);

    // Limpia bitacora anterior
    FILE *log = fopen("bitacora_semaforos.txt", "w");
    if (log) fclose(log);

    // Crea threads
    for (int i = 0; i < THREAD_COUNT; ++i) {
        ids[i] = i + 1;
        pthread_create(&threads[i], NULL, thread_func, &ids[i]);
    }

    for (int i = 0; i < THREAD_COUNT; ++i) {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&resource_sem);
    pthread_mutex_destroy(&log_mutex);

    printf("Simulación terminada. Revisa bitacora_semaforos.txt\n");
    return 0;
}