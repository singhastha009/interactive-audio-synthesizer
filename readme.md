# 🎶 Real-Time Audio Synthesizer – CoreAudio + C (macOS)

A command-line audio synthesizer built using **C** and **CoreAudio** (macOS only). This project generates real-time audio output using sine, square, and sawtooth waveforms, and allows dynamic control of frequency, volume, and waveform type via keyboard input.

---

## 🧠 Features

- 🎵 Live sound generation using **CoreAudio**
- 🎛 Control frequency (`w/s`), volume (`+/−`), and waveform (`1/2/3`) while it's running
- 🔁 Continuous audio rendering in the background with interactive key commands
- 🎧 Displays current output device info and sample rate
- 🧵 Multithreaded input handling with `pthread`
- 🖥 Terminal-based user interface (no GUI needed)

---

## ⌨️ Controls

| Key | Action                  |
|-----|--------------------------|
| `w` | Increase frequency (+10Hz) |
| `s` | Decrease frequency (−10Hz) |
| `+` | Increase volume (up to 1.0) |
| `-` | Decrease volume (down to 0.0) |
| `1` | Sine wave                |
| `2` | Square wave              |
| `3` | Sawtooth wave            |
| `q` | Quit the program         |

---

## 🛠 Technologies Used

- **Language:** C  
- **Audio API:** CoreAudio + AudioToolbox (macOS native)  
- **Concurrency:** pthreads  
- **Terminal I/O:** termios

---

## 🚀 How to Build (macOS only)

1. Open Terminal on macOS
2. Compile using:

```bash
clang -o A8 A8.c -framework AudioToolbox -framework CoreAudio -lm -lpthread
Run - ./A8
