#ifndef CINEMAMODELS_H
#define CINEMAMODELS_H

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>
#include <ctime>

// ============================================================
//  CATEGORII LOC SI PRET
// ============================================================
enum class CategorieLocSeat { STANDARD, VIP, CUPLU };

inline std::string numeCategorie(CategorieLocSeat c) {
    switch (c) {
    case CategorieLocSeat::VIP:    return "VIP";
    case CategorieLocSeat::CUPLU:  return "CUPLU";
    default:                       return "STANDARD";
    }
}

inline double multiplicatorCategorie(CategorieLocSeat c) {
    switch (c) {
    case CategorieLocSeat::VIP:    return 1.5;
    case CategorieLocSeat::CUPLU:  return 1.8;
    default:                       return 1.0;
    }
}

// ============================================================
//  FILM
// ============================================================
class Film {
private:
    std::string titlu;
    bool este3D;
    double pretBaza;
    std::string gen;       // Actiune, Drama, Comedie etc.
    int durata;            // in minute
public:
    Film(std::string t, bool e3d, double p, std::string g = "Necunoscut", int d = 0)
        : titlu(t), este3D(e3d), pretBaza(p), gen(g), durata(d) {
    }
    std::string getTitlu()    const { return titlu; }
    bool        getEste3D()   const { return este3D; }
    double      getPretBaza() const { return pretBaza; }
    std::string getGen()      const { return gen; }
    int         getDurata()   const { return durata; }
};

// ============================================================
//  SALA  (cu categorii pe locuri)
// ============================================================
class Sala {
private:
    int id;
    std::string nume;
    int randuri;
    int coloane;
    // Matricea de ocupare per ora
    std::map<std::string, std::vector<std::vector<int>>> ocuparePeOre;
    // Categorii locuri: acelasi grid, static (nu depinde de ora)
    std::vector<std::vector<CategorieLocSeat>> categorii;

    void initCategoriiImplicit() {
        categorii.assign(randuri, std::vector<CategorieLocSeat>(coloane, CategorieLocSeat::STANDARD));
        // Ultimul rand = VIP
        if (randuri > 0)
            for (int j = 0; j < coloane; ++j)
                categorii[randuri - 1][j] = CategorieLocSeat::VIP;
        // Primul rand, ultimele 2 locuri din mijloc = CUPLU (daca sala e suficient de lata)
        if (coloane >= 4) {
            int mid = coloane / 2;
            categorii[0][mid - 1] = CategorieLocSeat::CUPLU;
            categorii[0][mid] = CategorieLocSeat::CUPLU;
        }
    }

public:
    Sala(int _id, std::string _nume, int _r, int _c)
        : id(_id), nume(_nume), randuri(_r), coloane(_c) {
        initCategoriiImplicit();
    }

    int         getId()      const { return id; }
    std::string getNume()    const { return nume; }
    int         getRanduri() const { return randuri; }
    int         getColoane() const { return coloane; }

    CategorieLocSeat getCategorieLocSeat(int r, int l) const {
        if (r >= 0 && r < randuri && l >= 0 && l < coloane)
            return categorii[r][l];
        return CategorieLocSeat::STANDARD;
    }

    void initializareOra(const std::string& ora) {
        if (ocuparePeOre.find(ora) == ocuparePeOre.end())
            ocuparePeOre[ora] = std::vector<std::vector<int>>(randuri, std::vector<int>(coloane, 0));
    }

    bool esteLocOcupat(const std::string& ora, int r, int l) {
        initializareOra(ora);
        if (r >= 0 && r < randuri && l >= 0 && l < coloane)
            return ocuparePeOre[ora][r][l] == 1;
        return true;
    }

    void ocupaLoc(const std::string& ora, int r, int l) {
        initializareOra(ora);
        if (r >= 0 && r < randuri && l >= 0 && l < coloane)
            ocuparePeOre[ora][r][l] = 1;
    }

    void elibereazaLoc(const std::string& ora, int r, int l) {
        if (ocuparePeOre.count(ora))
            if (r >= 0 && r < randuri && l >= 0 && l < coloane)
                ocuparePeOre[ora][r][l] = 0;
    }

