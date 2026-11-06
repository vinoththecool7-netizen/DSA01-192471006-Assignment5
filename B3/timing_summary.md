### Median traversal time (milliseconds; 3 repetitions)

| Bills | Virtual | Switch | Variant |
| --- | --- | --- | --- |
| 1,000 | 0.002792 | 0.002000 | 0.002292 |
| 1,000,000 | 2.760625 | 2.071292 | 2.263250 |
| 10,000,000 | 30.578125 | 25.601167 | 22.623625 |

### Median validation time (1,000,000 inputs; milliseconds)

| Invalid | Status | Exceptions | Optional |
| --- | --- | --- | --- |
| 0% | 2.264875 | 2.708166 | 2.256083 |
| 1% | 2.255458 | 231.753209 | 2.252667 |
| 50% | 2.252792 | 10954.148250 | 2.253916 |

For 10,000,000 bills, the observed medians were 30.578 ms virtual, 25.601 ms switch and 22.624 ms variant. Here the contiguous variant was fastest, with differences that include locality and allocation layout as well as dispatch. At 1% invalid inputs, exceptions took 231.753 ms compared with 2.255 ms for status. At 50%, repeated unwinding dominates. At 0%, the measured exception success path is close to the explicit-result paths.

Observed sizes in bytes: Base 8, OP 24, IP 32, DC 16, ER 32; owning pointer 8; switch 32; variant 32; optional<double> 16; status int 4. A virtual bill in the measured container therefore requests object bytes plus an 8-byte list slot; allocation overhead is extra. These values are platform-specific.
