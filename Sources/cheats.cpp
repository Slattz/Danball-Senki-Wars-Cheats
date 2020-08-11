#include "cheats.hpp"

//#define DEBUG
#define SAVE_PTR 0x654010

//See savefile.bt for actual save structure
enum Offsets : u32 {
    Offsets_Items = 0x118,
    Offsets_LBXInv = 0xAE94,
    Offsets_LBXItemArray = 0x12E8C
};

enum Sizes : u32 {
    Sizes_Item_Parts = 1956,
    Sizes_BigItems = 5229,
    Sizes_Unk1Items = 982,
    Sizes_Unk2Items = 1589,
    Sizes_Items_unk = 321,

    Sizes_LBXItemArray = 0x1200,
};

struct PACKED LBXItem_s {
    u16 Index;
    u16 ItemID;
    u16 unk_num;
    u16 unk2_FFFF;
    u16 Modifier1;
    u16 Modifier2;
    u16 Modifier3;
    u16 Modifier4;
    u16 unk5_FFFF;
};
static_assert(sizeof(LBXItem_s) == 0x12, "LBXItem_s is incorrect size");

u32 GetSaveOffset() {
    u32 offset = 0;
    if (Process::Read32(SAVE_PTR, offset))
        return offset;

    return 0;
}

u32 g_SetupAddLBXPartsAddr = 0x2BDBD8;
u8 __attribute__((naked)) _SetupAddLBXParts(u32 unused, u32 Item, u32 unk_ptr) {
    __asm__ __volatile__(
        "STMFD  SP!, {R3-R12,LR}   	         \n\t" //Store All Registers except R0 (Unused), R1 (Item), R2 (unknown not required pointer)
        "LDR   	R3, =g_SetupAddLBXPartsAddr  \n\t" //Set game address
        "MOV    LR, PC					     \n\t" //Set return address (PC = (current instruction + 8) apparently)
        "LDR    PC, [R3]				     \n\t" //Set PC to the game function. It stores LR so it will return to here+4
        "LDMFD  SP!, {R3-R12,PC}   		     \n\t" //Restore All Registers
    );
}

u8 SetupAddLBXParts(u32 Item) {
    _SetupAddLBXParts(0, Item, 0);
}

void Setup_LBXItem_Inv() {
    u32 bOffset = GetSaveOffset();
    if (bOffset == 0) {
        OSD::Notify(std::string(__PRETTY_FUNCTION__) + ": save offset was 0!");
        return;
    }

    u32 orig[1] = {0};
    Process::Patch(g_SetupAddLBXPartsAddr + 0x30, 0xEA000033, orig); //Fixes a game bug (lol) where it sets a memory address to 0 and continues the functions instead of returning from function

    u32 off_itemTable = bOffset + 0x1062;
    for (u32 i = 0; i < Sizes_BigItems; i++) {
        u16 itemID = 0xFFFF;
        Process::Read16(off_itemTable, itemID);
        off_itemTable += 4; //sizeof(id)

        SetupAddLBXParts(itemID);
    }
    Process::Patch(g_SetupAddLBXPartsAddr + 0x30, orig[0]); //Unpatch to keep original game code (unfix bug lol)
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

        //fill Items_LBXItems
        for (u32 i = 0; i < Sizes_BigItems; i++) {
            u32 val = 0;
            Process::Read32(offset, val);
            val &= 0xFFFF0000; //get rid of amount and BoughtStatus
            val |= CompleteItem;
            Process::Write32(offset, val);
            offset += 4; //sizeof(val)
        }
        Setup_LBXItem_Inv(); //Setup inventory for LBX items

        //fill Items_Unk1Items
        for (u32 i = 0; i < Sizes_Unk1Items; i++) {
            u32 val = 0;
            Process::Read32(offset, val);
            val &= 0xFFFF0000; //get rid of amount and BoughtStatus
            val |= CompleteItem;
            Process::Write32(offset, val);
            offset += 4; //sizeof(val)
        }

        //fill Unk2_Items
        for (u32 i = 0; i < Sizes_Unk2Items; i++) {
            u32 val = 0;
            Process::Read32(offset, val);
            val &= 0xFFFF0000; //get rid of amount and BoughtStatus
            val |= CompleteItem;
            Process::Write32(offset, val);
            offset += 4; //sizeof(val)
        }

        //fill Items_unk
        for (u32 i = 0; i < Sizes_Items_unk; i++) {
            Process::Write16(offset, CompleteItem);
            offset += 2; //sizeof(CompleteItem)
        }

        OSD::Notify("All Items received!");
    }

    if (!entry->Hotkeys[0].IsDown())
        btn = false;
}

