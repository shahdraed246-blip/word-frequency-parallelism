//Shahd Raed
//1222105
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/time.h>
#define WORD_LENGTH 100
#define INITIAL_CAPACITY 100000
// Allow this to be changed as needed
#define NUMBER_PROCESSES 8

// Word and frequency struct
typedef struct {
    char word[WORD_LENGTH];
    int frequency;
} Word;

// Array for word frequencies
typedef struct {
    Word *data;
    int size;
    int capacity;
} Array;

// Shared memory struct
typedef struct {
    int size;
    Word data[INITIAL_CAPACITY];
} SharedData;

// Function prototypes
void initializeArray(Array *arr);
void add(Array *arr, const char *word);
void mergeSort(Word *arr, int left, int right);
void merge(Word*arr, int left, int mid, int right);
char** readFromFile( int *total_words);
int main() {
    // Start timing
    struct timeval start, end;
    gettimeofday(&start, NULL);

    int total_words = 0;

    // Read words from file
    char **w= readFromFile( &total_words);

    // Create shared memory for word frequencies
    SharedData *shared_data = mmap(NULL, sizeof(SharedData),
                                       PROT_READ | PROT_WRITE,
                                       MAP_SHARED | MAP_ANONYMOUS,
                                       -1, 0);

    if (shared_data == MAP_FAILED) {
        printf("mmap failed!");
        for (int i = 0; i < total_words; i++) {
            free(w[i]);
        }
        free(w);
        return 1;
    }

    // Fork child processes
    pid_t pid_arr[NUMBER_PROCESSES];
    int chunk_size = total_words / NUMBER_PROCESSES;
    int remainder = total_words % NUMBER_PROCESSES;

    for (int i = 0; i < NUMBER_PROCESSES; i++) {
        pid_arr[i] = fork();

        if (pid_arr[i] == -1) {
            perror("fork failed");
            munmap(shared_data, sizeof(SharedData));
            for (int j = 0; j < total_words; j++) {
                free(w[j]);
            }
            free(w);
            exit(1);
        } else if (pid_arr[i] == 0) {
            // Child process
            int start = i * chunk_size + (i < remainder ? i : remainder);
            int end = start + chunk_size + (i < remainder ? 1 : 0);

            // Local word frequency array
            Array local_freq;
            initializeArray(&local_freq);

            // Count frequencies for this subset of words
            for (int j = start; j < end; j++) {
                add(&local_freq, w[j]);
            }

            // Transfer to shared memory with atomic-like synchronization
            for (int j = 0; j < local_freq.size; j++) {
                int found = 0;
                for (int k = 0; k < shared_data->size; k++) {
                    if (strcmp(shared_data->data[k].word, local_freq.data[j].word) == 0) {
                        shared_data->data[k].frequency += local_freq.data[j].frequency;
                        found = 1;
                        break;
                    }
                }

                // Add new word if not found
                if (!found && shared_data->size < INITIAL_CAPACITY) {
                    strcpy(shared_data->data[shared_data->size].word, local_freq.data[j].word);
                    shared_data->data[shared_data->size].frequency = local_freq.data[j].frequency;
                    shared_data->size++;
                }
            }

            // Free local resources
            free(local_freq.data);
            exit(0);
        }
    }

    // Parent process waits  children
    for (int i = 0; i < NUMBER_PROCESSES; i++) {
        int status;
        waitpid(pid_arr[i], &status, 0);
    }

    // Sort words by frequency
    mergeSort(shared_data->data, 0, shared_data->size - 1);

    // End timing calculation
    gettimeofday(&end, NULL);
    double execution_time = (end.tv_sec - start.tv_sec) +
                            (end.tv_usec - start.tv_usec) / 1000000.0;

    printf("Number of Processes Used= %d\n", NUMBER_PROCESSES);

    // Print top 10 most frequent words
    printf("Top 10 Most Frequent Words:\n");
    int print_limit = (10 < shared_data->size) ? 10: shared_data->size;
    for (int i = 0; i < print_limit; i++) {
        printf("%d-%s= %d\n",i+1, shared_data->data[i].word, shared_data->data[i].frequency);
    }


    printf("Execution Time= %f seconds\n", execution_time);

    // Free resources
    for (int i = 0; i < total_words; i++) {
        free(w[i]);
    }
    free(w);
    munmap(shared_data, sizeof(SharedData));

    return 0;
}
//To initialize array of word and freqncy
void initializeArray(Array *arr) {
    arr->data = malloc(INITIAL_CAPACITY * sizeof(Word));
    if (!arr->data) {
        printf("Memory allocation failed");
        exit(1);
    }
    arr->size = 0;
    arr->capacity = INITIAL_CAPACITY;
}

