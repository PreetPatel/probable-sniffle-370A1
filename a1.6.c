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
#include<sys/wait.h> 

#define SIZE    2

struct block {
    int size;
    int *first;
};

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

        merge_sort(&left_block);
        merge_sort(&right_block);
        merge(&left_block, &right_block);
    }
}

/* Threaded Merge Sort function that creates the two threads */
void *init_merge_sort(void *ptr) {
    struct block *my_data = (struct block*)ptr;

    // print_block_data(my_data);
    if (my_data->size > 1) {
        struct block left_block;
        struct block right_block;
        left_block.size = my_data->size / 2;
        left_block.first = my_data->first;
        right_block.size = left_block.size + (my_data->size % 2);
        right_block.first = my_data->first + left_block.size;

        int fd[2];
        // creating pipe
        if (pipe(fd) < 0) {
            fprintf(stderr, "Pipe creation failed");
        }
        int left_sort_pid, right_sort_pid;

        left_sort_pid = fork();
        if (left_sort_pid == 0) {
            close(fd[0]); // close read end of left pipe
            merge_sort(&left_block);
            write(fd[1], left_block.first, left_block.size * sizeof(int)); // write bits of left block to left pipe
            close(fd[1]);
            exit(0);
        } else {
            merge_sort(&right_block);
            close(fd[1]); // close write end of left pipe
            read(fd[0], left_block.first, left_block.size * sizeof(int)); // read left block from left pipe
            close(fd[0]);
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
    init_merge_sort(&start_block);
    printf("---ending\n");
    printf(is_sorted(data, size) ? "sorted\n" : "not sorted\n");
    exit(EXIT_SUCCESS);
}