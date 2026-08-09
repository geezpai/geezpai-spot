# Known Issue: DA217 Motion Interrupt

> **Status**: Open — 运动中断未按预期触发。
> 详细调试进展见下方假设列表。修复后请更新本文档。

## Reproduction Steps
1. Build and download current firmware to the board.
2. Boot the device and wait until the app enters normal running state.
3. Shake or move the device to trigger the DA217 motion interrupt.
4. Observe serial log and LCD MOVE indication behavior.

## Hypotheses & Verification
| ID | Hypothesis | Likelihood | Effort | Evidence |
|----|------------|------------|--------|----------|
| A | DA217 interrupt source is configured, but `DA217_REG_MOTION_FLAG` never reports `ACTIVE_INT` because threshold/duration is too strict. | High | Low | Pending |
| B | DA217 motion interrupt is generated internally, but `P0.31` GPIO interrupt edge/polarity does not match the sensor output behavior. | High | Low | Pending |
| C | DA217 GPIO callback runs, but the work handler reads the flag too late or clears it implicitly, so software misses the event. | Medium | Low | Pending |
| D | FHN motion detector lifecycle is not actually enabling the DA217 path when we expect, so the sensor stays inactive. | Medium | Low | Pending |
| E | The LCD MOVE card is unrelated to the root cause; the sensor interrupt itself is missing, and the UI is only exposing that underlying failure. | Medium | Low | Pending |

## Log Evidence
- Pending instrumentation

## Verification Conclusion
- Pending
