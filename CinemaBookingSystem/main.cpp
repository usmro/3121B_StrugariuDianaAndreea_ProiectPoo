#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "CinemaModels.h"

#ifdef _WIN32
#include <conio.h>
#define CLEAR "cls"
#else
#include <termios.h>
#include <unistd.h>
#define CLEAR "clear"
int _getch() {
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}
#endif

class Cinematograf : public ICinemaService {
private:
    std::vector<Film> filme;
    std::vector<Sala> sali;
    std::vector<Rezervare*> istoriculRezervarilor;

public:
    void adaugaFilm(Film f) override { filme.push_back(f); }
    void adaugaSala(Sala s) override { sali.push_back(s); }
    const std::vector<Film>& getToateFilmele() const { return filme; }

    // NOU: Pentru a putea selecta sala din main
    Sala* getSalaById(int id) {
        for (auto& s : sali) if (s.getId() == id) return &s;
        return nullptr;
    }

    void incarcaFilmeDinFisier(std::string numeFisier) {
        std::ifstream fisier(numeFisier);
        if (!fisier.is_open()) return;
        std::string linie;
        while (std::getline(fisier, linie)) {
            if (linie.empty()) continue;
            std::stringstream ss(linie);
            std::string titlu, s_este3D, s_pret;
            if (std::getline(ss, titlu, ',') && std::getline(ss, s_este3D, ',') && std::getline(ss, s_pret, ',')) {
                try {
                    bool este3D = (s_este3D == "1");
                    double pret = std::stod(s_pret);
                    filme.push_back(Film(titlu, este3D, pret));
                }
                catch (...) { continue; }
            }
        }
        fisier.close();
    }

    void afiseazaFilme() override {
        for (const auto& f : filme) {
            std::cout << "- " << f.getTitlu() << " (" << (f.getEste3D() ? "3D" : "2D") << ")\n";
        }
    }

    void afiseazaLocuri(int idSala) override {
        for (const auto& s : sali) {
            if (s.getId() == idSala) {
                s.afiseazaHarta();
                return;
            }
        }
    }

    // Cerinta A: Salvarea in fisier text
    void salveazaRezervareInFisier(std::string titlu, std::string sala, int r, int l, double pret, std::string email) {
        std::ofstream f("toate_rezervarile.txt", std::ios::app);
        if (f.is_open()) {
            f << "Film: " << titlu << " | Sala: " << sala << " | Loc: R" << r << " L" << l
                << " | Pret: " << pret << " RON | Client: " << (email.empty() ? "Offline" : email) << "\n";
            f.close();
        }
    }

    void realizeazaRezervare(int idSala, int r, int l, std::string titluFilm, std::string email) override {
        // Logica este apelata acum dupa calculul de pret din main
    }

    // Versiune extinsa pentru cerintele noi
    void proceseazaRezervareCompleta(int idSala, int r, int l, Film* film, std::string email, double multiplicator) {
        try {
            Sala* s = getSalaById(idSala);
            if (!s) return;

            s->ocupaLoc(r, l);
            double pretFinal = (film->getPretBaza() + (film->getEste3D() ? 10.0 : 0.0)) * multiplicator;

            Rezervare* rez;
            if (!email.empty()) rez = new RezervareOnline(*film, r, l, pretFinal, email);
            else rez = new Rezervare(*film, r, l, pretFinal);

            istoriculRezervarilor.push_back(rez);

            // Salvare (Cerinta A)
            salveazaRezervareInFisier(film->getTitlu(), s->getNume(), r + 1, l + 1, pretFinal, email);

            std::cout << "\nREZERVARE REUSITA!\n";
            rez->afiseazaDetalii();
        }
        catch (const std::exception& e) {
            std::cerr << "\nEROARE: " << e.what() << "\n";
        }
    }

    ~Cinematograf() {
        for (auto r : istoriculRezervarilor) delete r;
    }
};

// Functiile tale de cautare live raman neschimbate
bool contineSubsir(std::string text, std::string cautare) {
    if (cautare.empty()) return true;
    std::transform(text.begin(), text.end(), text.begin(), ::tolower);
    std::transform(cautare.begin(), cautare.end(), cautare.begin(), ::tolower);
    return text.find(cautare) != std::string::npos;
}

