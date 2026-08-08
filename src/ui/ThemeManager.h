#pragma once
#include "../../include/FLTypes.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>

namespace FL {

// ---------------------------------------------------------------------------
// ColorScheme
// A named set of UI colors covering every major interface element.
// ---------------------------------------------------------------------------
struct ColorScheme {
    std::string name;

    // Window / panel background tones
    Color background         = Color::fromPacked(0xFF1A1A1A);
    Color backgroundAlt      = Color::fromPacked(0xFF222222);
    Color panelBorder        = Color::fromPacked(0xFF333333);

    // Primary accent (buttons, active controls)
    Color accent             = Color::fromPacked(0xFF00B4D8);
    Color accentHover        = Color::fromPacked(0xFF48CAE4);
    Color accentPressed      = Color::fromPacked(0xFF0077B6);

    // Text
    Color textPrimary        = Color::fromPacked(0xFFE0E0E0);
    Color textSecondary      = Color::fromPacked(0xFF909090);
    Color textDisabled       = Color::fromPacked(0xFF505050);

    // Piano roll / sequencer grid
    Color gridLine           = Color::fromPacked(0xFF2E2E2E);
    Color gridLineBeat       = Color::fromPacked(0xFF3A3A3A);
    Color noteActive         = Color::fromPacked(0xFF00B4D8);
    Color noteSelected       = Color::fromPacked(0xFFFF9F1C);
    Color noteMuted          = Color::fromPacked(0xFF505050);

    // Mixer
    Color faderTrack         = Color::fromPacked(0xFF2A2A2A);
    Color faderThumb         = Color::fromPacked(0xFF00B4D8);
    Color vuGreen            = Color::fromPacked(0xFF44CF6C);
    Color vuYellow           = Color::fromPacked(0xFFFFD166);
    Color vuRed              = Color::fromPacked(0xFFEF476F);
    Color channelMuted       = Color::fromPacked(0xFF666666);
    Color channelSoloed      = Color::fromPacked(0xFFFFD166);
};

// Built-in scheme factories
ColorScheme darkScheme();
ColorScheme lightScheme();
ColorScheme neonScheme();

// ---------------------------------------------------------------------------
// ThemeManager
// Loads, stores and activates named color schemes.
// Themes can be serialized to / from simple key=value text files.
// ---------------------------------------------------------------------------
class ThemeManager {
public:
    ThemeManager();
    ~ThemeManager() = default;

    // Register a color scheme.  Replaces any existing scheme with the same name.
    void registerScheme(ColorScheme scheme);

    // Activate a scheme by name.  Returns false if the name is not found.
    bool activateScheme(const std::string& name);

    // Currently active scheme (may be empty/default if none activated).
    const ColorScheme& activeScheme() const noexcept { return m_active; }
    const std::string& activeSchemeName() const noexcept { return m_active.name; }

    // List all registered scheme names.
    std::vector<std::string> schemeNames() const;

    // Retrieve a scheme by name (nullptr if not found).
    const ColorScheme* getScheme(const std::string& name) const;

    // Serialize active scheme to a string (key=hex_value lines).
    std::string serializeActive() const;

    // Load a scheme from a key=hex_value string.  Registers and activates it.
    // Returns false on parse errors.
    bool loadFromString(const std::string& name, const std::string& data);

    // Save / load from file path (.fst).
    bool saveToFile(const std::string& filePath) const;
    bool loadFromFile(const std::string& filePath);

private:
    std::unordered_map<std::string, ColorScheme> m_schemes;
    ColorScheme                                   m_active;
};

} // namespace FL
