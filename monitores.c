#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define THREAD_COUNT 5
#define ITERATIONS 5
#define INITIAL_RESOURCES 5

int available_resources = INITIAL_RESOURCES;
pthread_mutex_t monitor_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t resource_available = PTHREAD_COND_INITIALIZER;
pthread_mutex_t log_mutex;

// Bitácora
void log_message(const char *msg, int thread_id) {
    pthread_mutex_lock(&log_mutex);
    FILE *log = fopen("bitacora_monitor.txt", "a");
    if (log) {
        fprintf(log, "Thread %d: %s\n", thread_id, msg);
        fclose(log);
    }
    pthread_mutex_unlock(&log_mutex);
}

// Disminuye recursos
int decrease_count(int count, int thread_id) {
    pthread_mutex_lock(&monitor_mutex);

    char msg[100];
    snprintf(msg, sizeof(msg), "Solicitando %d recursos...", count);
    log_message(msg, thread_id);

    // Espera hasta que hayan suficientes
    while (available_resources < count) {
        snprintf(msg, sizeof(msg), "Esperando por %d recursos... (hay %d)", count, available_resources);
        log_message(msg, thread_id);
        pthread_cond_wait(&resource_available, &monitor_mutex);
    }

    available_resources -= count;
    snprintf(msg, sizeof(msg), "Obtuvo %d recursos. Quedan %d.", count, available_resources);
    log_message(msg, thread_id);

    pthread_mutex_unlock(&monitor_mutex);
    return 0;
}

// Aumentar recursos
int increase_count(int count, int thread_id) {
    pthread_mutex_lock(&monitor_mutex);

    available_resources += count;

    char msg[100];
    snprintf(msg, sizeof(msg), "Liberó %d recursos. Disponibles: %d.", count, available_resources);
    log_message(msg, thread_id);

    pthread_cond_broadcast(&resource_available);
    pthread_mutex_unlock(&monitor_mutex);
    return 0;
}

void *thread_func(void *arg) {
    int id = *(int *)arg;
    char msg[100];

    for (int i = 0; i < ITERATIONS; ++i) {
        int needed = rand() % 3 + 1; // Recursos aleatorios

        decrease_count(needed, id);

        int uso = rand() % 3 + 1;
        snprintf(msg, sizeof(msg), "Usando recursos (%ds)", uso);
        log_message(msg, id);
        sleep(uso);

        increase_count(needed, id); // Libera recursos
    }

    pthread_exit(NULL);
}

int main() {
    srand(time(NULL));
    pthread_t threads[THREAD_COUNT];
    int ids[THREAD_COUNT];

    pthread_mutex_init(&log_mutex, NULL);

    // Limpiar bitacora anterior
    FILE *log = fopen("bitacora_monitor.txt", "w");
    if (log) fclose(log);

    // Crea threads
    for (int i = 0; i < THREAD_COUNT; ++i) {
        ids[i] = i + 1;
        pthread_create(&threads[i], NULL, thread_func, &ids[i]);
    }

    for (int i = 0; i < THREAD_COUNT; ++i) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&monitor_mutex);
    pthread_cond_destroy(&resource_available);
    pthread_mutex_destroy(&log_mutex);

    printf("Simulación terminada. Revisa bitacora_monitor.txt\n");
    return 0;
}