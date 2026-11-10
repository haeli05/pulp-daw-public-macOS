#pragma once
// The macOS menu bar. Every command in the shortcut table (Shortcuts.h) appears in it under its
// group, with the key it has now, so the menus and Settings > SHORTCUTS always agree. Keys still
// reach the focused window first: the menu bar acts on a key only when no window used it.
// Settings... (Command-comma) sits in the application menu, as on every Mac program.
// On other systems there is no menu bar and this class is not built.
#include "MainComponent.h"

#if JUCE_MAC
class MacMenuBar : public juce::MenuBarModel, private juce::ApplicationCommandTarget {
public:
    explicit MacMenuBar(MainComponent& main);
    ~MacMenuBar() override;

    // MenuBarModel
    juce::StringArray getMenuBarNames() override;
    juce::PopupMenu getMenuForIndex(int index, const juce::String& name) override;
    void menuItemSelected(int, int) override {}

private:
    // ApplicationCommandTarget
    juce::ApplicationCommandTarget* getNextCommandTarget() override { return nullptr; }
    void getAllCommands(juce::Array<juce::CommandID>& commands) override;
    void getCommandInfo(juce::CommandID id, juce::ApplicationCommandInfo& info) override;
    bool perform(const InvocationInfo& info) override;

    void refresh();             // the shortcut table changed: new keys in the menus
    void addGroup(juce::PopupMenu& m, const juce::String& group);

    MainComponent& main_;
    juce::ApplicationCommandManager commands_;
    juce::PopupMenu appleExtras_;
};
#endif
