//Shahd Raed
//1222105
 #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#define WORD_LENGTH 100
#define INITIAL_CAPACITY 100000
#define NUMBER_THREADS 6

typedef struct {
    char word[WORD_LENGTH];
    int frequency;
} Word;

typedef struct {
    Word *data;
    int size;
    int capacity;
} Array;

typedef struct {
    int index;
    char **words;
    int total_words;
    pthread_mutex_t *lock;
} WorkQueue;

Array global_freq;
pthread_mutex_t mutex;

void initializeArray(Array *arr);
void add(Array *arr, const char *word);
void mergeSort(Word *arr, int left, int right);
void merge(Word *arr, int left, int mid, int right);
char **readFromFile(int *total_words);
void *countFrequencies(void *arg);
int getNextIndex(WorkQueue *queue);

int main() {
    struct timeval start, end;
    gettimeofday(&start, NULL);

    int total_words = 0;
    char **words = readFromFile(&total_words);

    initializeArray(&global_freq);
    pthread_mutex_init(&mutex, NULL);

    WorkQueue work_queue = {0, words, total_words, &mutex};

    pthread_t threads[NUMBER_THREADS];

    for (int i = 0; i < NUMBER_THREADS; i++) {
        pthread_create(&threads[i], NULL, countFrequencies, &work_queue);
    }

    for (int i = 0; i < NUMBER_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    mergeSort(global_freq.data, 0, global_freq.size - 1);

    gettimeofday(&end, NULL);
    double execution_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

    printf("Number of Threads Used= %d\n", NUMBER_THREADS);
    printf("Top 10 Most Frequent Words:\n");
    int print_limit = (10 < global_freq.size) ? 10 : global_freq.size;
    for (int i = 0; i < print_limit; i++) {
        printf("%d-%s= %d\n", i + 1, global_freq.data[i].word, global_freq.data[i].frequency);
    }

    printf("Execution Time= %f seconds\n", execution_time);

    for (int i = 0; i < total_words; i++) {
        free(words[i]);
    }
    free(words);
    free(global_freq.data);
    pthread_mutex_destroy(&mutex);

    return 0;
}

void initializeArray(Array *arr) {
    arr->data = malloc(INITIAL_CAPACITY * sizeof(Word));
    if (!arr->data) {
        printf("Memory allocation failed");
        exit(1);
    }
    arr->size = 0;
    arr->capacity = INITIAL_CAPACITY;
}

void add(Array *arr, const char *word) {
    for (int i = 0; i < arr->size; i++) {
        if (strcmp(arr->data[i].word, word) == 0) {
            arr->data[i].frequency++;
            return;
        }
    }

    if (arr->size >= arr->capacity) {
        arr->capacity *= 2;
        arr->data = realloc(arr->data, arr->capacity * sizeof(Word));
        if (!arr->data) {
            printf("Memory reallocation failed!");
            exit(1);
        }
    }

    strncpy(arr->data[arr->size].word, word, WORD_LENGTH - 1);
    arr->data[arr->size].word[WORD_LENGTH - 1] = '\0';
    arr->data[arr->size].frequency = 1;
    arr->size++;
}

void merge(Word *arr, int left, int mid, int right) {
    int left_size = mid - left + 1;
    int right_size = right - mid;

    Word *left_arr = malloc(left_size * sizeof(Word));
    Word *right_arr = malloc(right_size * sizeof(Word));

    if (!left_arr || !right_arr) {
        perror("Merge allocation failed");
        free(left_arr);
        free(right_arr);
        return;
    }

    memcpy(left_arr, &arr[left], left_size * sizeof(Word));
    memcpy(right_arr, &arr[mid + 1], right_size * sizeof(Word));

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

    free(left_arr);
    free(right_arr);
}

void mergeSort(Word *arr, int left, int right) {
    if (left >= right) return;

    int mid = left + (right - left) / 2;
    mergeSort(arr, left, mid);
    mergeSort(arr, mid + 1, right);
    merge(arr, left, mid, right);
}

char **readFromFile(int *total_words) {
    FILE *file = fopen("text8.txt", "r");
    if (!file) {
        printf("Error reading file");
        return NULL;
    }

    int capacity = INITIAL_CAPACITY;
    char **words = malloc(capacity * sizeof(char *));
    *total_words = 0;

    char w[WORD_LENGTH];

    while (fscanf(file, "%99s", w) == 1) {
        if (*total_words >= capacity) {
            capacity *= 2;
            char **temp = realloc(words, capacity * sizeof(char *));
            if (!temp) {
                printf("Memory reallocation failed");
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

void *countFrequencies(void *arg) {
    WorkQueue *queue = (WorkQueue *)arg;
    Array local_freq;
    initializeArray(&local_freq);

    int index;
    while ((index = getNextIndex(queue)) != -1) {
        add(&local_freq, queue->words[index]);
    }

    pthread_mutex_lock(&mutex);
    for (int i = 0; i < local_freq.size; i++) {
        int found = 0;
        for (int j = 0; j < global_freq.size; j++) {
            if (strcmp(global_freq.data[j].word, local_freq.data[i].word) == 0) {
                global_freq.data[j].frequency += local_freq.data[i].frequency;
                found = 1;
                break;
            }
        }

        if (!found && global_freq.size < INITIAL_CAPACITY) {
            strcpy(global_freq.data[global_freq.size].word, local_freq.data[i].word);
            global_freq.data[global_freq.size].frequency = local_freq.data[i].frequency;
            global_freq.size++;
        }
    }
    pthread_mutex_unlock(&mutex);

    free(local_freq.data);
    return NULL;
}

int getNextIndex(WorkQueue *queue) {
    int index = -1;
    pthread_mutex_lock(queue->lock);
    if (queue->index < queue->total_words) {
        index = queue->index++;
    }
    pthread_mutex_unlock(queue->lock);
    return index;
}
