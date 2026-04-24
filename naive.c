//Shahd Raed
//1222105
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
// Initial capacity for dynamic allocation
#define INITIAL_CAPACITY 100000
#define WORD_LENGTH 100

// Structure to store a word & its frequency
typedef struct {
    char word[WORD_LENGTH];
    int frequency;
} Word;

// Function prototypes
void countFrequencies(char **wordsArray, int wordCount, Word **words, int *size, int *capacity);
void merge(Word *arr, int left, int mid, int right);
void mergeSort(Word *arr, int left, int right);
/////////////////////////////////////////////////////////////////////////////////////////////////////////

int main() {
    //To measure execution time
    clock_t start, end;
    start = clock();

    // Open the file for reading
    FILE *in = fopen("text8.txt", "r");
    if (!in) {
        printf("Error reading file!");
        return 1;
    }

    // Read words into an array
    char **wordsArray = malloc(INITIAL_CAPACITY * sizeof(char *));
    if (!wordsArray) {
        printf("Memory allocation failed!");
        fclose(in);
        return 1;
    }

    char w[WORD_LENGTH];
    int wordCount = 0;
    int arrayCapacity = INITIAL_CAPACITY;

    while (fscanf(in, "%99s", w) == 1) {
        if (strlen(w) > 0) {
            // Resize the array if needed
            if (wordCount >= arrayCapacity) {
                arrayCapacity *= 2;
                wordsArray = realloc(wordsArray, arrayCapacity * sizeof(char *));
                if (!wordsArray) {
                    printf("Memory reallocation failed!");
                    fclose(in);
                    return 1;
                }
            }
            // Allocate memory for the word and copy it
            wordsArray[wordCount] = malloc(strlen(w) + 1);
            if (!wordsArray[wordCount]) {
                printf("Memory allocation failed!");
                fclose(in);
                return 1;
            }
            strcpy(wordsArray[wordCount], w);
            wordCount++;
        }
    }
    fclose(in);

    // Array to store words and their frequencies
    int capacity = 100;
    int size = 0;
    Word *words = malloc(capacity * sizeof(Word));
    if (!words) {
        printf("Memory allocation failed!");
        return 1;
    }
    // to find speedup
    //clock_t countStart, countEnd;
    //countStart = clock();

    // Count frequencies
    countFrequencies(wordsArray, wordCount, &words, &size, &capacity);
    // countEnd = clock();  // End time measurement for countFrequencies
    //double countTime = ((double)countEnd - countStart) / CLOCKS_PER_SEC;  // Calculate time in seconds
    //printf("Execution time of countFrequencies: %f seconds\n", countTime);

    // Sort the words by frequency using merge sort
    mergeSort(words, 0, size - 1);

    // Print the top 10 most frequent words
    printf("Top 10 Most Frequent Words:\n");
    for (int i = 0; i < 10 && i < size ; i++) {
        printf("%d-%s= %d\n",i+1, words[i].word, words[i].frequency);
    }

    // Free dynamically allocated memory
    for (int i = 0; i < wordCount; i++) {
        free(wordsArray[i]);
    }
    free(wordsArray);
    free(words);

    end = clock();  // End time measurement
    double time_taken = ((double)end - start) / CLOCKS_PER_SEC;  // Calculate time in seconds
    printf("Execution time= %f seconds\n", time_taken);

    return 0;
}
//Functions implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////

// Function to count frequencies of words
void countFrequencies(char **wordsArray, int wordCount, Word **words, int *size, int *capacity) {
    for (int i = 0; i < wordCount; i++) {
        char *word = wordsArray[i];
        int found = 0;

        // Check if the word already exists in the array
        for (int j = 0; j < *size; j++) {
            if (strcmp((*words)[j].word, word) == 0) {
                (*words)[j].frequency++;
                found = 1;
                break;
            }
        }

        // If not found, add it as a new word
        if (!found) {
            if (*size >= *capacity) {
                *capacity *= 2;  // Double the capacity
                *words = realloc(*words, (*capacity) * sizeof(Word));
                if (!(*words)) {
                    perror("Memory reallocation failed");
                    exit(1);
                }
            }
            strcpy((*words)[*size].word, word);
            (*words)[*size].frequency = 1;
            (*size)++;
        }
    }
}

// Merge function
void merge(Word *arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    Word *leftArr = malloc(n1 * sizeof(Word));
    Word *rightArr = malloc(n2 * sizeof(Word));
    // Copy data into temp arrays
    for (int i = 0; i < n1; i++) {
        leftArr[i] = arr[left + i];
    }
    for (int j = 0; j < n2; j++) {
        rightArr[j] = arr[mid + 1 + j];
    }
    // Merge the temp arrays back into the original array
    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (leftArr[i].frequency >= rightArr[j].frequency) {
            arr[k++] = leftArr[i++];
        } else {
            arr[k++] = rightArr[j++];
        }
    }
    // Copy remaining elements of leftArr[]
    while (i < n1) {
        arr[k++] = leftArr[i++];
    }
    // Copy remaining elements of rightArr[]
    while (j < n2) {
        arr[k++] = rightArr[j++];
    }

    // Free the temporary arrays
    free(leftArr);
    free(rightArr);
}

// Merge sort function
void mergeSort(Word *arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        // Sort the first half
        mergeSort(arr, left, mid);
        // Sort the second half
        mergeSort(arr, mid + 1, right);
        // Merge the sorted halves
        merge(arr, left, mid, right);
    }
}
