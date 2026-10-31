
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <atomic>
#include <chrono>
#include <random>
#include <string>

using namespace std;

struct Task {
    int id;
    int producerId;
    int value;
};

class BoundedQueue {
private:
    queue<Task> tasks;
    mutex mtx;
    condition_variable notEmpty;
    condition_variable notFull;
    size_t capacity;
    int activeProducers;

public:
    BoundedQueue(size_t cap, int producers)
        : capacity(cap), activeProducers(producers) {}

    void push(const Task& task) {
        unique_lock<mutex> lock(mtx);

        notFull.wait(lock, [this]() {
            return tasks.size() < capacity;
        });

        tasks.push(task);
        notEmpty.notify_one();
    }

    bool pop(Task& task) {
        unique_lock<mutex> lock(mtx);

        notEmpty.wait(lock, [this]() {
            return !tasks.empty() || activeProducers == 0;
        });

        if (tasks.empty())
            return false;

        task = tasks.front();
        tasks.pop();

        notFull.notify_one();
        return true;
    }

    void producerFinished() {
        {
            lock_guard<mutex> lock(mtx);
            activeProducers--;
        }

        notEmpty.notify_all();
    }
};

mutex logMutex;

void logMessage(const string& message) {
    lock_guard<mutex> lock(logMutex);
    cout << message << endl;
}

atomic<int> nextTaskId{1};
atomic<int> producedCount{0};
atomic<int> consumedCount{0};

void producer(int id, int taskCount, BoundedQueue& buffer) {
    mt19937 generator(
        static_cast<unsigned>(
            chrono::steady_clock::now()
                .time_since_epoch().count()
        ) + id
    );

    uniform_int_distribution<int> distribution(10, 999);

    for (int i = 0; i < taskCount; i++) {
        Task task;

        task.id = nextTaskId.fetch_add(1);
        task.producerId = id;
        task.value = distribution(generator);

        buffer.push(task);
        producedCount++;

        logMessage(
            "Producer " + to_string(id) +
            " created Task " + to_string(task.id) +
            " Value: " + to_string(task.value)
        );

        this_thread::sleep_for(chrono::milliseconds(20));
    }

    buffer.producerFinished();

    logMessage("Producer " + to_string(id) + " finished.");
}

void consumer(int id, BoundedQueue& buffer) {
    Task task;

    while (buffer.pop(task)) {
        logMessage(
            "Consumer " + to_string(id) +
            " processed Task " + to_string(task.id) +
            " from Producer " + to_string(task.producerId)
        );

        consumedCount++;

        this_thread::sleep_for(chrono::milliseconds(35));
    }

    logMessage("Consumer " + to_string(id) + " finished.");
}

int main(int argc, char* argv[]) {
    int producerCount = 3;
    int consumerCount = 2;
    int tasksPerProducer = 8;
    int queueCapacity = 5;

    try {
        if (argc > 1) producerCount = stoi(argv[1]);
        if (argc > 2) consumerCount = stoi(argv[2]);
        if (argc > 3) tasksPerProducer = stoi(argv[3]);
        if (argc > 4) queueCapacity = stoi(argv[4]);
    }
    catch (...) {
        cerr << "Invalid command-line arguments." << endl;
        return 1;
    }

    if (producerCount < 1 || consumerCount < 1 ||
        tasksPerProducer < 0 || queueCapacity < 1) {
        cerr << "Invalid configuration." << endl;
        return 1;
    }

    BoundedQueue buffer(queueCapacity, producerCount);

    vector<thread> producers;
    vector<thread> consumers;

    logMessage("Starting Producer-Consumer Simulation");

    // Start consumer threads
    for (int i = 1; i <= consumerCount; i++) {
        consumers.emplace_back(consumer, i, ref(buffer));
    }

    // Start producer threads
    for (int i = 1; i <= producerCount; i++) {
        producers.emplace_back(
            producer, i, tasksPerProducer, ref(buffer)
        );
    }

    // Wait for producers
    for (auto& t : producers) {
        t.join();
    }

    // Wait for consumers
    for (auto& t : consumers) {
        t.join();
    }

    logMessage("Simulation Completed!");

    cout << "\nTotal Tasks Produced: "
         << producedCount.load() << endl;

    cout << "Total Tasks Consumed: "
         << consumedCount.load() << endl;

    if (producedCount == consumedCount) {
        cout << "SUCCESS: All tasks processed successfully!"
             << endl;
    } else {
        cout << "ERROR: Task count mismatch!" << endl;
        return 2;
    }

    return 0;
}
