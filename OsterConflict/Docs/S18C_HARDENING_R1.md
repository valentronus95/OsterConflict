# S18C HARDENING R1

Source-only correction pass aligned to MASTER-TЗ v4.2.

## Implemented
- Direct Connect appends `Protocol=<OCBuildVersion::NetworkProtocol>`.
- `AOCGameMode::PreLogin` rejects explicit protocol mismatch with `VERSION_MISMATCH`; Shipping rejects missing protocol, non-Shipping logs legacy missing protocol.
- `UOCGameInstance` captures `UEngine::OnNetworkFailure` and `OnTravelFailure`, exposes canonical user-facing status and retry-safe state.
- Sandbox mode no longer grants admin by itself. Local/listen host is allowed in non-Shipping; dedicated dev/test server requires server-owned `?SandboxAdminAll=1`; Shipping disables this shortcut.
- Voice Chat controls are hidden from P0/P1 UI because Voice Chat remains P2. Backend reservation stays for future migration.
- `NetServerMaxTickRate=30` aligned to S20A frozen server target.
- `SettingsSchemaVersion=1` groundwork added for Oster-specific config migration.

## Still execution-pending
- Real UE 5.8 UHT/C++ compile.
- Packaged network failure UX validation.
- Persistent admin identity/authentication beyond dev/test server switch.
- Full S19C localization conversion and gamepad-only UI flow.
- Lag compensation/rewind (P1 in corrected baseline).
