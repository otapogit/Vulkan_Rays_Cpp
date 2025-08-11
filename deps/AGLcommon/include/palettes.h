#pragma once

namespace autis {
    enum class Palette {
        GrayScale,      // neutral, shown in linear space (appears darker)
        GradientPalette, // https://neurophysics.ucsd.edu/Manuals/National%20Instruments/NI%20Vision%20Concepts%20Manual.pdf
        IncreasedBrightness, // gamma corrected (converted from linear to gamma)
        Rainbow         // rainbow palette
    };
}