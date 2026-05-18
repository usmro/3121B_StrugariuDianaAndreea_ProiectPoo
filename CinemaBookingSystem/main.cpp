#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <ctime>
#include <cstdlib>
#include "UI.h"

int main() {
    std::srand((unsigned)time(nullptr));

    Cinematograf cinema;
    cinema.initConturi();
    cinema.incarcaFilme("filme.txt");
    cinema.adaugaSala(Sala(1,"SALA 1 STANDARD",5,10));
    cinema.adaugaSala(Sala(2,"SALA 2 VIP",3,6));
    cinema.incarcaRezervari();

    if(!menuAutentificare(cinema)) {
        system(CLEAR);
        std::cout << "La revedere!\n";
        return 0;
    }

    while(true) {
        ContUtilizator* u=cinema.getCurent();
        std::string info = u ? ("  Conectat: "+u->username+(u->esteAdmin?" [ADMIN]":"")) : "";

        std::vector<std::string> optMain={
            "Rezervare Noua",
            "Browse dupa gen",
            "Harta Sala",
            "Anulare Rezervare",
            "Profilul Meu"
        };
        if(u&&u->esteAdmin) optMain.push_back("Panou Admin");
        optMain.push_back("Deconectare");

        int al=meniuInteractiv(optMain,"CINEMA - MENIU PRINCIPAL",info);
        int nOpt=(int)optMain.size();
        bool eAdmin=(u&&u->esteAdmin);

        
        if(al==-1||al==nOpt-1) {
            cinema.logout();
            if(!menuAutentificare(cinema)) break;
            continue;
        }

        if(al==0) {
            fluxRezervare(cinema);
        }
        else if(al==1) {
            
            Film* f=meniuBrowseGenuri(cinema,-1,false);
            if(f) {
                int c=meniuInteractiv(
                    {"Da, rezerva acest film","Nu, inapoi"},
                    "AI SELECTAT: "+f->getTitlu()
                );
                if(c==0) fluxRezervare(cinema,f);
            }
        }
        else if(al==2) {
            
            std::vector<std::string> optSali;
            for(auto& s:cinema.getSali()) optSali.push_back(s.getNume());
            int idxS=meniuInteractiv(optSali,"ALEGE SALA");
            if(idxS==-1) continue;
            int idSala=cinema.getSali()[idxS].getId();

            auto zile=urmatoroarele7Zile();
            int idxD=selecteazaData(zile);
            if(idxD==-1) continue;
            Data data=zile[idxD];

            int idxO=meniuInteractiv(
                std::vector<std::string>(cinema.getOre().begin(),cinema.getOre().end()),
                "ALEGE ORA"
            );
            if(idxO==-1) continue;
            system(CLEAR);
            cinema.getSalaById(idSala)->afiseazaHarta(data,cinema.getOre()[idxO]);
            asteptaTasta();
        }
        else if(al==3) {
            
            std::vector<std::string> optSali;
            for(auto& s:cinema.getSali()) optSali.push_back(s.getNume());
            int idxS=meniuInteractiv(optSali,"ALEGE SALA - ANULARE");
            if(idxS==-1) continue;
            int idSala=cinema.getSali()[idxS].getId();
            Sala* sala=cinema.getSalaById(idSala);

            auto zile=urmatoroarele7Zile();
            int idxD=selecteazaData(zile);
            if(idxD==-1) continue;
            Data data=zile[idxD];

            int idxO=meniuInteractiv(
                std::vector<std::string>(cinema.getOre().begin(),cinema.getOre().end()),
                "ALEGE ORA"
            );
            if(idxO==-1) continue;
            std::string ora=cinema.getOre()[idxO];

            
            auto [r,l]=selecteazaLocInteractiv(*sala,data,ora);
            if(r==-1) continue;
            if(!sala->esteOcupat(data,ora,r,l)) {
                std::cout << "\n  [!] Locul nu e ocupat.\n"; asteptaTasta(); continue;
            }
            std::string confirm=citesteString(
                "  Anulezi rezervarea R"+std::to_string(r+1)+" L"+std::to_string(l+1)+"? (d/n): "
            );
            if(confirm=="d"||confirm=="D")
                cinema.anuleazaRezervare(idSala,data,ora,r,l);
            asteptaTasta();
        }
        else if(al==4) {
            meniuProfil(cinema);
        }
        else if(al==5&&eAdmin) {
            meniuAdmin(cinema);
        }
    }

    system(CLEAR);
    std::cout << "La revedere!\n";
    return 0;
}
