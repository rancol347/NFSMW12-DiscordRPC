/*
 * Need for Speed: Most Wanted (2012) Discord Rich Presence Plugin
 * 
 * Author: rancol347
 * Version: 1.0
 * 
 * GitHub: https://github.com/rancol347/NFSMW12-DiscordRPC/
 */
#include <windows.h>
#include <iostream>
#include <thread>
#include <string>
#include <ctime>
#include "discord_rpc.h"

#pragma comment(lib, "discord-rpc.lib")

// --- НАСТРОЙКИ DISCORD И ТАЙМЕРА ---
const char* APPLICATION_ID = "1395997218061680660";
time_t g_StartTime = 0; // Неубиваемый таймер на базе time_t

// --- НАСТРОЙКИ ПАМЯТИ ИГРЫ (ОФФСЕТЫ) ---
static const uintptr_t lobbyBaseOffset = 0x00DA69E8;
static const unsigned int lobbyOffsets[] = { 0x64C };

static const uintptr_t heatBaseOffset = 0x00D0FD08;
static const unsigned int heatOffsets[] = { 0x8, 0x8, 0x10, 0x13C, 0x3E8 };
static const int heatOffsetsCount = 5;

static const uintptr_t carIdBase = 0x00D0FD08;
static const unsigned int carIdOffsets[] = { 0x8, 0x50, 0x308, 0x18 };

static const uintptr_t gameTimeBaseOffset = 0x00D0FD08; // База совпадает с копами и авто
static const unsigned int gameTimeOffsets[] = { 0x8, 0x8, 0x10, 0x628 };
static const int gameTimeOffsetsCount = 4;

// --- БЕЗОПАСНОЕ ЧТЕНИЕ ПАМЯТИ ---
float SafeReadFloatChain(uintptr_t baseOffset, const unsigned int offsets[], int count) {
    uintptr_t hModule = (uintptr_t)GetModuleHandle(NULL);
    if (!hModule) return 0.0f;

    uintptr_t currentAddr = hModule + baseOffset;

    for (int i = 0; i < count; i++) {
        uintptr_t nextAddr = 0;
        if (!ReadProcessMemory(GetCurrentProcess(), (LPCVOID)currentAddr, &nextAddr, sizeof(nextAddr), NULL)) {
            return 0.0f;
        }
        currentAddr = nextAddr + offsets[i];
    }

    float finalValue = 0.0f;
    if (ReadProcessMemory(GetCurrentProcess(), (LPCVOID)currentAddr, &finalValue, sizeof(finalValue), NULL)) {
        return finalValue;
    }

    return 0.0f;
}

int SafeReadIntChain(uintptr_t baseOffset, const unsigned int offsets[], int count) {
    uintptr_t hModule = (uintptr_t)GetModuleHandle(NULL);
    if (!hModule) return 0;

    uintptr_t currentAddr = 0;
    if (!ReadProcessMemory(GetCurrentProcess(), (LPCVOID)(hModule + baseOffset), &currentAddr, sizeof(currentAddr), NULL)) {
        return 0;
    }

    for (int i = 0; i < count - 1; i++) {
        if (currentAddr == 0) return 0;

        if (!ReadProcessMemory(GetCurrentProcess(), (LPCVOID)(currentAddr + offsets[i]), &currentAddr, sizeof(currentAddr), NULL)) {
            return 0;
        }
    }

    currentAddr += offsets[count - 1];

    int finalValue = 0;
    if (ReadProcessMemory(GetCurrentProcess(), (LPCVOID)currentAddr, &finalValue, sizeof(finalValue), NULL)) {
        return finalValue;
    }

    return 0;
}

