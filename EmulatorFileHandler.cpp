#include "EmulatorFileHandler.h"
#include "EmulatorSettings.h"
#include "Emulator.h"
#include <string>
#include <sstream>
#include <System.h>
#include <NLS.h>
#include <Port.h>
#include <GBA.h>
#include <GB.h>
#include <Globals.h>
#include <RTC.h>
#include <robuffer.h>
#include "EmulatorSettings.h"
#include <fstream>
#include "Util.h"
#include <Gb_Apu.h>
#include <Sound.h>
#include <gbSound.h>
#include <gbMemory.h>
#include <gbCheats.h>
#include <Cheats.h>
#include "stringhelper.h"

using namespace Platform;
using namespace std;

extern bool cheatsEnabled;
extern int gbaSaveType;
extern int romSize;
extern int emulating;

// Extern functions used for read and save state
extern bool soundInit();
extern void reset_apu();
extern void write_SGCNT0_H( int data );
extern void apply_muting();
extern void CPUUpdateWindow0();
extern void CPUUpdateWindow1();
extern void sramWrite(u32, u8);
extern void CPUReadHelper(void);

extern int gbBattery;
extern int gbRomType;
extern u8 *gbRam;
extern int gbRamSizeMask;
extern u8 *gbMemoryMap[16];
extern mapperMBC3 gbDataMBC3;
extern mapperTAMA5 gbDataTAMA5;
extern u8 *gbMemory;
extern u8 *gbTAMA5ram;
extern int gbTAMA5ramSize;
extern int gbRamSize;

namespace VBA10
{
	public value struct ROMConfig
	{
		int saveType;
		int flashSize;
		int rtcEnabled;
		int mirroringEnabled;
	};

	Map<String ^, ROMConfig> ^romConfigs = nullptr;

	int ROMSize = 0;
	bool ROMLoaded = false;
	StorageFile ^ROMFile = nullptr;
	StorageFolder ^ROMFolder = nullptr;
	int SavestateSlot = 0;
	bool iniParsed = false;
	bool gbaROMLoaded = true;

	Windows::Foundation::Collections::IVector<CheatData ^> ^ROMCheats = nullptr;
	bool ShouldApplyNewCheats = false;

	int firstIndexOf(string &s, char c) 
	{
		for(int i = 0; i < s.size(); ++i) 
		{
			if(s[i] == c) return i;
		}
		return -1;
	}

	vector<string> &split(const string &s, char delim, vector<string> &elems) 
	{
		stringstream ss(s);
		string item;
		while(getline(ss, item, delim)) 
		{
			elems.push_back(item);
		}
		return elems;
	}

	vector<string> split(const string &s, char delim) 
	{
		vector<string> elems;
		return split(s, delim, elems);
	}

	bool stringWhitespace(const string &s)
	{
		for (int i = 0; i < s.size(); i++)
		{
			if(!isspace(s.at(i)))
			{
				return false;
			}
		}
		return true;
	}

	Platform::Array<unsigned char> ^GetSnapshotBuffer(unsigned char *backbuffer, size_t pitch, int imageWidth, int imageHeight)
	{
		Platform::Array<unsigned char> ^buffer = ref new Platform::Array<unsigned char>(imageWidth * imageHeight * 4);

		/*Microsoft::WRL::ComPtr<IBufferByteAccess> byteAccess;
		reinterpret_cast<IUnknown*>(buffer)->QueryInterface(IID_PPV_ARGS(&byteAccess));
		byte *buf;
		byteAccess->Buffer(&buf);
		uint16 *targetBuffer = (uint16 *) buf;*/
		int dstPitch = imageWidth * 4;
		for (int i = 0; i < imageHeight; i++)
		{
			for (int j = 0; j < imageWidth * 4; j += 4)
			{
				// red
				buffer[dstPitch * i + j] = *(backbuffer + pitch * i + j + 2);

				// green
				buffer[dstPitch * i + j + 1] = *(backbuffer + pitch * i + j + 1);

				// blue
				buffer[dstPitch * i + j + 2] = *(backbuffer + pitch * i + j + 0);

				// alpha
				buffer[dstPitch * i + j + 3] = 0xff;


				//*(targetBuffer + imageWidth * i + j) = *(backbuffer + (pitch / 2) * i + j) ;
			}
		}

		return buffer;
	}

	task<void> ParseVBAIniAsync()
	{
		//if(iniParsed)
		//{
		//	return create_task([](){}); //this will make make the operation continue on another thread!!!!
		//}

		auto reader = make_shared<DataReader ^>();

		StorageFolder ^installDir = Windows::ApplicationModel::Package::Current->InstalledLocation;
		return create_task(installDir->GetFolderAsync("Assets/")).then([](task<StorageFolder ^> t)
		{
			StorageFolder ^assetsFolder = t.get();
			return assetsFolder->GetFileAsync("vba-over.ini");
		}).then([=](task<StorageFile ^> fileTask)
		{
			StorageFile ^file = fileTask.get();
			return file->OpenReadAsync();
		}).then([reader](IRandomAccessStream ^stream)
		{
			*reader = ref new DataReader(stream);
			return create_task((*reader)->LoadAsync((unsigned int) stream->Size));
		}).then([reader](unsigned int bytesRead)
		{
			String ^text = nullptr;
			Map<Platform::String ^, ROMConfig> ^map = ref new Map<Platform::String ^, ROMConfig>();

			text = (*reader)->ReadString(bytesRead);

			if(text == nullptr)
				return;

			string str(text->Begin(), text->End());
			vector<string> lines = split(str, '\n');

			for(vector<string>::const_iterator i = lines.begin(); i != lines.end(); ++i)
			{
				string line = *i;
				int startBraces = firstIndexOf(line, '[');
				if(startBraces == -1)
				{
					continue;
				}
				int endBraces = firstIndexOf(line, ']');
				if(endBraces == -1)
				{
					continue;
				}
				ROMConfig config;
				config.flashSize = -1;
				config.mirroringEnabled = -1;
				config.rtcEnabled = -1;
				config.saveType = -1;

				string romCode = line.substr(startBraces + 1, endBraces - startBraces - 1);

				for (++i; i != lines.end() && !stringWhitespace(line = *i); ++i)
				{
					int equalsIndex = firstIndexOf(line, '=');
					if(equalsIndex == -1)
					{
						continue;
					}
					if(equalsIndex + 1 >= line.size())
					{
						continue;
					}
					string configName = line.substr(0, equalsIndex);
					string configValue = line.substr(equalsIndex + 1);

					stringstream ss;
					ss << configValue;
					int value;
					ss >> value;

					const char *configNameStr = configName.c_str();
					if(strcmp(configNameStr, "rtcEnabled") == 0)
					{
						config.rtcEnabled = value;
					}else if(strcmp(configNameStr, "flashSize") == 0)
					{
						config.flashSize = value;
					}else if(strcmp(configNameStr, "saveType") == 0)
					{
						config.saveType = value;
					}else if(strcmp(configNameStr, "mirroringEnabled") == 0)
					{
						config.mirroringEnabled = value;
					}
				}
				wstring wRomCode(romCode.begin(), romCode.end());
				map->Insert(ref new String(wRomCode.c_str()), config);

				if(i == lines.end())
				{
					break;
				}
			}

			romConfigs = map;

		}).then([](task<void> t)
		{
			try
			{
				iniParsed = true;
				t.get();
			}catch(Platform::Exception ^e)
			{
				romConfigs = nullptr;
#if _DEBUG
				String ^message = e->Message;
				wstring wstr(message->Begin(), message->End());
				OutputDebugStringW((L"ParseVBAIniAsync: " + wstr).c_str());
#endif
			}
		});
	}

	bool IsROMLoaded(void)
	{
		return (ROMFile && ROMFolder && ROMLoaded);
	}

	bool IsGBAROMLoaded(void)
	{
		return (ROMFile && ROMFolder && ROMLoaded && gbaROMLoaded);
	}

	task<void> LoadROMAsync(StorageFile ^file, StorageFolder ^folder)
	{
		bool gba = false;
		String^ filename = file->Name;
		if(filename->Length() >= 3)
		{
			const wchar_t *end = filename->End();
			const wchar_t *extStart = end - 3;
			if(extStart >= filename->Begin())
			{
				gba = ((extStart[0] == 'g' || extStart[0] == 'G') && 
					(extStart[1] == 'b' || extStart[1] == 'B') &&
					(extStart[2] == 'a' || extStart[2] == 'A'));
			}
		}
		if(gba)
		{
			return LoadGBAROMAsync(file, folder);
		}else
		{
			return LoadGBROMAsync(file, folder);
		}
	}

