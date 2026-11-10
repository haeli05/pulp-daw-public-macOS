#include "MacMenu.h"

#if JUCE_MAC
#include "Shortcuts.h"

namespace {
// Command numbers: the few the menu bar adds itself, then one per row of the shortcut table.
enum : juce::CommandID { kNew = 1, kSettings, kTableBase = 1000 };

// The shortcut table's group names as macOS names its menus.
juce::String menuNameFor(const juce::String& group) { return group == "Windows" ? juce::String("Window") : group; }
// Commands that sit in the application menu or Help rather than with their group.
bool keptOutOfGroups(const juce::String& id) { return id == "settings" || id == "help"; }
int tableIndex(const juce::String& id) {
    const auto& cmds = Shortcuts::get().commands();
    for (int i = 0; i < (int)cmds.size(); ++i) if (cmds[(size_t)i].id == id) return i;
    return -1;
}
}

MacMenuBar::MacMenuBar(MainComponent& main) : main_(main) {
    commands_.setFirstCommandTarget(this);
    refresh();
    Shortcuts::get().onChanged = [this] { refresh(); };
    juce::MenuBarModel::setMacMainMenu(this, &appleExtras_);
}

MacMenuBar::~MacMenuBar() {
    Shortcuts::get().onChanged = nullptr;
    juce::MenuBarModel::setMacMainMenu(nullptr);
}

void MacMenuBar::refresh() {
    commands_.clearCommands();
    commands_.registerAllCommandsForTarget(this);        // each command's key is the one it has now
    appleExtras_ = juce::PopupMenu();
    appleExtras_.addCommandItem(&commands_, kSettings);
    // setMacMainMenu keeps a copy of the application menu's extras: hand it the new one
    if (juce::MenuBarModel::getMacMainMenu() == this) juce::MenuBarModel::setMacMainMenu(this, &appleExtras_);
    menuItemsChanged();
}

void MacMenuBar::getAllCommands(juce::Array<juce::CommandID>& ids) {
    ids.addArray({kNew, kSettings});
    for (int i = 0; i < (int)Shortcuts::get().commands().size(); ++i) ids.add(kTableBase + i);
}

void MacMenuBar::getCommandInfo(juce::CommandID id, juce::ApplicationCommandInfo& info) {
    if (id == kNew) { info.setInfo("New", "Start a new project", "File", 0); return; }
    if (id == kSettings) {
        info.setInfo("Settings...", "Audio, MIDI, plugins and shortcuts", "PULP", 0);
        info.addDefaultKeypress(',', juce::ModifierKeys::commandModifier);
        return;
    }
    const auto& cmds = Shortcuts::get().commands();
    const int i = id - kTableBase;
    if (i < 0 || i >= (int)cmds.size()) return;
    const auto& c = cmds[(size_t)i];
    info.setInfo(theme::keys(c.name), c.name, c.group, 0);
    // A key with Command or Control goes in the menu as its key equivalent. A plain key (Space, L,
    // F5) is shown too: macOS only lets the menu act on keys with a modifier, so the window that has
    // the focus still gets those first, as it does today.
    if (c.key.isValid()) info.addDefaultKeypress(c.key.getKeyCode(), c.key.getModifiers());
}

bool MacMenuBar::perform(const InvocationInfo& info) {
    if (info.commandID == kNew) { main_.fileNew(); return true; }       // asks about unsaved work itself
    if (info.commandID == kSettings) {
        if (auto* w = main_.window("settings")) w->showAndRaise();
        return true;
    }
    auto& cmds = Shortcuts::get().commands();
    const int i = info.commandID - kTableBase;
    if (i < 0 || i >= (int)cmds.size()) return false;
    if (cmds[(size_t)i].run) cmds[(size_t)i].run();
    return true;
}

juce::StringArray MacMenuBar::getMenuBarNames() {
    juce::StringArray names{"File", "Edit"};
    for (auto& c : Shortcuts::get().commands()) names.addIfNotAlreadyThere(menuNameFor(c.group));
    names.addIfNotAlreadyThere("Help");
    return names;
}

void MacMenuBar::addGroup(juce::PopupMenu& m, const juce::String& menuName) {
    const auto& cmds = Shortcuts::get().commands();
    for (int i = 0; i < (int)cmds.size(); ++i)
        if (menuNameFor(cmds[(size_t)i].group) == menuName && !keptOutOfGroups(cmds[(size_t)i].id))
            m.addCommandItem(&commands_, kTableBase + i);
}

juce::PopupMenu MacMenuBar::getMenuForIndex(int, const juce::String& name) {
    juce::PopupMenu m;
    if (name == "File") {
        m.addCommandItem(&commands_, kNew);
        m.addSeparator();
    }
    if (name == "Help") {
        if (const int i = tableIndex("help"); i >= 0) m.addCommandItem(&commands_, kTableBase + i);
        return m;
    }
    addGroup(m, name);
    return m;
}
#endif
