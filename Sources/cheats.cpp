#include "cheats.hpp"

//#define DEBUG
#define SAVE_PTR 0x654010

//See savefile.bt for actual save structure
enum Offsets : u32 {
    Offsets_Items = 0x120
};

enum Sizes : u32 {
    Sizes_Item_Parts = 1952,
    Sizes_BigItems = 6211,
    Sizes_WeirdItems = 1588,
    Sizes_Items_unk = 323
};

u32 GetSaveOffset() {
    u32 offset = 0;
    if (Process::Read32(SAVE_PTR, offset))
        return offset;

    return 0;
}

void AllItems(MenuEntry *entry) {
    static constexpr u16 CompleteItem = 0x01FF;

    static bool btn = false;
    if (entry->Hotkeys[0].IsDown() && !btn) {
        btn = true;
        u32 offset = GetSaveOffset();
        if (offset == 0) {
            OSD::Notify("AllItems: save offset was 0!");
            return;
        }

        offset += Offsets_Items;
        u32 orig = offset;

        //fill Items_Parts
        for (u32 i = 0; i < Sizes_Item_Parts; i++) {
            Process::Write16(offset, CompleteItem);
            offset += 2; //sizeof(CompleteItem)
        }

        //fill Items_Parts
        for (u32 i = 0; i < Sizes_BigItems; i++) {
            u32 val = 0;
            Process::Read32(offset, val);
            val &= 0xFFFF0000; //get rid of amount and BoughtStatus
            val |= CompleteItem;
            Process::Write32(offset, val);
            offset += 4; //sizeof(val)
        }

        //fill Items_Parts
        for (u32 i = 0; i < Sizes_WeirdItems; i++) {
            u32 val = 0;
            Process::Read32(offset, val);
            val &= 0xFFFF0000; //get rid of amount and BoughtStatus
            val |= CompleteItem;
            Process::Write32(offset, val);
            offset += 4; //sizeof(val)
        }

        //fill Items_Parts
        for (u32 i = 0; i < Sizes_Items_unk; i++) {
            Process::Write16(offset, CompleteItem);
            offset += 2; //sizeof(CompleteItem)
        }

        #ifdef DEBUG
        MessageBox(Utils::Format("Orig: %X, Current: %X\nExpected Diff: 0x8BA2\nActual Diff: %X", orig, offset, offset-orig))();
        #endif
    }

    if (!entry->Hotkeys[0].IsDown())
        btn = false;
}

void ExpGainEditor(MenuEntry *entry) {
    return;

    Keyboard keyboard("Which exp rate would you like to use?");
    static std::vector<std::string> list = {
        "x1 (Normal)",
        "x2",
        "x3",
        "x5",
        "x10",
        "x20",
        "x25",
        "x50",
        "x75",
        "x100"
    };
    keyboard.Populate(list);
    int userChoice = keyboard.Open();

    if (userChoice != -1) {
        //DO HOOK SYSTEM HERE
    }
}