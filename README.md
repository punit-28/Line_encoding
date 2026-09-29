# 📡 Digital Signal Encoder / Decoder

A C++17 implementation of fundamental digital communication and signal encoding techniques.

## ✨ Features

- Digital binary data generation (8–256 bits)
- Longest palindromic bit-run detection
- NRZ-L
- NRZ-I
- Manchester
- Differential Manchester
- AMI
- B8ZS scrambling
- HDB3 scrambling
- Pulse Code Modulation (PCM)
- Delta Modulation (DM)
- Signal encoding and decoding
- Automatic SVG waveform generation

## 🧠 Digital Encoding

The application can generate random binary data and encode it using multiple line-coding techniques.

| Technique | Working |
|---|---|
| NRZ-L | 0 → low level, 1 → high level |
| NRZ-I | 1 → transition, 0 → no transition |
| Manchester | Mid-bit transition represents the bit |
| Differential Manchester | Always has a clock transition; data is represented by start transition |
| AMI | 0 → 0V, 1 → alternating +V / -V |
| B8ZS | AMI with replacement of 8 consecutive zeros |
| HDB3 | AMI with replacement of 4 consecutive zeros |

When AMI is selected, the program can additionally apply B8ZS or HDB3 scrambling.

## 📊 Analog Encoding

The analog mode generates a synthetic waveform and supports:

### PCM
Analog samples are quantized into 2–8 bits per sample, converted into a binary stream, line-coded, decoded, and reconstructed.

### Delta Modulation
One bit is generated per sample to indicate whether the signal is above or below the current staircase approximation.

## 📈 Visualization

All waveforms are written as standalone SVG files. Generated plots are stored in the `output/` directory.

Typical outputs:

```
output/digital_NRZ_L.svg
output/digital_Manchester.svg
output/digital_AMI.svg
output/pcm_line_signal.svg
output/dm_line_signal.svg
output/decoder_staircase.svg
```

SVG files can be opened directly in Chrome, Edge, Firefox, VS Code, or any SVG-compatible application.

## 🏗️ Project Structure

```
Lint_encoding/
├── include/
│   ├── DataGenerator.h
│   ├── DeltaModulation.h
│   ├── LineCoder.h
│   ├── PCM.h
│   ├── Palindrome.h
│   └── SignalPlotter.h
│
├── src/
│   ├── DataGenerator.cpp
│   ├── DeltaModulation.cpp
│   ├── LineCoder.cpp
│   ├── PCM.cpp
│   ├── Palindrome.cpp
│   ├── SignalPlotter.cpp
│   └── main.cpp
│
├── .vscode/
├── Makefile
└── README.md
```

### Core Modules

- **main.cpp** — interactive CLI and complete program flow
- **LineCoder** — polymorphic implementation of line-coding schemes
- **PCM** — quantization, binary encoding, and reconstruction
- **DeltaModulation** — 1-bit-per-sample encoding and staircase decoding
- **DataGenerator** — random bit generation and forced zero-run generation
- **Palindrome** — longest palindromic run detection using a Manacher-style approach
- **SignalPlotter** — standalone SVG waveform generation

## ⚙️ Requirements

- C++17 compatible compiler
- g++
- make

No third-party C++ libraries are required.

## 🚀 Installation & Usage

### 1. Clone

```bash
git clone https://github.com/punit-28/Lint_encoding.git
cd Lint_encoding
```

### 2. Build

```bash
make
```

The Makefile builds an executable named:

```
signal_codec
```

### 3. Run

Linux / macOS:

```bash
./signal_codec
```

Windows / MinGW:

```bash
./signal_codec.exe
```

## 🎮 Program Flow

On startup, choose:

```
1. Digital input
2. Analog input
```

### Digital Flow

```
Generate binary data
        ↓
Find longest palindrome
        ↓
Choose line coding
        ↓
Encode bits → signal levels
        ↓
Generate SVG waveform
        ↓
Optionally decode
        ↓
Compare decoded bits with original data
```

### Analog Flow

```
Generate synthetic analog signal
        ↓
PCM / Delta Modulation
        ↓
Binary stream
        ↓
Line coding
        ↓
Generate SVG waveform
        ↓
Decode
        ↓
Reconstruct staircase
```

## 🧩 Architecture

The line coding implementation uses an abstract `LineCoder` interface with virtual `encode()` and `decode()` operations.

Each encoding scheme is implemented as its own class, allowing the application to select schemes polymorphically rather than placing all encoding logic in one large conditional block.

Signals are represented using two voltage samples per bit interval:

```
[first half of bit, second half of bit]
```

This supports both constant-level schemes such as NRZ-L/AMI and transition-based schemes such as Manchester.

## 🔢 Complexity

Most processing stages are linear in the number of bits/samples.

| Component | Complexity |
|---|---:|
| Data generation | O(n) |
| Line encoding | O(n) |
| Line decoding | O(n) |
| PCM encode/decode | O(n) |
| Delta Modulation | O(n) |
| Palindrome detection | O(n) |
| SVG generation | O(n) |

## 🧹 Clean

Remove build artifacts and generated SVG files:

```bash
make clean
```

Then rebuild with:

```bash
make
```

## 🛠️ Future Improvements

- File-based signal input
- Custom analog signal input
- Noise/channel simulation
- Bit Error Rate (BER) analysis
- Additional line-coding schemes
- More waveform visualizations
- Automated unit tests
- GUI-based visualization

## 🎓 Academic Purpose

This project is a practical implementation of concepts from Data Communication / Digital Communication, covering the path from source data to encoded signal, decoding, and signal reconstruction.

## 👨‍💻 Author

**Punit Mundotiya**

B.Tech in Information Technology, NIT Srinagar

GitHub: https://github.com/punit-28

## 📄 License

This project is intended for academic and educational use. Feel free to study, modify, and extend the implementation.
