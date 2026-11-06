# B4 - Extension and engineering decision
## Practical extension
extension.hpp adds TelemedicineBill with minutes * per-minute rate and CorporatePayer paying 90% of any Bill total. These derive only from the existing interfaces; include/billing.hpp and the existing bill/payer classes are unchanged by the extension. main.cpp reruns the exact B1 originalTests() function, then accesses both new objects through base-class unique_ptrs. A 30-minute consultation at 40 totals 1200; corporate pays 1080. A government claim on the new TELE type throws UnknownPackage and is recovered without destroying the stored bill prematurely. The unknown-package rejection is deliberate: a fixed package payer must not invent a rate for a new bill type.

## Resource evidence
The extension run compiles with -fsanitize=address -g -fno-omit-frame-pointer. It exits zero with no sanitizer diagnostic while including thrown and caught UnknownPackage. The B2 error runs likewise release their scoped ownership and restore the guard count. The isolated A2 undefined-behaviour run is excluded from this clean claim. leak_probe.txt records that detect_leaks=1 is unsupported on this local platform, so these results establish clean observed address checks and RAII behaviour, not independent LeakSanitizer certification. No Linux CI result is asserted.

## Decision for the hospital group
The group adds two bill types and three payer policies a year across 15 hospitals. The extension shows that adding one of each requires two new classes and composition/test updates, with no changes to the four original bill classes or three original payer classes. This favours polymorphic interfaces at the business boundary. The dispatch measurement is a numeric loop with no databases, invoices, audit persistence or network calls; 200000 bills a day alone does not establish a throughput bottleneck. Multiplying a tiny benchmark by daily bill count would conceal those unmeasured costs.

Use exceptions for unexpected missing records, allocation failure and internal faults, with per-request recovery and durable audit. Use explicit validation results for frequent routine claim rejection: the measured high-error exception path is expensive. A production system should preserve bills transactionally, represent currency in integer minor units with specified rounding, validate configuration before activation, and retry only operations with an idempotency policy. Those storage and accounting features are beyond this in-memory assignment, so "never lose a bill" here means retaining the owning collection during claim errors, not durability through power loss.

## Configuration plug-in alternative
```
ALGORITHM EvaluateConfiguredBill
INPUT ValidatedRule, ChargeFields
OUTPUT Total
BEGIN
    SET Total TO 0
    FOR EACH Term IN ValidatedRule.Terms DO
        VALIDATE referenced charge field
        SET Total TO Total + evaluated Term
    END FOR
    OUTPUT Total
END
```

A configuration registry could map bill codes to rate formulas and payer codes to coverage rules. Validate schemas, bounds, allowed operations and versions before activation; retain the rule version with each bill so historical amounts stay reproducible. A rule with r terms costs O(r) per bill instead of one O(1) cached virtual total; parsing and validation cost O(R) for R total rule size at load time. Storage is O(R) plus per-bill referenced data. If rules are compiled/cached, the online calculation can approach the existing constant cost. A fixed rule format is easy to update for rates but cannot express arbitrary new clinical logic without an engine extension. Unrestricted native plug-ins add ABI, lifetime and crash risks; a rate configuration is data, not automatically executable code. Class extensions are type checked but need rebuilding and deployment. Versioned configuration is appropriate for rates; a small stable polymorphic layer remains useful for distinct behaviour.
