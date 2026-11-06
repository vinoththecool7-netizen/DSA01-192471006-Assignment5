# Assignment 5 - CO5: Reliable software with runtime polymorphism and exceptions

**S Vinoth Kumar | Register number 192471006**

Repository: https://github.com/vinoththecool7-netizen/DSA01-192471006-Assignment5

[PDF report](Assignment5_Report.pdf) | [Report source](Assignment5_Report.md) | [Original specification](reference/specification.txt)

## Build and reproduce

Compiler actually used: **Apple clang 21.0.0 (clang-2100.4.1.1)**, target arm64-apple-darwin27.2.0. On this machine `g++` is the Apple clang driver, not GNU GCC. C++17 and libc++ were used. No third-party C++ libraries are needed.

From the repository root:

```sh
mkdir -p build
python3 scripts/verify.py all
```

The verifier builds each executable independently and checks exact supplied outputs, independent bill/payer constants, prediction bytes, expected diagnostic categories and benchmark checksums. It runs the deliberately undefined A2 deletion in an isolated process and records its actual outcome. That experiment is excluded from successful sanitizer results. `python3 scripts/verify.py B1` runs only B1; A2, A5, B2, B3 and B4 also work as selectors. A1, A3 and A4 are design tasks with no separate required executable.

All program inputs are embedded fixtures in the source files. `B1/expected.txt` and `B2/expected.txt` are consumed by the verifier for exact stdout comparisons. No stdin or unused sample input files are required. Large B3 timing runs take roughly a minute here and allocate millions of records. B3's `--check` run tests the 10 hand-calculated bills with ASan, without timing the large workload.

Correctness/safety flags: `-std=c++17 -Wall -Wextra -fsanitize=address -g -fno-omit-frame-pointer`. Timing flags: `-std=c++17 -Wall -Wextra -O2`, without sanitizer instrumentation. Every actual command and compiler output is retained in task evidence.

## Task results

| Task | Evidence and result |
| --- | --- |
| [A1](A1/README.md) | Four bill types; common interface; UML; daily-total and switch pseudocode; complexity |
| [A2](A2/README.md) | Prediction matches; conceptual vtables; exact keyword-removal compile failure; isolated unsafe deletion |
| [A3](A3/README.md) | Three payer policies and corporate extension design; function-policy comparison |
| [A4](A4/README.md) | Exception hierarchy; justified noexcept contracts; status alternative |
| [A5](A5/README.md) | Unwinding prediction matches; catch/rethrow and RAII recovery algorithms |
| [B1](B1/README.md) | Completed skeleton matches; 12 payer checks and three boundaries pass; early binding and pure-virtual diagnostics |
| [B2](B2/README.md) | Completed skeleton and prediction match; nine recovered errors; bad_alloc injection; slicing demonstration |
| [B3](B3/README.md) | Ten hand results agree; three repetitions at all requested sizes/error rates; switch, virtual, variant, status, optional and exception timings |
| [B4](B4/README.md) | New bill/payer without existing-class changes; original tests pass; exception recovered; address checks clean |

## Interpretation and evidence limits

Insurance uses a 20% copay and rejects an insurer share above the cover; government rates are explicit exercise assumptions. SelfPay returns the amount paid by the patient, so it returns the full total. Constructors validate and cache amounts. The full IP model includes medicines; the supplied B1 skeleton remains its original surgery-only formula.

Predictions were saved with UTC timestamps and source hashes before first execution in this preparation session. No earlier personal execution or development dates are claimed. One genuine task commit is retained per task; commits were produced during this work session, not spread over invented dates.

The unsafe A2 deletion terminated locally with SIGTRAP (return code -5), with empty stdout/stderr; no ASan diagnostic category is asserted. Merely removing `virtual` while retaining the derived destructor `override` fails compilation. The legal early-binding demonstration is separate from the invalid pure-virtual edit.

Correct programs exit zero under AddressSanitizer. Local `ASAN_OPTIONS=detect_leaks=1` is unsupported (actual probe retained), so this is not a LeakSanitizer certification. Allocation failure is deterministic class-specific `operator new` fault injection, not a claim of physical memory exhaustion. The in-memory collection retains bills across claim errors; durable database recovery is beyond this assignment.

Each `evidence/output_screenshot.png` is a real browser screenshot of `evidence/viewer.html`, which displays the actual captured logs and/or design figure. It is labelled as a recorded-evidence viewer, not an original terminal screenshot. Text logs, JSON return codes and source digests are the primary evidence. Only project/home path prefixes are sanitised in exported process output.

## Exact compile and run commands

Successful commands below require `mkdir -p build` first. Teaching diagnostics intentionally return nonzero and have `.cpp.txt` extensions so they do not break an ordinary source-file build.

### a2_invalid_destructor

```sh
g++ -std=c++17 -Wall -Wextra -fsanitize=address -g -fno-omit-frame-pointer -x c++ A2/nonvirtual_destructor.cpp.txt -o build/a2_invalid_destructor
```

Compile exit: 1. [Capture](A2/evidence/a2_invalid_destructor.json)

### a2_predict

