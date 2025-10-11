```bash
sbhusal@rivendell ~/w/custom-hashmap-cpp (main)> ./bench 8000; ./bench 64000; ./bench 512000; ./bench 4096000; ./bench 32768000
2025-10-10T20:49:59-04:00
Running ./bench
Run on (48 X 4790.11 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x24)
  L1 Instruction 32 KiB (x24)
  L2 Unified 1024 KiB (x24)
  L3 Unified 32768 KiB (x4)
Load Average: 1.02, 1.01, 0.89
***WARNING*** Library was built as DEBUG. Timings may be affected.
-----------------------------------------------------
Benchmark           Time             CPU   Iterations
-----------------------------------------------------
TestStdMap      53556 ns        53525 ns        12203
2025-10-10T20:49:59-04:00
Running ./bench
Run on (48 X 4791.76 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x24)
  L1 Instruction 32 KiB (x24)
  L2 Unified 1024 KiB (x24)
  L3 Unified 32768 KiB (x4)
Load Average: 1.02, 1.01, 0.89
***WARNING*** Library was built as DEBUG. Timings may be affected.
-----------------------------------------------------
Benchmark           Time             CPU   Iterations
-----------------------------------------------------
TestStdMap    1184791 ns      1184199 ns          568
2025-10-10T20:50:00-04:00
Running ./bench
Run on (48 X 4803.38 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x24)
  L1 Instruction 32 KiB (x24)
  L2 Unified 1024 KiB (x24)
  L3 Unified 32768 KiB (x4)
Load Average: 1.02, 1.01, 0.89
***WARNING*** Library was built as DEBUG. Timings may be affected.
-----------------------------------------------------
Benchmark           Time             CPU   Iterations
-----------------------------------------------------
TestStdMap    9595660 ns      9593120 ns           70
2025-10-10T20:50:01-04:00
Running ./bench
Run on (48 X 4107.28 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x24)
  L1 Instruction 32 KiB (x24)
  L2 Unified 1024 KiB (x24)
  L3 Unified 32768 KiB (x4)
Load Average: 1.02, 1.01, 0.89
***WARNING*** Library was built as DEBUG. Timings may be affected.
-----------------------------------------------------
Benchmark           Time             CPU   Iterations
-----------------------------------------------------
TestStdMap   76970925 ns     76954670 ns            9
2025-10-10T20:50:04-04:00
Running ./bench
Run on (48 X 2197 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x24)
  L1 Instruction 32 KiB (x24)
  L2 Unified 1024 KiB (x24)
  L3 Unified 32768 KiB (x4)
Load Average: 1.02, 1.01, 0.89
***WARNING*** Library was built as DEBUG. Timings may be affected.
-----------------------------------------------------
Benchmark           Time             CPU   Iterations
-----------------------------------------------------
TestStdMap  611803139 ns    611631203 ns            1
```

