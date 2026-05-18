#ifndef UI_H
#define UI_H

#include "Platform.h"
#include "CinemaModels.h"
#include "Cinema.h"
#include <iomanip>


inline bool contineSubsir(std::string text, std::string cautare) {
    std::transform(text.begin(),text.end(),text.begin(),::tolower);
    std::transform(cautare.begin(),cautare.end(),cautare.begin(),::tolower);
    return text.find(cautare)!=std::string::npos;
}

inline void asteptaTasta(const std::string& msg="\n  Apasa orice tasta...") {
    std::cout << msg << std::flush;
    int ch=0; citesteInput(ch);
}

inline int meniuInteractiv(const std::vector<std::string>& opt,
                            const std::string& titlu,
                            const std::string& subtext="") {
    int sel=0;
    while(true) {
        system(CLEAR);
        std::cout << "=== " << titlu << " ===\n";
        if(!subtext.empty()) std::cout << subtext << "\n";
        std::cout << "\n";
        for(int i=0;i<(int)opt.size();++i) {
            if(i==sel) std::cout << "  [>] " << opt[i] << "\n";
            else       std::cout << "      " << opt[i] << "\n";
        }
        std::cout << "\n  Sus/Jos + Enter | ESC = inapoi\n";
        int ch=0; Tasta t=citesteInput(ch);
        if(t==T_SUS&&sel>0) --sel;
        if(t==T_JOS&&sel<(int)opt.size()-1) ++sel;
        if(t==T_ENTER) return sel;
        if(t==T_ESC)   return -1;
    }
}


inline std::pair<int,int> selecteazaLocInteractiv(Sala& sala,
                                                   const Data& data,
                                                   const std::string& ora) {
    int selR=0, selL=0;
   
    for(int r=0;r<sala.getRanduri()&&selR==0&&selL==0;++r)
        for(int l=0;l<sala.getColoane();++l)
            if(!sala.esteOcupat(data,ora,r,l)) { selR=r; selL=l; goto gasit; }
    gasit:;

    while(true) {
        system(CLEAR);
        sala.afiseazaHarta(data, ora, selR, selL);

        CategorieLocSeat cat=sala.getCategorieLocSeat(selR,selL);
        bool ocupat=sala.esteOcupat(data,ora,selR,selL);
        std::cout << "  Selectat: R" << selR+1 << " L" << selL+1;
        if(ocupat) std::cout << "  [OCUPAT]";
        else std::cout << "  [" << numeCategorie(cat) << "]";
        std::cout << "\n  Sageti=naviga | Enter=alege | ESC=inapoi\n";

        int ch=0; Tasta t=citesteInput(ch);
        if(t==T_ESC)    return {-1,-1};
        if(t==T_SUS    &&selR>0)                     --selR;
        if(t==T_JOS    &&selR<sala.getRanduri()-1)   ++selR;
        if(t==T_STANGA &&selL>0)                     --selL;
        if(t==T_DREAPTA&&selL<sala.getColoane()-1)   ++selL;
        if(t==T_ENTER) {
            if(ocupat) { std::cout << "\n  [!] Loc ocupat! Alege altul.\n"; asteptaTasta(""); }
            else return {selR, selL};
        }
    }
}


inline int selecteazaData(std::vector<Data>& zile) {
    std::vector<std::string> optZile;
    Data azi=dataAzi();
    for(auto& z : zile) {
        std::string label = numeZiSaptamana(z) + " " + z.toString();
        if(z==azi) label += "  (azi)";
        optZile.push_back(label);
    }
    return meniuInteractiv(optZile, "ALEGE DATA");
}


