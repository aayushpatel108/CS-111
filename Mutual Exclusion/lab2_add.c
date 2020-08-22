//  Aayush Patel
//  105111196
//  aayushpatel108@ucla.edu

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sched.h>

long long counter;
long num_threads;
long num_iterations;
int opt_yield;
int sync_flag;
char add_mode[2];
pthread_mutex_t mutex_lock;
int sync_lock;

void add(long long *pointer, long long value) {
    long long sum = *pointer + value;
    if (opt_yield)
            sched_yield();
    *pointer = sum;
}

void mutex_add(long long *pointer, long long value) {
    pthread_mutex_lock(&mutex_lock);
    add(pointer, value);
    pthread_mutex_unlock(&mutex_lock);
}
void spin_add(long long *pointer, long long value) {
    while (__sync_lock_test_and_set(&sync_lock, 1))
        ;
    add(pointer, value);
    __sync_lock_release(&sync_lock);
}
void compare_add(long long *pointer, long long value) {
    long long prev;
    do {
        prev = counter;
        if (opt_yield)
            sched_yield();
    } while (__sync_val_compare_and_swap(pointer, prev, prev+value) != prev);
}

void *thread_add() {
    int i;
    if (sync_flag) {
        switch (add_mode[0]) {
            case 'm':
                for (i = 0; i < num_iterations; i++) {
                    mutex_add(&counter, 1);
                    mutex_add(&counter, -1);
                }
                break;
            case 's':
                for (i = 0; i < num_iterations; i++) {
                    spin_add(&counter, 1);
                    spin_add(&counter, -1);
                }
                break;
            case 'c':
                for (i = 0; i < num_iterations; i++) {
                    compare_add(&counter, 1);
                    compare_add(&counter, -1);
                }
                break;
        }
    } else {
        for (i = 0; i < num_iterations; i++) {
            add(&counter, 1);
            add(&counter, -1);
        }
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    counter = 0;
    int c;
    num_threads = 1;
    num_iterations = 1;
    opt_yield = 0;
    sync_lock = 0;
    sync_flag = 0;
    struct option long_options[] = {
        {"threads", required_argument, NULL, 't'},
        {"iterations", required_argument, NULL, 'i'},
        {"yield", no_argument, &opt_yield, 1},
        {"sync", required_argument, NULL, 's'},
        {0,0,0,0}
    };

    char test_string[20] = "add-";

    while ((c = getopt_long(argc, argv, "", long_options, NULL)) != -1) {
        switch (c) {
            case 't':
                num_threads = atoi(optarg);
                break;
            case 'i':
                num_iterations = atoi(optarg);
                break;
            case 0:
                break;
            case 's':
                sync_flag = 1;
                if (strlen(optarg) > 2) {
                    fprintf(stderr, "Error Message");
                    exit(1);
                } else {
                    add_mode[0] = optarg[0];
                    add_mode[1] = '\0';
                }
                break;
            default:
                fprintf(stderr, "Unrecognized argument. Usage: /.lab2a_add --threads=<# threads> --iterations=<# iterations> [--yield] [--sync=<m/s/c>] \n");
                exit(1);
                break;

        }
    }
    
    if (opt_yield)
        strcat(test_string,"yield-");
    if (sync_flag) {
        strcat(test_string, add_mode);
    } else {
        strcat(test_string, "none");
    }
    
    struct timespec start;
    struct timespec end;
    if (clock_gettime(CLOCK_MONOTONIC, &start) < 0) {
        fprintf(stderr, "Error getting end time\n");
        exit(1);
    }

    pthread_t* thread_ids = malloc(num_threads *sizeof(pthread_t));
    if (thread_ids == NULL) {
        fprintf(stderr, "Error allocating memory\n");
        exit(1);
    }
    int i;
    for (i = 0; i < num_threads; i++) {
        if (pthread_create(&thread_ids[i], NULL, thread_add, NULL) != 0) {
            fprintf(stderr, "Error: Could not create threads.\n");
            free(thread_ids);
            exit(1);
        }
    }
 
    for (i = 0; i < num_threads; i++) {
        if (pthread_join(thread_ids[i], NULL) != 0) {
            fprintf(stderr, "Error: Could not join threads.\n");
            free(thread_ids);
            exit(1);
        }
    }

    if (clock_gettime(CLOCK_MONOTONIC, &end) < 0) {
        fprintf(stderr, "Error getting end time\n");
        exit(1);
    }

    long long total_run_time = 1000000000L * (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec);
    long long num_operations = num_threads * num_iterations * 2;
    long avg_run_time = total_run_time / num_operations;
    fprintf(stdout, "%s,%ld,%ld,%lld,%lld,%ld,%lld\n", test_string, num_threads, num_iterations, num_operations, total_run_time, avg_run_time, counter);

    exit(0);
}
