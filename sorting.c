#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> 

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

// Avoid making changes to this function skeleton, apart from data type changes if required
// In this starter code we have used uint32_t, feel free to change it to any other data type if required
void sort_array(uint32_t *arr, size_t size) {
    // Enter your logic here
    merge_sort(arr, 0, size - 1);
}

/* Fill in the array with random numbers */
void populate_array(uint32_t *arr, size_t size) {
    
    for (int i = 0; i < size; i++) {
        arr[i] = size - i;
    }
}

/* Print out array */
void print_array(uint32_t *arr, size_t size, char message[]) {
    
    printf(message);
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%d,", arr[i]);
    }
    printf("]\n");
}


int main() {
    //Initialise the array
    size_t size = 10;
    uint32_t *sorted_arr = malloc(size * sizeof(uint32_t)); // Allocate memory for the sorted array
    
    // Populate the array
    populate_array(sorted_arr, size);

    // Print array before sorting
    print_array(sorted_arr, size, "Array Before Sorting\n");

    // Sort the copied array
    sort_array(sorted_arr, size);

    // Print the sorted array
    print_array(sorted_arr, size, "Array After Sorting\n");

    free(sorted_arr);
    
    return 0;
}

       