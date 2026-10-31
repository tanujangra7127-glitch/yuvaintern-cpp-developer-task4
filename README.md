
# Week 4: Concurrency Simulation with Threading

## Project Overview
This project implements the classic Producer-Consumer problem
using multithreading in C++.

Multiple producer threads generate tasks, while multiple
consumer threads process them through a shared bounded queue.

## Technologies Used
- C++
- C++17
- Multithreading
- Mutex
- Condition Variables
- Atomic Variables
- STL Queue

## Features
1. Multiple producer threads.
2. Multiple consumer threads.
3. Thread-safe bounded queue.
4. Mutex-based synchronization.
5. Condition variables.
6. Unique task identification.
7. Runtime activity logging.
8. Deadlock prevention.
9. Task completion verification.

## Files
- main.cpp
- README.md
- CMakeLists.txt

## Compilation

Using GCC:

g++ -std=c++17 -O2 -pthread main.cpp -o producer_consumer

## Execution

./producer_consumer

## Custom Execution

./producer_consumer 4 3 10 6

Arguments:
1. Number of producers
2. Number of consumers
3. Tasks per producer
4. Queue capacity

## Conclusion
This project demonstrates safe thread synchronization,
efficient task sharing, and concurrent execution in C++.
