#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>

// Structure to pass arguments to the thread function
typedef struct {
    uint32_t* a;
    size_t lo;
    size_t hi;
} SortParams;

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
void populate_array(uint32_t *arr, uint32_t *arr2, uint32_t *arr3, size_t size) {
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        uint32_t x = ((uint32_t)rand() << 16) | (uint32_t)rand();
        arr[i] = x;
        arr2[i] = x;
        arr3[i] = x;
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


// int partition(uint32_t *a, size_t i, size_t j) {
//     uint32_t v0 = a[i], v1 = a[(i+j+1)/2], v2 = a[j];
//     /* pivot: median of v0,v1,v2 */
//     uint32_t v = v0 < v1 ? v1 < v2 ? v1 : v0 < v2 ? v2 : v0 : v0 < v2 ? v0 : v1 < v2 ? v2 : v1;
    
//     //v = a[i];  // For some reason this fails with median of three partitioning
//     i--;
//     j++;
//     while (1) {
//         do {i++;} while (a[i] < v);
//         do {j--;} while (a[j] > v);

//         if (i >= j) return j;

//         uint32_t temp = a[i];
//         a[i] = a[j];
//         a[j] = temp;
//     }

// }


int partition(uint32_t *a, size_t i, size_t j) {

    size_t v0 = a[i], v1 = a[(i+j+1)/2], v2 = a[j];
    /* pivot: median of v0,v1,v2 */
    size_t v = v0 < v1 ? v1 < v2 ? v1 : v0 < v2 ? v2 : v0 : v0 < v2 ? v0 : v1 < v2 ? v2 : v1;
    while (i < j) {
        while (a[i] < v && ++i < j);
        while (v < a[j] && i < --j);
        uint32_t t = a[j]; 
        a[j] = a[i]; 
        a[i]= t; //swap
    }
    /* i == j; that's where the pivot belongs */
    a[i] = v;
    return j;
}

void quick_sort(uint32_t a[], size_t lo, size_t hi) {

    while (lo < hi) {
        size_t j = partition(a, lo, hi);
        if (j - lo < hi -j) {
            quick_sort(a, lo, j-1);
            lo = j+1;
        } else {
            quick_sort(a, j+1, hi);
            hi = j-1;
        }
    }
}

/* Thread function for parallel quicksort */ 
void* parallel_quick_sort_thread(void* arg) {
    SortParams* params = (SortParams*)arg;
    quick_sort(params->a, params->lo, params->hi);
    free(params);
    return NULL;
}

/* Parallel quick sort function */
void parallel_quick_sort(uint32_t a[], size_t lo, size_t hi) {
    while (lo < hi) {
        size_t j = partition(a, lo, hi);
        pthread_t thread;
        SortParams* params;
        
        if (j - lo < hi - j) {
            // Create a new thread for the smaller partition
            params = malloc(sizeof(SortParams));
            params->a = a;
            params->lo = lo;
            params->hi = j - 1;
            pthread_create(&thread, NULL, parallel_quick_sort_thread, params);
            
            // Continue with the larger partition in this thread
            lo = j + 1;
        } else {
            // Create a new thread for the smaller partition
            params = malloc(sizeof(SortParams));
            params->a = a;
            params->lo = j + 1;
            params->hi = hi;
            pthread_create(&thread, NULL, parallel_quick_sort_thread, params);
            
            // Continue with the larger partition in this thread
            hi = j - 1;
        }
        
        // Wait for the child thread to complete
        pthread_join(thread, NULL);
        return;
        
    }
}

/* Even more optimized parallel quick sort */
void optimized_parallel_quick_sort(uint32_t a[], size_t lo, size_t hi) {
    while (lo < hi) {
        size_t j = partition(a, lo, hi);
        
        /* Only thread if partition is size 1000 and above */
        if (hi - lo > 1000) {
            pthread_t thread;
            SortParams* params;
            
            if (j - lo < hi - j) {
                // Create a new thread for the smaller partition
                params = malloc(sizeof(SortParams));
                params->a = a;
                params->lo = lo;
                params->hi = j - 1;
                pthread_create(&thread, NULL, parallel_quick_sort_thread, params);
                
                // Continue with the larger partition in this thread
                lo = j + 1;
            } else {
                // Create a new thread for the smaller partition
                params = malloc(sizeof(SortParams));
                params->a = a;
                params->lo = j + 1;
                params->hi = hi;
                pthread_create(&thread, NULL, parallel_quick_sort_thread, params);
                
                // Continue with the larger partition in this thread
                hi = j - 1;
            }
            
            // Wait for the child thread to complete
            pthread_join(thread, NULL);
            return;
        } else {
            // For smaller subarrays, continue with sequential algorithm
            if (j - lo < hi - j) {
                quick_sort(a, lo, j - 1);
                lo = j + 1;
            } else {
                quick_sort(a, j + 1, hi);
                hi = j - 1;
            }
        }
    }
}

int is_sorted(uint32_t *arr, size_t size) {
    for (int i = 0;  i+1 < size;  ++i){
        if (arr[i] >= arr[i+1])
            return 0;
    }
    return 1;
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


void perform_quick_sort_experiments() {
    size_t size = 10;

    while (size <= 10000) {

        printf("Array Length: %ld\n", size);

        for (int i = 0; i < 3; i++) {
            printf("Trial %d\n", i+1);

            // Initialize the array
            uint32_t *sorted_arr1 = malloc(size * sizeof(uint32_t)); 
            uint32_t *sorted_arr2 = malloc(size * sizeof(uint32_t));
            uint32_t *sorted_arr3 = malloc(size * sizeof(uint32_t));
            
            // Populate the array
            // populate_array(sorted_arr1, sorted_arr2, sorted_arr3, size);
            populate_from_file("uniform10.txt", &sorted_arr1, size);
            populate_from_file("uniform10.txt", &sorted_arr2, size);
            populate_from_file("uniform10.txt", &sorted_arr3, size);
            
            print_array(sorted_arr1, size, "Unsorted: ");

            // Sort the copied array
            uint64_t start = rdtsc();
            quick_sort(sorted_arr1, 0, size - 1);
            uint64_t end = rdtsc();
            uint64_t serial_quick_sort_time = end - start;

            if(is_sorted(sorted_arr1, size))
                printf("Serial: %llu Ticks\n", serial_quick_sort_time);
            else
                printf("Serial Quicksort did not sort correctly\n");
        


            start = rdtsc();
            parallel_quick_sort(sorted_arr2, 0, size - 1);
            end = rdtsc();
            uint64_t parallel_quick_sort_time = end - start;
            if(is_sorted(sorted_arr2, size))
                printf("Parallel: %llu Ticks\n", parallel_quick_sort_time);
            else
                printf("Parallel Quicksort did not sort correctly\n");

            
            start = rdtsc();
            optimized_parallel_quick_sort(sorted_arr3, 0, size - 1);
            end = rdtsc();
            uint64_t optimized_parallel_quick_sort_time = end - start;
            if(is_sorted(sorted_arr3, size))
                printf("Optimized Parallel: %llu Ticks\n", optimized_parallel_quick_sort_time);
            else
                printf("Optimized Parallel Quicksort did not sort correctly\n");
            

            free(sorted_arr1);
            free(sorted_arr2);
            free(sorted_arr3);
            printf("---------------------------------------\n");
        }

        size *= 10;
        printf("----------------------------------------------------------\n");
        printf("----------------------------------------------------------\n");
    }
}



int main() {

        size_t size = 10000;

        // Initialize the array
        uint32_t *sorted_arr1 = malloc(size * sizeof(uint32_t)); 
        uint32_t *sorted_arr2 = malloc(size * sizeof(uint32_t));
        uint32_t *sorted_arr3 = malloc(size * sizeof(uint32_t));

        // Populate the array
        populate_array(sorted_arr1, sorted_arr2, sorted_arr3, size);
        // populate_from_file("uniform10.txt", &sorted_arr1, size);
        // populate_from_file("uniform10.txt", &sorted_arr2, size);
        // populate_from_file("uniform10.txt", &sorted_arr3, size);

        //print_array(sorted_arr1, size, "Unsorted: ");

        // Sort the copied array
        uint64_t start = rdtsc();
        quick_sort(sorted_arr1, 0, size - 1);
        uint64_t end = rdtsc();
        uint64_t serial_quick_sort_time = end - start;
        if(is_sorted(sorted_arr1, size))
        printf("Serial: %llu Ticks\n", serial_quick_sort_time);
        else
        printf("Serial Quicksort did not sort correctly\n");

        //print_array(sorted_arr1, size, "Sorted: ");
        exit(0);

        start = rdtsc();
        parallel_quick_sort(sorted_arr2, 0, size - 1);
        end = rdtsc();
        uint64_t parallel_quick_sort_time = end - start;
        if(is_sorted(sorted_arr2, size))
        printf("Parallel: %llu Ticks\n", parallel_quick_sort_time);
        else
        printf("Parallel Quicksort did not sort correctly\n");


        start = rdtsc();
        optimized_parallel_quick_sort(sorted_arr3, 0, size - 1);
        end = rdtsc();
        uint64_t optimized_parallel_quick_sort_time = end - start;
        if(is_sorted(sorted_arr3, size))
        printf("Optimized Parallel: %llu Ticks\n", optimized_parallel_quick_sort_time);
        else
        printf("Optimized Parallel Quicksort did not sort correctly\n");


        free(sorted_arr1);
        free(sorted_arr2);
        free(sorted_arr3);
    
    return 0;
}