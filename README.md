# Cinema Booking System

Sistem de rezervări de bilete la cinematograf cu interfață grafică Qt, realizat în C++.

## Funcționalități

- Autentificare și înregistrare utilizatori
- Detecție automată a locației prin IP — afișează cinematografele din orașul tău
- Rețea de 24 de cinematografe în toată România, organizate pe regiuni
- Sistem de programări automat cu reguli de gen și ore realiste
- Rezervare în grup (până la 8 locuri simultan)
- Comandă food & drinks opțională la rezervare
- Evaluări filme cu actualizare rating global
- Export rezervare în calendar (.ics)
- Prețuri dinamice (weekend, tip loc, tier locație)
- Harta sălii interactivă cu preferință de loc memorată
- Browse filme după gen cu filtre 3D și status
- Panou admin cu statistici în timp real
- Postere automate din TMDB (opțional)
- 16 teste unitare

## Structura proiectului

```
CinemaBookingSystem/
├── CinemaBookingSystem.sln      ← deschide în Visual Studio
├── CinemaBookingSystem.vcxproj
├── CMakeLists.txt               ← pentru Linux / g++
├── src/
│   ├── main.cpp
│   ├── mainwindow_builders.cpp
│   └── mainwindow_slots.cpp
├── include/
│   ├── mainwindow.h
│   ├── Cinema.h
│   ├── CinemaModels.h
│   └── config.h
├── data/
│   ├── filme.txt
│   └── bilete/
├── tests/
│   └── test_main.cpp
└── DOCUMENTATIE.md
```

## Rulare rapidă (executabil precompilat)

Descarcă `CinemaBookingSystem-v1.0.zip` din **Releases**, dezarhivează și rulează `CinemaBookingSystem.exe`. Nu necesită instalarea Qt sau Visual Studio.

## Build pe Windows (Visual Studio 2022 + Qt 6.7.3)

1. Clonează repository-ul
2. Deschide `CinemaBookingSystem.sln` în Visual Studio 2022
3. Click dreapta pe proiect → **Set as Startup Project**
4. `Ctrl+Shift+B` pentru build
5. `F5` pentru rulare

Qt-ul și DLL-urile necesare sunt copiate automat în folderul de output prin post-build event.

## Build pe Linux (g++ / CMake)

```bash
sudo apt install qt6-base-dev cmake build-essential libgl-dev
cmake -B build && cmake --build build
./build/CinemaBookingSystem
```

## Teste unitare (fără Qt, fără biblioteci externe)

```bash
g++ -std=c++20 -I include tests/test_main.cpp -o tests/run_tests
./tests/run_tests
```

## Postere filme (opțional)

Aplicația poate încărca automat posterele din [TMDB](https://www.themoviedb.org/):

1. Creează cont gratuit pe themoviedb.org
2. **Settings → API** → generează cheie API (v3)
3. În `include/config.h` înlocuiește:
```cpp
#define TMDB_API_KEY "cheia_ta_aici"
```
4. Rebuild

## Credențiale implicite

| Username | Parolă   | Rol   |
|----------|----------|-------|
| admin    | admin123 | Admin |
