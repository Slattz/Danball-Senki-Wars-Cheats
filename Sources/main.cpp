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
            menu += EntryWithHotkey(new MenuEntry("All Items", AllItems, "Press the hotkeys to give the current save all items"), Hotkey(Key::L | Key::Up, "All items hotkey") );
            menu += new MenuEntry("EXP Rate Modifier", nullptr, ExpGainEditor, "Change the EXP rate using the keyboard icon");
        // Create your entries here, or elsewhere
        // You can create your entries whenever/wherever you feel like it
    }
 
    int     main(void)
    {
        PluginMenu *menu = new PluginMenu("Danball Senki Wars Plugin", 1, 0, 0, \
                                          "A CTRPF Plugin for Danball Senki Wars\nCommissioned by PHotman\nMade by Slattz");

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