// --- БАЗА ДАННЫХ МАШИН ---
std::string GetCarNameByID(int carID) {
    switch (carID) {
    case 1085698: return "Alfa Romeo 4C Concept";
    case 2196263: return "Alfa Romeo Mito QV";
    case 1160082: return "Ariel Atom 500 V8";
    case 2196529: return "Aston Martin DB5";
    case 2196595: return "Aston Martin DBS";
    case 122672:  return "Aston Martin V12 Vantage";
    case 1160139: return "Audi A1 clubsport quattro";
    case 122675:  return "Audi R8 GT Spyder";
    case 2196131: return "Audi RS3 Sportback";
    case 1160025: return "BAC Mono";
    case 866774:  return "Bentley Supersports ISR";
    case 2196065: return "BMW 1 Series M Coupe";
    case 122682:  return "BMW M3 Coupe";
    case 393276:  return "BMW M3 GTR";
    case 122692:  return "Bugatti Veyron Super Sport";
    case 2076005: return "Bugatti Veyron Vitesse";
    case 393001:  return "Caterham Superlight R500";
    case 621053:  return "Chevrolet Camaro ZL1";
    case 122701:  return "Chevrolet Corvette ZR1";
    case 2196466: return "Dodge Charger R/T";
    case 300097:  return "Ford Raptor SVT F150";
    case 2196197: return "Ford Fiesta ST";
    case 1160480: return "Ford Focus RS500";
    case 122714:  return "Ford GT";
    case 122716:  return "Ford Mustang Boss 302";
    case 2076137: return "Hennessey Venom GT Spyder";
    case 1399602: return "Jaguar XKR";
    case 589261:  return "Koenigsegg Agera R";
    case 392074:  return "Lamborghini Aventador";
    case 2076203: return "Lamborghini Aventador J";
    case 1085091: return "Lamborghini Countach";
    case 2076339: return "Lamborghini Diablo";
    case 1085633: return "Lamborghini Gallardo";
    case 1097100: return "Lancia Delta HF Integrale";
    case 1085958: return "Range Rover Evoque";
    case 1085511: return "Lexus LFA";
    case 1551590: return "Marussia B2";
    case 122757:  return "Maserati GT MC Stradale";
    case 2076070: return "McLaren F1 LM";
    case 122765:  return "McLaren MP4-12C";
    case 122769:  return "Mercedes-Benz SL 65 AMG";
    case 122773:  return "Mercedes-Benz SLS AMG";
    case 866832:  return "Mitsubishi EVOLUTION X";
    case 2076462: return "NISSAN 350Z";
    case 435169:  return "NISSAN GTR EGOIST";
    case 2076524: return "NISSAN SKYLINE GT-R (R34)";
    case 535435:  return "Pagani Huayra";
    case 2076266: return "Pagani Zonda R";
    case 2196399: return "Pontiac Firebird T/A";
    case 1085007: return "Porsche 911 Carrera S";
    case 2076399: return "Porsche 911 GT2";
    case 1085186: return "Porsche 911 Turbo 3.0";
    case 2196000: return "Porsche 918 Spyder";
    case 1085830: return "Porsche 918 Spyder Concept";
    case 535621:  return "Porsche Panamera Turbo S";
    case 1085576: return "Shelby COBRA 427";
    case 2196334: return "Shelby GT500";
    case 122704:  return "Dodge Challenger SRT8";
    case 1160350: return "SRT Viper GTR";
    case 122814:  return "Subaru Cosworth Impreza";
    case 1085764: return "Tesla Roadster Sport";
    case 1467242: return "SWAT Van";
    case 1399322: return "Dodge Charger SRT8";
    case 1467164: return "Ford Raptor (Pre-Order)";
    case 1467172: return "Maserati MC12 (Pre-Order)";
    case 1467179: return "Mercedes-Benz SL 65 (Pre-Order)";
    case 1467186: return "Porsche 911 (Pre-Order)";
    case 1467156: return "Caterham R500 (Pre-Order)";
    case 510072:  return "Ford Explorer (Police)";
    case 122713:  return "Ford Crown Victoria (Police)";
    case 122706:  return "Dodge Charger SRT8 (Police)";
    case 122699:  return "Chevrolet Corvette Z06 (Police)";
    case 1399539: return "Ford Focus ST (Cops/Civilian)";
    case 2277489: case 2277456: case 2277467: case 2277478: return "Lamborghini Diablo";
    case 2277252: return "BMW M3 GTR";
    case 2277210: case 2277224: case 2277238: return "BMW M3 Coupe";
    case 2255978: case 2255945: case 2255956: case 2255967: return "NISSAN 350Z";
    case 2255934: case 2255901: case 2255912: case 2255923: return "NISSAN SKYLINE GT-R (R34)";
    case 2277368: case 2277332: case 2277344: case 2277356: return "Porsche 911 GT2";
    case 122816: case 122818: case 122819: case 122821: case 122822:
    case 122824: case 122827: case 122828: case 122829: case 122830:
    case 392322: case 1085448: case 1097663: case 1160687: case 1097968:
        return "kalduncar";
    default:
        if (carID > 0) return "kalduncar";
        wchar_t localeName[LOCALE_NAME_MAX_LENGTH];
        GetUserDefaultLocaleName(localeName, LOCALE_NAME_MAX_LENGTH);
        bool isRussian = (localeName[0] == L'r' && localeName[1] == L'u');
        return isRussian ? "11 маршруте" : "foot";
    }
}

