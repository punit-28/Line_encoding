#pragma once
#include <string>
#include <vector>
#include "LineCoder.h"

// Renders waveforms to standalone SVG files. SVG was chosen over OpenGL
// because it needs no windowing system / GPU context to produce a real,
// inspectable graphical output file in this environment; it plays the
// same role the assignment suggests for OpenGL/JOGL/matplotlib.
namespace SignalPlotter {

    // Draws a bipolar/unipolar step waveform (as used by all line codes).
    void plotSignal(const SignalLevels& signal, const std::string& filePath, const std::string& title);

    // Draws an analog curve overlaid with a quantized staircase
    // reconstruction (used for PCM/DM decoder output).
    void plotStaircase(const std::vector<double>& original,
                        const std::vector<double>& staircase,
                        const std::string& filePath,
                        const std::string& title);

} // namespace SignalPlotter