// Add word to frequency array with dynamic resizing
void add(Array *arr, const char *word) {
    // Check if word already exists
    for (int i = 0; i < arr->size; i++) {
        if (strcmp(arr->data[i].word, word) == 0) {
            arr->data[i].frequency++;
            return;
        }
    }

    // Resize array if needed
    if (arr->size >= arr->capacity) {
        arr->capacity *= 2;
        arr->data = realloc(arr->data, arr->capacity * sizeof(Word));
        if (!arr->data) {
            printf("Memory reallocation failed!");
            exit(1);
        }
    }

    // Add new word
    strncpy(arr->data[arr->size].word, word, WORD_LENGTH - 1);
    arr->data[arr->size].word[WORD_LENGTH - 1] = '\0';
    arr->data[arr->size].frequency = 1;
    arr->size++;
}

// Merge subarrays during sorting
void merge(Word *arr, int left, int mid, int right) {
    int left_size = mid - left + 1;
    int right_size = right - mid;

    // Temporary arrays
    Word *left_arr = malloc(left_size * sizeof(Word));
    Word *right_arr = malloc(right_size * sizeof(Word));

    if (!left_arr || !right_arr) {
        perror("Merge allocation failed");
        free(left_arr);
        free(right_arr);
        return;
    }

    // To copy data to temporary arrays
    memcpy(left_arr, &arr[left], left_size * sizeof(Word));
    memcpy(right_arr, &arr[mid + 1], right_size * sizeof(Word));

    // Merge back
    int i = 0, j = 0, k = left;
    while (i < left_size && j < right_size) {
        if (left_arr[i].frequency >= right_arr[j].frequency) {
            arr[k] = left_arr[i];
            i++;
        } else {
            arr[k] = right_arr[j];
            j++;
        }
        k++;
    }

    // Copy remaining elements
    while (i < left_size) {
        arr[k] = left_arr[i];
        i++;
        k++;
    }

    while (j < right_size) {
        arr[k] = right_arr[j];
        j++;
        k++;
    }

    // Free temporary arrays
    free(left_arr);
    free(right_arr);
}

// Recursive merge sort for word frequencies
void  mergeSort(Word *arr, int left, int right) {
    if (left >= right) return;

    int mid = left + (right - left) / 2;
    mergeSort(arr, left, mid);
    mergeSort(arr, mid + 1, right);
    merge(arr, left, mid, right);
}

// Read words from input file with dynamic memory allocation
char** readFromFile( int *total_words) {
    FILE *file = fopen("text8.txt", "r");
    if (!file) {
        printf("Error reading file");
        return NULL;
    }

    // Initial allocation
    int capacity = INITIAL_CAPACITY;
    char **words = malloc(capacity * sizeof(char*));
    *total_words = 0;

    char w[WORD_LENGTH];

    // Read words with dynamic reallocation
    while (fscanf(file, "%99s", w) == 1) {
        if (*total_words >= capacity) {
            capacity *= 2;
            char **temp = realloc(words, capacity * sizeof(char*));
            if (!temp) {
                printf("Memory reallocation failed");
                // Free previously allocated memory
                for (int i = 0; i < *total_words; i++) {
                    free(words[i]);
                }
                free(words);
                fclose(file);
                return NULL;
            }
            words = temp;
        }

        words[*total_words] = strdup(w);
        (*total_words)++;
    }

    fclose(file);
    return words;
}
