from pathlib import Path

path = Path(__file__).resolve().parent / "OsterConflict/Source/OsterConflict/Private/OCR13FrontendMenuSubsystem.cpp"
text = path.read_text(encoding="utf-8")
old1 = '        LegacyFrontend->SetVisibility(ESlateVisibility::Collapsed);\n        LegacyFrontend->SetIsEnabled(false);\n        LegacyFrontend->RemoveFromParent();'
new1 = '        LegacyFrontend->SetVisibility(ESlateVisibility::Collapsed);\n        LegacyFrontend->SetIsEnabled(false);\n        // Pass 27: keep the native frontend attached to its original WidgetTree. Detaching a widget\n        // after UUserWidget::RebuildWidget has already produced Slate children creates an avoidable\n        // structural lifetime edge; collapsed + disabled is sufficient to suppress it.'
old2 = '        LegacyFrontend->SetVisibility(ESlateVisibility::Collapsed);\n        LegacyFrontend->SetIsEnabled(false);\n        if (LegacyFrontend->GetParent()) LegacyFrontend->RemoveFromParent();'
new2 = '        LegacyFrontend->SetVisibility(ESlateVisibility::Collapsed);\n        LegacyFrontend->SetIsEnabled(false);\n        // Pass 27: never detach the root-owned legacy frontend after Slate has been built.'
for old, new, label in [(old1, new1, "BuildFrontend detach"), (old2, new2, "SuppressLegacy detach")]:
    if old not in text:
        raise SystemExit(f"PASS27B failed: missing {label}")
    text = text.replace(old, new, 1)
if 'LegacyFrontend->RemoveFromParent()' in text:
    raise SystemExit('PASS27B failed: legacy frontend detach remains')
path.write_text(text, encoding="utf-8")
print('PASS27B APPLIED: native WidgetTree frontend stays attached/collapsed')