inline Film* meniuCautareLive(Cinematograf& cinema, bool doarActive=false) {
    std::string input; int idx=0;
    while(true) {
        system(CLEAR);
        std::cout << "=== CAUTARE FILM ===\n";
        std::cout << "  Scrie: " << input << "_\n";
        std::cout << std::string(52,'-') << "\n";

        std::vector<Film*> rez;
        for(auto& f : cinema.getFilme())
            if((!doarActive || f.getStatus()==StatusFilm::RULAZA_ACUM)
               && contineSubsir(f.getTitlu(),input))
                rez.push_back(const_cast<Film*>(&f));

        if(idx>=(int)rez.size()) idx=(int)rez.size()-1;
        if(idx<0) idx=0;

        for(int i=0;i<(int)rez.size()&&i<14;++i) {
            std::string extra="";
            if(rez[i]->getEste3D()) extra+=" [3D]";
            if(rez[i]->getDurata()>0) extra+=" "+std::to_string(rez[i]->getDurata())+"min";
            extra+=" | "+rez[i]->getGen();
            if(rez[i]->getStatus()==StatusFilm::IN_CURAND) extra+=" [IN CURAND]";
            if(i==idx) std::cout << "  [>] " << rez[i]->getTitlu() << extra << "\n";
            else       std::cout << "      " << rez[i]->getTitlu() << extra << "\n";
        }
        if((int)rez.size()>14)
            std::cout << "  ... si " << rez.size()-14 << " mai multe\n";
        std::cout << "\n  Enter=selecteaza | ESC=inapoi\n";

        int ch=0; Tasta t=citesteInput(ch);
        if(t==T_SUS&&idx>0) --idx;
        else if(t==T_JOS&&idx<(int)rez.size()-1) ++idx;
        else if(t==T_ENTER) return rez.empty()?nullptr:rez[idx];
        else if(t==T_ESC)   return nullptr;
        else if(t==T_BACKSPACE) { if(!input.empty()) input.pop_back(); idx=0; }
        else if(t==T_ALTUL&&isprint(ch)) { input+=(char)ch; idx=0; }
    }
}


inline std::vector<std::string> colecteazaGenuri(const std::vector<Film>& filme) {
    std::vector<std::string> g;
    for(auto& f:filme) {
        if(f.getGen().empty()||f.getGen()=="Necunoscut") continue;
        if(std::find(g.begin(),g.end(),f.getGen())==g.end()) g.push_back(f.getGen());
    }
    std::sort(g.begin(),g.end());
    return g;
}

