# 🔤 Word Frequency — Parallelism in C

Compares three approaches for counting word frequencies in a large text file.
Built in C for Operating Systems course (ENCS3390).

## Approaches
| Approach | Best Time |
|----------|-----------|
| Naive (single-threaded) | 697.9 sec |
| Multiprocessing (8 processes) | 167.16 sec ✅ Best |
| Multithreading (8 threads) | 336.5 sec |

## Files
- `naive.c` — Single-threaded approach
- `processes.c` — Fork-based parallel approach
- `threads.c` — Pthread-based approach

## How to Run

**Naive:**
   gcc naive.c -o naive && ./naive

**Multiprocessing:**
   gcc processes.c -o multi_proc && ./multi_proc

**Multithreading:**
   gcc threads.c -o multi_thread -lpthread && ./multi_thread

## Requirements
- Linux / WSL
- GCC compiler
- Dataset file: `text8.txt` in the same folder

## Key Findings
- Multiprocessing outperforms multithreading for CPU-bound tasks
- Amdahl's Law confirmed: speedup limited by 7% serial portion
- Max theoretical speedup ≈ 7.3x on 14-core system

## Author
Shahd Raed Shweketeh — ENCS3390 OS Course | 2025
