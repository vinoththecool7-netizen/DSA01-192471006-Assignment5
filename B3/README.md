# B3 - Measured dispatch and error cost
## Workload and correctness
benchmark.cpp uses four numeric bill layouts with deterministic data: type = i mod 4, a = 100 + (i mod 97), b = 2, c = 50. OP is a+b, IP is a*b+c, DC is a, ER is a+b*c. The 10 independent hand results are 102, 252, 102, 203, 106, 260, 106, 207, 110, 268: sum 1716. Virtual, switch and variant results each match all ten constants. Larger sizes use a separately accumulated analytic expectation, and every measured sum must match it.

The virtual version stores separately allocated objects through unique_ptr. The switch and variant versions are contiguous value arrays. Variant alternatives are value-only types so they do not redundantly carry a virtual base. Patient strings, payer work and constructor validation are excluded equally: this experiment measures numeric traversal and representation, not complete hospital throughput. Creation and destruction are outside each timed interval. Each representation is built and released separately to limit peak memory.

## Measurement method
Compile with g++ -std=c++17 -Wall -Wextra -O2, without ASan, as requested for timings. Each condition runs three times using steady_clock; raw milliseconds and checksums are retained. All three dispatch paths use a noinline wrapper so calls cannot simply be folded away. Virtual dispatch includes a second indirect call and pointer traversal, and contiguous alternatives benefit from locality. Results therefore compare the implemented representations and include wrapper overhead; they do not isolate an abstract instruction cost. No volatile operation is placed in the inner loop; the final sum is checked and consumed by a volatile sink.

Validators also have the same noinline boundary. A prebuilt million-element input has exactly 0, 10000, or 500000 negative values. The pattern uses (i*37 mod 100) to intersperse errors deterministically. It is not random hospital data. Validation loops count rejects and check the sum. Exceptions construct and throw invalid_argument for each negative, while status and optional return a rejection without throwing. No audit logging is timed.

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

## Interpretation and alternatives
All fixed-type traversals take O(n) time and O(n) input storage. A virtual object commonly adds a vptr, a list pointer and separate allocator metadata. A switch stores its numeric payload plus tag and alignment padding. A variant stores its largest alternative plus discriminator and padding. Actual sizeof measurements above exclude allocator overhead and vector spare capacity. Shared virtual tables cost O(k) per hierarchy, not one entire table per object.

std::visit provides a closed set of alternatives and compile-time checking of the visitor. New alternatives require editing the variant alias and ensuring all visitors support them. It can retain mixed input order in one contiguous list. It is suitable when the set is known; a subclass interface is suitable when extensions arrive independently. A switch is straightforward for a small closed set but duplicates dispatch logic wherever operations are added.

For n validations with fraction p errors, all approaches remain O(n) for fixed-depth handling. Exception costs include O(p*n) allocations/unwinds with large implementation-dependent constants; status/optional have cheap explicit branches for expected invalid data. Measurements are three local repetitions, not independent end-to-end production trials. Cache state, order, allocator and the compiler matter. A 1000-element traversal is especially affected by clock and call overhead. There is no claim of a universal ranking or extrapolated service-level guarantee.
