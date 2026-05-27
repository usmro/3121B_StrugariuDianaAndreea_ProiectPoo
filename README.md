# Cinema Lumière

Sistem de rezervări de bilete la cinematograf cu interfață grafică Qt, realizat în C++.

## Funcționalități

- Autentificare și înregistrare utilizatori
- Detecție automată a locației prin IP
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

## Rulare rapidă (executabil precompilat Windows)

Descarcă `CinemaLumiere-v1.1.zip` din **Releases**, dezarhivează și rulează `CinemaBookingSystem.exe`. Nu necesită instalarea Qt sau Visual Studio.

## Build pe Windows (Visual Studio 2022 + Qt 6.7.3)

1. Clonează repository-ul
2. Deschide `CinemaBookingSystem.sln` în Visual Studio 2022
3. Click dreapta pe proiect → **Set as Startup Project**
4. `Ctrl+Shift+B` pentru build
5. `F5` pentru rulare

Qt-ul și DLL-urile necesare sunt copiate automat în folderul de output prin post-build event.

## Build pe Linux (Ubuntu)

Pași de la zero pe Ubuntu proaspăt instalat:

**1. Activează repository-urile universe și actualizează:**
```bash
sudo add-apt-repository universe
sudo apt-get update
```

**2. Instalează dependențele:**
```bash
sudo apt-get install -y \
    qt6-base-dev \
    qt6-base-dev-tools \
    libqt6svg6-dev \
    cmake \
    build-essential \
    ninja-build \
    libgl-dev \
    libglu1-mesa-dev \
    libxkbcommon-dev \
    libxkbcommon-x11-0 \
    libxcb-cursor0 \
    libxcb-icccm4 \
    libxcb-image0 \
    libxcb-keysyms1 \
    libxcb-render-util0 \
    libxcb-shape0 \
    libxcb-xinerama0 \
    libfontconfig1 \
    libfreetype6 \
    fonts-noto-color-emoji
```

**3. Clonează și compilează:**
```bash
git clone https://github.com/usmro/3121B_StrugariuDianaAndreea_ProiectPoo.git
cd 3121B_StrugariuDianaAndreea_ProiectPoo
cmake -B build && cmake --build build
```

**4. Rulează:**
```bash
cd build
./CinemaBookingSystem
```

> **Notă:** `fonts-noto-color-emoji` este necesar pentru afișarea corectă a emoji-urilor (stele, food, badge-uri). Fără el unele simboluri apar ca dreptunghiuri goale.

> **Notă:** Dacă detecția automată a locației nu funcționează (rețele de campus pot bloca traficul HTTP), aplicația folosește Suceava ca locație implicită și funcționează normal.

## Teste unitare (fără Qt, fără biblioteci externe)

```bash
g++ -std=c++20 -I include tests/test_main.cpp -o tests/run_tests
./tests/run_tests
```

## Versiunea Consolă (fără Qt)

Branch-ul `console` conține o versiune completă a aplicației care rulează în terminal, fără nicio dependență Qt.

```bash
git checkout console
g++ -std=c++20 -I include src/main_console.cpp -o cinema
./cinema
```

Funcționalități disponibile în versiunea consolă:
- Autentificare și înregistrare utilizatori
- Browse filme cu prețuri și rating
- Rezervare completă cu group booking și food ordering
- Hartă sală ASCII interactivă
- Evaluări filme
- Panou admin
- Toate regulile de prețuri (weekend, tier, tip loc)

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