Film* meniuCautareLive(Cinematograf& cinema) {
    std::string input = "";
    int indexSelectat = 0;
    const int COLOANE = 3;
    const int LATIME_COLOANA = 30;
    const int MAX_FILME_ECRAN = 15;

    while (true) {
        system(CLEAR);
        std::cout << "=== CAUTARE FILM (SAGETI + SCRIERE) ===\n";
        std::cout << "Cautare: " << input << "_\n";
        std::cout << "--------------------------------------------------------------------------\n";

        std::vector<Film*> rezultate;
        for (const auto& f : cinema.getToateFilmele()) {
            if (contineSubsir(f.getTitlu(), input)) {
                rezultate.push_back(const_cast<Film*>(&f));
            }
        }

        if (rezultate.empty()) {
            std::cout << "      (Niciun rezultat gasit)\n";
        }
        else {
            if (indexSelectat >= (int)rezultate.size()) indexSelectat = 0;
            int startAfisare = (indexSelectat / MAX_FILME_ECRAN) * MAX_FILME_ECRAN;
            int limita = std::min(startAfisare + MAX_FILME_ECRAN, (int)rezultate.size());

            for (int i = startAfisare; i < limita; ++i) {
                std::string prefix = (i == indexSelectat) ? "[>] " : "    ";
                std::string nume = rezultate[i]->getTitlu();
                if (nume.length() > LATIME_COLOANA - 5) nume = nume.substr(0, LATIME_COLOANA - 8) + "...";
                std::cout << std::left << std::setw(LATIME_COLOANA) << (prefix + nume);
                if ((i - startAfisare + 1) % COLOANE == 0) std::cout << "\n";
            }
            std::cout << "\n\n[Pagina " << (startAfisare / MAX_FILME_ECRAN) + 1 << " din " << ((int)rezultate.size() - 1) / MAX_FILME_ECRAN + 1 << "]";
        }

        int tasta = _getch();
        if (tasta == 224 || tasta == 0) {
            tasta = _getch();
            if (tasta == 72 && indexSelectat >= COLOANE) indexSelectat -= COLOANE;
            else if (tasta == 80 && indexSelectat + COLOANE < (int)rezultate.size()) indexSelectat += COLOANE;
            else if (tasta == 75 && indexSelectat > 0) indexSelectat--;
            else if (tasta == 77 && indexSelectat < (int)rezultate.size() - 1) indexSelectat++;
        }
        else if (tasta == 27) return nullptr;
        else if (tasta == 13 || tasta == 10) return rezultate.empty() ? nullptr : rezultate[indexSelectat];
        else if (tasta == 8) { if (!input.empty()) input.pop_back(); indexSelectat = 0; }
        else if (isprint(tasta)) { input += (char)tasta; indexSelectat = 0; }
    }
}

int meniuInteractiv(std::vector<std::string> optiuni, std::string titlu) {
    int selectie = 0;
    while (true) {
        system(CLEAR);
        std::cout << "=== " << titlu << " ===\n";
        for (int i = 0; i < (int)optiuni.size(); ++i) {
            if (i == selectie) std::cout << "  [>] " << optiuni[i] << " <--\n";
            else std::cout << "      " << optiuni[i] << "\n";
        }
        int tasta = _getch();
        if (tasta == 224 || tasta == 0) {
            tasta = _getch();
            if (tasta == 72 && selectie > 0) selectie--;
            if (tasta == 80 && selectie < (int)optiuni.size() - 1) selectie++;
        }
        else if (tasta == 13 || tasta == 10) return selectie;
    }
}

int main() {
    Cinematograf cinema;
    cinema.incarcaFilmeDinFisier("filme.txt");

    // Cerinta B: Adaugam sali multiple cu configuratii diferite
    cinema.adaugaSala(Sala(1, "SALA 2D STANDARD", 5, 10));
    cinema.adaugaSala(Sala(2, "SALA VIP - DOLBY", 3, 6));

    std::vector<std::string> optiuniMeniu = { "Rezervare Noua", "Vezi Harta Sali", "Iesire" };

    while (true) {
        int alegere = meniuInteractiv(optiuniMeniu, "SISTEM REZERVARI CINEMA");
        if (alegere == 0) {
            Film* f = meniuCautareLive(cinema);
            if (f) {
                // Cerinta B: Alegere Sala
                int idSalaAleasa = meniuInteractiv({ "Sala 1 (Standard)", "Sala 2 (VIP)" }, "ALEGE SALA") + 1;
                Sala* sPtr = cinema.getSalaById(idSalaAleasa);

                // Cerinta C: Alegere Categorie (Multiplicator pret)
                int cat = meniuInteractiv({ "Adult (Pret Intreg)", "Student (-25%)", "Pensionar (-50%)" }, "CATEGORIE CLIENT");
                double mult = (cat == 1) ? 0.75 : (cat == 2 ? 0.5 : 1.0);

                int r = -1, l = -1;
                while (true) {
                    system(CLEAR);
                    std::cout << "Film: " << f->getTitlu() << " | Sala: " << sPtr->getNume() << "\n\n";
                    cinema.afiseazaLocuri(idSalaAleasa);
                    std::cout << "\nRand (1-" << sPtr->getRanduri() << "): ";
                    if (!(std::cin >> r) || r < 1 || r > sPtr->getRanduri()) {
                        std::cout << "\nEROARE: Rand invalid!"; std::cin.clear(); std::cin.ignore(1000, '\n'); _getch(); continue;
                    }
                    std::cout << "Loc (1-" << sPtr->getColoane() << "): ";
                    if (!(std::cin >> l) || l < 1 || l > sPtr->getColoane()) {
                        std::cout << "\nEROARE: Loc invalid!"; std::cin.clear(); std::cin.ignore(1000, '\n'); _getch(); continue;
                    }
                    break;
                }
                std::string email;
                std::cout << "Email (Enter pentru skip): ";
                std::cin.ignore();
                std::getline(std::cin, email);

                // Procesare cu noile date
                cinema.proceseazaRezervareCompleta(idSalaAleasa, r - 1, l - 1, f, email, mult);

                std::cout << "\nApasati orice tasta pentru a continua...";
                _getch();
            }
        }
        else if (alegere == 1) {
            int idS = meniuInteractiv({ "Harta Sala 1", "Harta Sala 2" }, "SELECTEAZA SALA PENTRU VIZUALIZARE") + 1;
            system(CLEAR);
            cinema.afiseazaLocuri(idS);
            std::cout << "\nApasati orice tasta pentru a continua...";
            _getch();
        }
        else if (alegere == 2) break;
    }
    return 0;
}