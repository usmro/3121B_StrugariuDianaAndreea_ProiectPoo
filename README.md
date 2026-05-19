# Cinema Booking System

Sistem de rezervări de bilete la cinematograf cu interfață grafică Qt, realizat în C++.

## Funcționalități

- Autentificare și înregistrare utilizatori
- Rezervare bilete (alegere film, sală, dată, oră, loc)
- Pagină de detalii film cu postere automate din TMDB
- Browse filme după gen cu filtre 3D și status
- Hartă sală interactivă cu vizualizare locuri libere/ocupate
- Anulare rezervări
- Profil utilizator cu istoric rezervări
- Panou admin (gestionare filme, ore, utilizatori, rapoarte vânzări)
- Confirmare rezervare cu referință unică de booking
- Tranziții animate între pagini

## Structura proiectului

```
CinemaBookingSystem/
├── CinemaBookingSystem.sln      ← deschide în Visual Studio
├── CinemaBookingSystem.vcxproj
├── CMakeLists.txt               ← pentru Linux / GitHub Actions
├── src/                         ← fișiere sursă .cpp
│   ├── main.cpp
│   ├── mainwindow_builders.cpp
│   └── mainwindow_slots.cpp
├── include/                     ← headere .h
│   ├── mainwindow.h
│   ├── Cinema.h
│   ├── CinemaModels.h
│   └── config.h                 ← cheia API TMDB se adaugă aici
├── data/                        ← fișiere de date
│   └── filme.txt
└── .github/workflows/
    └── build.yml                ← CI/CD GitHub Actions
```

## Rulare pe Windows (Visual Studio 2022)

1. Clonează repository-ul
2. Deschide `CinemaBookingSystem.sln` în Visual Studio 2022
3. Setează `CinemaBookingSystem` ca startup project (click dreapta → Set as Startup Project)
4. Build → Build Solution (`Ctrl+Shift+B`)
5. Run (`F5`)

> Qt-ul și DLL-urile necesare sunt copiate automat la build prin post-build event.

## Rulare pe Linux (Ubuntu)

```bash
sudo apt install qt6-base-dev qt6-base-dev-tools cmake build-essential libgl-dev
cmake -B build && cmake --build build
./build/CinemaBookingSystem
```

## Postere filme (opțional)

Aplicația poate încărca automat posterele filmelor din [TMDB](https://www.themoviedb.org/):

1. Creează un cont gratuit pe themoviedb.org
2. Mergi la **Settings → API** și generează o cheie API (v3)
3. Deschide `include/config.h` și înlocuiește:
```cpp
#define TMDB_API_KEY "cheia_ta_aici"
```
4. Rebuild

## Credențiale implicite

| Username | Parolă | Rol   |
|----------|--------|-------|
| admin    | admin  | Admin |
| Diana    | 1234   | User  |
