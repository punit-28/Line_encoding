#include <iostream>
#include <sstream>
#include <functional>
#include <vector>
#include <string>
#include <cmath>
#include <filesystem>

#include "DataGenerator.h"
#include "Palindrome.h"
#include "LineCoder.h"
#include "PCM.h"
#include "DeltaModulation.h"
#include "SignalPlotter.h"

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Small console helpers
// ---------------------------------------------------------------------------
static int askInt(const std::string &prompt, int lo, int hi)
{
    while (true)
    {
        std::cout << prompt;
        int v;
        if (std::cin >> v && v >= lo && v <= hi)
            return v;
        std::cout << "  (please enter a number between " << lo << " and " << hi << ")\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

static bool askYesNo(const std::string &prompt)
{
    while (true)
    {
        std::cout << prompt << " (y/n): ";
        std::string s;
        std::cin >> s;
        if (!s.empty() && (s[0] == 'y' || s[0] == 'Y'))
            return true;
        if (!s.empty() && (s[0] == 'n' || s[0] == 'N'))
            return false;
    }
}

static void printBits(const std::vector<int> &bits)
{
    for (int b : bits)
        std::cout << b;
    std::cout << "\n";
}

// A registry entry for a selectable line coding scheme. Using a table of
// {label, factory} pairs (rather than an if/else or switch ladder) is what
// lets main() dispatch on the user's numeric choice with a single index
// lookup -- the actual per-scheme *logic* lives entirely inside each
// LineCoder subclass via polymorphism.
struct SchemeOption
{
    std::string label;
    std::function<LineCoderPtr()> factory;
};

static std::vector<SchemeOption> lineCodingMenu()
{
    return {
        {"NRZ-L", []
         { return LineCoderPtr(new NRZL()); }},
        {"NRZ-I", []
         { return LineCoderPtr(new NRZI()); }},
        {"Manchester", []
         { return LineCoderPtr(new Manchester()); }},
        {"Differential Manchester", []
         { return LineCoderPtr(new DifferentialManchester()); }},
        {"AMI", []
         { return LineCoderPtr(new AMI()); }},
    };
}

static LineCoderPtr chooseLineCoder()
{
    auto menu = lineCodingMenu();
    std::cout << "\nChoose a line coding scheme:\n";
    for (size_t i = 0; i < menu.size(); ++i)
        std::cout << "  " << (i + 1) << ". " << menu[i].label << "\n";
    int choice = askInt("> ", 1, static_cast<int>(menu.size())) - 1;
    LineCoderPtr coder = menu[choice].factory();

    if (menu[choice].label == "AMI")
    {
        if (askYesNo("Apply scrambling on top of AMI?"))
        {
            std::cout << "  1. B8ZS (substitutes runs of 8 zeros)\n";
            std::cout << "  2. HDB3 (substitutes runs of 4 zeros)\n";
            int s = askInt("  > ", 1, 2);
            coder = (s == 1) ? LineCoderPtr(new B8ZS()) : LineCoderPtr(new HDB3());
        }
    }
    return coder;
}

static void runDigitalFlow(const std::string &outDir)
{
    DataGenerator gen;

    std::cout << "\nHow long should the digital data stream be? ";
    int length = askInt("(8-256): ", 8, 256);

    std::cout << "\nData generation mode:\n";
    std::cout << "  1. Completely random data\n";
    std::cout << "  2. Random data with forced runs of consecutive zeros (8 and 4)\n";
    int mode = askInt("> ", 1, 2);

    std::vector<int> data = (mode == 1) ? gen.generateRandom(length)
                                        : gen.generateWithZeroRuns(length, {8, 4});

    std::cout << "\nGenerated data stream (" << length << " bits):\n";
    printBits(data);

    auto [pStart, pLen] = Palindrome::longestPalindrome(data);
    std::cout << "Longest palindromic run: length " << pLen << ", starting at index " << pStart << " -> ";
    printBits(std::vector<int>(data.begin() + pStart, data.begin() + pStart + std::max(0, pLen)));

    LineCoderPtr coder = chooseLineCoder();
    SignalLevels signal = coder->encode(data);

    std::cout << "\nEncoded with " << coder->name() << ". First-half-of-bit voltage levels:\n";
    for (int i = 0; i < coder->bitCount(signal); ++i)
        std::cout << signal[2 * i] << " ";
    std::cout << "\n";

    std::string safeName = coder->name();
    for (auto &c : safeName)
        if (!isalnum(static_cast<unsigned char>(c)))
            c = '_';
    std::string svgPath = outDir + "/digital_" + safeName + ".svg";
    SignalPlotter::plotSignal(signal, svgPath, coder->name() + " encoded signal");
    std::cout << "Signal waveform saved to: " << svgPath << "\n";

    if (askYesNo("\n[Extra credit] Decode the signal back into bits?"))
    {
        std::vector<int> decoded = coder->decode(signal); // decode() only ever sees `signal`
        std::cout << "Decoded bits: ";
        printBits(decoded);
        bool match = (decoded == data);
        std::cout << (match ? "Decoded stream matches the original data. \n"
                            : "Mismatch between decoded and original data.\n");
    }
}

static std::vector<double> generateSyntheticAnalogSignal(int samples)
{
    std::vector<double> s(samples);
    for (int i = 0; i < samples; ++i)
    {
        double t = static_cast<double>(i) / samples;
        const double PI = 3.14159265358979323846;

        s[i] = 0.8 * std::sin(2 * PI * 2 * t) + 0.15 * std::sin(2 * PI * 9 * t);
    }
    return s;
}

static void runAnalogFlow(const std::string &outDir)
{
    std::cout << "\nAnalog encoding technique:\n  1. PCM\n  2. Delta Modulation\n";
    int tech = askInt("> ", 1, 2);

    int samples = askInt("\nHow many analog samples to generate? (8-64): ", 8, 64);
    std::vector<double> analog = generateSyntheticAnalogSignal(samples);

    std::cout << "Analog samples: ";
    for (double v : analog)
        std::cout << v << " ";
    std::cout << "\n";

    std::vector<int> bits;
    std::vector<double> staircase;

    if (tech == 1)
    {
        int qbits = askInt("\nQuantization bits per sample (2-8): ", 2, 8);
        PCM pcm(qbits, -1.0, 1.0);
        std::vector<int> levels;
        bits = pcm.encode(analog, &levels);

        std::cout << "Quantization levels: ";
        for (int l : levels)
            std::cout << l << " ";
        std::cout << "\nPCM bitstream: ";
        printBits(bits);

        LineCoderPtr coder = chooseLineCoder();
        SignalLevels signal = coder->encode(bits);
        std::string svgPath = outDir + "/pcm_line_signal.svg";
        SignalPlotter::plotSignal(signal, svgPath, "PCM bits -> " + coder->name());
        std::cout << "Line-coded PCM signal saved to: " << svgPath << "\n";

        std::vector<int> recoveredBits = coder->decode(signal);
        staircase = pcm.decode(recoveredBits);
    }
    else
    {
        double step = 0.9 / samples * 4.0; // a reasonable default step size
        std::cout << "\nUsing delta-modulation step size = " << step << "\n";
        DeltaModulation dm(step);
        bits = dm.encode(analog);
        std::cout << "DM bitstream: ";
        printBits(bits);

        LineCoderPtr coder = chooseLineCoder();
        SignalLevels signal = coder->encode(bits);
        std::string svgPath = outDir + "/dm_line_signal.svg";
        SignalPlotter::plotSignal(signal, svgPath, "DM bits -> " + coder->name());
        std::cout << "Line-coded DM signal saved to: " << svgPath << "\n";

        std::vector<int> recoveredBits = coder->decode(signal);
        staircase = dm.decode(recoveredBits);
    }

    std::string stairPath = outDir + "/decoder_staircase.svg";
    SignalPlotter::plotStaircase(analog, staircase, stairPath, "Original signal vs. decoder staircase");
    std::cout << "Decoder staircase reconstruction saved to: " << stairPath << "\n";
}

int main()
{
    std::cout << "=======================================================\n";
    std::cout << " ITT 036 - Programming Assignment 1: Digital Signal\n";
    std::cout << "           Encoder / Decoder\n";
    std::cout << "=======================================================\n";

    std::string outDir = "output";
    fs::create_directories(outDir);

    std::cout << "\nWhat kind of input do you want to provide?\n";
    std::cout << "  1. Digital input (line coding schemes)\n";
    std::cout << "  2. Analog input (PCM / Delta Modulation)\n";
    int choice = askInt("> ", 1, 2);

    if (choice == 1)
        runDigitalFlow(outDir);
    else
        runAnalogFlow(outDir);

    std::cout << "\nDone. All generated plots are in the '" << outDir << "' directory.\n";
    return 0;
}
