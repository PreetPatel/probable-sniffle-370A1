/*
    The Merge Sort to use for Operating Systems Assignment 1 2019
    written by Robert Sheehan

    Modified by: Preet Patel
    UPI: put ppat504

    By submitting a program you are claiming that you and only you have made
    adjustments and additions to this code.
 */

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h>
#include <sys/resource.h>
#include <stdbool.h>
#include <pthread.h>

#define SIZE    2
long number_of_processors = 0;
pthread_mutex_t lock;

struct block {
    int size;
    int *first;
};

// void print_block_data(struct block *blk) {
//     printf("size: %d address: %p\n", blk->size, blk->first);
// }

/* Combine the two halves back together. */
void merge(struct block *left, struct block *right) {
	int combined[left->size + right->size];
	int dest = 0, l = 0, r = 0;
	while (l < left->size && r < right->size) {
		if (left->first[l] < right->first[r])
			combined[dest++] = left->first[l++];
		else
			combined[dest++] = right->first[r++];
	}
	while (l < left->size)
		combined[dest++] = left->first[l++];
	while (r < right->size)
		combined[dest++] = right->first[r++];
    memmove(left->first, combined, (left->size + right->size) * sizeof(int));
}

/* Merge sort the data. */
// Modified the headers of this function to conform with the requirements of the thread creation
void *merge_sort(void *ptr) {
    struct block *my_data = (struct block*)ptr;

    // print_block_data(my_data);
    if (my_data->size > 1) {
        struct block left_block;
        struct block right_block;
        left_block.size = my_data->size / 2;
        left_block.first = my_data->first;
        right_block.size = left_block.size + (my_data->size % 2);
        right_block.first = my_data->first + left_block.size;

        pthread_attr_t attributesForThread;
        size_t size = 2 * right_block.size * sizeof(int);
        pthread_attr_init(&attributesForThread);
        int ret = pthread_attr_setstacksize(&attributesForThread,size);
        
        pthread_t leftThread, rightThread; 

        pthread_mutex_lock(&lock);
        if (number_of_processors >= 1) {
            
            number_of_processors--;
            pthread_mutex_unlock(&lock);

            pthread_create(&leftThread, &attributesForThread, merge_sort, &left_block); 
            pthread_create(&rightThread, &attributesForThread, merge_sort, &right_block); 
            
            pthread_join(leftThread, NULL); 
            pthread_join(rightThread, NULL); 
            merge(&left_block, &right_block);

            pthread_mutex_lock(&lock);
            number_of_processors++;
            pthread_mutex_unlock(&lock);

        } else {
            pthread_mutex_unlock(&lock);
            merge_sort(&left_block);
            merge_sort(&right_block);
            merge(&left_block, &right_block);
        }
        
    }
}

/* Check to see if the data is sorted. */
bool is_sorted(int data[], int size) {
    bool sorted = true;
    for (int i = 0; i < size - 1; i++) {
        if (data[i] > data[i + 1])
            sorted = false;
    }
    return sorted;
}

int main(int argc, char *argv[]) {
	long size;
    struct rlimit rl;

    // Find number of cores available to run program
    number_of_processors = sysconf(_SC_NPROCESSORS_ONLN);

    // Initialise mutex for counter locking
    if (pthread_mutex_init(&lock, PTHREAD_MUTEX_DEFAULT) != 0) { 
        printf("There was an error initialising the mutex for counter. \n"); 
        return 1; 
    } 

    // Get the stack limit 
   getrlimit (RLIMIT_STACK, &rl); 
  
    // printf("\n Default value is : %lld\n", (long long int)rl.rlim_cur); 
   // Change the stack limit 
   // tried 100000000 * sizeof(int) but it caused seg dump
   rl.rlim_cur = 1000000000; 
  
   // Now call setrlimit() to set the  
   // changed value. 
   setrlimit (RLIMIT_STACK, &rl); 
  
   // Again get the limit and check 
   getrlimit (RLIMIT_STACK, &rl); 

//    printf("\n Default value now is : %lld\n", (long long int)rl.rlim_cur); 
	if (argc < 2) {
		size = SIZE;
	} else {
		size = atol(argv[1]);
	}

    struct block start_block;
    int data[size];
    start_block.size = size;
    start_block.first = data;
    for (int i = 0; i < size; i++) {
        data[i] = rand();
    }

    printf("starting---\n");
    merge_sort(&start_block);
    printf("---ending\n");
    printf(is_sorted(data, size) ? "sorted\n" : "not sorted\n");
    exit(EXIT_SUCCESS);
}