	task<void> LoadGBROMAsync(StorageFile ^file, StorageFolder ^folder)
	{

		EmulatorGame *emulator = EmulatorGame::GetInstance();
		
		gbaLoopForceQuit = true;



		gbexecute = false; //added by Duc Le

		return create_task([emulator]()
		{
			return emulator->StopROMAsync();
		}).then([file]()
		{
			return GetBytesFromFileAsync(file);
		}).then([file, folder](ROMData data)
		{
			int size = data.Length;

			if(rom != NULL) {
				CPUCleanUp();
			}

			systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;

			extern u8 *gbRom;
			extern int gbRomSize;
			extern bool gbBatteryError;
			extern int gbHardware;
			extern int gbBorderOn;
			extern int gbCgbMode;

			for(int i = 0; i < 24;) 
			{
				systemGbPalette[i++] = (0x1f) | (0x1f << 5) | (0x1f << 10);
				systemGbPalette[i++] = (0x15) | (0x15 << 5) | (0x15 << 10);
				systemGbPalette[i++] = (0x0c) | (0x0c << 5) | (0x0c << 10);
				systemGbPalette[i++] = 0;
			}

			gbRom = (u8 *) malloc(size);
			if(gbRom == NULL) {
				systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"),
					"ROM");
				CPUCleanUp();
			}  

			memcpy_s(gbRom, size, data.ROM, size);

			if(data.ROM)
			{
				delete [] data.ROM;
			}

			gbRomSize = size;
			gbBatteryError = false;

			if(bios != NULL) {
				free(bios);
				bios = NULL;
			}
			bios = (u8 *)calloc(1,0x100);

			gbUpdateSizes();
			gbGetHardwareType();

			gbReset();			
			EmulatorGame::emulator = GBSystem;

			gbBorderOn = false;

			soundInit();
			gbSoundReset();

			ROMSize = size;
			ROMFile = file;
			ROMFolder = folder;
			gbaROMLoaded = false;

			return;
		}).then([]()
		{
			return LoadSRAMAsync();
		}).then([emulator]()
		{
			ROMLoaded = true;
			emulator->Unpause();
			emulator->Start();	
			emulating = true;
		}).then([]()
		{
			return LoadCheats();
		}).then([] (task<Windows::Foundation::Collections::IVector<CheatData ^> ^> tcheats)
		{
			try
			{
				ROMCheats = tcheats.get();
				ApplyCheats(ROMCheats);

			}
			catch (COMException ^ex)
			{
#if _DEBUG
				Platform::String ^str = ex->Message;
				wstring wstr(str->Begin(), str->End());
				OutputDebugStringW(wstr.c_str());
				//EngineLog(LOG_LEVEL::Error, wstr);
#endif
			}
		});
	}

	task<void> LoadGBAROMAsync(StorageFile ^file, StorageFolder ^folder)//(unsigned char *rom, size_t length)
	{		
		EmulatorGame *emulator = EmulatorGame::GetInstance();
		gbaLoopForceQuit = false;

		return ParseVBAIniAsync().then([emulator]()
		{
			return emulator->StopROMAsync();
		}).then([file]()
		{
			return GetBytesFromFileAsync(file);
		})
			.then([file, folder](ROMData data)
		{

			int size = 0x2000000;

			if(rom != NULL) {
				CPUCleanUp();
			}

			systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;

			rom = (u8 *)malloc(0x2000000);
			if(rom == NULL) {
				systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"),
					"ROM");
			}
			workRAM = (u8 *)calloc(1, 0x40000);
			if(workRAM == NULL) {
				systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"),
					"WRAM");
			}

			u8 *whereToLoad = rom;
			if(cpuIsMultiBoot)
				whereToLoad = workRAM;

			int read = size = data.Length < size ? data.Length : size;
			memcpy_s(whereToLoad, read, data.ROM, read);

			u16 *temp = (u16 *)(rom+((size+1)&~1));
			int i;
			for(i = (size+1)&~1; i < 0x2000000; i+=2) {
				WRITE16LE(temp, (i >> 1) & 0xFFFF);
				temp++;
			}

			bios = (u8 *)calloc(1,0x4000);
			if(bios == NULL) {
				systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"),
					"BIOS");
				CPUCleanUp();
			}    
			internalRAM = (u8 *)calloc(1,0x8000);
			if(internalRAM == NULL) {
				systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"),
					"IRAM");
				CPUCleanUp();
			}    
			paletteRAM = (u8 *)calloc(1,0x400);
			if(paletteRAM == NULL) {
				systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"),
					"PRAM");
				CPUCleanUp();
			}      
			vram = (u8 *)calloc(1, 0x20000);
			if(vram == NULL) {
				systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"),
					"VRAM");
				CPUCleanUp();
			}      
			oam = (u8 *)calloc(1, 0x400);
			if(oam == NULL) {
				systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"),
					"OAM");
				CPUCleanUp();
			}      

			/*pix = (u8 *)calloc(1, 4 * 241 * 162);
			if(pix == NULL) {
			systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"),
			"PIX");
			CPUCleanUp();
			}      
			extern size_t gbaPitch;
			gbaPitch = 964;*/

			ioMem = (u8 *)calloc(1, 0x400);
			if(ioMem == NULL) {
				systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"),
					"IO");
				CPUCleanUp();
			}      

			memset(flashSaveMemory, 0xff, sizeof(flashSaveMemory));
			memset(eepromData, 255, sizeof(eepromData));

			extern void CPUUpdateRenderBuffers(bool);
			CPUUpdateRenderBuffers(true);

			ROMSize = read;
			romSize = read;

			if(data.ROM)
			{
				delete [] data.ROM;
			}

			// read from vba-over.ini
			Map<String ^, ROMConfig> ^configs = romConfigs;

			if(configs != nullptr)
			{
				char buffer[5];
				strncpy_s(buffer, (const char *) &rom[0xac], 4);
				buffer[4] = 0;

				string codeA = string(buffer);
				String ^code = ref new String(wstring(codeA.begin(), codeA.end()).c_str());
#if _DEBUG
				stringstream ss;
				ss << "Game code: ";
				ss << codeA << "\n";
				OutputDebugStringA(ss.str().c_str());
#endif
				if(configs->HasKey(code))
				{
					ROMConfig config = configs->Lookup(code);
					if(config.flashSize != -1)
					{
						flashSetSize(config.flashSize);
					}
					if(config.mirroringEnabled != -1)
					{
						doMirroring(config.mirroringEnabled != 0);
					}
					if(config.rtcEnabled != -1)
					{
						rtcEnable(config.rtcEnabled != 0);
					}
					if(config.saveType != -1)
					{
						cpuSaveType = config.saveType;
					}
				}
			}

			skipBios = true;

			EmulatorGame::emulator = GBASystem;

			soundInit();

			CPUInit(nullptr, false);
			CPUReset();


			ROMFile = file;
			ROMFolder = folder;
			gbaROMLoaded = true;

			return;
		}).then([]()
		{
			return LoadSRAMAsync();
		}).then([emulator]()
		{
			ROMLoaded = true;
			emulator->Unpause();
			emulator->Start();	
			emulating = true;
		}).then([]()
		{
			return LoadCheats();
		}).then([](task<Windows::Foundation::Collections::IVector<CheatData ^>^ > tcheats)
		{
			try
			{ 
				ROMCheats = tcheats.get();
				ApplyCheats(ROMCheats);

			}
			catch(COMException ^ex)
			{
#if _DEBUG
				Platform::String ^str = ex->Message;
				wstring wstr(str->Begin(), str->End());
				OutputDebugStringW(wstr.c_str());
				//EngineLog(LOG_LEVEL::Error, wstr);
#endif
			}
		});
	}

	task<void> SaveSRAMAsync ()
	{
		if(gbaROMLoaded)
		{
			return SaveGBASRAMAsync();
		}else
		{
			return SaveGBSRAMAsync();
		}
	}

	task<void> SaveGBSRAMAsync()
	{
		if(!ROMFile || !ROMFolder)
			return task<void>([](){});

		Platform::String ^name = ROMFile->Name;
		const wchar_t *end = name->End();
		while(*end != '.') end--;
		size_t diff = name->End() - end;

		Platform::String ^nameWithoutExt = ref new Platform::String(name->Begin(), name->Length() - diff);
		Platform::String ^sramName = nameWithoutExt->Concat(nameWithoutExt, ".sav");

		return create_task([sramName]()
		{
			// try to open the file			
			return ROMFolder->CreateFileAsync(sramName, CreationCollisionOption::OpenIfExists);
		}).then([](StorageFile ^file)
		{
			if(gbBattery)
			{
				switch(gbRomType)
				{
				case 0xff:
				case 0x03:
					// MBC1
					if(gbRam)
					{
						SaveBytesToFileAsync(file, gbRam, gbRamSizeMask + 1).wait();
					}
					break;
				case 0x06:
					// MBC2
					if(gbRam)
					{
						SaveBytesToFileAsync(file, gbMemoryMap[0x0a], 512).wait();
					}
					break;
				case 0x0d:
					// MMM01
					if(gbRam)
					{
						SaveBytesToFileAsync(file, gbRam, gbRamSizeMask + 1).wait();
					}
					break;
				case 0x0f:
				case 0x10:
					// MBC3
					if(gbRam)
					{
						int tmpSize = gbRamSizeMask + 1 + 10 * sizeof(int) + sizeof(time_t);
						u8 *tmp = new u8[tmpSize];
						memcpy_s(tmp, gbRamSizeMask + 1, gbRam, gbRamSizeMask + 1);
						memcpy_s(tmp + gbRamSizeMask + 1, 10 * sizeof(int) + sizeof(time_t), &gbDataMBC3.mapperSeconds, 10 * sizeof(int) + sizeof(time_t));

						SaveBytesToFileAsync(file, tmp, tmpSize).wait();

						delete [] tmp;
					}else
					{
						SaveBytesToFileAsync(file, (u8 *) &gbDataMBC3.mapperSeconds, 10 * sizeof(int) + sizeof(time_t)).wait();
					}
					break;
				case 0x13:
				case 0xfc:
					// MBC3 - 2
					if(gbRam)
					{
						SaveBytesToFileAsync(file, gbRam, gbRamSizeMask + 1).wait();
					}
					break;
				case 0x1b:
				case 0x1e:
					// MBC5
					if(gbRam)
					{
						SaveBytesToFileAsync(file, gbRam, gbRamSizeMask + 1).wait();
					}
					break;
				case 0x22:
					// MBC7
					if(gbRam)
					{
						SaveBytesToFileAsync(file, &gbMemory[0xa000], 256).wait();
					}
					break;
				case 0xfd:
					if(gbRam)
					{
						int tmpSize = gbRamSizeMask + 1 + gbTAMA5ramSize + 14 * sizeof(int) + sizeof(time_t);
						u8 *tmp = new u8[tmpSize];
						memcpy_s(tmp, gbRamSizeMask + 1, gbRam, gbRamSizeMask + 1);
						memcpy_s(tmp + gbRamSizeMask + 1, gbTAMA5ramSize, gbTAMA5ram, gbTAMA5ramSize);
						memcpy_s(tmp + gbRamSizeMask + 1 + gbTAMA5ramSize, 14 * sizeof(int) + sizeof(time_t), &gbDataTAMA5.mapperSeconds, 14 * sizeof(int) + sizeof(time_t));

						SaveBytesToFileAsync(file, tmp, tmpSize).wait();

						delete [] tmp;
					}else
					{
						int tmpSize = gbTAMA5ramSize + 14 * sizeof(int) + sizeof(time_t);
						u8 *tmp = new u8[tmpSize];
						memcpy_s(tmp, gbTAMA5ramSize, gbTAMA5ram, gbTAMA5ramSize);
						memcpy_s(tmp + gbTAMA5ramSize, 14 * sizeof(int) + sizeof(time_t), &gbDataTAMA5.mapperSeconds, 14 * sizeof(int) + sizeof(time_t));

						SaveBytesToFileAsync(file, tmp, tmpSize).wait();

						delete [] tmp;
					}
					break;
				}
			}
		}).then([sramName](task<void> t)
		{
			try
			{
				t.get();
			}catch(Platform::COMException ^ex)
			{
#if _DEBUG
				Platform::String ^error = ex->Message;
				wstring wname(sramName->Begin(), sramName->End());
				wstring werror(error->Begin(), error->End());
				//EngineLog(LOG_LEVEL::Error, wname + L": " + werror);
				OutputDebugStringW((wname + L": " + werror).c_str());
#endif
			}
		});
	}

	task<void> SaveGBASRAMAsync()
	{
		if(!ROMFile || !ROMFolder)
			return task<void>([](){});

		Platform::String ^name = ROMFile->Name;
		Platform::String ^nameWithoutExt = ref new Platform::String(name->Begin(), name->Length() - 4);
		Platform::String ^sramName = nameWithoutExt->Concat(nameWithoutExt, ".sav");

		return create_task([sramName]()
		{
			// try to open the file			
			return ROMFolder->CreateFileAsync(sramName, CreationCollisionOption::OpenIfExists);
		}).then([](StorageFile ^file)
		{
			if(gbaSaveType == 0) {
				if(eepromInUse)
					gbaSaveType = 3;
				else switch(saveType) {
				case 1:
					gbaSaveType = 1;
					break;
				case 2:
					gbaSaveType = 2;
					break;
				}
			}

			if((gbaSaveType) && (gbaSaveType!=5)) 
			{
				// only save if Flash/Sram in use or EEprom in use
				if(gbaSaveType != 3) 
				{
					if(gbaSaveType == 2) 
					{
						SaveBytesToFileAsync(file, flashSaveMemory, flashSize).wait();
					} 
					else 
					{
						SaveBytesToFileAsync(file, flashSaveMemory, 0x10000).wait();
					}
				} 
				else 
				{
					SaveBytesToFileAsync(file, eepromData, eepromSize).wait();
				}
			}

		}).then([sramName](task<void> t)
		{
			try
			{
				t.get();
			}catch(Platform::COMException ^ex)
			{
#if _DEBUG
				Platform::String ^error = ex->Message;
				wstring wname(sramName->Begin(), sramName->End());
				wstring werror(error->Begin(), error->End());
				//EngineLog(LOG_LEVEL::Error, wname + L": " + werror);
				OutputDebugStringW((wname + L": " + werror).c_str());
#endif
			}
		});
	}

	task<void> SaveSRAMCopyAsync(void)
	{
		if(gbaROMLoaded)
		{
			return SaveGBASRAMCopyAsync();
		}else
		{
			return SaveGBSRAMCopyAsync();
		}
	}

	task<void> SaveGBSRAMCopyAsync(void)
	{
		if(!ROMFile || !ROMFolder)
			return task<void>([](){});


		Platform::String ^name = ROMFile->Name;
		const wchar_t *end = name->End();
		while(*end != '.') end--;
		size_t diff = name->End() - end;

		Platform::String ^nameWithoutExt = ref new Platform::String(name->Begin(), name->Length() - diff);
		Platform::String ^sramName = nameWithoutExt->Concat(nameWithoutExt, ".sav");

		EmulatorGame *emulator = EmulatorGame::GetInstance();

		emulator->Pause();

		u8 *gbRamCopy = NULL;
		int gbRamSizeCopy = gbRamSize;
		int gbRamSizeMaskCopy = gbRamSizeMask;
		u8 *gbMemoryMapACopy = NULL;
		mapperMBC3 gbDataMBC3Copy = gbDataMBC3;
		u8 *gbMemoryCopy = NULL;
		u8 *gbTAMA5ramCopy = NULL;
		mapperTAMA5 gbDataTAMA5Copy = gbDataTAMA5;
		int gbTAMA5ramSizeCopy = gbTAMA5ramSize;
		int gbBatteryCopy = gbBattery;
		int gbRomTypeCopy = gbRomType;


		if(gbRam)
		{
			gbRamCopy = new u8[gbRamSize];
			memcpy_s(gbRamCopy, gbRamSize, gbRam, gbRamSize);
		}
		if(gbRomType == 0x06)
		{
			gbMemoryMapACopy = new u8[512];
			memcpy_s(gbMemoryMapACopy, 512, gbMemoryMap[0x0a], 512);
		}
		if(gbRomType == 0x22)
		{
			gbMemoryCopy = new u8[256];
			memcpy_s(gbMemoryCopy, 256, &gbMemory[0xa000], 256);
		}
		if(gbRomType == 0xfd)
		{
			gbTAMA5ramCopy = new u8[gbTAMA5ramSize];
			memcpy_s(gbTAMA5ramCopy, gbTAMA5ramSize, gbTAMA5ram, gbTAMA5ramSize);
		}

		emulator->Unpause();

		auto gbRamPtr = make_shared<u8 *>(gbRamCopy);
		auto gbMemoryMapACopyPtr = make_shared<u8 *>(gbMemoryMapACopy);
		auto gbMemoryCopyPtr = make_shared<u8 *>(gbMemoryCopy);
		auto gbTAMA5ramCopyPtr = make_shared<u8 *>(gbTAMA5ramCopy);

		return create_task([sramName]()
		{
			// try to open the file			
			return ROMFolder->CreateFileAsync(sramName, CreationCollisionOption::OpenIfExists);
		}).then([=](StorageFile ^file)
		{
			if(gbBatteryCopy)
			{
				switch(gbRomTypeCopy)
				{
				case 0xff:
				case 0x03:
					// MBC1
					if(*gbRamPtr)
					{
						SaveBytesToFileAsync(file, *gbRamPtr, gbRamSizeMaskCopy + 1).wait();
					}
					break;
				case 0x06:
					// MBC2
					if(*gbRamPtr)
					{
						SaveBytesToFileAsync(file, *gbMemoryMapACopyPtr, 512).wait();
					}
					break;
				case 0x0d:
					// MMM01
					if(*gbRamPtr)
					{
						SaveBytesToFileAsync(file, *gbRamPtr, gbRamSizeMaskCopy + 1).wait();
					}
					break;
				case 0x0f:
				case 0x10:
					// MBC3
					if(gbRamCopy)
					{
						int tmpSize = gbRamSizeMaskCopy + 1 + 10 * sizeof(int) + sizeof(time_t);
						u8 *tmp = new u8[tmpSize];
						memcpy_s(tmp, gbRamSizeMaskCopy + 1, *gbRamPtr, gbRamSizeMaskCopy + 1);
						memcpy_s(tmp + gbRamSizeMaskCopy + 1, 10 * sizeof(int) + sizeof(time_t), &gbDataMBC3Copy.mapperSeconds, 10 * sizeof(int) + sizeof(time_t));

						SaveBytesToFileAsync(file, tmp, tmpSize).wait();

						delete [] tmp;
					}else
					{
						SaveBytesToFileAsync(file, (u8 *) &gbDataMBC3Copy.mapperSeconds, 10 * sizeof(int) + sizeof(time_t)).wait();
					}
					break;
				case 0x13:
				case 0xfc:
					// MBC3 - 2
					if(*gbRamPtr)
					{
						SaveBytesToFileAsync(file, *gbRamPtr, gbRamSizeMaskCopy + 1).wait();
					}
					break;
				case 0x1b:
				case 0x1e:
					// MBC5
					if(*gbRamPtr)
					{
						SaveBytesToFileAsync(file, *gbRamPtr, gbRamSizeMaskCopy + 1).wait();
					}
					break;
				case 0x22:
					// MBC7
					if(*gbRamPtr)
					{
						SaveBytesToFileAsync(file, *gbMemoryCopyPtr, 256).wait();
					}
					break;
				case 0xfd:
					if(*gbRamPtr)
					{
						int tmpSize = gbRamSizeMaskCopy + 1 + gbTAMA5ramSizeCopy + 14 * sizeof(int) + sizeof(time_t);
						u8 *tmp = new u8[tmpSize];
						memcpy_s(tmp, gbRamSizeMaskCopy + 1, *gbRamPtr, gbRamSizeMaskCopy + 1);
						memcpy_s(tmp + gbRamSizeMaskCopy + 1, gbTAMA5ramSizeCopy, *gbTAMA5ramCopyPtr, gbTAMA5ramSizeCopy);
						memcpy_s(tmp + gbRamSizeMaskCopy + 1 + gbTAMA5ramSizeCopy, 14 * sizeof(int) + sizeof(time_t), &gbDataTAMA5Copy.mapperSeconds, 14 * sizeof(int) + sizeof(time_t));

						SaveBytesToFileAsync(file, tmp, tmpSize).wait();

						delete [] tmp;
					}else
					{
						int tmpSize = gbTAMA5ramSizeCopy + 14 * sizeof(int) + sizeof(time_t);
						u8 *tmp = new u8[tmpSize];
						memcpy_s(tmp, gbTAMA5ramSizeCopy, *gbTAMA5ramCopyPtr, gbTAMA5ramSizeCopy);
						memcpy_s(tmp + gbTAMA5ramSizeCopy, 14 * sizeof(int) + sizeof(time_t), &gbDataTAMA5Copy.mapperSeconds, 14 * sizeof(int) + sizeof(time_t));

						SaveBytesToFileAsync(file, tmp, tmpSize).wait();

						delete [] tmp;
					}
					break;
				}
			}
		}).then([=](task<void> t)
		{
			try
			{
				if(*gbRamPtr)
				{
					delete [] *gbRamPtr;
					*gbRamPtr = nullptr;
				}
				if(*gbMemoryMapACopyPtr)
				{
					delete [] *gbMemoryMapACopyPtr;
					*gbMemoryMapACopyPtr = nullptr;
				}
				if(*gbMemoryCopyPtr)
				{
					delete [] *gbMemoryCopyPtr;
					*gbMemoryCopyPtr = nullptr;
				}
				if(*gbTAMA5ramCopyPtr)
				{
					delete [] *gbTAMA5ramCopyPtr;
					*gbTAMA5ramCopyPtr = nullptr;
				}

				t.get();
			}catch(Platform::COMException ^ex)
			{
#if _DEBUG
				Platform::String ^error = ex->Message;
				wstring wname(sramName->Begin(), sramName->End());
				wstring werror(error->Begin(), error->End());
				//EngineLog(LOG_LEVEL::Error, wname + L": " + werror);
				OutputDebugStringW((wname + L": " + werror).c_str());
#endif
			}
		});
	}

	task<void> SaveGBASRAMCopyAsync(void)
	{
		if(!ROMFile || !ROMFolder)
			return task<void>([](){});

		Platform::String ^name = ROMFile->Name;
		Platform::String ^nameWithoutExt = ref new Platform::String(name->Begin(), name->Length() - 4);
		Platform::String ^sramName = nameWithoutExt->Concat(nameWithoutExt, ".sav");

		EmulatorGame *emulator = EmulatorGame::GetInstance();

		emulator->Pause();

		u8 *flashSaveCopy = new u8[131072];
		u8 *eepromCopy = new u8[8192];

		memcpy_s(flashSaveCopy, 131072, flashSaveMemory, 131072);
		memcpy_s(eepromCopy, 8192, eepromData, 8192);

		emulator->Unpause();

		auto flashCopyPtr = make_shared<u8 *>(flashSaveCopy);
		auto eepromCopyPtr = make_shared<u8 *>(eepromCopy);


		return create_task([sramName]()
		{
			// try to open the file			
			return ROMFolder->CreateFileAsync(sramName, CreationCollisionOption::OpenIfExists);
		}).then([flashCopyPtr, eepromCopyPtr](StorageFile ^file)
		{
			if(gbaSaveType == 0) {
				if(eepromInUse)
					gbaSaveType = 3;
				else switch(saveType) {
				case 1:
					gbaSaveType = 1;
					break;
				case 2:
					gbaSaveType = 2;
					break;
				}
			}

			if((gbaSaveType) && (gbaSaveType!=5)) 
			{
				// only save if Flash/Sram in use or EEprom in use
				if(gbaSaveType != 3) 
				{
					if(gbaSaveType == 2) 
					{
						SaveBytesToFileAsync(file, *flashCopyPtr, flashSize).wait();
					} 
					else 
					{
						SaveBytesToFileAsync(file, *flashCopyPtr, 0x10000).wait();
					}
				} 
				else 
				{
					SaveBytesToFileAsync(file, *eepromCopyPtr, eepromSize).wait();
				}
			}
			delete [] *flashCopyPtr;
			*flashCopyPtr = nullptr;

			delete [] *eepromCopyPtr;
			*eepromCopyPtr = nullptr;

		}).then([sramName, flashCopyPtr, eepromCopyPtr](task<void> t)
		{
			try
			{
				if(*flashCopyPtr)
				{
					delete [] *flashCopyPtr;
					*flashCopyPtr = nullptr;
				}
				if(*eepromCopyPtr)
				{
					delete [] *eepromCopyPtr;
					*eepromCopyPtr = nullptr;
				}

				t.get();
			}catch(Platform::COMException ^ex)
			{
#if _DEBUG
				Platform::String ^error = ex->Message;
				wstring wname(sramName->Begin(), sramName->End());
				wstring werror(error->Begin(), error->End());
				//EngineLog(LOG_LEVEL::Error, wname + L": " + werror);
				OutputDebugStringW((wname + L": " + werror).c_str());
#endif
			}
		});
	}

	task<void> LoadSRAMAsync ()
	{
		if(gbaROMLoaded)
		{
			return LoadGBASRAMAsync();
		}else
		{
			return LoadGBSRAMAsync();
		}
	}


	task<void> LoadGBSRAMAsync ()
	{
		if(!ROMFile || !ROMFolder)
			return task<void>([](){});

		Platform::String ^name = ROMFile->Name;
		const wchar_t *end = name->End();
		while(*end != '.') end--;
		size_t diff = name->End() - end;

		Platform::String ^nameWithoutExt = ref new Platform::String(name->Begin(), name->Length() - diff);
		Platform::String ^sramName = nameWithoutExt->Concat(nameWithoutExt, ".sav");

		return create_task([sramName]()
		{
			return ROMFolder->GetFileAsync(sramName);
		}).then([](StorageFile ^file)
		{
			return GetBytesFromFileAsync(file);
		}).then([](ROMData data)
		{
			if(gbBattery)
			{
				switch(gbRomType)
				{
				case 0x03:
					// MBC1
					if(gbRam)
					{
						memcpy_s(gbRam, gbRamSizeMask + 1, data.ROM, gbRamSizeMask + 1);
					}
					break;
				case 0x06:
					// MBC2
					if(gbRam)
					{
						memcpy_s(gbMemoryMap[0x0a], 512, data.ROM, 512);
					}
					break;
				case 0x0d:
					// MMM01
					if(gbRam)
					{
						memcpy_s(gbRam, gbRamSizeMask + 1, data.ROM, gbRamSizeMask + 1);
					}
					break;
				case 0x0f:
				case 0x10:
					// MBC3
					try{
						if(gbRam)
						{
							memcpy_s(gbRam, gbRamSizeMask + 1, data.ROM, gbRamSizeMask + 1);
							memcpy_s(&gbDataMBC3.mapperSeconds, sizeof(int) * 10 + sizeof(time_t), data.ROM + gbRamSizeMask + 1, sizeof(int) * 10 + sizeof(time_t));
						}else
						{
							memcpy_s(&gbDataMBC3.mapperSeconds, sizeof(int) * 10 + sizeof(time_t), data.ROM, sizeof(int) * 10 + sizeof(time_t));
						}
					}catch(...)
					{
						time(&gbDataMBC3.mapperLastTime);
						struct tm *lt;
						lt = localtime(&gbDataMBC3.mapperLastTime);
						gbDataMBC3.mapperSeconds = lt->tm_sec;
						gbDataMBC3.mapperMinutes = lt->tm_min;
						gbDataMBC3.mapperHours = lt->tm_hour;
						gbDataMBC3.mapperDays = lt->tm_yday & 255;
						gbDataMBC3.mapperControl = (gbDataMBC3.mapperControl & 0xfe) |
							(lt->tm_yday > 255 ? 1: 0);
					}
					break;
				case 0x13:
				case 0xfc:
					// MBC3 - 2
					if(gbRam)
					{
						memcpy_s(gbRam, gbRamSizeMask + 1, data.ROM, gbRamSizeMask + 1);
						memcpy_s(&gbDataMBC3.mapperSeconds, sizeof(int) * 10 + sizeof(time_t), data.ROM + gbRamSizeMask + 1, sizeof(int) * 10 + sizeof(time_t));
					}else
					{
						memcpy_s(&gbDataMBC3.mapperSeconds, sizeof(int) * 10 + sizeof(time_t), data.ROM, sizeof(int) * 10 + sizeof(time_t));
					}
					break;
				case 0x1b:
				case 0x1e:
					// MBC5
					if(gbRam)
					{
						memcpy_s(gbRam, gbRamSizeMask + 1, data.ROM, gbRamSizeMask + 1);
					}
					break;
				case 0x22:
					// MBC7
					if(gbRam)
					{
						memcpy_s(&gbMemory[0xa000], 256, data.ROM, 256);
					}
					break;
				case 0xfd:
					try
					{
						if(gbRam)
						{
							memcpy_s(gbRam, gbRamSizeMask + 1, data.ROM, gbRamSizeMask + 1);
							memcpy_s(gbTAMA5ram, gbTAMA5ramSize, data.ROM + gbRamSizeMask + 1, gbTAMA5ramSize);
							memcpy_s(&gbDataTAMA5.mapperSeconds, sizeof(int)*14 + sizeof(time_t), data.ROM + gbRamSizeMask + 1 + gbTAMA5ramSize, sizeof(int)*14 + sizeof(time_t));
						}else
						{
							memcpy_s(gbTAMA5ram, gbTAMA5ramSize, data.ROM, gbTAMA5ramSize);
							memcpy_s(&gbDataTAMA5.mapperSeconds, sizeof(int)*14 + sizeof(time_t), data.ROM + gbTAMA5ramSize, sizeof(int)*14 + sizeof(time_t));
						}
					}catch(...)
					{
						u8 gbDaysinMonth [12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
						time(&gbDataTAMA5.mapperLastTime);
						struct tm *lt;
						lt = localtime(&gbDataTAMA5.mapperLastTime);
						gbDataTAMA5.mapperSeconds = lt->tm_sec;
						gbDataTAMA5.mapperMinutes = lt->tm_min;
						gbDataTAMA5.mapperHours = lt->tm_hour;
						gbDataTAMA5.mapperDays = 1;
						gbDataTAMA5.mapperMonths = 1;
						gbDataTAMA5.mapperYears = 1970;
						int days = lt->tm_yday+365*3;
						while (days)
						{
							gbDataTAMA5.mapperDays++;
							days--;
							if (gbDataTAMA5.mapperDays>gbDaysinMonth[gbDataTAMA5.mapperMonths-1])
							{
								gbDataTAMA5.mapperDays = 1;
								gbDataTAMA5.mapperMonths++;
								if (gbDataTAMA5.mapperMonths>12)
								{
									gbDataTAMA5.mapperMonths = 1;
									gbDataTAMA5.mapperYears++;
									if ((gbDataTAMA5.mapperYears & 3) == 0)
										gbDaysinMonth[1] = 29;
									else
										gbDaysinMonth[1] = 28;
								}
							}
						}
						gbDataTAMA5.mapperControl = (gbDataTAMA5.mapperControl & 0xfe) |
							(lt->tm_yday > 255 ? 1: 0);
					}
					break;
				}
			}

			delete [] data.ROM;

		}).then([sramName](task<void> t)
		{
			try
			{
				t.get();
			}
			catch(Platform::COMException ^ex)
			{
#if _DEBUG
				Platform::String ^error = ex->Message;
				wstring wname(sramName->Begin(), sramName->End());
				wstring werror(error->Begin(), error->End());
				//EngineLog(LOG_LEVEL::Info, wname + L": " + werror);
				OutputDebugStringW((wname + L": " + werror).c_str());
#endif
			}catch(Platform::AccessDeniedException ^ex)
			{
#if _DEBUG
				Platform::String ^error = ex->Message;
				wstring wname(sramName->Begin(), sramName->End());
				wstring werror(error->Begin(), error->End());
				//EngineLog(LOG_LEVEL::Info, wname + L": " + werror);
				OutputDebugStringW((wname + L": " + werror).c_str());
#endif
			}catch(Platform::Exception ^ex)
			{
#if _DEBUG
				Platform::String ^error = ex->Message;
				wstring wname(sramName->Begin(), sramName->End());
				wstring werror(error->Begin(), error->End());
				//EngineLog(LOG_LEVEL::Info, wname + L": " + werror);
				OutputDebugStringW((wname + L": " + werror).c_str());
#endif
			}
		});	
	}

	task<void> LoadGBASRAMAsync ()
	{
		if(!ROMFile || !ROMFolder)
			return task<void>([](){});

		Platform::String ^name = ROMFile->Name;
		Platform::String ^nameWithoutExt = ref new Platform::String(name->Begin(), name->Length() - 4);
		Platform::String ^sramName = nameWithoutExt->Concat(nameWithoutExt, ".sav");

		return create_task([sramName]()
		{
			return ROMFolder->GetFileAsync(sramName);
		}).then([](StorageFile ^file)
		{
			return GetBytesFromFileAsync(file);
		}).then([](ROMData data)
		{
			systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;

			if(data.Length == 512 || data.Length == 0x2000) 
			{
				memcpy_s(eepromData, data.Length, data.ROM, data.Length);
			} 
			else 
			{
				if(data.Length == 0x20000) 
				{
					memcpy_s(flashSaveMemory, 0x20000, data.ROM, 0x20000);
					flashSetSize(0x20000);
				} 
				else if(data.Length >= 0x10000)
				{
					memcpy_s(flashSaveMemory, 0x10000, data.ROM, 0x10000);
					flashSetSize(0x10000);
				}
			}

			delete [] data.ROM;

		}).then([sramName](task<void> t)
		{
			try
			{
				t.get();
			}
			catch(Platform::COMException ^ex)
			{
#if _DEBUG
				Platform::String ^error = ex->Message;
				wstring wname(sramName->Begin(), sramName->End());
				wstring werror(error->Begin(), error->End());
				//EngineLog(LOG_LEVEL::Info, wname + L": " + werror);
				OutputDebugStringW((wname + L": " + werror).c_str());
#endif
			}catch(Platform::AccessDeniedException ^ex)
			{
#if _DEBUG
				Platform::String ^error = ex->Message;
				wstring wname(sramName->Begin(), sramName->End());
				wstring werror(error->Begin(), error->End());
				//EngineLog(LOG_LEVEL::Info, wname + L": " + werror);
				OutputDebugStringW((wname + L": " + werror).c_str());
#endif
			}catch(Platform::Exception ^ex)
			{
#if _DEBUG
				Platform::String ^error = ex->Message;
				wstring wname(sramName->Begin(), sramName->End());
				wstring werror(error->Begin(), error->End());
				//EngineLog(LOG_LEVEL::Info, wname + L": " + werror);
				OutputDebugStringW((wname + L": " + werror).c_str());
#endif
			}
		});	
	}

	void ResetSync()
	{
		if (!ROMFile || !ROMFolder)
			return;

		EmulatorGame *emulator = EmulatorGame::GetInstance();
		emulator->Pause();
		gbexecute = false; //TODO: need to uncomment when we update the VBA-M engine

		EmulatorGame::emulator.emuReset();
		emulator->Unpause();
	}
	//task<void> ResetAsync(void)
	//{
	//	if(!ROMFile || !ROMFolder)
	//		return task<void>([](){});
	//	return LoadROMAsync(ROMFile, ROMFolder);
	//}

	task<void> SaveStateAsync(void)
	{
		if(gbaROMLoaded)
		{
			return SaveGBAStateAsync();
		}else
		{
			return SaveGBStateAsync();
		}
	}

	task<void> SaveGBStateAsync(void)
	{
		EmulatorGame *emulator = EmulatorGame::GetInstance();
		return create_task([emulator]()
		{
			if(!ROMFile || !ROMFolder)
			{
				throw ref new Exception(E_FAIL, L"No ROM loaded.");
			}
			emulator->Pause();

			// Generate random file name to store in temp folder			
			Platform::String ^folderpath = Windows::Storage::ApplicationData::Current->TemporaryFolder->Path;
			string folderPathStr(folderpath->Begin(), folderpath->End());

			stringstream tmpFileNameStream;
			tmpFileNameStream << folderPathStr << "\\";
			tmpFileNameStream << rand() << rand() << ".sgm";
			string fileNameA = tmpFileNameStream.str();

			FILE *file;
			auto error = fopen_s(&file, fileNameA.c_str(), "wb");
			if(!file)
			{
				stringstream ss;
				ss << "Unable to open tmp file '";
				ss << fileNameA;
				ss << "' to store savestate (";
				ss << error;
				ss << ").";
				//throw GameException(ss.str().c_str());
				string str(ss.str());
				throw ref new Exception(E_FAIL, ref new Platform::String(wstring(str.begin(), str.end()).c_str()));
			}
			fclose(file);

			ofstream stream (fileNameA.c_str(), ios::binary);
			if(!stream.is_open())
			{
#if _DEBUG
				stringstream ss;
				ss << "Unable to open tmp file '";
				ss << fileNameA;
				ss << "' to store savestate.";
				string str(ss.str());
				throw ref new Exception(E_FAIL, ref new Platform::String(wstring(str.begin(), str.end()).c_str()));
#endif
			}

			extern u8* gbRom;
			extern bool useBios;
			extern bool inBios;
			extern variable_desc gbSaveGameStruct[78];
			extern u16 IFF;
			extern int gbSgbMode;
			extern variable_desc gbSgbSaveStructV3[11];
			extern u8 *gbSgbBorder;
			extern u8 *gbSgbBorderChar;
			extern u8 gbSgbPacket[112];
			extern u16 gbSgbSCPPalette[2048];
			extern u8 gbSgbATF[360];
			extern u8 gbSgbATFList[16200];
			extern mapperMBC1 gbDataMBC1;
			extern mapperMBC2 gbDataMBC2;
			extern mapperMBC3 gbDataMBC3;
			extern mapperMBC5 gbDataMBC5;
			extern mapperHuC1 gbDataHuC1;
			extern mapperHuC3 gbDataHuC3;
			extern mapperTAMA5 gbDataTAMA5;
			extern u8 *gbTAMA5ram;
			extern int gbTAMA5ramSize;
			extern mapperMMM01 gbDataMMM01;
			extern u16 gbPalette[128];
			extern u8 *gbMemory;
			extern int gbRamSize;
			extern u8 *gbRam;
			extern int gbCgbMode;
			extern u8 *gbVram;
			extern u8 *gbWram;
			extern variable_desc gb_state[20];
			extern int gbCheatNumber;
			extern gbCheat gbCheatList[100];
			extern int gbLcdModeDelayed;
			extern int gbLcdTicksDelayed;
			extern int gbLcdLYIncrementTicksDelayed;
			extern u8 gbSpritesTicks[300];
			extern bool gbTimerModeChange;
			extern bool gbTimerOnChange;
			extern int gbHardware;
			extern bool gbBlackScreen;
			extern u8 oldRegister_WY;
			extern int gbWindowLine;
			extern int inUseRegister_WY;
			extern bool gbScreenOn;
			int marker = 0x12345678;

			int version = 12;
			stream.write(reinterpret_cast<const char *>(&version), sizeof(int));
			stream.write(reinterpret_cast<const char *>(&gbRom[0x134]), 15);

			int ub = useBios;
			int ib = inBios;
			stream.write(reinterpret_cast<const char *>(&ub), sizeof(int));
			stream.write(reinterpret_cast<const char *>(&ib), sizeof(int));

			int i = 0;
			for (; i < ARRAYSIZE(gbSaveGameStruct); i++)
			{
				if(gbSaveGameStruct[i].size > 0)
				{
					stream.write(reinterpret_cast<const char *>(gbSaveGameStruct[i].address), gbSaveGameStruct[i].size);
				}
			}

			stream.write(reinterpret_cast<const char *>(&IFF), sizeof(u16));

			if(gbSgbMode)
			{
				i = 0;
				for (; i < ARRAYSIZE(gbSgbSaveStructV3); i++)
				{
					if(gbSgbSaveStructV3[i].size > 0)
					{
						stream.write(reinterpret_cast<const char *>(gbSgbSaveStructV3[i].address), gbSgbSaveStructV3[i].size);
					}
				}

				stream.write(reinterpret_cast<const char *>(gbSgbBorder), 2048);
				stream.write(reinterpret_cast<const char *>(gbSgbBorderChar), 32*256);

				stream.write(reinterpret_cast<const char *>(gbSgbPacket), 16 * 7);

				stream.write(reinterpret_cast<const char *>(gbSgbSCPPalette), 4 * 512 * sizeof(u16));
				stream.write(reinterpret_cast<const char *>(gbSgbATF), 20 * 18);
				stream.write(reinterpret_cast<const char *>(gbSgbATFList), 45 * 20 * 18);
			}

			stream.write(reinterpret_cast<const char *>(&gbDataMBC1), sizeof(gbDataMBC1));
			stream.write(reinterpret_cast<const char *>(&gbDataMBC2), sizeof(gbDataMBC2));
			stream.write(reinterpret_cast<const char *>(&gbDataMBC3), sizeof(gbDataMBC3));
			stream.write(reinterpret_cast<const char *>(&gbDataMBC5), sizeof(gbDataMBC5));
			stream.write(reinterpret_cast<const char *>(&gbDataHuC1), sizeof(gbDataHuC1));
			stream.write(reinterpret_cast<const char *>(&gbDataHuC3), sizeof(gbDataHuC3));
			stream.write(reinterpret_cast<const char *>(&gbDataTAMA5), sizeof(gbDataTAMA5));
			if(gbTAMA5ram != NULL)
			{
				stream.write(reinterpret_cast<const char *>(gbTAMA5ram), gbTAMA5ramSize);
			}
			stream.write(reinterpret_cast<const char *>(&gbDataMMM01), sizeof(gbDataMMM01));

			stream.write(reinterpret_cast<const char *>(gbPalette), 128 * sizeof(u16));

			stream.write(reinterpret_cast<const char *>(&gbMemory[0x8000]), 0x8000);

			if(gbRamSize && gbRam)
			{
				stream.write(reinterpret_cast<const char *>(&gbRamSize), sizeof(int));
				stream.write(reinterpret_cast<const char *>(gbRam), gbRamSize);
			}

			if(gbCgbMode)
			{
				stream.write(reinterpret_cast<const char *>(gbVram), 0x4000);
				stream.write(reinterpret_cast<const char *>(gbWram), 0x8000);
			}

			// Sound
			gbSoundSaveGame2();
			i = 0;
			for (; i < ARRAYSIZE(gb_state); i++)
			{
				if(gb_state[i].size > 0)
				{
					stream.write(reinterpret_cast<const char *>(gb_state[i].address), gb_state[i].size);
				}
			}

			// Cheats
			stream.write(reinterpret_cast<const char *>(&gbCheatNumber), sizeof(int));
			if(gbCheatNumber > 0)
			{
				stream.write(reinterpret_cast<const char *>(&gbCheatList[0]), sizeof(gbCheat)*gbCheatNumber);
			}

			int spriteTicks = gbSpritesTicks[299];
			int timerModeChange = gbTimerModeChange;
			int timerOnChange = gbTimerOnChange;
			int blackScreen = gbBlackScreen;
			int oldRegister = oldRegister_WY;
			int screenOn = gbScreenOn;
			stream.write(reinterpret_cast<const char *>(&gbLcdModeDelayed), sizeof(int));
			stream.write(reinterpret_cast<const char *>(&gbLcdTicksDelayed), sizeof(int));
			stream.write(reinterpret_cast<const char *>(&gbLcdLYIncrementTicksDelayed), sizeof(int));
			stream.write(reinterpret_cast<const char *>(&spriteTicks), sizeof(int));
			stream.write(reinterpret_cast<const char *>(&timerModeChange), sizeof(int));
			stream.write(reinterpret_cast<const char *>(&timerOnChange), sizeof(int));
			stream.write(reinterpret_cast<const char *>(&gbHardware), sizeof(int));
			stream.write(reinterpret_cast<const char *>(&blackScreen), sizeof(int));
			stream.write(reinterpret_cast<const char *>(&oldRegister), sizeof(int));
			stream.write(reinterpret_cast<const char *>(&gbWindowLine), sizeof(int));
			stream.write(reinterpret_cast<const char *>(&inUseRegister_WY), sizeof(int));
			stream.write(reinterpret_cast<const char *>(&screenOn), sizeof(int));
			stream.write(reinterpret_cast<const char *>(&marker), sizeof(int));

			stream.flush();
			stream.close();

			wstring wname(fileNameA.begin(), fileNameA.end());
			Platform::String ^str = ref new Platform::String(wname.c_str());

			return StorageFile::GetFileFromPathAsync(str);

		}).then([](StorageFile ^file)
		{
			// Generate target file name and extension
			StorageFile ^romFile = ROMFile;

			Platform::String ^tmp = ROMFile->Name;
			const wchar_t *end = tmp->End();
			while(*end != '.') end--;
			size_t diff = tmp->End() - end;

			Platform::String ^name = ref new Platform::String(romFile->Name->Begin(), romFile->Name->Length() - diff) + SavestateSlot;
			Platform::String ^extension = file->FileType;
			Platform::String ^fullName = name + extension;
			return file->MoveAsync(ROMFolder, fullName, NameCollisionOption::ReplaceExisting);
		}).then([emulator](task<void> t)
		{
			try
			{
				emulator->Unpause();	
				t.get();
			}catch(Exception ^ex)
			{
#if _DEBUG
				Platform::String ^message = ex->Message;
				wstring str(message->Begin(), message->End());
				OutputDebugStringW((L"Save state: " + str).c_str());
#endif
			}
		});
	}

	task<void> SaveGBAStateAsync(void)
	{
		EmulatorGame *emulator = EmulatorGame::GetInstance();
		return create_task([emulator]()
		{
			if(!ROMFile || !ROMFolder)
			{
				throw ref new Exception(E_FAIL, L"No ROM loaded.");
			}
			emulator->Pause();

			// Generate random file name to store in temp folder			
			Platform::String ^folderpath = Windows::Storage::ApplicationData::Current->TemporaryFolder->Path;
			string folderPathStr(folderpath->Begin(), folderpath->End());

			stringstream tmpFileNameStream;
			tmpFileNameStream << folderPathStr << "\\";
			tmpFileNameStream << rand() << rand() << ".sgm";
			string fileNameA = tmpFileNameStream.str();

			FILE *file;
			auto error = fopen_s(&file, fileNameA.c_str(), "wb");
			if(!file)
			{
				stringstream ss;
				ss << "Unable to open tmp file '";
				ss << fileNameA;
				ss << "' to store savestate (";
				ss << error;
				ss << ").";
				//throw GameException(ss.str().c_str());
				string str(ss.str());
				throw ref new Exception(E_FAIL, ref new Platform::String(wstring(str.begin(), str.end()).c_str()));
			}
			fclose(file);

			ofstream stream (fileNameA.c_str(), ios::binary);
			if(!stream.is_open())
			{
#if _DEBUG
				stringstream ss;
				ss << "Unable to open tmp file '";
				ss << fileNameA;
				ss << "' to store savestate.";
				string str(ss.str());
				throw ref new Exception(E_FAIL, ref new Platform::String(wstring(str.begin(), str.end()).c_str()));
#endif
			}

			extern Gb_Apu *gb_apu;
			extern gb_apu_state_ss state;
			extern variable_desc saveGameStruct[116];
			extern variable_desc eepromSaveData[8];
			extern variable_desc flashSaveData3[6];
			extern variable_desc gba_state[32];
			extern RTCCLOCKDATA rtcClockData;
			extern bool stopState;
			extern int IRQTicks;
			extern int dummy_state [16];

			int version = SAVE_GAME_VERSION;
			stream.write(reinterpret_cast<const char *>(&version), sizeof(int));
			stream.write(reinterpret_cast<const char *>(&rom[0xa0]), 16);
			stream.write(reinterpret_cast<const char *>(&useBios), sizeof(bool));
			stream.write(reinterpret_cast<const char *>(&reg[0]), sizeof(reg));
			int i = 0;
			for (; i < ARRAYSIZE(saveGameStruct); i++)
			{
				if(saveGameStruct[i].size > 0)
				{
					stream.write(reinterpret_cast<const char *>(saveGameStruct[i].address), saveGameStruct[i].size);
				}
			}
			stream.write(reinterpret_cast<const char *>(&stopState), sizeof(bool));
			stream.write(reinterpret_cast<const char *>(&IRQTicks), sizeof(int));
			stream.write(reinterpret_cast<const char *>(internalRAM), 0x8000);
			stream.write(reinterpret_cast<const char *>(paletteRAM), 0x400);
			stream.write(reinterpret_cast<const char *>(workRAM), 0x40000);
			stream.write(reinterpret_cast<const char *>(vram), 0x20000);
			stream.write(reinterpret_cast<const char *>(oam), 0x400);
			//stream.write(reinterpret_cast<const char *>(pix), 0x400);
			stream.write(reinterpret_cast<const char *>(ioMem), 0x400);

			// EEPROM
			for (i = 0; i < ARRAYSIZE(eepromSaveData); i++)
			{
				if(eepromSaveData[i].size > 0)
				{
					stream.write(reinterpret_cast<const char *>(eepromSaveData[i].address), eepromSaveData[i].size);
				}
			}
			stream.write(reinterpret_cast<const char *>(&eepromSize), sizeof(int));
			stream.write(reinterpret_cast<const char *>(eepromData), 0x2000);

			// Flash
			for (i = 0; i < ARRAYSIZE(flashSaveData3); i++)
			{
				if(flashSaveData3[i].size > 0)
				{
					stream.write(reinterpret_cast<const char *>(flashSaveData3[i].address), flashSaveData3[i].size);
				}
			}

			// Sound
			gb_apu->save_state(&state.apu);
			memset(dummy_state, 0, sizeof dummy_state);
			for (i = 0; i < ARRAYSIZE(gba_state); i++)
			{
				if(gba_state[i].size > 0)
				{
					stream.write(reinterpret_cast<const char *>(gba_state[i].address), gba_state[i].size);
				}
			}

			stream.write(reinterpret_cast<const char *>(&rtcClockData), sizeof(rtcClockData));

			stream.flush();
			stream.close();

			wstring wname(fileNameA.begin(), fileNameA.end());
			Platform::String ^str = ref new Platform::String(wname.c_str());

			return StorageFile::GetFileFromPathAsync(str);

		}).then([](StorageFile ^file)
		{
			// Generate target file name and extension
			StorageFile ^romFile = ROMFile;
			Platform::String ^name = ref new Platform::String(romFile->Name->Begin(), romFile->Name->Length() - 4) + SavestateSlot;
			Platform::String ^extension = file->FileType;
			Platform::String ^fullName = name + extension;
			return file->MoveAsync(ROMFolder, fullName, NameCollisionOption::ReplaceExisting);
		}).then([emulator](task<void> t)
		{
			try
			{
				emulator->Unpause();	
				t.get();
			}catch(Exception ^ex)
			{
#if _DEBUG
				Platform::String ^message = ex->Message;
				wstring str(message->Begin(), message->End());
				OutputDebugStringW((L"Save state: " + str).c_str());
#endif
			}
		});
	}

	task<void> LoadStateAsync(int slot)
	{
		if(gbaROMLoaded)
		{
			return LoadGBAStateAsync(slot);
		}else
		{
			return LoadGBStateAsync(slot);
		}
	}

	task<void> LoadGBStateAsync(int slot)
	{
		int whichslot;
		if (slot <0)
			whichslot = SavestateSlot;
		else
			whichslot = slot;


		EmulatorGame *emulator = EmulatorGame::GetInstance();
		return create_task([emulator, whichslot]()
		{
			if(!ROMFile || !ROMFolder)
			{
				throw ref new Platform::Exception(E_FAIL, "No ROM loaded.");
			}
			emulator->Pause();
			wstringstream extension;
			extension << whichslot << L".sgm";

			Platform::String ^name = ROMFile->Name;
			const wchar_t *end = name->End();
			while(*end != '.') end--;
			size_t diff = name->End() - end;

			Platform::String ^nameWithoutExt = ref new Platform::String(name->Begin(), name->Length() - diff);
			Platform::String ^stateName = nameWithoutExt + ref new Platform::String(extension.str().c_str());
			return ROMFolder->GetFileAsync(stateName);

		}).then([](StorageFile ^file)
		{
			return file->CopyAsync(Windows::Storage::ApplicationData::Current->TemporaryFolder, file->Name, NameCollisionOption::ReplaceExisting);
		}).then([](StorageFile ^file)
		{
			Platform::String ^path = file->Path;
			wstring str = path->Data();

			ifstream stream(str, ios::binary);
			if(!stream.is_open())
			{
#if _DEBUG
				wstringstream ss;
				ss << L"Unable to open file '";
				ss << str;
				ss << L"' to load savestate.";
				wstring str(ss.str());
				throw ref new Exception(E_FAIL, ref new Platform::String(str.c_str()));
#endif
			}			

			extern u8* gbRom;
			extern bool useBios;
			extern bool inBios;
			extern variable_desc gbSaveGameStruct[78];
			extern u16 IFF;
			extern int gbSgbMode;
			extern variable_desc gbSgbSaveStructV3[11];
			extern u8 *gbSgbBorder;
			extern u8 *gbSgbBorderChar;
			extern u8 gbSgbPacket[112];
			extern u16 gbSgbSCPPalette[2048];
			extern u8 gbSgbATF[360];
			extern u8 gbSgbATFList[16200];
			extern mapperMBC1 gbDataMBC1;
			extern mapperMBC2 gbDataMBC2;
			extern mapperMBC3 gbDataMBC3;
			extern mapperMBC5 gbDataMBC5;
			extern mapperHuC1 gbDataHuC1;
			extern mapperHuC3 gbDataHuC3;
			extern mapperTAMA5 gbDataTAMA5;
			extern u8 *gbTAMA5ram;
			extern int gbTAMA5ramSize;
			extern mapperMMM01 gbDataMMM01;
			extern u16 gbPalette[128];
			extern u8 *gbMemory;
			extern int gbRamSize;
			extern u8 *gbRam;
			extern int gbCgbMode;
			extern u8 *gbVram;
			extern u8 *gbWram;
			extern variable_desc gb_state[20];
			extern int gbCheatNumber;
			extern gbCheat gbCheatList[100];
			extern int gbLcdModeDelayed;
			extern int gbLcdTicksDelayed;
			extern int gbLcdLYIncrementTicksDelayed;
			extern u8 gbSpritesTicks[300];
			extern bool gbTimerModeChange;
			extern bool gbTimerOnChange;
			extern int gbHardware;
			extern bool gbBlackScreen;
			extern u8 oldRegister_WY;
			extern int gbWindowLine;
			extern int inUseRegister_WY;
			extern bool gbScreenOn;
			extern int gbSgbMask;
			extern u8 gbSCYLine[300], register_SCY, gbSCXLine[300], register_SCX;
			extern u8 gbBgpLine[300];
			extern u8 gbBgp[4];
			extern u8 gbObp0Line[300], gbObp0[4], gbObp1Line[300], gbObp1[4];
			extern u8 register_SVBK, register_VBK;
			extern int gbBorderOn;
			extern int gbSpeed, gbLine99Ticks;

			int marker = 0x12345678;

			int version;
			stream.read(reinterpret_cast<char *>(&version), sizeof(int));

			u8 romname[20];
			stream.read(reinterpret_cast<char *>(romname), 15);

			int ub, ib;
			stream.read(reinterpret_cast<char *>(&ub), sizeof(int));
			stream.read(reinterpret_cast<char *>(&ib), sizeof(int));
			gbReset();
			inBios = ib ? true : false;

			int i = 0;
			for (; i < ARRAYSIZE(gbSaveGameStruct); i++)
			{
				if(gbSaveGameStruct[i].size > 0)
				{
					stream.read(reinterpret_cast<char *>(gbSaveGameStruct[i].address), gbSaveGameStruct[i].size);
				}
			}

			// Correct crash when loading color gameboy save in regular gameboy type.
			if (!gbCgbMode)
			{
				if(gbVram != NULL) {
					free(gbVram);
					gbVram = NULL;
				}
				if(gbWram != NULL) {
					free(gbWram);
					gbWram = NULL;
				}
			}
			else
			{
				if(gbVram == NULL)
					gbVram = (u8 *)malloc(0x4000);
				if(gbWram == NULL)
					gbWram = (u8 *)malloc(0x8000);
				memset(gbVram,0,0x4000);
				memset(gbPalette,0, 2*128);
			}

			stream.read(reinterpret_cast<char *>(&IFF), sizeof(u16));

			if(gbSgbMode)
			{
				i = 0;
				for (; i < ARRAYSIZE(gbSgbSaveStructV3); i++)
				{
					if(gbSgbSaveStructV3[i].size > 0)
					{
						stream.read(reinterpret_cast<char *>(gbSgbSaveStructV3[i].address), gbSgbSaveStructV3[i].size);
					}
				}
				stream.read(reinterpret_cast<char *>(gbSgbBorder), 2048);
				stream.read(reinterpret_cast<char *>(gbSgbBorderChar), 32*256);

				stream.read(reinterpret_cast<char *>(gbSgbPacket), 16*7);
				stream.read(reinterpret_cast<char *>(gbSgbSCPPalette), 4 * 512 * sizeof(u16));
				stream.read(reinterpret_cast<char *>(gbSgbATF), 20 * 18);
				stream.read(reinterpret_cast<char *>(gbSgbATFList), 45 * 20 * 18);
			}else
			{
				gbSgbMask = 0;
			}

			stream.read(reinterpret_cast<char *>(&gbDataMBC1), sizeof(gbDataMBC1));
			stream.read(reinterpret_cast<char *>(&gbDataMBC2), sizeof(gbDataMBC2));
			stream.read(reinterpret_cast<char *>(&gbDataMBC3), sizeof(gbDataMBC3));
			stream.read(reinterpret_cast<char *>(&gbDataMBC5), sizeof(gbDataMBC5));
			stream.read(reinterpret_cast<char *>(&gbDataHuC1), sizeof(gbDataHuC1));
			stream.read(reinterpret_cast<char *>(&gbDataHuC3), sizeof(gbDataHuC3));
			stream.read(reinterpret_cast<char *>(&gbDataTAMA5), sizeof(gbDataTAMA5));
			if(gbTAMA5ram != NULL)
			{
				if(skipSaveGameBattery)
				{
					stream.seekg(gbTAMA5ramSize, ios_base::cur);

				}else
				{
					stream.read(reinterpret_cast<char *>(gbTAMA5ram), gbTAMA5ramSize);
				}
			}
			stream.read(reinterpret_cast<char *>(&gbDataMMM01), sizeof(gbDataMMM01));

			stream.read(reinterpret_cast<char *>(gbPalette), 128 * sizeof(u16));

			stream.read(reinterpret_cast<char *>(&gbMemory[0x8000]), 0x8000);

			if(gbRamSize && gbRam)
			{
				int ramSize;
				stream.read(reinterpret_cast<char *>(&ramSize), sizeof(int));
				if(skipSaveGameBattery)
				{
					stream.seekg((gbRamSize>ramSize) ? ramSize : gbRamSize, ios_base::cur);
				}else
				{
					stream.read(reinterpret_cast<char *>(gbRam), (gbRamSize>ramSize) ? ramSize : gbRamSize);
				}
				if(ramSize > gbRamSize)
				{
					stream.seekg(ramSize-gbRamSize, ios_base::cur);
				}
			}

			memset(gbSCYLine, register_SCY, sizeof(gbSCYLine));
			memset(gbSCXLine, register_SCX, sizeof(gbSCXLine));
			memset(gbBgpLine, (gbBgp[0] | (gbBgp[1]<<2) | (gbBgp[2]<<4) |
				(gbBgp[3]<<6)), sizeof(gbBgpLine));
			memset(gbObp0Line, (gbObp0[0] | (gbObp0[1]<<2) | (gbObp0[2]<<4) |
				(gbObp0[3]<<6)), sizeof(gbObp0Line));
			memset(gbObp1Line, (gbObp1[0] | (gbObp1[1]<<2) | (gbObp1[2]<<4) |
				(gbObp1[3]<<6)), sizeof(gbObp1Line));
			memset(gbSpritesTicks, 0x0, sizeof(gbSpritesTicks));

			if (inBios)
			{
				gbMemoryMap[0x00] = &gbMemory[0x0000];
				memcpy ((u8 *)(gbMemory), (u8 *)(gbRom), 0x1000);
				memcpy ((u8 *)(gbMemory), (u8 *)(bios), 0x100);
			}
			else 
			{ 
				gbMemoryMap[0x00] = &gbRom[0x0000];			
			}
			gbMemoryMap[0x01] = &gbRom[0x1000];
			gbMemoryMap[0x02] = &gbRom[0x2000];
			gbMemoryMap[0x03] = &gbRom[0x3000];
			gbMemoryMap[0x04] = &gbRom[0x4000];
			gbMemoryMap[0x05] = &gbRom[0x5000];
			gbMemoryMap[0x06] = &gbRom[0x6000];
			gbMemoryMap[0x07] = &gbRom[0x7000];
			gbMemoryMap[0x08] = &gbMemory[0x8000];
			gbMemoryMap[0x09] = &gbMemory[0x9000];
			gbMemoryMap[0x0a] = &gbMemory[0xa000];
			gbMemoryMap[0x0b] = &gbMemory[0xb000];
			gbMemoryMap[0x0c] = &gbMemory[0xc000];
			gbMemoryMap[0x0d] = &gbMemory[0xd000];
			gbMemoryMap[0x0e] = &gbMemory[0xe000];
			gbMemoryMap[0x0f] = &gbMemory[0xf000];

			switch(gbRomType) 
			{
			case 0x00:
			case 0x01:
			case 0x02:
			case 0x03:
				// MBC 1
				memoryUpdateMapMBC1();
				break;
			case 0x05:
			case 0x06:
				// MBC2
				memoryUpdateMapMBC2();
				break;
			case 0x0b:
			case 0x0c:
			case 0x0d:
				// MMM01
				memoryUpdateMapMMM01();
				break;
			case 0x0f:
			case 0x10:
			case 0x11:
			case 0x12:
			case 0x13:
				// MBC 3
				memoryUpdateMapMBC3();
				break;
			case 0x19:
			case 0x1a:
			case 0x1b:
				// MBC5
				memoryUpdateMapMBC5();
				break;
			case 0x1c:
			case 0x1d:
			case 0x1e:
				// MBC 5 Rumble
				memoryUpdateMapMBC5();
				break;
			case 0x22:
				// MBC 7
				memoryUpdateMapMBC7();
				break;
			case 0x56:
				// GS3
				memoryUpdateMapGS3();
				break;
			case 0xfd:
				// TAMA5
				memoryUpdateMapTAMA5();
				break;
			case 0xfe:
				// HuC3
				memoryUpdateMapHuC3();
				break;
			case 0xff:
				// HuC1
				memoryUpdateMapHuC1();
				break;
			}

			if(gbCgbMode)
			{
				stream.read(reinterpret_cast<char *>(gbVram), 0x4000);
				stream.read(reinterpret_cast<char *>(gbWram), 0x8000);

				int value = register_SVBK;
				if(value == 0)
					value = 1;

				gbMemoryMap[0x08] = &gbVram[register_VBK * 0x2000];
				gbMemoryMap[0x09] = &gbVram[register_VBK * 0x2000 + 0x1000];
				gbMemoryMap[0x0d] = &gbWram[value * 0x1000];
			}

			gbSoundReadGame2();
			i = 0;
			for (; i < ARRAYSIZE(gb_state); i++)
			{
				if(gb_state[i].size > 0)
				{
					stream.read(reinterpret_cast<char *>(gb_state[i].address), gb_state[i].size);
				}
			}
			gbSoundReadGame3();

			if (gbCgbMode && gbSgbMode) {
				gbSgbMode = 0;
			}

			if(gbBorderOn && !gbSgbMask) {
				gbSgbRenderBorder();
			}

			// systemDrawScreen(); // Deadlock!

			int numberCheats;
			stream.read(reinterpret_cast<char *>(&numberCheats), sizeof(int));
			if(skipSaveGameCheats)
			{
				stream.seekg(numberCheats * sizeof(gbCheat), ios_base::cur);
			}else
			{
				gbCheatNumber = numberCheats;
				if(gbCheatNumber > 0)
				{
					stream.read(reinterpret_cast<char *>(&gbCheatList[0]), sizeof(gbCheat) * gbCheatNumber);
				}
			}
			gbCheatUpdateMap();

			int spriteTicks;
			int timerModeChange;
			int timerOnChange;
			int blackScreen;
			int oldRegister;
			int screenOn;
			stream.read(reinterpret_cast<char *>(&gbLcdModeDelayed), sizeof(int));
			stream.read(reinterpret_cast<char *>(&gbLcdTicksDelayed), sizeof(int));
			stream.read(reinterpret_cast<char *>(&gbLcdLYIncrementTicksDelayed), sizeof(int));
			stream.read(reinterpret_cast<char *>(&spriteTicks), sizeof(int));
			stream.read(reinterpret_cast<char *>(&timerModeChange), sizeof(int));
			stream.read(reinterpret_cast<char *>(&timerOnChange), sizeof(int));
			stream.read(reinterpret_cast<char *>(&gbHardware), sizeof(int));
			stream.read(reinterpret_cast<char *>(&blackScreen), sizeof(int));
			stream.read(reinterpret_cast<char *>(&oldRegister), sizeof(int));
			stream.read(reinterpret_cast<char *>(&gbWindowLine), sizeof(int));
			stream.read(reinterpret_cast<char *>(&inUseRegister_WY), sizeof(int));
			stream.read(reinterpret_cast<char *>(&screenOn), sizeof(int));
			gbSpritesTicks[299] = spriteTicks;
			gbTimerModeChange = timerModeChange ? true : false;
			gbTimerOnChange = timerOnChange ? true : false;
			gbBlackScreen = blackScreen ? true : false;
			oldRegister_WY = oldRegister;
			gbScreenOn = screenOn;


			if (gbSpeed)
				gbLine99Ticks *= 2;

			systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;

			return file->DeleteAsync();
		}).then([]()
		{
			return ApplyCheats(ROMCheats);
		}).then([](){}).then([emulator](task<void> t)
		{
			try
			{
				emulator->Unpause();
				t.get();
			}catch(Platform::Exception ^ex)
			{
#if _DEBUG
				wstring err = ex->Message->Data();
				OutputDebugStringW(err.c_str());
				//EngineLog(LOG_LEVEL::Error, L"Load savestate: " + err);
#endif
			}
		});
	}

	task<void> LoadGBAStateAsync(int slot)
	{
		int whichslot;
		if (slot <0)
			whichslot = SavestateSlot;
		else
			whichslot = slot;

		EmulatorGame *emulator = EmulatorGame::GetInstance();
		return create_task([emulator, whichslot]()
		{
			if(!ROMFile || !ROMFolder)
			{
				throw ref new Platform::Exception(E_FAIL, "No ROM loaded.");
			}
			emulator->Pause();
			wstringstream extension;
			extension << whichslot << L".sgm";

			Platform::String ^name = ROMFile->Name;
			Platform::String ^nameWithoutExt = ref new Platform::String(name->Begin(), name->Length() - 4);
			Platform::String ^stateName = nameWithoutExt + ref new Platform::String(extension.str().c_str());
			return ROMFolder->GetFileAsync(stateName);
		}).then([](StorageFile ^file)
		{
			return file->CopyAsync(Windows::Storage::ApplicationData::Current->TemporaryFolder, file->Name, NameCollisionOption::ReplaceExisting);
		}).then([](StorageFile ^file)
		{
			Platform::String ^path = file->Path;
			wstring str = path->Data();

			ifstream stream(str, ios::binary);
			if(!stream.is_open())
			{
#if _DEBUG
				wstringstream ss;
				ss << L"Unable to open file '";
				ss << str;
				ss << L"' to load savestate.";
				wstring str(ss.str());
				throw ref new Exception(E_FAIL, ref new Platform::String(str.c_str()));
#endif
			}

			extern Gb_Apu *gb_apu;
			extern gb_apu_state_ss state;
			extern variable_desc saveGameStruct[116];
			extern variable_desc eepromSaveData[8];
			extern variable_desc flashSaveData3[6];
			extern variable_desc gba_state[32];
			extern RTCCLOCKDATA rtcClockData;
			extern bool stopState;
			extern int IRQTicks;
			extern int dummy_state [16];
			extern bool intState;

			int version;
			u8 romname[17];
			romname[16] = 0;
			bool ub;
			stream.read(reinterpret_cast<char *>(&version), sizeof(int));
			stream.read(reinterpret_cast<char *>(romname), 16);
			stream.read(reinterpret_cast<char *>(&ub), sizeof(bool));
			stream.read(reinterpret_cast<char *>(&reg[0]), sizeof(reg));
			int i = 0;
			for (; i < ARRAYSIZE(saveGameStruct); i++)
			{
				if(saveGameStruct[i].size > 0)
				{
					stream.read(reinterpret_cast<char *>(saveGameStruct[i].address), saveGameStruct[i].size);
				}
			}
			stream.read(reinterpret_cast<char *>(&stopState), sizeof(bool));
			stream.read(reinterpret_cast<char *>(&IRQTicks), sizeof(int));
			if(IRQTicks > 0)
			{
				intState = true;
			}else
			{
				intState = false;
				IRQTicks = 0;
			}
			stream.read(reinterpret_cast<char *>(internalRAM), 0x8000);
			stream.read(reinterpret_cast<char *>(paletteRAM), 0x400);
			stream.read(reinterpret_cast<char *>(workRAM), 0x40000);
			stream.read(reinterpret_cast<char *>(vram), 0x20000);
			stream.read(reinterpret_cast<char *>(oam), 0x400);
			stream.read(reinterpret_cast<char *>(ioMem), 0x400);


			if(skipSaveGameBattery)
			{
				// Skip EEPROM
				for (i = 0; i < ARRAYSIZE(eepromSaveData); i++)
				{
					stream.seekg(eepromSaveData[i].size, ios_base::cur);
				}
				stream.seekg(sizeof(int), ios_base::cur);
				stream.seekg(0x2000, ios_base::cur);

				// Skip Flash
				for (i = 0; i < ARRAYSIZE(flashSaveData3); i++)
				{
					stream.seekg(flashSaveData3[i].size, ios_base::cur);
				}

			}else
			{
				// Read EEPROM
				for (i = 0; i < ARRAYSIZE(eepromSaveData); i++)
				{
					if(eepromSaveData[i].size > 0)
					{
						stream.read(reinterpret_cast<char *>(eepromSaveData[i].address), eepromSaveData[i].size);
					}
				}
				stream.read(reinterpret_cast<char *>(&eepromSize), sizeof(int));
				stream.read(reinterpret_cast<char *>(eepromData), 0x2000);

				// Read Flash
				for (i = 0; i < ARRAYSIZE(flashSaveData3); i++)
				{
					if(flashSaveData3[i].size > 0)
					{
						stream.read(reinterpret_cast<char *>(flashSaveData3[i].address), flashSaveData3[i].size);
					}
				}
			}

			// Sound
			reset_apu();
			gb_apu->save_state(&state.apu);
			for (i = 0; i < ARRAYSIZE(gba_state); i++)
			{
				if(gba_state[i].size > 0)
				{
					stream.read(reinterpret_cast<char *>(gba_state[i].address), gba_state[i].size);
				}
			}
			gb_apu->load_state(state.apu);
			write_SGCNT0_H(READ16LE(&ioMem[SGCNT0_H]) & 0x770F);
			apply_muting();

			stream.read(reinterpret_cast<char *>(&rtcClockData), sizeof(rtcClockData));

			stream.close();

			layerEnable = layerSettings & DISPCNT;
			CPUUpdateRender();
			CPUUpdateRenderBuffers(true);
			CPUUpdateWindow0();
			CPUUpdateWindow1();

			gbaSaveType = 0;
			switch(saveType) {
			case 0:
				cpuSaveGameFunc = flashSaveDecide;
				break;
			case 1:
				cpuSaveGameFunc = sramWrite;
				gbaSaveType = 1;
				break;
			case 2:
				cpuSaveGameFunc = flashWrite;
				gbaSaveType = 2;
				break;
			case 3:
				break;
			case 5:
				gbaSaveType = 5;
				break;
			default:
				systemMessage(MSG_UNSUPPORTED_SAVE_TYPE,
					N_("Unsupported save type %d"), saveType);
				break;
			}
			if(eepromInUse)
				gbaSaveType = 3;

			CPUReadHelper();

			return file->DeleteAsync();
		}).then([]()
		{
			return ApplyCheats(ROMCheats);
		}).then([](){}).then([emulator](task<void> t)
		{
			try
			{
				emulator->Unpause();
				t.get();
			}catch(Platform::Exception ^ex)
			{
#if _DEBUG
				wstring err = ex->Message->Data();
				OutputDebugStringW(err.c_str());
				//EngineLog(LOG_LEVEL::Error, L"Load savestate: " + err);
#endif
			}
		});
	}


	task<void> SuspendAsync(void)
	{
		return create_task([]()
		{			
			ApplicationDataContainer ^localSettings = ApplicationData::Current->LocalSettings;

			if(!IsROMLoaded())
			{
				localSettings->Values->Remove("ROMPath");
				localSettings->Values->Remove("ROMFolderPath");
				localSettings->Values->Remove("SelectedSaveStateSlot");
				return;
			}
			int oldSlot = GetSavestateSlot();
			SelectSavestateSlot(AUTOSAVESTATE_SLOT);
			SaveStateAsync().wait();
			SelectSavestateSlot(oldSlot);			
			localSettings->Values->Insert("ROMPath", dynamic_cast<Windows::Foundation::PropertyValue ^>(Windows::Foundation::PropertyValue::CreateString(ROMFile->Path)));
			localSettings->Values->Insert("ROMFolderPath", dynamic_cast<Windows::Foundation::PropertyValue ^>(Windows::Foundation::PropertyValue::CreateString(ROMFolder->Path)));
			localSettings->Values->Insert("SelectedSaveStateSlot", dynamic_cast<Windows::Foundation::PropertyValue ^>(Windows::Foundation::PropertyValue::CreateInt32(GetSavestateSlot())));

			StoreSettings();
			SaveSRAMAsync().wait();
		});
	}

	task<void> RestoreFromApplicationDataAsync(void)
	{
		return create_task([]()
		{
			if(IsROMLoaded())
			{
				throw ref new Platform::Exception(E_FAIL, L"Unable to restore state because a ROM is already loaded.");
			}

			ApplicationDataContainer ^localSettings = ApplicationData::Current->LocalSettings;
			auto values = localSettings->Values;

			Platform::String ^romPath = safe_cast<Platform::String ^>(values->Lookup("ROMPath"));
			Platform::String ^romFolderPath = safe_cast<Platform::String ^>(values->Lookup("ROMFolderPath"));
			auto savestateSlotEntry = safe_cast<Windows::Foundation::IPropertyValue^>(values->Lookup("SelectedSaveStateSlot"));
			int savestateSlot = 0;

			if(savestateSlotEntry)
			{
				savestateSlot = savestateSlotEntry->GetInt32();
			}

			if(!romPath || !romFolderPath)
			{
				throw ref new Platform::Exception(E_FAIL, L"Unable to restore state after termination.");
			}

			StorageFile ^romfile = nullptr;
			StorageFolder ^romfolder = nullptr;
			create_task(StorageFile::GetFileFromPathAsync(romPath)).then([&romfile, romFolderPath](StorageFile ^file)
			{
				romfile = file;
				return StorageFolder::GetFolderFromPathAsync(romFolderPath);
			}).then([&romfolder](StorageFolder ^folder)
			{
				romfolder = folder;
			}).wait();

			LoadROMAsync(romfile, romfolder).wait();
			LoadStateAsync(AUTOSAVESTATE_SLOT).wait();

			RestoreSettings();
		}).then([](task<void> t)
		{
			try
			{
				t.get();
			}catch(Platform::Exception ^ex)
			{
#if _DEBUG
				Platform::String ^str = ex->Message;
				std::wstring wstr(str->Begin(), str->End());
				OutputDebugStringW(wstr.c_str());
#endif
			}
		});
		return task<void>();
	}

	void SelectSavestateSlot(int slot)
	{
		SavestateSlot = slot % MAX_SAVESTATE_SLOTS;
	}

	int GetSavestateSlot(void)
	{
		return SavestateSlot;
	}
	
	task<Windows::Foundation::Collections::IVector<CheatData ^> ^> LoadCheats(void)
	{
		Windows::Foundation::Collections::IVector<CheatData ^> ^cheats = ref new Vector<CheatData ^>();

		if(!ROMFile || !ROMFolder)
			return task<Windows::Foundation::Collections::IVector<CheatData ^> ^>(([cheats](){ return cheats;}));

		Platform::String ^name = ROMFile->Name;
		const wchar_t *end = name->End();
		while(*end != '.') end--;
		size_t diff = name->End() - end;

		Platform::String ^nameWithoutExt = ref new Platform::String(name->Begin(), name->Length() - diff);
		Platform::String ^cheatName = nameWithoutExt->Concat(nameWithoutExt, ".cht");

		return create_task([cheatName]()
		{
			return ROMFolder->GetFileAsync(cheatName);
		}).then([](StorageFile ^file)
		{
			return file->OpenReadAsync();
		}).then([cheats](IRandomAccessStreamWithContentType ^stream)
		{
			auto readStream = stream->GetInputStreamAt(0);
			DataReader ^reader = ref new DataReader(readStream);
			
			return create_task([reader, stream, cheats]()
			{
				create_task(reader->LoadAsync(stream->Size)).wait();
				String ^codes = reader->ReadString(stream->Size);

				if(codes != nullptr && !codes->IsEmpty())
				{
					string codesString(codes->Begin(), codes->End());
					strreplace(codesString, '\n', '\r');
					vector<string> v;
					strSplitLines(codesString, v);

					for (int i = 0; i < v.size(); i+=3)
					{
						if(v.size() - i < 3)
							continue;
						string tmp;
						wstring wtmp;
						CheatData ^data = ref new CheatData();
						
						tmp = v.at(i);
						wtmp = wstring(tmp.begin(), tmp.end());
						data->Description = ref new String(wtmp.c_str());

						tmp = v.at(i + 1);
						strreplace(tmp, '$', '\n');  //replace $ by \n
						wtmp = wstring(tmp.begin(), tmp.end());

						data->CheatCode = ref new String(wtmp.c_str());

						tmp = v.at(i + 2);
						data->Enabled = (strcmp(tmp.c_str(), "1") == 0);

						cheats->Append(data);
					}
				}
			});

		}).then([cheats](task<void> t)
		{
			try
			{
				t.get();
			}
			catch(Platform::Exception ^e)
			{ 
#if _DEBUG
				String ^msg = e->Message;
				wstring wstr (msg->Begin(), msg->End());
				OutputDebugStringW(wstr.c_str());
#endif
			}
			return cheats;
		});
	}

	task<bool> SaveCheats()
	{
		if(!ROMFile || !ROMFolder)
			return task<bool>(([](){ return false;}));

		//ROMCheats = cheats;

		Platform::String ^name = ROMFile->Name;
		const wchar_t *end = name->End();
		while(*end != '.') end--;
		size_t diff = name->End() - end;

		Platform::String ^nameWithoutExt = ref new Platform::String(name->Begin(), name->Length() - diff);
		Platform::String ^cheatName = nameWithoutExt->Concat(nameWithoutExt, ".cht");

		return create_task([cheatName]()
		{
			return ROMFolder->CreateFileAsync(cheatName, CreationCollisionOption::ReplaceExisting);
		}).then([](StorageFile ^file)
		{
			return file->OpenAsync(FileAccessMode::ReadWrite);
		}).then([](IRandomAccessStream ^stream)
		{
			IOutputStream ^outStream = stream->GetOutputStreamAt(0);
			DataWriter ^writer = ref new DataWriter(outStream);

			writer->UnicodeEncoding = UnicodeEncoding::Utf8;
			writer->ByteOrder = ByteOrder::LittleEndian;

			for (int i = 0; i < ROMCheats->Size; i++)
			{
				if(i > 0)
				{
					writer->WriteString("\n");
				}
				writer->WriteString(ROMCheats->GetAt(i)->Description);
				writer->WriteString("\n");

				//replace \n in cheat code by $
				wstring code( ROMCheats->GetAt(i)->CheatCode->Begin(), ROMCheats->GetAt(i)->CheatCode->End() );

				strreplace(code, '\n', '$');

				Platform::String^ pcode = ref new Platform::String(code.c_str());

				writer->WriteString(pcode);
				writer->WriteString("\n");
				writer->WriteString(ROMCheats->GetAt(i)->Enabled ? "1" : "0");
			}
			return create_task([writer]()
			{
				create_task(writer->StoreAsync()).wait();
				create_task(writer->FlushAsync()).wait();
				writer->DetachStream();	
			});
			
		}).then([]()
		{
			return ApplyCheats(ROMCheats);
		}).then([](task<void> t)
		{
			try
			{
				t.get();
				return true;
			}catch(Platform::Exception ^e)
			{
				return false;
			}
		});
	}

	void ApplyCheats(Windows::Foundation::Collections::IVector<CheatData ^> ^cheats)
	{
		if (gbaROMLoaded)
			ApplyCheatsGBA(cheats);
		else
			ApplyCheatsGB(cheats);
	}



	void ApplyCheatsGBA(Windows::Foundation::Collections::IVector<CheatData ^> ^cheats)
	{

		cheatsDeleteAll(false);

		cheatsEnabled = (cheats->Size > 0);

		for (int i = 0; i < cheats->Size; i++)
		{
			auto data = cheats->GetAt(i);
			if (!data->Enabled)
				continue;

			Platform::String ^code = data->CheatCode;
			Platform::String ^desc = data->Description;

			string codeString(code->Begin(), code->End());
			string descString(desc->Begin(), desc->End());

			//split the compound code into multiple part
			vector<string> codeParts;
			strreplace(codeString, '\n', '\r');
			strSplitLines(codeString, codeParts);

			for (int j = 0; j < codeParts.size(); j++)
			{
				string code = codeParts.at(j);

				if (code.length() == 13)
				{
					// Code Breaker
					cheatsAddCBACode(code.c_str(), descString.c_str());
				}
				else if (code.length() == 16)
				{
					//gameshark v1, 2
					cheatsAddGSACode(code.c_str(), descString.c_str(), false);
				}
				else if (code.length() == 17)
				{
					//gameshark v3
					//remove space				
					code = code.substr(0, 8) + code.substr(9, 8);

					cheatsAddGSACode(code.c_str(), descString.c_str(), true);
				}
			}
		}
	}


	void ApplyCheatsGB(Windows::Foundation::Collections::IVector<CheatData ^> ^cheats)
	{

		gbCheatRemoveAll();

		cheatsEnabled = (cheats->Size > 0);

		for (int i = 0; i < cheats->Size; i++)
		{
			auto data = cheats->GetAt(i);
			if (!data->Enabled)
				continue;

			Platform::String ^code = data->CheatCode;
			Platform::String ^desc = data->Description;

			string codeString(code->Begin(), code->End());
			string descString(desc->Begin(), desc->End());

			//split the compound code into multiple part
			vector<string> codeParts;
			strreplace(codeString, '\n', '\r');
			strSplitLines(codeString, codeParts);

			for (int j = 0; j < codeParts.size(); j++)
			{
				string code = codeParts.at(j);

				if (code.length() == 11 || code.length() == 7)
				{
					// GameGenie
					gbAddGgCheat(code.c_str(), descString.c_str());
				}
				else if (code.length() == 8)
				{
					// Gameshark
					gbAddGsCheat(code.c_str(), descString.c_str());
				}
			}
		}
	}


	task<void> SaveBytesToFileAsync(StorageFile ^file, unsigned char *bytes, size_t length)
	{
		Platform::String ^name = file->Name;

		return create_task([file]()
		{
			return file->OpenAsync(FileAccessMode::ReadWrite);
		}).then([=](IRandomAccessStream ^stream)
		{
			IOutputStream ^outputStream = stream->GetOutputStreamAt(0L);;
			DataWriter ^writer = ref new DataWriter(outputStream);
			
			Platform::Array<unsigned char> ^array = ref new Array<unsigned char>(length);
			memcpy(array->Data, bytes, length);

			writer->WriteBytes(array);
			create_task(writer->StoreAsync()).wait();
			writer->DetachStream();
			return create_task(outputStream->FlushAsync());
		}).then([name](bool b)
		{
			if(!b)
			{
#if _DEBUG
				wstring wname(name->Begin(), name->End());
				//EngineLog(LOG_LEVEL::Error, wname + L": Error while writing to the file.");
				OutputDebugStringW((wname + L": Error while writing to the file.").c_str());
#endif
			}
		}).then([name](task<void> t)
		{
			try
			{
				t.get();
			}catch(COMException ^ex)
			{
#if _DEBUG
				Platform::String ^error = ex->Message;
				wstring wname(name->Begin(), name->End());
				wstring werror(error->Begin(), error->End());
				//EngineLog(LOG_LEVEL::Error, wname + L": " + werror);

				OutputDebugStringW((wname + L": " + werror).c_str());
#endif
			}
		});
	}


	task<ROMData> GetBytesFromFileAsync(StorageFile ^file)
	{
		auto inputStream = make_shared<IInputStream ^>();
		auto openTask = create_task(file->OpenSequentialReadAsync());

		return openTask.then([=] (IInputStream ^stream)
		{ 
			*inputStream = stream;
			return file->GetBasicPropertiesAsync();
		}).then([=](BasicProperties ^properties)
		{
			Buffer ^buffer = ref new Buffer(properties->Size);
			return (*inputStream)->ReadAsync(buffer, properties->Size, InputStreamOptions::None);
		})
			.then([=](IBuffer ^buffer)
		{			
			DataReader ^reader = DataReader::FromBuffer(buffer);
			Array<BYTE> ^bytes = ref new Array<BYTE>(buffer->Length);
			reader->ReadBytes(bytes);
			BYTE *rawBytes = new BYTE[buffer->Length];
			for (int i = 0; i < buffer->Length; i++)
			{
				rawBytes[i] = bytes[i]; 
			}

			ROMData data;
			data.Length = buffer->Length;
			data.ROM = rawBytes;

			return data;
		});
	}




}