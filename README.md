⚡ XCrossfire

""License: MIT" (https://img.shields.io/badge/License-MIT-yellow.svg)" (https://opensource.org/licenses/MIT)
""Python" (https://img.shields.io/badge/Python-3.8%2B-blue.svg)" (https://www.python.org/)
""C++" (https://img.shields.io/badge/C%2B%2B-17-blue.svg)"
""SIMD" (https://img.shields.io/badge/SIMD-AVX2-red.svg)"

XCrossfire is a high-performance hardware-aware search engine designed for large in-memory datasets. The project combines a native C++ execution core with Python bindings and leverages SIMD (AVX2) instructions and multi-threaded execution to reduce practical search latency on large unsorted arrays.

«Note: Benchmark results reported in this repository were obtained on the author's test hardware and should be independently reproduced on other systems.»

---

Why XCrossfire?

Searching very large unsorted datasets efficiently is a common systems programming challenge.

XCrossfire explores a hardware-aware approach that focuses on:

- Efficient CPU utilization
- SIMD vector processing
- Low-overhead native execution
- Python/C++ interoperability
- Zero-copy style memory interaction where applicable

The goal is to improve practical execution performance while keeping the original dataset unchanged.

---

Features

- Native C++ search engine
- Python bindings
- AVX2 SIMD acceleration
- Multi-threaded execution
- Large dataset support
- Low memory overhead
- Benchmark framework included
- Open-source implementation

---

Architecture

Python Application
        │
        ▼
Python Binding Layer
        │
        ▼
Native C++ Engine
        │
        ├── SIMD AVX2 Processing
        ├── Multi-thread Execution
        └── Hardware-aware Memory Access

---

Benchmarks

Example benchmark on the author's development machine.

Method| Dataset| Time
Python Linear Search| 10 Million| 326.47 ms
Python Built-in Index| 10 Million| 77.09 ms
Binary Search (including sorting)| 10 Million| 3532.05 ms
XCrossfire| 10 Million| 3.65 ms

Measured Throughput

- Up to ~10 GB/s on the tested hardware.

«These values are benchmark-specific and may vary depending on CPU architecture, compiler, operating system, cache behavior, memory bandwidth, and compiler optimization flags.»

---

Hardware

Development Environment

- CPU: x86_64 Processor with AVX2 support
- Language: C++17
- Python: 3.8+
- Compiler: g++
- Optimization: -O3
- SIMD: AVX2
- Platform: Windows

---

Installation

Clone the repository

git clone https://github.com/khankamil67130-jpg/xcrossfire.git
cd xcrossfire

Build the project

mkdir build
cd build
cmake ..
cmake --build . --config Release

---

Python Example

import xcrossfire

data = [1, 5, 10, 20, 30]

index = xcrossfire.search(data, 20)

print(index)

---

Design Goals

- High practical performance
- Minimal memory overhead
- Native execution
- Python integration
- Hardware-aware implementation
- Reproducible benchmarking

---

Limitations

- Optimized for x86_64 processors with AVX2 support.
- Performance depends on hardware, compiler, operating system, and workload characteristics.
- Benchmark numbers should not be interpreted as universal performance guarantees.

---

Roadmap

- Python package distribution
- Additional benchmarks
- Linux support improvements
- ARM/NEON exploration
- Continuous Integration
- Performance regression testing
- Extended documentation

---

License

This project is released under the MIT License.

---

Author

Kamil Khan

Student | Systems Programming | High Performance Computing | C++ | Python | AI & LLM Engineering

GitHub:
https://github.com/khankamil67130-jpg

---

Disclaimer

This repository presents an experimental high-performance search engine implementation. Performance results are based on benchmark measurements performed under the documented test conditions. Independent verification on different hardware platforms is encouraged.