#define MIN_EXP 1
#define MAX_EXP 100

bool CheckExpInput(const void *input, std::string &error) {
        int in = *static_cast<const int *>(input);
        if (in >= MIN_EXP && in <= MAX_EXP)
            return true;

        error = "The value must be between " TOSTRING(MIN_EXP) " and " TOSTRING(MAX_EXP);
        return false;
}

//GENERAL EXP
#define EXP_HOOK_ADDR 0x318950

u8 g_expMultiplier = 1;

void __attribute__((naked)) ExpMultiplier(void) {
    __asm__ __volatile__(
        "STMFD  SP!, {R3}       \n\t"
        "LDR    R3, =g_expMultiplier     \n\t"
        "LDRB   R3, [R3]        \n\t"
        "MUL    R0, R0, R3      \n\t" //R0 is the general exp to gain
        "LDMFD  SP!, {R3}       \n\t"
        "BX LR                  \n\t"
    );
}

void ExpGainEditor(MenuEntry *entry) {
    static Hook* hook = nullptr;
    if (hook == nullptr) {
        hook = new Hook;
        hook->Initialize(EXP_HOOK_ADDR, (u32)&ExpMultiplier);
    }

    Keyboard keyboard(Utils::Format("Enter an exp multiplier between\n" TOSTRING(MIN_EXP) " (Normal) and " TOSTRING(MAX_EXP) ".\n\n\nCurrent General EXP Modifier: x%d", g_expMultiplier));
    keyboard.IsHexadecimal(false);
    keyboard.SetCompareCallback(CheckExpInput);

    int ret = keyboard.Open(g_expMultiplier, g_expMultiplier);
    if (ret != -1) {
        if (g_expMultiplier < MIN_EXP || g_expMultiplier > MAX_EXP)
            g_expMultiplier = MIN_EXP;

        if (g_expMultiplier == MIN_EXP) {
            hook->Disable();
        }

        else {
            hook->Enable();
        }

        entry->Name() = Utils::Format("General EXP Multiplier (Current: x%d)", g_expMultiplier);
    }
}

// FRIENDSHIP EXP
#define FRIEND_EXP_HOOK_ADDR 0X318740

u8 g_friendExpMultiplier = 1;
void __attribute__((naked)) FriendExpMultiplier(void) {
    __asm__ __volatile__(
        "STMFD  SP!, {R2}       \n\t"
        "LDR    R2, =g_friendExpMultiplier     \n\t"
        "LDRB   R2, [R2]        \n\t"
        "MUL    R3, R3, R2      \n\t" //R3 is the friend exp to gain
        "LDMFD  SP!, {R2}       \n\t"
        "BX LR                  \n\t"
    );
}

void FriendExpGainEditor(MenuEntry *entry) {
    static Hook* hook = nullptr;
    if (hook == nullptr) {
        hook = new Hook;
        hook->Initialize(FRIEND_EXP_HOOK_ADDR, (u32)&FriendExpMultiplier);
    }

    Keyboard keyboard(Utils::Format("Enter an exp multiplier between\n" TOSTRING(MIN_EXP) " (Normal) and " TOSTRING(MAX_EXP) ".\n\n\nCurrent Friend EXP Modifier: x%d", g_friendExpMultiplier));
    keyboard.IsHexadecimal(false);
    keyboard.SetCompareCallback(CheckExpInput);

    int ret = keyboard.Open(g_friendExpMultiplier, g_friendExpMultiplier);
    if (ret != -1) {
        if (g_friendExpMultiplier < MIN_EXP || g_friendExpMultiplier > MAX_EXP)
            g_friendExpMultiplier = MIN_EXP;

        if (g_friendExpMultiplier == MIN_EXP) {
            hook->Disable();
        }

        else {
            hook->Enable();
        }

        entry->Name() = Utils::Format("Friend EXP Multiplier (Current: x%d)", g_friendExpMultiplier);
    }
}