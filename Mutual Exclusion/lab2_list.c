//  Aayush Patel
//  105111196
//  aayushpatel108@ucla.edu

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <sched.h>
#include <signal.h>
#include "SortedList.h"


long num_threads;
long num_iterations;
long num_elements;
int opt_yield;
int sync_flag;
int sync_lock;
char add_mode[2];
char yield_mode[4];
pthread_mutex_t mutex_lock;
SortedList_t *list_head;
SortedListElement_t* elements_to_insert;

void handle_seg_fault(){
	fprintf(stderr, "Error: Segmentation fault\n");
	exit(2);
}

void delete_wrapper(SortedListElement_t *ptr){
    if (ptr == NULL) {
        fprintf(stderr, "Error: SortedList_lookup failed\n");
        exit(2);
    }
    if (SortedList_delete(ptr)) {
        fprintf(stderr, "Error: SortedList_delete failed\n");
        exit(2);
    }
}

void length_wrapper() {
    if (SortedList_length(list_head) == -1) {
        fprintf(stderr, "Error: SortedList_length failed\n");
        exit(2);
    }
}

void* thread_list_operations(void * elements_offset) {
    int i;
    SortedListElement_t *ptr;
    int offset = * ((int *) elements_offset);
    if (sync_flag) {
        switch (add_mode[0]) {
            case 'm':
                for (i = offset; i < num_iterations+offset; i++) {
                    pthread_mutex_lock(&mutex_lock);
                    SortedList_insert(list_head, &elements_to_insert[i]);
                    pthread_mutex_unlock(&mutex_lock);
                }
                pthread_mutex_lock(&mutex_lock);
                length_wrapper();
                pthread_mutex_unlock(&mutex_lock);
                for (i = offset; i < num_iterations+offset; i++) {
                    pthread_mutex_lock(&mutex_lock);
                    ptr = SortedList_lookup(list_head, elements_to_insert[i].key);
                    delete_wrapper(ptr);
                    pthread_mutex_unlock(&mutex_lock);
                }
                break;
            case 's':
                for (i = offset; i < num_iterations+offset; i++) {
                    while(__sync_lock_test_and_set(&sync_lock, 1) == 1);
                    SortedList_insert(list_head, &elements_to_insert[i]);
                    __sync_lock_release(&sync_lock);
                }
                while(__sync_lock_test_and_set(&sync_lock, 1) == 1);
                length_wrapper();
                __sync_lock_release(&sync_lock);

                for (i = offset; i < num_iterations+offset; i++) {
                    while(__sync_lock_test_and_set(&sync_lock, 1) == 1);
                    ptr = SortedList_lookup(list_head, elements_to_insert[i].key);
                    delete_wrapper(ptr);
                    __sync_lock_release(&sync_lock);
                }
                break;
         }
    } else {
        for (i = offset; i < num_iterations+offset; i++)
            SortedList_insert(list_head, &elements_to_insert[i]);
        length_wrapper();
        for (i = offset; i < num_iterations+offset; i++) {
            ptr = SortedList_lookup(list_head, elements_to_insert[i].key);
            delete_wrapper(ptr);
        }
    }
  return NULL;
}

void randomly_initialize_list(){
	srand((unsigned int) time(NULL));
	int i;
	for (i = 0; i < num_elements; i++){
        char* random_key = malloc(2*sizeof(char));
		random_key[0] = (rand() % 126) + 1;
        random_key[1] = '\0';
		elements_to_insert[i].key = random_key;
	}
}

int main(int argc, char* argv[]) {
    int c;
    num_threads = 1;
    num_iterations = 1;
    opt_yield = 0;
    sync_flag = 0;
    sync_lock = 0;
    struct option long_options[] = {
        {"threads", required_argument, NULL, 't'},
        {"iterations", required_argument, NULL, 'i'},
        {"yield", required_argument, NULL, 'y'},
        {"sync", required_argument, NULL, 's'},
        {0,0,0,0}
    };

    char test_string[20] = "list-";
    while ((c = getopt_long(argc, argv, "", long_options, NULL)) != -1) {
        switch (c) {
            case 't':
                num_threads = atoi(optarg);
                break;
            case 'i':
                num_iterations = atoi(optarg);
                break;
            case 'y':
                opt_yield = 1;
                unsigned long i;
                for (i = 0; i < strlen(optarg); i++) {
                    switch (optarg[i]) {
                        case 'i':
                            opt_yield |= INSERT_YIELD;
                            break;
                        case 'd':
                            opt_yield |= DELETE_YIELD;
                            break;
                        case 'l':
                            opt_yield |= LOOKUP_YIELD;
                            break;
                    }
                }
                strcpy(yield_mode, optarg);
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
                fprintf(stderr, "Unrecognized argument. Usage: /.lab2a_list --threads=<# threads> --iterations=<# iterations> [--yield=<i,d,l>] [--sync=<m/s/>]");
                exit(1);
                break;
        }
    }
    signal(SIGSEGV, handle_seg_fault);
    if (opt_yield) {
        strcat(test_string, yield_mode);
    } else {
        strcat(test_string, "none");
    }
    if (sync_flag) {
        strcat(test_string, "-");
        strcat(test_string, add_mode);
        if (add_mode[0] == 'm')
            pthread_mutex_init(&mutex_lock, NULL);
    } else {
        strcat(test_string, "-none");
    }
    list_head = malloc(sizeof(SortedList_t));
    list_head->key = NULL;
    list_head->prev = list_head;
    list_head->next = list_head;
    num_elements = num_iterations * num_threads;
    elements_to_insert = (SortedListElement_t*) malloc(num_elements *sizeof(SortedListElement_t));
    if (elements_to_insert == NULL) {
        fprintf(stderr, "Error allocating memory\n");
        exit(1);
    }
    randomly_initialize_list();
    pthread_t *thread_ids = malloc(num_threads *sizeof(pthread_t));
    if (thread_ids == NULL) {
        fprintf(stderr, "Error allocating memory\n");
        exit(1);
    }
    struct timespec start;
    struct timespec end;
    if (clock_gettime(CLOCK_MONOTONIC, &start) < 0) {
        fprintf(stderr, "Error getting start time\n");
        exit(1);
    }
    int* thread_arguments = (int*) malloc(sizeof(int) * num_threads);
    int i;
    for (i = 0; i < num_threads; i++) {
        thread_arguments[i] = i*num_iterations;
        if (pthread_create(&thread_ids[i], NULL, thread_list_operations, &thread_arguments[i]) != 0) {
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
    if (SortedList_length(list_head) != 0) {
        fprintf(stderr, "List not sucessfully cleared\n");
        exit(2);
    }
    free(thread_arguments);
    free(thread_ids);
    free(list_head);
    free(elements_to_insert);
    long long total_run_time = 1000000000L * (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec);
    long long num_operations = num_threads * num_iterations * 3;
    long long avg_run_time = total_run_time / num_operations;
    int num_lists=1;
    fprintf(stdout, "%s,%ld,%ld,%d,%lld,%lld,%lld\n", test_string, num_threads, num_iterations, num_lists, num_operations, total_run_time, avg_run_time);

    exit(0);
}