void InitDiscord() {
    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));
    Discord_Initialize(APPLICATION_ID, &handlers, 1, NULL);
}

// --- ОСНОВНАЯ ФУНКЦИЯ ОБНОВЛЕНИЯ СТАТУСА ---
void UpdatePresence() {
    wchar_t localeName[LOCALE_NAME_MAX_LENGTH];
    GetUserDefaultLocaleName(localeName, LOCALE_NAME_MAX_LENGTH);

    bool isRussian = (localeName[0] == L'r' && localeName[1] == L'u');
    int lobbyOffsetsCount = 1;
    int carIdOffsetsCount = 4;

    // Читаем данные из игры
    int lobbyStatus = SafeReadIntChain(lobbyBaseOffset, lobbyOffsets, lobbyOffsetsCount);
    float rawHeat = SafeReadFloatChain(heatBaseOffset, heatOffsets, heatOffsetsCount);
    // Читаем время вручную, чтобы точно применить отрицательный оффсет
    float gameTime = 0.0f;
    uintptr_t hModule = (uintptr_t)GetModuleHandle(NULL);
    uintptr_t addr = 0;

    // Идем по цепочке до предпоследнего оффсета (0x628)
    if (ReadProcessMemory(GetCurrentProcess(), (LPCVOID)(hModule + 0x00D0FD08), &addr, sizeof(addr), NULL) && addr != 0) {
        if (ReadProcessMemory(GetCurrentProcess(), (LPCVOID)(addr + 0x8), &addr, sizeof(addr), NULL) && addr != 0) {
            if (ReadProcessMemory(GetCurrentProcess(), (LPCVOID)(addr + 0x8), &addr, sizeof(addr), NULL) && addr != 0) {
                if (ReadProcessMemory(GetCurrentProcess(), (LPCVOID)(addr + 0x10), &addr, sizeof(addr), NULL) && addr != 0) {
                    if (ReadProcessMemory(GetCurrentProcess(), (LPCVOID)(addr + 0x628), &addr, sizeof(addr), NULL) && addr != 0) {

                        // ПРИМЕНЯЕМ ОТРИЦАТЕЛЬНЫЙ ОФФСЕТ
                        uintptr_t finalTimeAddr = addr - 0x4D0954;
                        ReadProcessMemory(GetCurrentProcess(), (LPCVOID)finalTimeAddr, &gameTime, sizeof(gameTime), NULL);
                    }
                }
            }
        }
    }


    // Читаем ID и имя машины
    uintptr_t carStructAddr = SafeReadIntChain(carIdBase, carIdOffsets, carIdOffsetsCount);
    int currentCarID = 0;
    if (carStructAddr != 0) {
        uintptr_t finalCarAddr = carStructAddr - 0x160A4;
        ReadProcessMemory(GetCurrentProcess(), (LPCVOID)finalCarAddr, &currentCarID, sizeof(currentCarID), NULL);
    }
    std::string carName = GetCarNameByID(currentCarID);

    float fHours = (float)gameTime / 3600.0f;
    int hours = (int)fHours % 24;

    std::string timeOfDayText = "";
    std::string timeImageKey = "512512";

    // 2. Распределяем по сезонам
    if (hours >= 6 && hours < 12)       timeOfDayText = isRussian ? "Утро" : "Morning";
    else if (hours >= 12 && hours < 18) timeOfDayText = isRussian ? "День" : "Day";
    else if (hours >= 18 && hours < 24) timeOfDayText = isRussian ? "Вечер" : "Evening";
    else                                timeOfDayText = isRussian ? "Ночь" : "Night";

    // Собираем структуру Discord Presence
    DiscordRichPresence discordPresence;
    memset(&discordPresence, 0, sizeof(discordPresence));

    switch (lobbyStatus) {
    case 1:  discordPresence.state = isRussian ? "В одиночной игре" : "Singleplayer"; break;
    case 2:  discordPresence.state = isRussian ? "В закрытом лобби" : "In Private Lobby"; break;
    case 3:  discordPresence.state = isRussian ? "Только для друзей" : "Friends Only"; break;
    case 4:  discordPresence.state = isRussian ? "В мультиплеере" : "In Multiplayer"; break;
    default: discordPresence.state = isRussian ? "Запуск..." : "Startup..."; break;
    }

    discordPresence.largeImageKey = timeImageKey.c_str();
    discordPresence.startTimestamp = (int64_t)g_StartTime;

    // Рассчитываем уровень погони
    int heatLevel = 0;
    if (rawHeat >= 0.1f) {
        if (rawHeat < 2.0f)       heatLevel = 1;
        else if (rawHeat < 8.0f)   heatLevel = 2;
        else if (rawHeat < 16.0f)  heatLevel = 3;
        else if (rawHeat < 26.0f)  heatLevel = 4;
        else if (rawHeat < 38.0f)  heatLevel = 5;
        else                       heatLevel = 6;
    }

    static std::string staticDetails;
    static std::string staticLargeText;

    // Настраиваем вывод основных строк и ТЕКСТА КАРТИНКИ
    if (lobbyStatus == 1) { // Одиночная игра
        if (heatLevel > 0) {
            staticDetails = isRussian ? "В погоне (HEAT: " + std::to_string(heatLevel) + ") на " + carName
                : "In Pursuit (HEAT: " + std::to_string(heatLevel) + ") in " + carName;
        }
        else {
            staticDetails = isRussian ? "Катается по городу на " + carName
                : "Cruising Fairhaven in " + carName;
        }
        // Для сингла при наведении пишем: "Fairhaven City - Утро"
        staticLargeText = isRussian ? "Fairhaven City - " + timeOfDayText : "Fairhaven City - " + timeOfDayText;
    }
    else if (lobbyStatus > 1) { // Онлайн-сессии
        staticDetails = isRussian ? "Покоряет онлайн-сессию на " + carName
            : "Dominating the online session in " + carName;
        // Для онлайна при наведении пишем: "В сетевой сессии - Утро"
        staticLargeText = isRussian ? "В сетевой сессии - " + timeOfDayText : "In Online Session - " + timeOfDayText;
    }

    discordPresence.details = staticDetails.c_str();
    discordPresence.largeImageText = staticLargeText.c_str();

    // Ограничение флуда в Дискорд
    static int lastHeat = -1;
    static int lastLobby = -1;
    static std::string lastCar = "";
    static std::string lastTimeOfDay = "";

    if (heatLevel != lastHeat || lobbyStatus != lastLobby || carName != lastCar || timeOfDayText != lastTimeOfDay) {

        Discord_UpdatePresence(&discordPresence);

        lastHeat = heatLevel;
        lastLobby = lobbyStatus;
        lastCar = carName;
        lastTimeOfDay = timeOfDayText;
    }
}

DWORD WINAPI MainThread(LPVOID lpParam) {
    InitDiscord();
    g_StartTime = time(NULL);

    while (true) {
        UpdatePresence();
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        CreateThread(NULL, 0, MainThread, NULL, 0, NULL);
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
        Discord_Shutdown();
    }
    return TRUE;
}