inline Film* meniuBrowseGenuri(Cinematograf& cinema, int filtru3D=-1,
                                bool doarActive=false) {
    
    std::vector<std::string> genuri=colecteazaGenuri(cinema.getFilme());
    if(genuri.empty()) return nullptr;

    int genIdx=0;
    std::map<std::string,int> offG, idxG;
    for(auto& g:genuri) { offG[g]=0; idxG[g]=0; }
    const int VIZ=5;

    auto trunc=[](const std::string& s, int n)->std::string{
        if((int)s.size()<=n) return s+std::string(n-s.size(),' ');
        return s.substr(0,n-2)+"..";
    };

    while(true) {
        std::string& gen=genuri[genIdx];

        
        std::vector<Film*> filmeGen;
        for(auto& f:cinema.getFilme()) {
            if(f.getGen()!=gen) continue;
            if(doarActive&&f.getStatus()!=StatusFilm::RULAZA_ACUM) continue;
            if(filtru3D==0&&f.getEste3D()) continue;
            if(filtru3D==1&&!f.getEste3D()) continue;
            filmeGen.push_back(const_cast<Film*>(&f));
        }

        int& off=offG[gen]; int& iLoc=idxG[gen];
        int total=(int)filmeGen.size();
        if(iLoc>=total&&total>0) iLoc=total-1;

        system(CLEAR);
        std::cout << "\n  +----------------------------------------------------------+\n";
        std::cout << "  |              BROWSE DUPA GEN  -  CINEMA                 |\n";
        std::cout << "  +----------------------------------------------------------+\n\n";

        
        std::cout << "  ";
        for(int i=0;i<(int)genuri.size();++i) {
            if(i==genIdx) std::cout << "[" << genuri[i] << "] ";
            else          std::cout << " " << genuri[i] << "  ";
        }
        std::cout << "\n  " << std::string(62,'-') << "\n\n";

        if(total==0) {
            std::cout << "  (niciun film in acest gen / filtru)\n";
        } else {
            int afisate=std::min(VIZ,total-off);
            std::cout << "  ";
            if(off>0) std::cout << "< ";
            else      std::cout << "  ";
            for(int i=0;i<afisate;++i) {
                std::string tit=trunc(filmeGen[off+i]->getTitlu(),16);
                if(i==iLoc) std::cout << "[" << tit << "] ";
                else        std::cout << " " << tit << "  ";
            }
            if(off+afisate<total) std::cout << " >";
            std::cout << "\n\n";

            Film* sel=filmeGen[off+iLoc];
            std::string pretStr=std::to_string((int)sel->getPretBaza())+" RON"
                               +(sel->getEste3D()?" + 10 RON (3D)":"");
            std::string statusStr=numeStatus(sel->getStatus());

            std::cout << "  +--------------------------------------------+\n";
            std::cout << "  | " << std::left << std::setw(42) << sel->getTitlu().substr(0,42) << " |\n";
            std::cout << "  | Gen:    " << std::left << std::setw(34) << sel->getGen()    << " |\n";
            std::cout << "  | Durata: " << std::left << std::setw(34)
                      << (std::to_string(sel->getDurata())+" min")                         << " |\n";
            std::cout << "  | Format: " << std::left << std::setw(34)
                      << (sel->getEste3D()?"3D":"2D")                                      << " |\n";
            std::cout << "  | Pret:   " << std::left << std::setw(34) << pretStr          << " |\n";
            std::cout << "  | Status: " << std::left << std::setw(34) << statusStr        << " |\n";
            std::cout << "  +--------------------------------------------+\n";
            std::cout << "\n  Film " << (off+iLoc+1) << " / " << total << "\n";
        }

        std::cout << "\n  Sus/Jos=gen | St/Dr=film | Enter=rezerva | ESC=inapoi\n";
        if(filtru3D==-1)
            std::cout << "  [F] Filtreaza: toate formatele\n";
        else if(filtru3D==0)
            std::cout << "  [F] Filtreaza: doar 2D\n";
        else
            std::cout << "  [F] Filtreaza: doar 3D\n";
        if(doarActive)
            std::cout << "  [A] Afiseaza: doar 'Ruleaza acum'\n";
        else
            std::cout << "  [A] Afiseaza: toate (inclusiv 'In curand')\n";

        int ch=0; Tasta t=citesteInput(ch);

        if(t==T_ESC) return nullptr;
        if(t==T_SUS) genIdx=(genIdx>0)?genIdx-1:(int)genuri.size()-1;
        else if(t==T_JOS) genIdx=(genIdx+1)%(int)genuri.size();
        else if(t==T_STANGA&&total>0) {
            if(iLoc>0) --iLoc;
            else if(off>0) { --off; }
        }
        else if(t==T_DREAPTA&&total>0) {
            if(iLoc<std::min(VIZ,total-off)-1) ++iLoc;
            else if(off+VIZ<total) { ++off; }
        }
        else if(t==T_ENTER&&total>0) {
            Film* sel=filmeGen[off+iLoc];
            if(sel->getStatus()==StatusFilm::IN_CURAND) {
                std::cout << "\n  [!] Acest film nu ruleaza inca.\n";
                asteptaTasta("");
            } else return sel;
        }
        else if(t==T_ALTUL&&(ch=='f'||ch=='F')) {
            filtru3D=(filtru3D+2)%3-1; // cicleza -1->0->1->-1
            for(auto& g:genuri) { offG[g]=0; idxG[g]=0; }
        }
        else if(t==T_ALTUL&&(ch=='a'||ch=='A')) {
            doarActive=!doarActive;
            for(auto& g:genuri) { offG[g]=0; idxG[g]=0; }
        }
    }
}


inline bool afiseazaConfirmare(const std::string& titluFilm, const std::string& salaNume,
                                const Data& data, const std::string& ora,
                                int r, int l, CategorieLocSeat cat, double pret) {
    system(CLEAR);
    std::cout << "\n  +------------------------------------------+\n";
    std::cout << "  |           SUMAR REZERVARE                |\n";
    std::cout << "  +------------------------------------------+\n";
    std::cout << "  | Film  : " << std::left << std::setw(32) << titluFilm.substr(0,32) << " |\n";
    std::cout << "  | Sala  : " << std::left << std::setw(32) << salaNume.substr(0,32)  << " |\n";
    std::cout << "  | Data  : " << std::left << std::setw(32) << data.toString()        << " |\n";
    std::cout << "  | Ora   : " << std::left << std::setw(32) << ora                    << " |\n";
    std::cout << "  | Loc   : " << std::left << std::setw(32)
              << ("R"+std::to_string(r+1)+" L"+std::to_string(l+1)
                 +" ["+numeCategorie(cat)+"]")                                           << " |\n";
    std::cout << "  | Pret  : " << std::left << std::setw(28)
              << (std::to_string((int)pret)+"."+
                  (pret-(int)pret>0.04?"50":"00")+" RON")                               << " |\n";
    std::cout << "  +------------------------------------------+\n\n";
    std::string c=citesteString("  Confirmi rezervarea? (d/n): ");
    return (c=="d"||c=="D");
}


