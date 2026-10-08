# Mantra Saturator

A VST3 saturator plugin for Windows 64-bit. Four knobs: **Drive**, **Tone**, **Mix**, **Output**.
4x oversampled tanh saturation, smoothed parameters, dark minimal UI.

You do not need to install anything to build this. GitHub compiles it in the cloud.

## What it does

- **Drive** (0–100%): pushes the signal into a tanh waveshaper. Subtle warmth at the low end, aggressive clipping at the top.
- **Tone** (0–100%): lowpass filter on the saturated signal, 2 kHz to 20 kHz. Tames harshness.
- **Mix** (0–100%): parallel blend of dry and saturated signal.
- **Output** (-12 to +12 dB): makeup gain.

## How to build it (no coding, no installs)

1. Make a free account at https://github.com (if you don't have one).
2. Create a new repository. Name it `mantra-saturator`. Public or private, either works.
3. On the repo page, click **"uploading an existing file"**.
4. Drag the entire contents of this folder in (keep the folder structure: `Source/`, `.github/`, `CMakeLists.txt`, this README).
5. Click **Commit changes**.
6. Go to the **Actions** tab. You'll see "Build VST3" running. Wait for the green checkmark (roughly 15–25 minutes the first time).
7. Click the finished run, download **MantraSaturator-Windows-VST3** under Artifacts.
8. Unzip it. Copy the `Mantra Saturator.vst3` folder into `C:\Program Files\Common Files\VST3\`.
9. Open FL Studio, rescan plugins. It's under Mantra in your plugin list.

If the build ever fails (red X instead of green check), send the error log to Muse and it gets fixed.

## Tweaking it later

Everything about how it sounds lives in `Source/PluginProcessor.cpp`:
- The saturation character: the `std::tanh (driven)` line and the drive curve above it.
- The tone filter range: the `2000.0f * std::pow (10.0f, tone01)` line.
- Knob ranges and defaults: `createParameterLayout()` in the same file.

The look lives in `Source/PluginEditor.cpp` (colors are at the top of the file).

## Project layout

```
mantra-saturator/
├── CMakeLists.txt                  # build configuration (pulls JUCE 9.0.3 automatically)
├── README.md                       # this file
├── .github/workflows/build.yml     # the cloud build instructions
└── Source/
    ├── PluginProcessor.h / .cpp    # DSP: parameters, oversampling, saturation
    └── PluginEditor.h / .cpp       # UI: knobs, dark theme
```
