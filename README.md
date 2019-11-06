# container::RingQueue
An ring priority queue for C++.

## Features
* STL-like
* Single-header implementation. Just drop it in your project.
* Not thread-safe
* C++11 implementation
* Fully portable
* Self-sort like std::set

## Basic use
The entire queue's implementation is contained in one header `ring_list.h`.

Simple example:
```
#include "ring_queue.h"

container::RingQueue<int> q;

q.push(2);
q.push(1);
assert(q.front() == 1);

q.pop();
q.clear();

```
Description of methods:
* `push(const T& value)` Insert an element into queue
* `emplace(Args&&... args)` Insert an element into queue
* `pop()` Remove the smallest element from header
* `erase(const iterator& position)` Remove element in 'position'
* `front()` The first element
* `empty()` The queue is empty or not
* `size()` The element number
* `clear()` Clear queue
* `begin()` The begin iterator
* `end()` The end iterator

## Tests
I've written quite a few unit tests. The tests depend on [googletest](https://github.com/google/googletest), you need install it firstly if you want to run the test. I run the test in windows10/ubuntu18-x64/ubuntu16-armv8, use valgrind check the test as well, There may still be bugs. If anyone is seeing buggy behaviour, I'd like to hear about it! Just open an issue on GitHub.