inline void fluxRezervare(Cinematograf& cinema, Film* film=nullptr) {
    
    if(!film) {
        int mod=meniuInteractiv({"Cauta dupa titlu","Browse dupa gen"},"ALEGE FILM");
        if(mod==-1) return;
        film = (mod==0) ? meniuCautareLive(cinema,true)
                        : meniuBrowseGenuri(cinema,-1,true);
    }
    if(!film) return;
    if(film->getStatus()==StatusFilm::IN_CURAND) {
        std::cout << "\n  [!] Filmul nu ruleaza inca.\n"; asteptaTasta(); return;
    }

    
    std::vector<std::string> optSali;
    for(auto& s:cinema.getSali())
        optSali.push_back(s.getNume()+" ("+std::to_string(s.getRanduri())
                         +"x"+std::to_string(s.getColoane())+")");
    int idxS=meniuInteractiv(optSali,"ALEGE SALA");
    if(idxS==-1) return;
    int idSala=cinema.getSali()[idxS].getId();
    Sala* sala=cinema.getSalaById(idSala);

    
    auto zile=urmatoroarele7Zile();
    int idxD=selecteazaData(zile);
    if(idxD==-1) return;
    Data data=zile[idxD];

    
    int idxO=meniuInteractiv(
        std::vector<std::string>(cinema.getOre().begin(),cinema.getOre().end()),
        "ALEGE ORA",
        "  Locuri libere: "+std::to_string(sala->locuriLibere(data,cinema.getOre()[0]))
    );
    if(idxO==-1) return;
    std::string ora=cinema.getOre()[idxO];

    
    auto [r,l]=selecteazaLocInteractiv(*sala,data,ora);
    if(r==-1) return;

    
    CategorieLocSeat cat=sala->getCategorieLocSeat(r,l);
    double pret=(film->getPretBaza()+(film->getEste3D()?10.0:0.0))*multiplicatorCategorie(cat);
    if(!afiseazaConfirmare(film->getTitlu(),sala->getNume(),data,ora,r,l,cat,pret)) {
        std::cout << "  Anulat.\n"; asteptaTasta(); return;
    }

    
    std::string email=citesteString("  Email (optional, Enter skip): ");
    while(!email.empty()&&(email.front()==' '||email.front()=='\r')) email.erase(email.begin());
    while(!email.empty()&&(email.back()==' '||email.back()=='\r'))   email.pop_back();

    cinema.proceseazaRezervare(idSala,data,r,l,film,ora,email);
    std::cout << "  [OK] Rezervare confirmata!\n";
    asteptaTasta();
}


inline bool menuAutentificare(Cinematograf& cinema) {
    while(true) {
        int al=meniuInteractiv({"Autentificare","Inregistrare cont nou","Iesire"},
                                "CINEMA - AUTENTIFICARE");
        if(al==2||al==-1) return false;
        system(CLEAR);
        if(al==0) {
            std::cout << "=== AUTENTIFICARE ===\n\n";
            std::string usr=citesteString("  Username: ");
            std::string par=citesteString("  Parola:   ");
            if(cinema.login(usr,par)) return true;
            std::cout << "\n  [!] Username sau parola incorecta.\n"; asteptaTasta();
        } else {
            std::cout << "=== INREGISTRARE ===\n\n";
            std::string usr=citesteString("  Username nou: ");
            std::string par=citesteString("  Parola:       ");
            std::string par2=citesteString("  Confirma:     ");
            if(par!=par2) { std::cout << "\n  [!] Parolele nu coincid.\n"; asteptaTasta(); continue; }
            if(cinema.inregistrare(usr,par)) {
                std::cout << "\n  [OK] Cont creat! Bun venit, " << usr << "!\n";
                asteptaTasta(); return true;
            } else {
                std::cout << "\n  [!] Username deja existent.\n"; asteptaTasta();
            }
        }
    }
}


