#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

// Structure to pass arguments to the thread function
typedef struct {
    uint32_t* arr;
    size_t low;
    size_t high;
} sort_args;

// Populate memory array from a provided file of unsigned ints
int populate_from_file(const char *filename, uint32_t **array, size_t size) {
    FILE *file = NULL;

    *array = (uint32_t *)malloc(size * sizeof(uint32_t));
    if (*array == NULL) {
        perror("Memory allocation failed.");
        return -1;
    }

    file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file.");
        free(*array);
        return -1;
    }

    for (size_t i = 0; i < size; ++i) {
        if (fscanf(file, "%u", &(*array)[i]) != 1) {
            fprintf(stderr, "Error reading integer at position %zu\n", i);
            free(*array);
            fclose(file);
            return -1;
        }
    }

    fclose(file);

    return 0;
}

/* Fill in the array with random numbers */
void populate_array(uint32_t *arr, size_t size) {
    for (int i = 0; i < size; i++) {
        arr[i] = size - i;
    }
}

/* Print out array */
void print_array(uint32_t *arr, size_t size, char message[]) {
    
    printf("%s", message);
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%d,", arr[i]);
    }
    printf("]\n");
}

// Helper function to merge two sorted subarrays
void merge(uint32_t *arr, size_t left, size_t mid, size_t right) {

    size_t n1 = mid - left + 1;
    size_t n2 = right - mid;

    // Create temporary arrays
    uint32_t *L = malloc(n1 * sizeof(uint32_t));
    uint32_t *R = malloc(n2 * sizeof(uint32_t));

    // Copy data to temporary arrays L[] and R[]
    for (size_t i = 0; i < n1; i++)
        L[i] = arr[left + i];
    for (size_t j = 0; j < n2; j++)
        R[j] = arr[mid + 1 + j];

    // Merge the temporary arrays back into arr[left..right]
    size_t i = 0;
    size_t j = 0;
    size_t k = left;

    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    // At this point, either L or R is empty

    // Copy the remaining elements of L[], if there are any
    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    // Copy the remaining elements of R[], if there are any
    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }

    // Free the temporary arrays
    free(L);
    free(R);
}

// Helper function to recursively divide and merge the array
void merge_sort(uint32_t *arr, size_t left, size_t right) {
    if (left < right) {
        size_t mid = left + (right - left) / 2;

        // Sort first and second halves
        merge_sort(arr, left, mid);
        merge_sort(arr, mid + 1, right);

        // Merge the sorted halves
        merge(arr, left, mid, right);
    }
}

/* Partition helper function for quick sort*/
int partition (uint32_t *arr, size_t low, size_t high) {
    size_t pivot = low;
    unsigned long i = low;
    unsigned long j = high;

    while (i <= j) {
        while (i <= high && arr[i] <= arr[pivot]) {
            i++;
        }

        while (j >= low && arr[j] > arr[pivot]) {
            j--;
        }

        if (i < j) {
            // Swap arr[i] and arr[j]
            uint32_t temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }

    // Put pivot in its final position
    uint32_t temp = arr[low];
    arr[low] = arr[j];
    arr[j] = temp;

    return j;

}

/* Serial quick sort function */
void quick_sort(uint32_t *arr, size_t low, size_t high) {
    if (low < high) {
        unsigned long pivot_location = partition(arr, low, high);
        quick_sort(arr, low, pivot_location - 1);
        quick_sort(arr, pivot_location + 1, high);
    }
}

/* Parallel quick sort function */
void* parallel_quicksort(void* arg) {
    sort_args* args = (sort_args*)arg;
    uint32_t* arr = args->arr;
    size_t low = args->low;
    size_t high = args->high;
    
    if (low < high) {
        unsigned long pivot_location = partition(arr, low, high);
        
        pthread_t thread;
        sort_args left_args = {arr, low, pivot_location-1};
        
        // Create new thread for left partition
        pthread_create(&thread, NULL, parallel_quicksort, &left_args);
        
        // Current thread handles right partition
        sort_args right_args = {arr, pivot_location + 1, high};
        parallel_quicksort(&right_args);
        
        // Wait for left partition to complete
        pthread_join(thread, NULL);
    }
    
    return NULL;
}

// Starter function to initiate parallel quicksort
void start_parallel_quicksort(uint32_t *arr, size_t low, size_t high) {
    sort_args args = {arr, low, high};
    parallel_quicksort(&args);
}

// from homework one starter code
static inline uint64_t rdtsc(){
        unsigned long a, d;
        asm volatile ("rdtsc" : "=a"(a), "=d"(d));
        return a | ((uint64_t)d << 32);
}

/* THIS IS STARTER CODE FROM TA, NOT REALLY NEEDED RN, JUST KEEPING IT THO*/
// Avoid making changes to this function skeleton, apart from data type changes if required
// In this starter code we have used uint32_t, feel free to change it to any other data type if required
void sort_array(uint32_t *arr, size_t size) {
    //merge_sort(arr, 0, size - 1);
    //quick_sort(arr, 0, size - 1);
}


int main() {

    // QUICK SORT EXPERIMENTS BEGIN

    //Initialise the array
    size_t size = 10000;
    uint32_t *sorted_arr1 = malloc(size * sizeof(uint32_t)); // Allocate memory for the sorted array
    uint32_t *sorted_arr2 = malloc(size * sizeof(uint32_t)); // Allocate memory for the sorted array
    
    // Populate the array
    populate_from_file("uniform10.txt", &sorted_arr1, size);
    populate_from_file("uniform10.txt", &sorted_arr2, size);

    // Sort the copied array
    uint64_t start = rdtsc();
    quick_sort(sorted_arr1, 0, size - 1);
    uint64_t end = rdtsc();
    uint64_t serial_quick_sort_time = end - start;
    printf("Serial: %ld Ticks\n", serial_quick_sort_time);

    start = rdtsc();
    start_parallel_quicksort(sorted_arr2, 0, size - 1);
    end = rdtsc();
    uint64_t parallel_quick_sort_time = end - start;
    printf("Parallel: %ld Ticks\n", parallel_quick_sort_time);

    free(sorted_arr1);
    free(sorted_arr2);

    // print_array(sorted_arr2, size, "Parallel Sort Result: ");

    // QUICK SORT EXPERIMENTS END


    // MERGE SORT EXPERIMENTS BEGIN


    // MERGE SORT EXPERIMENTS END
    
    return 0;
}

       
