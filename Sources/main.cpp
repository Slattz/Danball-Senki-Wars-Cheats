#include "3ds.h"
#include "csvc.h"
#include "CTRPluginFramework.hpp"
#include "cheats.hpp"

#include <vector>

namespace CTRPluginFramework
{
    MenuEntry *EntryWithHotkey(MenuEntry *entry, const Hotkey &hotkey) {
        if (entry != nullptr) {
            entry->Hotkeys += hotkey;
            entry->SetArg(new std::string(entry->Name()));
            entry->Name() += " " + hotkey.ToString();
            entry->Hotkeys.OnHotkeyChangeCallback([](MenuEntry *entry, int index) {
                std::string *name = reinterpret_cast<std::string *>(entry->GetArg());

                entry->Name() = *name + " " + entry->Hotkeys[0].ToString();
            });
        }

        return (entry);
    }

    // This function is called before main and before the game starts
    // Useful to do code edits safely
    void    PatchProcess(FwkSettings &settings)
    {
    }

    // This function is called when the process exits
    // Useful to save settings, undo patchs or clean up things
    void    OnProcessExit(void)
    {

    }

    void    InitMenu(PluginMenu &menu)
    {
        menu += new MenuEntry("Shop Is Free", ShopIsFree, "Enabling this cheat makes everything in the shop free and also disregards LBX Crafting Level when buying");
        menu += EntryWithHotkey(new MenuEntry("All Items", AllItems, "Press the hotkeys to give yourself all items.\n\nNote: for LBX items, this only unlocks them in the shop.\nUse \'Shop Is Free\' to buy them."), Hotkey(Key::L | Key::Up, "All items hotkey"));
        menu += new MenuEntry("General EXP Multiplier (Current: x1)", nullptr, ExpGainEditor, "Change the General EXP rate using the keyboard icon!");
        menu += new MenuEntry("Friend EXP Multiplier (Current: x1)", nullptr, FriendExpGainEditor, "Change the Friend EXP rate using the keyboard icon!");
    }
 
    int     main(void)
    {
        PluginMenu *menu = new PluginMenu("Danball Senki Wars Plugin", 1, 0, 1, \
                                          "A CTRPF Plugin for Danball Senki Wars\nRequested by PHotman\nMade by Slattz");

        // Synnchronize the menu with frame event
        menu->SynchronizeWithFrame(true);

        // Init our menu entries & folders
        InitMenu(*menu);

        // Launch menu and mainloop
        menu->Run();

        delete menu;

        // Exit plugin
        return (0);
    }
}