inline void meniuProfil(Cinematograf& cinema) {
    ContUtilizator* u=cinema.getCurent();
    if(!u) return;
    while(true) {
        int al=meniuInteractiv({"Istoricul meu","Schimba parola","Inapoi"},
                                "PROFIL","  Utilizator: "+u->username);
        if(al==-1||al==2) return;
        system(CLEAR);
        if(al==0) {
            std::cout << "=== ISTORICUL REZERVARILOR ===\n\n";
            if(u->istoricRezervari.empty())
                std::cout << "  (nicio rezervare)\n";
            else
                for(int i=0;i<(int)u->istoricRezervari.size();++i)
                    std::cout << "  " << i+1 << ". " << u->istoricRezervari[i] << "\n";
            asteptaTasta();
        } else {
            std::cout << "=== SCHIMBA PAROLA ===\n\n";
            std::string veche=citesteString("  Parola actuala: ");
            if(veche!=u->parola) { std::cout << "  [!] Parola incorecta.\n"; asteptaTasta(); continue; }
            std::string noua=citesteString("  Parola noua: ");
            std::string conf=citesteString("  Confirma:    ");
            if(noua!=conf) { std::cout << "  [!] Nu coincid.\n"; asteptaTasta(); continue; }
            cinema.schimbaParola(u->username,noua);
            u->parola=noua;
            std::cout << "  [OK] Parola schimbata.\n"; asteptaTasta();
        }
    }
}