```sh
g++ -std=c++17 -Wall -Wextra -fsanitize=address -g -fno-omit-frame-pointer A2/predict.cpp -o build/a2_predict
build/a2_predict
```

Compile exit: 0. Run exit: 0. [Capture](A2/evidence/a2_predict.json)

### a2_unsafe

```sh
g++ -std=c++17 -Wall -Wextra -fsanitize=address -g -fno-omit-frame-pointer A2/unsafe_delete.cpp -o build/a2_unsafe
build/a2_unsafe
```

Compile exit: 0. Run exit: -5. [Capture](A2/evidence/a2_unsafe.json)

### a5_predict

```sh
g++ -std=c++17 -Wall -Wextra -fsanitize=address -g -fno-omit-frame-pointer A5/predict.cpp -o build/a5_predict
build/a5_predict
```

Compile exit: 0. Run exit: 0. [Capture](A5/evidence/a5_predict.json)

### b1_complete

```sh
g++ -std=c++17 -Wall -Wextra -fsanitize=address -g -fno-omit-frame-pointer B1/complete.cpp -o build/b1_complete
build/b1_complete
```

Compile exit: 0. Run exit: 0. [Capture](B1/evidence/b1_complete.json)

### b1_early

```sh
g++ -std=c++17 -Wall -Wextra -fsanitize=address -g -fno-omit-frame-pointer B1/early_binding.cpp -o build/b1_early
build/b1_early
```

Compile exit: 0. Run exit: 0. [Capture](B1/evidence/b1_early.json)

### b1_nonvirtual_pure

```sh
g++ -std=c++17 -Wall -Wextra -fsanitize=address -g -fno-omit-frame-pointer -x c++ B1/nonvirtual_pure.cpp.txt -o build/b1_nonvirtual_pure
```

Compile exit: 1. [Capture](B1/evidence/b1_nonvirtual_pure.json)

### b1_tests

```sh
g++ -std=c++17 -Wall -Wextra -fsanitize=address -g -fno-omit-frame-pointer B1/main.cpp -o build/b1_tests
build/b1_tests
```

Compile exit: 0. Run exit: 0. [Capture](B1/evidence/b1_tests.json)

### b2_allocation

```sh
g++ -std=c++17 -Wall -Wextra -fsanitize=address -g -fno-omit-frame-pointer B2/allocation.cpp -o build/b2_allocation
build/b2_allocation
```

Compile exit: 0. Run exit: 0. [Capture](B2/evidence/b2_allocation.json)

### b2_complete

```sh
g++ -std=c++17 -Wall -Wextra -fsanitize=address -g -fno-omit-frame-pointer B2/complete.cpp -o build/b2_complete
build/b2_complete
```

Compile exit: 0. Run exit: 0. [Capture](B2/evidence/b2_complete.json)

### b2_errors

```sh
g++ -std=c++17 -Wall -Wextra -fsanitize=address -g -fno-omit-frame-pointer B2/errors.cpp -o build/b2_errors
build/b2_errors
```

Compile exit: 0. Run exit: 0. [Capture](B2/evidence/b2_errors.json)

### b2_predict

```sh
g++ -std=c++17 -Wall -Wextra -fsanitize=address -g -fno-omit-frame-pointer B2/predict.cpp -o build/b2_predict
build/b2_predict
```

Compile exit: 0. Run exit: 0. [Capture](B2/evidence/b2_predict.json)

### b2_slicing

```sh
g++ -std=c++17 -Wall -Wextra -fsanitize=address -g -fno-omit-frame-pointer B2/slicing.cpp -o build/b2_slicing
build/b2_slicing
```

Compile exit: 0. Run exit: 0. [Capture](B2/evidence/b2_slicing.json)

### b3_benchmark

```sh
g++ -std=c++17 -Wall -Wextra -O2 B3/benchmark.cpp -o build/b3_benchmark
build/b3_benchmark
```

Compile exit: 0. Run exit: 0. [Capture](B3/evidence/b3_benchmark.json)

### b3_small_asan

```sh
g++ -std=c++17 -Wall -Wextra -fsanitize=address -g -fno-omit-frame-pointer B3/benchmark.cpp -o build/b3_small_asan
build/b3_small_asan --check
```

Compile exit: 0. Run exit: 0. [Capture](B3/evidence/b3_small_asan.json)

### b4_extension

```sh
g++ -std=c++17 -Wall -Wextra -fsanitize=address -g -fno-omit-frame-pointer B4/main.cpp -o build/b4_extension
build/b4_extension
```

Compile exit: 0. Run exit: 0. [Capture](B4/evidence/b4_extension.json)

## Rebuild report and output images

The PDF and SVG generation uses Python `reportlab`. Run `python3 scripts/build_report.py` after regenerating evidence; it refreshes timing tables from the captured CSV. The task READMEs are the report content sources. The consolidated Markdown and PDF are generated copies. The report contains readable vector diagrams and 11-point or larger text.

Browser evidence capture uses Node.js with `playwright` and an installed Google Chrome: `node scripts/capture_evidence.cjs`. This rebuilds viewer HTML and PNG screenshots from raw logs; it does not rerun C++ programs. Report dependencies are for artifact generation only.
