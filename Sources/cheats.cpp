#include "cheats.hpp"

u32 GetSaveOffset() {
    u32 offset = 0;
    if (Process::Read32(SAVE_PTR, offset))
        return offset;

    return 0;
}

/*
DISABLED: THE GAME ONLY HAS 256 SLOTS FOR ITEMS, MEANING THERE'S NOT ENOUGH FOR AT LEAST 1 OF EVERY ITEM.
AS AN ALTERNATIVE, USER 'AllItems' UNLOCKS THE ITEMS. USER THEN USES 'Shop Is Free' TO BUY THE ITEMS THEY WANTED.

u32 g_SetupAddLBXPartsAddr = 0x2BDBD8;
void __attribute__((naked)) _SetupAddLBXParts(u32 unused, u32 Item, u32 unk_ptr) {
    __asm__ __volatile__(
        "STMFD  SP!, {R3-R12,LR}   	         \n\t" //Store All Registers except R0 (Unused), R1 (Item), R2 (unknown not required pointer)
        "LDR   	R3, =g_SetupAddLBXPartsAddr  \n\t" //Set game address
        "MOV    LR, PC					     \n\t" //Set return address (PC = (current instruction + 8) apparently)
        "LDR    PC, [R3]				     \n\t" //Set PC to the game function. It stores LR so it will return to here+4
        "LDMFD  SP!, {R3-R12,PC}   		     \n\t" //Restore All Registers
    );
}

void SetupAddLBXParts(u32 Item) {
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
*/

void AllItems(MenuEntry *entry) {
    static constexpr u16 CompleteItem = 0x01FF;

    static bool active = false;
    if (entry->WasJustActivated() && !active) {
        active = true;
        u32 offset = GetSaveOffset();
        if (offset == 0) {
            OSD::Notify("AllItems: save offset was 0!");
            entry->Disable();
            active = false;
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
        //Setup_LBXItem_Inv(); //Setup inventory for LBX items

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
        entry->Disable();
        active = false;
    }
}

bool CheckExpInput(const void *input, std::string &error) {
        int in = *static_cast<const int *>(input);
        if (in >= MIN_EXP && in <= MAX_EXP)
            return true;

        error = "The value must be between " TOSTRING(MIN_EXP) " and " TOSTRING(MAX_EXP);
        return false;
}

//GENERAL EXP
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
    constexpr const u32 off_Hook_GeneralEXP = 0x318950;

    static Hook* hook = nullptr;
    if (hook == nullptr) {
        hook = new Hook;
        hook->Initialize(off_Hook_GeneralEXP, (u32)&ExpMultiplier);
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
    constexpr const u32 off_Hook_FriendshipEXP = 0x318740;

    static Hook* hook = nullptr;
    if (hook == nullptr) {
        hook = new Hook;
        hook->Initialize(off_Hook_FriendshipEXP, (u32)&FriendExpMultiplier);
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

void ShopIsFree(MenuEntry *entry) {
    constexpr const u32 off_LBX_PriceZero = 0x2B8BAC;
    constexpr const u32 off_Modifiers_PriceZero = 0x2BB04C;
    constexpr const u32 off_IgnoreCraftLevel = 0x2B8C0C;

    static bool active = false;
    if (entry->WasJustActivated() && !active) {
        //Changes code to set all LBX Item prices to 0 and bypasses the money required check
        Process::Patch(off_LBX_PriceZero,   0xE3A00000); //MOV R0, #0
        Process::Patch(off_LBX_PriceZero+4, 0xE5840008); //STR R0, [R4,#8]

        //Changes code to set all Item modifier prices to 0 and bypasses the money required check
        Process::Patch(off_Modifiers_PriceZero,   0xE3A00000); //MOV R0, #0
        Process::Patch(off_Modifiers_PriceZero+4, 0xE5840004); //STR R0, [R4,#4]
        Process::Patch(off_Modifiers_PriceZero+8, 0xEA000002); //B #0x10

        //Changes code to not care about 'lbx crafting level'
        Process::Patch(off_IgnoreCraftLevel, 0xE3C00040); //BIC R0, R0, #0x40

        OSD::Notify("Shop Is Free: " << Color::Green << "Enabled!");
        active = true;
    }

    else if (!entry->IsActivated() && active) {
        //unpatch 'LBX Item price is 0'
        Process::Patch(off_LBX_PriceZero,   0xE1500001); //CMP R0, R1
        Process::Patch(off_LBX_PriceZero+4, 0xAA000002); //BGE #0x10

        //unpatch 'Item modifier price is 0'
        Process::Patch(off_Modifiers_PriceZero,   0xE0811002);  //ADD R1, R1, R2
        Process::Patch(off_Modifiers_PriceZero+4, 0xE1500001);  //CMP R0, R1
        Process::Patch(off_Modifiers_PriceZero+8, 0xAA000002); //BGE #0x10

        //unpatch 'lbx crafting level doesnt matter'
        Process::Patch(off_IgnoreCraftLevel, 0xE3800040); //ORR R0, R0, #0x40

        OSD::Notify("Shop Is Free: " << Color::Red << "Disabled!");
        active = false;
    }
}