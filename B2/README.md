# B2 - Engineered failures and recovery
## Constructed errors
errors.cpp executes nine deliberate requests: negative consultation, insurer share above cover, unknown government package, text passed to stod, missing patient identifier, an unexpected integer exception, NaN charge, copay over 100%, and numeric parse overflow. Each request owns a vector of bills and an allocated array. A stack guard prints ~scope and maintains a live count. An extra bill owned through unique_ptr<Bill> prints ~tracked-bill from its overridden destructor, with a separate live count. All nine failures are caught at the per-request boundary and both live counts return to zero. The integer reaches catch(...), after which processing continues. This shows both typed recovery and the fallback path; standard-library RAII destructors are not individually instrumented.

allocation.cpp forces a real new expression to throw std::bad_alloc through a class-specific failing allocation function. This deterministic fault injection avoids an ASan oversized-allocation abort or operating-system overcommit, neither of which reliably produces a catchable bad_alloc. It tests the catch and unwinding path, not physical memory exhaustion. The guard is destroyed and the handler reports std::bad_alloc. This is the documented mechanism used to force allocation failure.

## Prediction and completed code
The exact B2 prediction is `ok done caught negative [log too large] caught too large ` followed by newline, recorded before execution and verified byte for byte. 100 passes validation and prints done. -5 throws invalid_argument and bypasses the out_of_range-only logger; main catches it as std::exception. 2000000 throws out_of_range, is logged in process(), rethrown, and is finally reported by main. Only that type is logged.

The supplied approve/settle skeleton is completed independently and matches expected.txt exactly: approved 40000; audit: exceeds cover; claim rejected: exceeds cover; error: negative amount, each on its own line. settle catches only ClaimError; a negative amount bypasses it. The outer handler catches ClaimError before std::exception.

throw; rethrows the current exception with its dynamic type and identity. throw e; creates a new exception object from e's static ClaimError type. In the supplied code the dynamic type is already ClaimError, so the visible output may not change. slicing.cpp throws a Detailed subclass and proves the distinction: throw; reaches the Detailed handler, while throw e; reaches the ClaimError handler after slicing. Throwing a base reference by value must not be described as preserving derived information.

## Optional alternative and cost
```
ALGORITHM ApproveOptional
INPUT Billed, Cover
OUTPUT OptionalPaid
BEGIN
    IF Billed < 0 OR Cover < 0 OR Billed > Cover THEN
        OUTPUT no value
    ELSE
        OUTPUT value Billed
    END IF
END
```

No value means rejected, so the caller must test presence; it loses a detailed rejection reason unless another error field is added. B3 measures optional, enum-style status and exceptions on the same million inputs at 0%, 1% and 50% negative values. Each is O(n) overall; the error proportion changes constants dramatically. optional<double> adds an engagement flag and possible padding; its observed size is recorded in the benchmark. An exception result returns an ordinary double on success and uses separate exceptional storage on failure, with metadata and unwind costs that sizeof(double) does not capture. See B3 for raw repetitions, medians, checksums and actual results.
