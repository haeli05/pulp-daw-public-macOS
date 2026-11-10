#pragma once
// Colours, fonts and small drawing helpers. The look is PULP's own.
#include <juce_gui_basics/juce_gui_basics.h>

namespace theme {
using juce::Colour;

const Colour bg{0xff24282c};          // workspace behind the windows
const Colour panel{0xff363b41};       // window and toolbar body
const Colour panelHi{0xff444a51};     // raised parts
const Colour panelLo{0xff2a2e33};     // recessed parts
const Colour inset{0xff1b1e21};       // displays, grids
const Colour line{0xff15181a};
const Colour text{0xffd9dee3};
const Colour textDim{0xff8e979f};
const Colour accent{0xffff9a2e};      // tangerine
const Colour accent2{0xff53c7b5};     // teal
const Colour good{0xff7ed957};
const Colour warn{0xffffd04a};
const Colour bad{0xffff5a4f};
const Colour stepOff{0xff565d65};
const Colour stepOffAlt{0xff6b5a57};
const Colour stepOn{0xffe8edf2};

// Each system's own interface fonts: Segoe UI and Consolas on Windows; on macOS Helvetica Neue and
// Menlo, which every Mac has and which sit at the same sizes.
#if JUCE_MAC
inline constexpr const char* kSansFace = "Helvetica Neue";
inline constexpr const char* kMonoFace = "Menlo";
#else
inline constexpr const char* kSansFace = "Segoe UI";
inline constexpr const char* kMonoFace = "Consolas";
#endif
inline juce::Font font(float size, bool bold = false) {
    return juce::Font(juce::FontOptions(kSansFace, size, bold ? juce::Font::bold : juce::Font::plain));
}
inline juce::Font mono(float size) {
    return juce::Font(juce::FontOptions(kMonoFace, size, juce::Font::plain));
}

// Key names as this computer's keyboard prints them. The interface text is written with Windows
// names; on a Mac the program's Ctrl shortcuts are Command shortcuts (juce's commandModifier) and Alt
// is the Option key, so the text says so. Whole words only ("Alternate" stays as it is).
inline juce::String keys(const juce::String& text) {
#if JUCE_MAC
    if (!text.contains("Ctrl") && !text.contains("Alt") && !text.contains("Exit")) return text;
    auto swap = [](const juce::String& in, const juce::String& from, const juce::String& to) {
        juce::String out;
        int i = 0;
        for (int at; (at = in.indexOf(i, from)) >= 0;) {
            const bool startOk = at == 0 || !juce::CharacterFunctions::isLetter(in[at - 1]);
            const bool endOk = !juce::CharacterFunctions::isLetter(in[at + from.length()]);
            out << in.substring(i, at) << (startOk && endOk ? to : from);
            i = at + from.length();
        }
        return out + in.substring(i);
    };
    return swap(swap(swap(text, "Ctrl", "Cmd"), "Alt", "Option"), "Exit", "Quit");
#else
    return text;
#endif
}

// The Delete key: a Mac keyboard's "delete" is the backspace key, its forward delete is fn+delete.
inline bool isDeleteKey(int keyCode) {
#if JUCE_MAC
    return keyCode == juce::KeyPress::deleteKey || keyCode == juce::KeyPress::backspaceKey;
#else
    return keyCode == juce::KeyPress::deleteKey;
#endif
}

// The plugin formats PULP hosts here: VST3 everywhere, and Audio Units on macOS.
#if JUCE_MAC
#define PULP_PLUGIN_KINDS "VST3 / AU"
#define PULP_PLUGIN_FOLDERS "/Library/Audio/Plug-Ins and ~/Library/Audio/Plug-Ins"
#else
#define PULP_PLUGIN_KINDS "VST3"
#define PULP_PLUGIN_FOLDERS "Common Files\\VST3"
#endif

// How the build script is run here, for messages that tell the user to run it.
#if JUCE_MAC
#define PULP_BUILD_SCRIPT_HINT(before, after) juce::String(before) + "./build.sh" + (after)
#else
#define PULP_BUILD_SCRIPT_HINT(before, after) juce::String(before) + "build.bat" + (after)
#endif

inline void bevel(juce::Graphics& g, juce::Rectangle<float> r, Colour fill, float radius = 3.0f) {
    g.setColour(fill);
    g.fillRoundedRectangle(r, radius);
    g.setColour(fill.brighter(0.12f));
    g.drawHorizontalLine((int)r.getY(), r.getX() + radius, r.getRight() - radius);
    g.setColour(line);
    g.drawRoundedRectangle(r.reduced(0.5f), radius, 1.0f);
}

inline void insetBox(juce::Graphics& g, juce::Rectangle<float> r, float radius = 2.0f) {
    g.setColour(inset);
    g.fillRoundedRectangle(r, radius);
    g.setColour(line);
    g.drawRoundedRectangle(r.reduced(0.5f), radius, 1.0f);
}

// Sixteen channel / clip colours.
inline Colour palette(int i) {
    static const juce::uint32 c[16] = {
        0xff6f8fa6, 0xffb9735a, 0xff7fa66f, 0xffa68f5f, 0xff8b78b0, 0xff5fa69c, 0xffb06f8f, 0xff9aa0a6,
        0xff4f7ea8, 0xffd08a3e, 0xff5b9a57, 0xffc7b24a, 0xff7460b8, 0xff3fa7a0, 0xffc0567a, 0xff78828c};
    return Colour(c[((i % 16) + 16) % 16]);
}

} // namespace theme
