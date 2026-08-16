# NETWORK DEGRADATION MATRIX R1

Correctness matrix for packaged Client/Server. Local performance baseline remains without emulation; these runs validate state correctness under imperfect network conditions.

| ID | Lag | Loss | Jitter | Required correctness |
|---|---:|---:|---:|---|
| NET-00 | 0 ms | 0% | 0 ms | Baseline connect/deploy/fire/revive/vehicle/objective |
| NET-01 | 80 ms | 1% | 10 ms | No duplicate fire/revive; movement remains recoverable |
| NET-02 | 150 ms | 3% | 20 ms | No immortal/stale pawn; objective/tickets remain canonical |
| NET-03 | 250 ms | 5% | 35 ms | No RPC flood/duplication/seat ownership split-brain |
| NET-04 | 500 ms | 10% | 50 ms | Stress/informational: no exploit, crash or permanent server corruption |

Use Unreal Engine Network Emulation commands (`NetEmulation.PktLag`, `NetEmulation.PktLoss`, `NetEmulation.PktJitter`). NET-04 is deliberately harsh and is not a responsiveness target.
