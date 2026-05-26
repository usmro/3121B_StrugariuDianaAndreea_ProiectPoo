#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <limits>
#include <iomanip>
#include <algorithm>
#include "Cinema.h"

static Cinematograf cinema;

void clearScreen(){ std::cout << "\033[2J\033[1;1H"; }

void pauseMenu(){
    std::cout << "\n  Apasa Enter pentru a continua...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
    std::cin.get();
}

void printHeader(const std::string& titlu){
    clearScreen();
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════╗\n";
    std::cout << "  ║          CINEMA BOOKING SYSTEM           ║\n";
    std::cout << "  ╠══════════════════════════════════════════╣\n";
    std::cout << "  ║  " << std::left << std::setw(41) << titlu << "║\n";
    std::cout << "  ╚══════════════════════════════════════════╝\n\n";
}

void printSeparator(){ std::cout << "  ────────────────────────────────────────────\n"; }

std::string inputStr(const std::string& prompt){
    std::string s;
    std::cout << "  " << prompt;
    std::getline(std::cin, s);
    return s;
}

int inputInt(const std::string& prompt, int min, int max){
    int v;
    while(true){
        std::cout << "  " << prompt;
        if(std::cin >> v && v >= min && v <= max){
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
            return v;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
        std::cout << "  Valoare invalida. Introdu un numar intre " << min << " si " << max << ".\n";
    }
}

void afiseazaHartaSala(Sala* sala, const Data& d, const std::string& ora,
                        const std::vector<std::pair<int,int>>& selectate = {}){
    if(!sala) return;
    std::cout << "\n  Sala: " << sala->getNume()
              << " | " << d.toString() << " | " << ora << "\n\n";
    std::cout << "       ";
    for(int j=1;j<=sala->getColoane();j++) std::cout << std::setw(3) << j;
    std::cout << "\n       " << std::string(sala->getColoane()*3+2,'-') << "\n";

    for(int i=0;i<sala->getRanduri();i++){
        std::cout << "  R" << std::setw(2) << i+1 << " |";
        for(int j=0;j<sala->getColoane();j++){
            bool sel=false;
            for(auto&[r,c]:selectate) if(r==i&&c==j){sel=true;break;}
            if(sel){ std::cout << " [*"; continue; }
            if(sala->esteOcupat(d,ora,i,j)){ std::cout << "  X"; continue; }
            switch(sala->getCategorieLocSeat(i,j)){
                case CategorieLocSeat::VIP:    std::cout << "  V"; break;
                case CategorieLocSeat::CUPLU:  std::cout << "  C"; break;
                default:                       std::cout << "  O"; break;
            }
        }
        std::cout << " |\n";
    }
    std::cout << "       " << std::string(sala->getColoane()*3+2,'-') << "\n";
    std::cout << "\n  Legenda: O=Standard  V=VIP  C=Cuplu  X=Ocupat  [*]=Selectat\n";
}

void menuLogin(){
    while(true){
        printHeader("Autentificare");
        std::cout << "  [1] Logare\n";
        std::cout << "  [2] Inregistrare\n";
        std::cout << "  [0] Iesire\n\n";
        int opt = inputInt("Optiunea ta: ", 0, 2);
        if(opt == 0) { std::cout << "\n  La revedere!\n\n"; exit(0); }

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
        std::string usr = inputStr("Username: ");
        std::string par = inputStr("Parola: ");

        if(opt == 1){
            if(cinema.login(usr, par)){
                std::cout << "\n  Bun venit, " << usr << "!\n";
                pauseMenu(); return;
            } else std::cout << "\n  Username sau parola incorecta!\n";
        } else {
            if(cinema.inregistrare(usr, par))
                std::cout << "\n  Cont creat! Bun venit, " << usr << "!\n";
            else std::cout << "\n  Username deja existent!\n";
            pauseMenu();
        }
    }
}

void menuAlegeLocatie(){
    printHeader("Alege Locatia");
    auto& locatii = cinema.getLocatii();
    std::cout << "  Locatii disponibile:\n\n";
    int i=1;
    for(auto& l : locatii){
        std::string tier = l.tier==CinemaTier::PREMIUM?"[PREMIUM]":
                           l.tier==CinemaTier::BASIC?"[BASIC]":"[STANDARD]";
        std::cout << "  [" << std::setw(2) << i++ << "] " << l.oras
                  << " - " << l.nume << " " << tier << "\n";
    }
    std::cout << "\n";
    int opt = inputInt("Alege locatia (1-"+std::to_string(locatii.size())+"): ", 1, (int)locatii.size());
    cinema.setLocatieCurenta(locatii[opt-1].id);
    cinema.incarcaRezervari();
    cinema.finalizeazaLocatie();
    std::cout << "\n  Locatie selectata: " << locatii[opt-1].nume << "\n";
    pauseMenu();
}

void menuBrowseFilme(){
    printHeader("Browse Filme");
    auto filme = cinema.getFilmePentruLocatie();
    if(filme.empty()){ std::cout << "  Niciun film disponibil.\n"; pauseMenu(); return; }
    std::cout << "  " << std::left << std::setw(4) << "#"
              << std::setw(35) << "Titlu"
              << std::setw(12) << "Gen"
              << std::setw(6) << "Durata"
              << std::setw(6) << "Pret"
              << "Rating\n";
    printSeparator();
    int i=1;
    for(auto* f : filme){
        std::cout << "  " << std::setw(4) << i++
                  << std::setw(35) << f->getTitlu().substr(0,34)
                  << std::setw(12) << f->getGen().substr(0,11)
                  << std::setw(6) << f->getDurata()
                  << std::setw(6) << (int)f->getPretBaza()
                  << (f->getRating()>0 ? std::to_string(f->getRating()).substr(0,3)+"/10" : "—")
                  << (f->getEste3D()?" [3D]":"")
                  << (f->getStatus()==StatusFilm::IN_CURAND?" [IN CURAND]":"")
                  << "\n";
    }
    pauseMenu();
}

void menuRezervare(){
    printHeader("Rezervare Bilet");
    auto* loc = cinema.getLocatieCurenta();
    if(!loc){ std::cout << "  Alege mai intai o locatie!\n"; pauseMenu(); return; }

    auto filme = cinema.getFilmePentruLocatie();
    std::vector<Film*> active;
    for(auto* f:filme) if(f->getStatus()==StatusFilm::RULAZA_ACUM) active.push_back(f);
    if(active.empty()){ std::cout << "  Niciun film disponibil.\n"; pauseMenu(); return; }

    std::cout << "  Filme disponibile:\n\n";
    int i=1;
    for(auto* f:active)
        std::cout << "  [" << i++ << "] " << f->getTitlu()
                  << (f->getEste3D()?" [3D]":"") << "\n";
    std::cout << "\n";
    int fOpt = inputInt("Alege filmul: ", 1, (int)active.size());
    Film* film = active[fOpt-1];

    auto zile = urmatoroarele7Zile();
    std::cout << "\n  Zile disponibile:\n\n";
    i=1;
    for(auto& z:zile)
        std::cout << "  [" << i++ << "] " << numeZiSaptamana(z) << " " << z.toString() << "\n";
    std::cout << "\n";
    int dOpt = inputInt("Alege data: ", 1, (int)zile.size());
    Data data = zile[dOpt-1];

    auto programari = cinema.getProgrameFilm(film->getTitlu(), data);
    if(programari.empty()){
        std::cout << "\n  Nicio programare pentru acest film in aceasta zi.\n";
        pauseMenu(); return;
    }

    std::cout << "\n  Programari disponibile:\n\n";
    i=1;
    for(auto* p:programari){
        Sala* s = cinema.getSalaByIdSafe(p->salaId);
        int lib = s ? s->locuriLibere(data,p->ora) : 0;
        std::cout << "  [" << i++ << "] " << p->ora << " - " << p->salaNume
                  << " (" << lib << " locuri libere)\n";
    }
    std::cout << "\n";
    int pOpt = inputInt("Alege programarea: ", 1, (int)programari.size());
    const Programare* prog = programari[pOpt-1];
    Sala* sala = cinema.getSalaByIdSafe(prog->salaId);
    if(!sala){ std::cout << "\n  Eroare sala!\n"; pauseMenu(); return; }

    std::vector<std::pair<int,int>> locuri;
    while(locuri.size() < 8){
        afiseazaHartaSala(sala, data, prog->ora, locuri);
        std::cout << "\n  Locuri selectate: " << locuri.size() << " / 8\n";
        std::cout << "  [0] Confirma selectia\n\n";
        int rand = inputInt("Rand (0 pentru confirmare): ", 0, sala->getRanduri());
        if(rand == 0) break;
        int col = inputInt("Coloana: ", 1, sala->getColoane());
        rand--; col--;
        bool dejaSel = false;
        for(auto&[r,c]:locuri) if(r==rand&&c==col){dejaSel=true;break;}
        if(dejaSel){ std::cout << "\n  Loc deja selectat!\n"; continue; }
        try {
            if(sala->esteOcupat(data,prog->ora,rand,col))
                std::cout << "\n  Loc ocupat! Alege altul.\n";
            else {
                locuri.push_back({rand,col});
                CategorieLocSeat cat = sala->getCategorieLocSeat(rand,col);
                double pret = (film->getPretBaza()+(film->getEste3D()?10.0:0.0))
                             * multiplicatorCategorie(cat)
                             * multiplicatorZi(data)
                             * cinema.getLocatieMultiplicator();
                std::cout << "\n  Loc adaugat: R" << rand+1 << " L" << col+1
                          << " [" << numeCategorie(cat) << "] - "
                          << std::fixed << std::setprecision(2) << pret << " RON\n";
            }
        } catch(const IndexInvalidException& e){
            std::cout << "\n  Index invalid: " << e.what() << "\n";
        }
    }

    if(locuri.empty()){ std::cout << "\n  Niciun loc selectat.\n"; pauseMenu(); return; }

    std::cout << "\n  ── Meniu Food (optional) ─────────────────\n\n";
    auto& meniu = cinema.getMeniu();
    FoodOrder food;
    std::string catCur = "";
    int fi = 1;
    std::vector<const FoodItem*> items;
    for(auto& item:meniu){
        if(item.categorie != catCur){
            catCur = item.categorie;
            std::cout << "  --- " << catCur << " ---\n";
        }
        std::cout << "  [" << fi++ << "] " << item.emoji << " " << item.nume
                  << " - " << (int)item.pret << " RON\n";
        items.push_back(&item);
    }
    std::cout << "  [0] Sari peste food\n\n";
    while(true){
        int fid = inputInt("Adauga produs (0 pentru a continua): ", 0, (int)items.size());
        if(fid == 0) break;
        int qty = inputInt("Cantitate: ", 1, 10);
        food.items[items[fid-1]->id] += qty;
        std::cout << "  Adaugat: " << qty << "x " << items[fid-1]->nume << "\n";
    }

    std::cout << "\n  ── Sumar Rezervare ───────────────────────\n\n";
    std::cout << "  Film    : " << film->getTitlu() << "\n";
    std::cout << "  Locatie : " << loc->nume << "\n";
    std::cout << "  Sala    : " << sala->getNume() << "\n";
    std::cout << "  Data    : " << data.toString() << " " << prog->ora << "\n";
    std::cout << "  Locuri  : ";
    double totalBilete = 0;
    for(auto&[r,c]:locuri){
        CategorieLocSeat cat = sala->getCategorieLocSeat(r,c);
        double pret = (film->getPretBaza()+(film->getEste3D()?10.0:0.0))
                     * multiplicatorCategorie(cat)
                     * multiplicatorZi(data)
                     * cinema.getLocatieMultiplicator();
        totalBilete += pret;
        std::cout << "R" << r+1 << "L" << c+1 << "[" << numeCategorie(cat) << "] ";
    }
    double foodTotal = food.total(meniu);
    std::cout << "\n  Bilete  : " << std::fixed << std::setprecision(2) << totalBilete << " RON";
    if(multiplicatorZi(data)>1.0) std::cout << " [Weekend +20%]";
    std::cout << "\n";
    if(!food.empty())
        std::cout << "  Food    : " << food.toStr(meniu) << " ("
                  << std::fixed << std::setprecision(2) << foodTotal << " RON)\n";
    std::cout << "  TOTAL   : " << std::fixed << std::setprecision(2)
              << totalBilete + foodTotal << " RON\n\n";

    std::cout << "  Confirmi rezervarea? [1=Da / 0=Nu]: ";
    int conf; std::cin >> conf;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
    if(conf != 1){ std::cout << "\n  Rezervare anulata.\n"; pauseMenu(); return; }

    std::string email = inputStr("Email (optional, Enter pentru a sari): ");

    try {
        cinema.proceseazaRezervare(prog->salaId, data, locuri, film, prog->ora, email, food);
        std::cout << "\n  ✓ Rezervare confirmata! Biletul a fost generat in data/bilete/\n";
    } catch(const LocOcupatException& e){
        std::cout << "\n  Eroare: " << e.what() << "\n";
    } catch(const std::exception& e){
        std::cout << "\n  Eroare: " << e.what() << "\n";
    }
    pauseMenu();
}

void menuIstoricSiEvaluari(){
    printHeader("Profilul Meu");
    auto* u = cinema.getCurent(); if(!u) return;

    std::cout << "  Istoric rezervari:\n\n";
    if(u->istoricRezervari.empty())
        std::cout << "  (nicio rezervare)\n";
    else {
        int i=1;
        for(auto& r:u->istoricRezervari)
            std::cout << "  " << i++ << ". " << r << "\n";
    }

    printSeparator();
    std::cout << "\n  Evalueaza un film? [1=Da / 0=Nu]: ";
    int opt; std::cin >> opt;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
    if(opt == 1){
        std::string titlu = inputStr("Titlu film: ");
        int stele = inputInt("Stele (1-5): ", 1, 5);
        cinema.evalueazaFilm(titlu, stele);
        std::cout << "\n  Multumim pentru evaluare!\n";
    }
    pauseMenu();
}

void menuAdmin(){
    printHeader("Panou Admin");
    std::cout << "  [1] Vezi toate rezervarile\n";
    std::cout << "  [2] Adauga film\n";
    std::cout << "  [3] Sterge film\n";
    std::cout << "  [0] Inapoi\n\n";
    int opt = inputInt("Optiunea ta: ", 0, 3);
    if(opt == 0) return;

    if(opt == 1){
        printHeader("Toate Rezervarile");
        std::ifstream f("data/toate_rezervarile.txt");
        if(!f.is_open()){ std::cout << "  (gol)\n"; pauseMenu(); return; }
        std::string ln; int i=1;
        while(std::getline(f,ln)) if(!ln.empty()) std::cout << "  " << i++ << ". " << ln << "\n";
        pauseMenu(); return;
    }
    if(opt == 2){
        printHeader("Adauga Film");
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
        std::string titlu = inputStr("Titlu: ");
        std::string gen   = inputStr("Gen: ");
        int dur  = inputInt("Durata (min): ", 1, 360);
        int pret = inputInt("Pret baza (RON): ", 1, 999);
        std::cout << "  3D? [1=Da / 0=Nu]: ";
        int e3d; std::cin >> e3d;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
        cinema.adaugaFilmManual(titlu,e3d==1,pret,gen,dur,StatusFilm::RULAZA_ACUM);
        std::cout << "\n  Film adaugat!\n"; pauseMenu(); return;
    }
    if(opt == 3){
        printHeader("Sterge Film");
        auto& filme = cinema.getFilme();
        int i=1; for(auto& f:filme) std::cout << "  [" << i++ << "] " << f.getTitlu() << "\n";
        std::cout << "\n";
        int idx = inputInt("Film de sters: ", 1, (int)filme.size());
        cinema.stergeFilm(idx-1);
        std::cout << "\n  Film sters!\n"; pauseMenu(); return;
    }
}

void menuHartaSala(){
    printHeader("Harta Sala");
    auto* loc = cinema.getLocatieCurenta();
    if(!loc){ std::cout << "  Alege mai intai o locatie!\n"; pauseMenu(); return; }

    auto& sali = loc->sali;
    int i=1;
    for(auto& s:sali) std::cout << "  [" << i++ << "] " << s.getNume() << "\n";
    std::cout << "\n";
    int sOpt = inputInt("Alege sala: ", 1, (int)sali.size());
    Sala* sala = cinema.getSalaByIdSafe(sali[sOpt-1].getId());

    auto zile = urmatoroarele7Zile();
    i=1;
    for(auto& z:zile) std::cout << "  [" << i++ << "] " << numeZiSaptamana(z) << " " << z.toString() << "\n";
    std::cout << "\n";
    int dOpt = inputInt("Alege data: ", 1, (int)zile.size());
    Data data = zile[dOpt-1];

    std::vector<std::string> ore;
    for(auto& p:loc->program)
        if(p.salaId==sala->getId()&&p.data==data)
            if(std::find(ore.begin(),ore.end(),p.ora)==ore.end()) ore.push_back(p.ora);
    std::sort(ore.begin(),ore.end());

    if(ore.empty()){ std::cout << "\n  Nicio programare pentru aceasta sala si data.\n"; pauseMenu(); return; }

    i=1; for(auto& o:ore) std::cout << "  [" << i++ << "] " << o << "\n";
    std::cout << "\n";
    int oOpt = inputInt("Alege ora: ", 1, (int)ore.size());
    afiseazaHartaSala(sala, data, ore[oOpt-1]);
    pauseMenu();
}

int main(){
    cinema.init();
    menuLogin();

    bool locSet = false;
    while(true){
        if(!locSet){ menuAlegeLocatie(); locSet=true; }

        printHeader("Meniu Principal");
        auto* u = cinema.getCurent();
        if(u) std::cout << "  Utilizator: " << u->username
                        << (u->esteAdmin?" [ADMIN]":"") << "\n";
        auto* loc = cinema.getLocatieCurenta();
        if(loc) std::cout << "  Locatie: " << loc->nume << " - " << loc->oras << "\n\n";

        std::cout << "  [1] Browse filme\n";
        std::cout << "  [2] Rezervare\n";
        std::cout << "  [3] Harta sala\n";
        std::cout << "  [4] Profilul meu\n";
        if(u&&u->esteAdmin) std::cout << "  [5] Panou admin\n";
        std::cout << "  [6] Schimba locatia\n";
        std::cout << "  [0] Deconectare\n\n";

        int opt = inputInt("Optiunea ta: ", 0, 6);
        switch(opt){
            case 0: cinema.logout(); locSet=false; menuLogin(); break;
            case 1: menuBrowseFilme(); break;
            case 2: menuRezervare(); break;
            case 3: menuHartaSala(); break;
            case 4: menuIstoricSiEvaluari(); break;
            case 5: if(u&&u->esteAdmin) menuAdmin(); break;
            case 6: locSet=false; break;
        }
    }
}
