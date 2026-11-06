# B1 - Billing implementation and validation
## Completed supplied program
complete.cpp replaces only the three TODO regions. The attachment's final closing brace was swallowed by a // comment during pasting; it is restored on its own line. No other teaching-skeleton behaviour is hardened or replaced. expected.txt is the supplied expected output, consumed by scripts/verify.py and compared byte for byte.

The observed output is OP Ravi: 1700, IP Meena: 47500, Total 49200 on three lines. The full implementation is separate in include/billing.hpp. It uses validated constructors, immutable cached totals and unique ownership. No interactive input is required; embedded fixtures are the actual inputs, not unused input files.

## Independent expected values
| Bill and hand calculation | Total | SelfPay | Insurance 20% copay | Government |
| --- | --- | --- | --- | --- |
| OP: 500 + 1200 | 1700 | 1700 | 1360 | 1000 |
| IP: 3*2500 + 40000 + 500 | 48000 | 48000 | 38400 | 40000 |
| DC: package | 20000 | 20000 | 16000 | 18000 |
| ER: 1000 + 2*750 | 2500 | 2500 | 2000 | 2500 |

The insurer cover is 50000. The day's total is 72200. Tests use independent constants for all 12 bill/payer combinations, plus zero charges, an exact-cover boundary and 100% copay. All pass. These examples use exactly representable totals, so assertions compare exactly; a general fractional-currency system needs integer minor units or a specified rounding policy.

## Runtime binding and destructor experiments
Removing virtual from a pure virtual total() is ill-formed: a non-virtual function cannot be declared pure, and the derived override declarations no longer override. See nonvirtual_pure.cpp.txt and the captured diagnostic. It has no runtime output. To demonstrate changed runtime behaviour, early_binding.cpp is a separate legal program: total() has a concrete non-virtual base implementation returning zero and derived overrides are replaced by hiding methods. Through Bill* it prints `0 0 Total 0`. This distinguishes a compile failure from early binding.

The A2 non-virtual-destructor experiment is reused for B1.4; it removes both the base virtual destructor keyword and the now-invalid derived override specifier, then runs in an isolated ASan process. Its nonzero result is an unsafe observation, not evidence of a clean program. See A2/evidence/a2_unsafe.json for the actual termination. Destructor-chain output is guaranteed only in the correct virtual-destructor version.

## Design comparison and template alternative
The implementation preserves A1/A3 interfaces and all four bill types. Validation is done at construction and totals are cached so noexcept is justified. The skeleton's IP example omits medicines while the full design includes them. Types and patient strings remain shared metadata; factories establish ownership.
```
ALGORITHM TotalHomogeneousContainers
INPUT OPBills, IPBills, DCBills, ERBills
OUTPUT Sum
BEGIN
    SET Sum TO 0
    FOR EACH Container IN the four statically typed containers DO
        FOR EACH Bill IN Container DO
            SET Sum TO Sum + statically bound total of Bill
        END FOR
    END FOR
    OUTPUT Sum
END
```

A C++ template function sum(container) instantiates the inner loop for each container. Total time is O(n); calls can inline and object storage can be contiguous without a vptr if value types are independent of Bill. Space is O(n+k) for n values and k container headers, with unused capacities depending on allocation strategy. A single arbitrary mixed-order list is lost unless an extra tag/index, variant, or type-erased layer restores that ordering. New bill types need another container and registration at the aggregate site. B3 compares dynamic representations, not a claim that templates are always faster.