inline void meniuAdmin(Cinematograf& cinema) {
    while(true) {
        int al=meniuInteractiv({
            "Toate rezervarile",
            "Raport vanzari",
            "Adauga film",
            "Editeaza film",
            "Sterge film",
            "Gestioneaza ore proiectie",
            "Lista utilizatori",
            "Sterge utilizator",
            "Inapoi"
        },"PANOU ADMIN","  Admin: "+cinema.getCurent()->username);
        if(al==-1||al==8) return;
        system(CLEAR);

        if(al==0) {
            std::cout << "=== TOATE REZERVARILE ===\n\n";
            cinema.afiseazaToateRezervari();
            asteptaTasta();
        }
        else if(al==1) {
            cinema.raportVanzari();
            asteptaTasta();
        }
        else if(al==2) {
            std::cout << "=== ADAUGA FILM ===\n\n";
            std::string titlu=citesteString("  Titlu: ");
            std::string gen=citesteString("  Gen: ");
            int dur=citesteInt("  Durata (min): ",1,300);
            double pret=0;
            while(true) {
                std::string sp=citesteString("  Pret (RON): ");
                try { pret=std::stod(sp); if(pret>0) break; } catch(...) {}
                std::cout << "  [!] Pret invalid.\n";
            }
            std::string s3d=citesteString("  Este 3D? (d/n): ");
            bool e3d=(s3d=="d"||s3d=="D");
            int istat=meniuInteractiv({"Ruleaza acum","In curand"},"STATUS FILM");
            StatusFilm st=(istat==1)?StatusFilm::IN_CURAND:StatusFilm::RULAZA_ACUM;
            cinema.adaugaFilmManual(titlu,e3d,pret,gen,dur,st);
            std::cout << "\n  [OK] Film adaugat.\n"; asteptaTasta();
        }
        else if(al==3) {
            
            auto& filme=cinema.getFilme();
            if(filme.empty()) { std::cout << "  (niciun film)\n"; asteptaTasta(); continue; }
            std::vector<std::string> optF;
            for(auto& f:filme) optF.push_back(f.getTitlu()+(f.getEste3D()?" [3D]":""));
            int idx=meniuInteractiv(optF,"ALEGE FILM DE EDITAT");
            if(idx==-1) continue;
            system(CLEAR);
            Film& f=filme[idx];
            std::cout << "=== EDITEAZA: " << f.getTitlu() << " ===\n";
            std::cout << "  (Enter gol = pastreaza valoarea curenta)\n\n";
            auto readOpt=[](const std::string& p, const std::string& cur)->std::string{
                std::cout << p << " [" << cur << "]: ";
#ifndef _WIN32
                struct termios oldt; tcgetattr(STDIN_FILENO,&oldt);
                struct termios newt=oldt; newt.c_lflag|=(ICANON|ECHO);
                tcsetattr(STDIN_FILENO,TCSANOW,&newt);
#endif
                std::string s; std::getline(std::cin,s);
                if(!s.empty()&&s.back()=='\r') s.pop_back();
#ifndef _WIN32
                tcsetattr(STDIN_FILENO,TCSANOW,&oldt);
#endif
                return s.empty()?cur:s;
            };
            std::string titlu=readOpt("  Titlu",f.getTitlu());
            std::string gen=readOpt("  Gen",f.getGen());
            std::string sdur=readOpt("  Durata",std::to_string(f.getDurata()));
            std::string spret=readOpt("  Pret",std::to_string((int)f.getPretBaza()));
            std::string s3d=readOpt("  3D (d/n)",(f.getEste3D()?"d":"n"));
            int istat=meniuInteractiv({"Ruleaza acum","In curand"},"STATUS");
            int dur=f.getDurata(); double pret=f.getPretBaza();
            try { dur=std::stoi(sdur); } catch(...) {}
            try { pret=std::stod(spret); } catch(...) {}
            StatusFilm st=(istat==1)?StatusFilm::IN_CURAND:StatusFilm::RULAZA_ACUM;
            cinema.editeazaFilm(idx,titlu,(s3d=="d"||s3d=="D"),pret,gen,dur,st);
            std::cout << "  [OK] Film actualizat.\n"; asteptaTasta();
        }
        else if(al==4) {
            auto& filme=cinema.getFilme();
            if(filme.empty()) { std::cout << "  (niciun film)\n"; asteptaTasta(); continue; }
            std::vector<std::string> optF;
            for(auto& f:filme) optF.push_back(f.getTitlu()+(f.getEste3D()?" [3D]":""));
            int idx=meniuInteractiv(optF,"STERGE FILM");
            if(idx==-1) continue;
            std::string confirm=citesteString("  Confirmi stergerea lui '"+filme[idx].getTitlu()+"'? (d/n): ");
            if(confirm=="d"||confirm=="D") {
                cinema.stergeFilm(idx);
                std::cout << "  [OK] Film sters.\n";
            }
            asteptaTasta();
        }
        else if(al==5) {
            
            while(true) {
                std::vector<std::string> optOre;
                for(auto& o:cinema.getOre()) optOre.push_back(o);
                optOre.push_back("+ Adauga ora noua");
                optOre.push_back("Inapoi");
                int idxO=meniuInteractiv(optOre,"ORE PROIECTIE");
                if(idxO==-1||idxO==(int)optOre.size()-1) break;
                if(idxO==(int)optOre.size()-2) {
                    std::string ora=citesteString("  Ora noua (HH:MM): ");
                    if(ora.size()==5&&ora[2]==':') {
                        cinema.adaugaOra(ora);
                        std::cout << "  [OK] Ora adaugata.\n";
                    } else std::cout << "  [!] Format invalid (HH:MM).\n";
                    asteptaTasta();
                } else {
                    std::string confirm=citesteString("  Stergi ora "+cinema.getOre()[idxO]+"? (d/n): ");
                    if(confirm=="d"||confirm=="D") cinema.stergeOra(idxO);
                    asteptaTasta();
                }
            }
        }
        else if(al==6) {
            std::cout << "=== UTILIZATORI ===\n\n";
            for(auto& u:cinema.getUtilizatori())
                std::cout << "  - " << u.username << (u.esteAdmin?" [ADMIN]":"") << "\n";
            asteptaTasta();
        }
        else if(al==7) {
            std::cout << "=== STERGE UTILIZATOR ===\n\n";
            for(auto& u:cinema.getUtilizatori())
                std::cout << "  - " << u.username << (u.esteAdmin?" [ADMIN]":"") << "\n";
            std::string usr=citesteString("\n  Username: ");
            if(cinema.stergeUtilizator(usr)) std::cout << "  [OK] Sters.\n";
            else std::cout << "  [!] Nu s-a putut sterge.\n";
            asteptaTasta();
        }
    }
}

#endif