    int locuriLibere(const std::string& ora) {
        initializareOra(ora);
        int cnt = 0;
        for (int r = 0; r < randuri; ++r)
            for (int l = 0; l < coloane; ++l)
                if (ocuparePeOre[ora][r][l] == 0) ++cnt;
        return cnt;
    }

    void afiseazaHarta(const std::string& ora) {
        initializareOra(ora);
        std::cout << "\n  ========= " << nume << " | ORA: " << ora << " =========\n\n";
        std::cout << "       ";
        for (int j = 1; j <= coloane; ++j)
            std::cout << std::setw(3) << j;
        std::cout << "\n       " << std::string(coloane * 3 + 2, '-') << "\n";

        for (int i = 0; i < randuri; ++i) {
            std::cout << "  R" << std::setw(2) << i + 1 << " |";
            for (int j = 0; j < coloane; ++j) {
                if (ocuparePeOre[ora][i][j] == 1) {
                    std::cout << "  X";
                }
                else {
                    switch (categorii[i][j]) {
                    case CategorieLocSeat::VIP:   std::cout << "  V"; break;
                    case CategorieLocSeat::CUPLU: std::cout << "  C"; break;
                    default:                      std::cout << "  O"; break;
                    }
                }
            }
            std::cout << " |\n";
        }
        std::cout << "       " << std::string(coloane * 3 + 2, '-') << "\n";
        std::cout << "  Legenda: O=Standard liber  V=VIP liber  C=Cuplu liber  X=Ocupat\n";
        std::cout << "  Pret:    Standard x1.0     VIP x1.5     Cuplu x1.8\n\n";
    }
};

// ============================================================
//  REZERVARE (baza) + ONLINE
// ============================================================
class Rezervare {
protected:
    Film        film;
    int         rand;
    int         loc;
    double      pretFinal;
    std::string ora;
    std::string salaNume;
    std::string categorieLoc;
public:
    Rezervare(Film f, int r, int l, double p, const std::string& o,
        const std::string& sn, const std::string& cat)
        : film(f), rand(r), loc(l), pretFinal(p), ora(o), salaNume(sn), categorieLoc(cat) {
    }

    virtual void afiseazaDetalii() const {
        std::cout << "Film: " << film.getTitlu()
            << " | Sala: " << salaNume
            << " | Ora: " << ora
            << " | Loc: R" << rand + 1 << " L" << loc + 1
            << " [" << categorieLoc << "]"
            << " | Pret: " << std::fixed << std::setprecision(2) << pretFinal << " RON\n";
    }

    std::string getSalaNume()     const { return salaNume; }
    std::string getOra()          const { return ora; }
    int         getRand()         const { return rand; }
    int         getLoc()          const { return loc; }
    std::string getTitluFilm()    const { return film.getTitlu(); }

    virtual ~Rezervare() {}
};

class RezervareOnline : public Rezervare {
private:
    std::string email;
public:
    RezervareOnline(Film f, int r, int l, double p, const std::string& o,
        const std::string& sn, const std::string& cat, const std::string& e)
        : Rezervare(f, r, l, p, o, sn, cat), email(e) {
    }

    void afiseazaDetalii() const override {
        std::cout << "[ONLINE] ";
        Rezervare::afiseazaDetalii();
        std::cout << "         Confirmare: " << email << "\n";
    }
    std::string getEmail() const { return email; }
};

// ============================================================
//  CONT UTILIZATOR
// ============================================================
struct ContUtilizator {
    std::string username;
    std::string parola;     // stocat ca plain text (scop educational)
    bool        esteAdmin;
    std::vector<std::string> istoricRezervari; // descriptii simple
};

// ============================================================
//  INTERFATA SERVICE
// ============================================================
class ICinemaService {
public:
    virtual void adaugaFilm(Film f) = 0;
    virtual void adaugaSala(Sala s) = 0;
    virtual ~ICinemaService() {}
};

#endif