#pragma once

#include "3ds.h"
#include "CTRPluginFramework.hpp"

using namespace CTRPluginFramework;

#define STRINGIFY(x)        #x
#define TOSTRING(x)         STRINGIFY(x)

void AllItems(MenuEntry *entry);
void ExpGainEditor(MenuEntry *entry);
void FriendExpGainEditor(MenuEntry *entry);