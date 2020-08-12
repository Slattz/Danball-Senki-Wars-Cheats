#pragma once

#include "3ds.h"
#include "CTRPluginFramework.hpp"

using namespace CTRPluginFramework;

#define STRINGIFY(x)        #x
#define TOSTRING(x)         STRINGIFY(x)

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

#define MIN_EXP 1
#define MAX_EXP 100

void AllItems(MenuEntry *entry);
void ShopIsFree(MenuEntry *entry);
void ExpGainEditor(MenuEntry *entry);
void FriendExpGainEditor(MenuEntry *entry);