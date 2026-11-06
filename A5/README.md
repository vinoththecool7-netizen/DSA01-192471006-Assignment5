# A5 - Recovery, rethrowing and resource lifetime
## Catch boundaries and bill retention
Construction validates a candidate before ownership enters the bill collection. During claim processing the bill remains owned by vector<unique_ptr<Bill>>; claim failure does not erase it. The claim service catches ClaimError by const reference, writes an audit message, and uses throw; to preserve the active exception. A per-request boundary reports the reason and continues to the next request. A final catch(...) handles unexpected types at that boundary. Continuing means the error was isolated to one request, not that an arbitrary corrupted process is recoverable.

```
ALGORITHM ProcessClaims
INPUT StoredBills and SelectedPayer
OUTPUT Per-bill decisions and retained bills
BEGIN
    FOR EACH Bill IN StoredBills DO
        TRY
            TRY
                SET Paid TO SelectedPayer.amountCovered(Bill)
            CATCH ClaimError AS Error
                LOG Error
                RETHROW current exception
            END TRY
            OUTPUT approved payment Paid
        CATCH standard exception AS Error
            OUTPUT rejection reason Error
        CATCH any other exception
            OUTPUT unexpected failure for current bill
        END TRY
        RETAIN Bill in StoredBills
    END FOR
END
```

A logging failure must not replace the original exception; the supplied demonstration uses ordinary non-throwing stream settings. A production audit sink should offer an explicit failure policy.

## Prediction and observation
prediction.json records `~f ~main caught 42` followed by newline before first execution. Actual output matches exactly. f's local guard dies first as its stack frame unwinds, then main's guard inside the try block dies, then the matching catch(int) prints 42. Control transfers out of f immediately on throw, so the statement printing "not printed" is never reached. main's guard is scoped inside the try, not around the handler.

RAII ties ownership to object lifetime: unique_ptr frees an allocation, vector releases owned elements, and a file stream closes its handle while unwinding. Only fully constructed objects are destroyed. If a constructor throws, its completed members and bases are destroyed, but that object's own destructor is not called.

## Alternative: manual cleanup
```
ALGORITHM WorkWithManualCleanup
INPUT Request
OUTPUT Result or failure
BEGIN
    ALLOCATE Resource
    TRY
        PROCESS Request using Resource
    CATCH any exception
        RELEASE Resource
        RETHROW current exception
    END TRY
    RELEASE Resource
END
```

For r resources, releases still take O(r) work with either design. Manual cleanup requires matching releases on success and every escaping path: r resources and e independent exits can create up to r*e cleanup obligations. Adding one unhandled exit creates up to r leak opportunities; a new acquisition that throws part way through needs special partial-construction cleanup. A shared cleanup block can reduce duplication, but then it is effectively rebuilding an RAII owner. RAII puts each resource's release rule in one owner and allows O(r) state, without adding a release to every new catch block. B2 checks printed guard destruction and live-count restoration, and B4 repeats original billing tests after extension under AddressSanitizer.
