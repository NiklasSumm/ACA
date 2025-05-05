#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

#ifndef NUM_THREADS
	#define NUM_THREADS 2
#endif

#define SAMPLES 1e8 // number of random points to generate

// Global variables
pthread_mutex_t mutex; // Mutex variable
int shared_variable = 0; // Shared variable

char Pi[16] = "3.1415926535897";
char myPi[16] = {0}; 

// Method to compute the precision of pi
int str_identical_index(const char *str1, const char *str2) {
    int i = 0;
    while (str1[i] == str2[i]) {
        i++;
    }
    return i-2;
}

void *thread_monte_carlo(int samples_per_thread){
    double x, y, sum;
    for (int i = 0; i < samples_per_thread; i++) {
        x = (double)rand() / RAND_MAX;
        y = (double)rand() / RAND_MAX;
        if (x * x + y * y <= 1.0) { // point inside the circle?
            sum++;
        }
    }

    pthread_mutex_lock(&mutex);
    shared_variable += sum; // Increment shared variable
    pthread_mutex_unlock(&mutex);

    // Exit the thread
    pthread_exit(NULL);
}

int main() {
    double pi;
    pthread_t threads[NUM_THREADS];

    srand(42);

    // Initialize mutex
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        printf("Mutex initialization failed\n");
        return 1;
    }

    struct timespec start, end;
    double elapsed_time;

    // Record start time
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, thread_monte_carlo, SAMPLES/NUM_THREADS);
    }

    // Wait for threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
        printf("Thread %d has returned.\n",i);
    }

    //// Generate random points and count those that fall within the unit circle
    //for (i = 0; i < SAMPLES; i++) {
    //    x = (double)rand() / RAND_MAX;
    //    y = (double)rand() / RAND_MAX;
    //    if (x * x + y * y <= 1.0) { // point inside the circle?
    //        hits++;
    //    }
    //}

    // Record end time
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calculate elapsed time
    elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    // Calculate Pi using the ratio of points inside/outside
    pi = 4.0 * ((double)shared_variable / SAMPLES);
    sprintf(myPi, "%.8f", pi);
    printf("Estimated value of Pi is %.8f and is accurate up to %d decimal places. Real time: %.9fs.\n", pi, str_identical_index(Pi, myPi), elapsed_time);

    // Destroy mutex
    pthread_mutex_destroy(&mutex);

    return 0;
}
