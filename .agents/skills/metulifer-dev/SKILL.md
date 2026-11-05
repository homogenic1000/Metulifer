---
name: metulifer-dev
description: Specialized workflows and build runbooks for the Metulifer JUCE audio plugin and its documentation site. Use when building, testing with pluginval, profiling DSP performance, or updating documentation.
---

# Metulifer Development & Workflow Runbook

This skill provides verified operational procedures for developing, building, testing, and documenting the **Metulifer** JUCE audio synthesizer and sequencer plugin.

---

## 1. Environment & Projucer Resave

Projucer generates the Xcode project and library links. Run this whenever `.jucer` is modified or after pulling git changes:

```bash
# Projucer alias or absolute path
/Applications/Projucer.app/Contents/MacOS/Projucer --resave Metulifer.jucer
```

---

## 2. Compilation (Debug & Release)

Always build **both** Debug and Release to keep local development binaries in sync:

```bash
# 1. Debug Standalone build
DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer xcodebuild \
  -project Builds/MacOSX/Metulifer.xcodeproj \
  -scheme "Metulifer - Standalone Plugin" \
  -configuration Debug build

# 2. Release All Targets build (VST3, AU, Standalone)
DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer xcodebuild \
  -project Builds/MacOSX/Metulifer.xcodeproj \
  -scheme "Metulifer - All" \
  -configuration Release build
```

The resulting binaries are placed in:
- `Builds/MacOSX/build/Debug/Metulifer.app`
- `Builds/MacOSX/build/Release/Metulifer.vst3`
- `Builds/MacOSX/build/Release/Metulifer.component` (AU)

---

## 3. Strict Real-Time Audio Constraints

When writing DSP code (`Source/DSP/` and `Source/PluginProcessor.cpp`):
1. **Never allocate on the audio thread**: No `std::vector::push_back`, `new`, `std::make_unique`, `malloc`, or string manipulations in `processBlock()` or `Voice::process()`.
2. **Never block or acquire locks**: No `std::mutex`, `juce::CriticalSection`, or file I/O on the audio thread.
3. **Voice sleep & tail management**: When all steps are inactive and envelope tails drop below `-80 dBFS` (`1e-4f`), set `isVoiceActive = false` to skip polyBLEP oscillators, filters, and saturated delays.
4. **WindowServer hardware acceleration**: Keep `setOpaque (true)` on `PluginEditor` to prevent window-drag lag under macOS WindowServer.
5. **No disk I/O in UI painting**: All skin lookups must use in-memory caches with negative caching (`Skin::imageCache`).

---

## 4. Verification with pluginval

Test both VST3 and AU formats with strict Level 5 validation:

```bash
# Test VST3
pluginval --strictness-level 5 --validate-in-process --validate "Builds/MacOSX/build/Release/Metulifer.vst3"

# Test AU
pluginval --strictness-level 5 --validate-in-process --validate "Builds/MacOSX/build/Release/Metulifer.component"
```

---

## 5. Documentation Workflow (Fumadocs / Next.js)

The interactive docs site is located in `docs/`:

```bash
# Install dependencies
cd docs && pnpm install

# Build static export for GitHub Pages
NEXT_PUBLIC_BASE_PATH=/Metulifer pnpm build
```

Output is generated in `docs/out/` with `.nojekyll` and deployed to GitHub Pages automatically via `.github/workflows/deploy-docs.yml` or the `gh-pages` branch.
