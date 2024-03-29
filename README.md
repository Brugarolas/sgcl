# SGCL
## About the SGCL
SGCL (Smart Garbage Collection Library) is an advanced memory management library for C++, designed with performance and ease of use in mind. SGCL introduces fully tracked smart pointers, providing an experience similar to shared_ptr but with added mechanisms for automatic garbage collection and optimization. Tailored for the modern C++ standards (C++17 and later), SGCL aims to facilitate safer and more efficient memory management without the overhead typically associated with garbage collection techniques.

This library is particularly suited for developers looking to implement or improve garbage collection mechanisms in their C++ projects, offering a smart blend of manual control and automatic memory management to achieve high-performance, real-time applications.
## Features
- **Thread-Safe Operations** 

    SGCL is designed for safe concurrent access, making it ideal for multi-threaded applications without the risk of data races.

- **Zero Reference Counting** 
    
    Unlike many smart pointer implementations, SGCL avoids the overhead and complexity of reference counters, leading to improved performance and lower memory usage.

- **Simplicity and Familiarity** 
    
    Utilizes an intuitive API, making the transition for developers accustomed to shared_ptr seamless and straightforward.

- **Reduced Memory Overhead** 

    Optimized to consume less memory than shared_ptr, facilitating more efficient resource usage in your applications.

- **Performance Optimized** 

    Benchmarks demonstrate that SGCL outperforms shared_ptr in various scenarios, ensuring faster execution times for your projects.

- **Pauseless Garbage Collection** 

    Designed to avoid "stop-the-world" pauses, SGCL ensures continuous application performance without disruptive GC interruptions.

- **Support for Cyclic Data Structures** 

    Handles cyclic references gracefully, eliminating the common pitfalls associated with manual memory management in complex structures.

- **Copy-On-Write (CoW) Optimization** 

    SGCL is CoW-friendly, supporting efficient memory usage patterns and optimizing scenarios where cloned data structures defer copying until mutation.

- **Lock-Free Atomic Pointers**

    Guarantees that atomic pointer operations are always lock-free, enhancing performance in concurrent usage scenarios.

These features make SGCL a robust and versatile choice for developers seeking to optimize their C++ applications with advanced garbage collection and memory management techniques, all while maintaining high performance and ease of use.
## Compiling
The SGCL library requires a compiler with support for C++17.
## SGCL pointers
SGCL introduces three types of smart pointers.

- `root_ptr, unique_ptr`

    These pointers can be utilized on the stack, heap, or managed heap. They serve as roots in the application's object graph. The unique_ptr provides a deterministic destruction mechanism.

- `tracked_ptr` 

    Crafted to be a part of structures or arrays created via make_tracked. Contrary to root_ptr and unique_ptr, tracked_ptr is designed for exclusive use in the managed heap.

## The make_tracked method
The `make_tracked` method is dedicated method for creating objects on the managed heap. This method returns a unique_ptr.
## Example
```cpp
#include "sgcl/sgcl.h"
#include <iostream>

int main() {
    using namespace sgcl;

    // Creating unique_ptr with deterministic destruction
    auto unique = make_tracked<int>(42);

    // Creating shared_ptr with deterministic destruction
    std::shared_ptr<int> shared = make_tracked<int>(1337);

    // Creating root_ptr, which does not have deterministic destruction (managed by GC)
    root_ptr<int> root = make_tracked<int>(2024);

    // Example of using root_ptr with a custom data type
    struct Node {
        tracked_ptr<Node> next;
        int value;
    };
    root_ptr<Node> node = make_tracked<Node>();
    node->value = 10; // Direct field access

    // Simple pointer operations
    root = std::move(unique); // Moving ownership from unique_ptr to root_ptr
    shared = make_tracked<int>(2048); // Assigning a new value to shared_ptr

    // Demonstrating array handling
    auto array = make_tracked<int[]>(5, 7); // Creating an array and initialization
    for (auto& elem: array) {
        std::cout << elem << " "; // Iteration and printing
    }
    std::cout << std::endl;

    // Using an atomic pointer
    atomic<root_ptr<int>> atomicRoot = make_tracked<int>(1234);

    // Creating a pointer alias
    // Note: only pointers to memory allocated on the managed heap, excluding arrays
    root_ptr<int> value(&node->value);

    // Using pointer casting
    root_ptr<void> any = root;
    root = static_pointer_cast<int>(any);
    if (any.is<int>() || any.type() == typeid(int)) {
        root = any.as<int>();
    }

    // Metadata usage
    // The metadata structure is defined in the configuration.h file
    struct: metadata {
        void to_string(void* p) override {
            std::cout << "to_string<int>: " << *static_cast<int*>(p) << std::endl;
        }
    } static mdata;
    metadata::set<int>(&mdata);
    any.metadata()->to_string(any.get()); // Currently any pointed a value of type int.

    // Methods useful for state analysis and supporting testing processes.
    {
        // Forced collection
        collector::force_collect();

        // Force collection and wait for the cycle to complete
        collector::force_collect(true);

        // Get number of living objects
        auto live_object_number = collector::live_object_count();
        std::cout << "live object number: " << live_object_number << std::endl;

        // Get live objects
        auto live_objects = collector::live_objects();
        for (auto& v: live_objects) {
            std::cout << v.get() << " " << v.type().name() << std::endl;
        }

        // Terminate collector
        collector::terminate();
    }
}
```
