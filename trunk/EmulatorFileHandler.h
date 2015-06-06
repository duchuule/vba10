#pragma once

#include <ppltasks.h>
#include "Emulator.h"
#include "CheatData.h"

using namespace concurrency;
using namespace Windows::Storage;

#define MAX_SAVESTATE_SLOTS 10
#define AUTOSAVESTATE_SLOT	9

namespace VBA10
{
	extern bool ROMLoaded;
	extern StorageFile ^ROMFile;
	extern StorageFolder ^ROMFolder;
	extern int SavestateSlot;

	task<void> ParseVBAIniAsync();
	bool IsROMLoaded(void);
	task<void> SaveStateAsync(void);
	task<void> SaveGBAStateAsync(void);
	task<void> SaveGBStateAsync(void);
	task<void> LoadStateAsync(void);
	task<void> LoadGBAStateAsync(void);
	task<void> LoadGBStateAsync(void);
	task<void> LoadGBROMAsync(StorageFile ^file, StorageFolder ^folder);
	task<void> LoadGBAROMAsync(StorageFile ^file, StorageFolder ^folder);
	task<void> LoadROMAsync(StorageFile ^file, StorageFolder ^folder);
	task<void> ResetAsync(void);
	task<ROMData> GetROMBytesFromFileAsync(StorageFile ^file);
	task<void> SaveSRAMAsync(void);
	task<void> SaveGBSRAMAsync(void);
	task<void> SaveGBASRAMAsync(void);
	task<void> SaveSRAMCopyAsync(void);
	task<void> SaveGBASRAMCopyAsync(void);
	task<void> SaveGBSRAMCopyAsync(void);
	task<void> LoadSRAMAsync(void);
	task<void> LoadGBASRAMAsync(void);
	task<void> LoadGBSRAMAsync(void);
	task<void> RestoreFromApplicationDataAsync(void);
	task<void> SuspendAsync(void);

	task<Windows::Foundation::Collections::IVector<CheatData ^> ^> LoadCheats(void);
	task<bool> SaveCheats(Windows::Foundation::Collections::IVector<CheatData ^> ^cheats);

	task<void> ApplyCheats(void);

	task<void> SaveBytesToFileAsync(StorageFile ^file, unsigned char *bytes, size_t length);
	task<ROMData> GetBytesFromFileAsync(StorageFile ^file);

	void SelectSavestateSlot(int slot);
	int GetSavestateSlot(void);
}