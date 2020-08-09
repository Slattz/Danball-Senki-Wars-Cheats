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
        "MUL    R0, R0, R3      \n\t" //R0 is the exp to gain
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

        entry->Name() = Utils::Format("General EXP Rate Modifier (Current: x%d)", g_expMultiplier);
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
        "MUL    R3, R3, R2      \n\t" //R0 is the exp to gain
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

        entry->Name() = Utils::Format("Friend EXP Rate Modifier (Current: x%d)", g_friendExpMultiplier);
    }
}

