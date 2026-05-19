#define _CRT_SECURE_NO_WARNINGS
#include "mainwindow.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <map>
#include <algorithm>

#define C_BG    "#0a0a0f"
#define C_CARD  "#18181f"
#define C_BOR2  "#1e1e2a"
#define C_RED   "#e50914"
#define C_GOLD  "#f5a623"
#define C_BLUE  "#38bdf8"
#define C_TXT   "#f0f0f5"
#define C_MUT   "#8b8fa8"
#define C_DIM   "#50536a"
#define C_GRN   "#22c55e"

// ══════════════════════════════════════════════════════════════════════════════
// populateFilmDetail
// ══════════════════════════════════════════════════════════════════════════════
void MainWindow::populateFilmDetail(Film* f){
    rezFilm = f;
    detailTitle->setText(QString::fromStdString(f->getTitlu()));

    // Genre / rating / format row
    QString gr = QString::fromStdString(f->getGen());
    if(f->getDurata()>0) gr += "  •  " + QString::number(f->getDurata()) + " min";
    if(f->getEste3D()) gr += "  •  <span style='color:#38bdf8;'>3D</span>";
    if(f->getRating()>0){
        QString stars;
        int s=qRound(f->getRating()/2.0);
        for(int i=0;i<s;i++) stars+="★";
        for(int i=s;i<5;i++) stars+="☆";
        gr += "  •  <span style='color:#f5a623;'>" + stars + "</span>"
            + "  <span style='color:#8b8fa8;font-size:12px;'>"
            + QString::number(f->getRating(),'f',1) + "/10</span>";
    }
    detailGenreRating->setText(gr);

    // Description
    QString desc = QString::fromStdString(f->getDescriere());
    detailDesc->setText(desc.isEmpty() ? "Nicio descriere disponibilă." : desc);

    // Meta table
    auto row=[](const QString& k,const QString& v)->QString{
        return "<tr><td style='color:#8b8fa8;padding-right:20px;white-space:nowrap;'>"
               +k+"</td><td style='color:#f0f0f5;'>"+v+"</td></tr>";
    };
    QString meta="<table style='font-size:13px;line-height:2;'>";
    if(!f->getRegizor().empty())      meta+=row("REGIZOR",     QString::fromStdString(f->getRegizor()));
    if(!f->getDistributie().empty())  meta+=row("DISTRIBUȚIE",  QString::fromStdString(f->getDistributie()));
    if(!f->getDataPremiera().empty()) meta+=row("PREMIERĂ",     QString::fromStdString(f->getDataPremiera()));
    if(!f->getTara().empty())         meta+=row("PRODUCȚIE",    QString::fromStdString(f->getTara()));
    meta+=row("LIMBĂ",         QString::fromStdString(f->getLimba().empty()?"—":f->getLimba()));
    meta+=row("CLASIFICARE",   QString::fromStdString(f->getClasificare().empty()?"AG":f->getClasificare()));
    meta+=row("FORMAT",        f->getEste3D()?"3D":"2D");
    meta+="</table>";
    detailMeta->setText(meta);

    // Price
    double pret = f->getPretBaza() + (f->getEste3D()?10.0:0.0);
    detailPrice->setText("de la  " + QString::number((int)pret) + " RON");

    // Poster
    if(!f->getPosterPath().empty()){
        QPixmap px(QString::fromStdString(f->getPosterPath()));
        if(!px.isNull()){
            detailPoster->setPixmap(px.scaled(220,330,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation));
            detailPoster->setStyleSheet("border:1px solid #1e1e2a;border-radius:12px;");
        } else {
            detailPoster->setText("🎬"); detailPoster->setStyleSheet("background:#18181f;border:1px solid #1e1e2a;border-radius:12px;font-size:48px;color:#50536a;");
        }
    } else {
        detailPoster->setText("🎬"); detailPoster->setStyleSheet("background:#18181f;border:1px solid #1e1e2a;border-radius:12px;font-size:48px;color:#50536a;");
    }

    // Book button availability
    bool avail = (f->getStatus()==StatusFilm::RULAZA_ACUM);
    btnDetailBook->setEnabled(avail);
    btnDetailBook->setVisible(avail);
    detailUnavail->setText(avail ? "" : "⏳  În curând");
    // Fetch poster from TMDB if no local poster
    if(f->getPosterPath().empty()) fetchPoster(f);
}

// ══════════════════════════════════════════════════════════════════════════════
// refreshFilmMiniHeaders
// ══════════════════════════════════════════════════════════════════════════════
void MainWindow::refreshFilmMiniHeaders(){
    if(!rezFilm) return;
    QString t = "🎬  " + QString::fromStdString(rezFilm->getTitlu());
    if(rezFilm->getEste3D()) t += "  [3D]";
    if(miniSala) miniSala->setText(t);
    if(miniData) miniData->setText(t);
    if(miniOra)  miniOra ->setText(t);
    if(miniLoc)  miniLoc ->setText(t);
    if(miniConf) miniConf->setText(t);
}

// ══════════════════════════════════════════════════════════════════════════════
// Hero banner
// ══════════════════════════════════════════════════════════════════════════════
void MainWindow::refreshHeroBanner(){
    std::vector<Film*> active;
    for(auto& f:cinema.getFilme())
        if(f.getStatus()==StatusFilm::RULAZA_ACUM) active.push_back(const_cast<Film*>(&f));
    if(active.empty()){ heroFrame->hide(); return; }
    heroFrame->show();
    heroIdx = (heroIdx+1) % (int)active.size();
    Film* f = active[heroIdx];
    lblHeroTitle->setText(QString::fromStdString(f->getTitlu()));
    QString info = QString::fromStdString(f->getGen());
    if(f->getDurata()>0) info += "  •  "+QString::number(f->getDurata())+" min";
    if(f->getEste3D()) info += "  •  3D";
    if(f->getRating()>0){
        int s=qRound(f->getRating()/2.0); QString st;
        for(int i=0;i<s;i++) st+="★"; for(int i=s;i<5;i++) st+="☆";
        info += "  •  " + st;
    }
    lblHeroInfo->setText(info);
}

// ══════════════════════════════════════════════════════════════════════════════
// Upcoming strip
// ══════════════════════════════════════════════════════════════════════════════
void MainWindow::refreshUpcomingStrip(){
    // Clear old cards
    QLayout* old = upcomingContainer->layout();
    if(old){ QLayoutItem* item; while((item=old->takeAt(0))!=nullptr){delete item->widget();delete item;} delete old; }

    std::vector<Film*> upcoming;
    for(auto& f:cinema.getFilme())
        if(f.getStatus()==StatusFilm::IN_CURAND) upcoming.push_back(const_cast<Film*>(&f));

    upcomingSection->setVisible(!upcoming.empty());
    if(upcoming.empty()) return;

    auto* hb = new QHBoxLayout(upcomingContainer);
    hb->setContentsMargins(0,0,0,0); hb->setSpacing(12);
    for(auto* f:upcoming){
        auto* card=new QFrame; card->setFixedSize(160,88);
        card->setStyleSheet("QFrame{background:#16120a;border:1px solid #3a2a0a;border-radius:10px;}");
        auto* cv=new QVBoxLayout(card); cv->setContentsMargins(14,12,14,12); cv->setSpacing(4);
        auto* tl=new QLabel(QString::fromStdString(f->getTitlu()));
        tl->setStyleSheet("font-size:12px;font-weight:700;color:#f0f0f5;background:transparent;border:none;");
        tl->setWordWrap(true);
        auto* dl=new QLabel(QString::fromStdString(f->getDataPremiera().empty()?"În curând":f->getDataPremiera()));
        dl->setStyleSheet("font-size:11px;color:#f5a623;background:transparent;border:none;");
        cv->addWidget(tl); cv->addWidget(dl);
        hb->addWidget(card);
    }
    hb->addStretch();
}

// ══════════════════════════════════════════════════════════════════════════════
// Seat legend counts
// ══════════════════════════════════════════════════════════════════════════════
void MainWindow::updateSeatLegendCounts(){
    Sala* sala=cinema.getSalaById(rezSalaId); if(!sala) return;
    int std_=0,vip=0,cpl=0;
    for(int r=0;r<sala->getRanduri();r++) for(int c=0;c<sala->getColoane();c++){
        if(sala->esteOcupat(rezData,rezOra,r,c)) continue;
        switch(sala->getCategorieLocSeat(r,c)){
            case CategorieLocSeat::VIP:   vip++; break;
            case CategorieLocSeat::CUPLU: cpl++; break;
            default:                      std_++;break;
        }
    }
    if(lblLegStd) lblLegStd->setText("("+QString::number(std_)+" libere)");
    if(lblLegVip) lblLegVip->setText("("+QString::number(vip)+" libere)");
    if(lblLegCpl) lblLegCpl->setText("("+QString::number(cpl)+" libere)");
}

// ══════════════════════════════════════════════════════════════════════════════
// refreshMainMenu
// ══════════════════════════════════════════════════════════════════════════════
void MainWindow::refreshMainMenu(){
    auto* u=cinema.getCurent(); if(!u) return;
    QString name=QString::fromStdString(u->username);
    lblNavUser->setText(u->esteAdmin ? name+" [ADMIN]" : name);
    lblWelcome->setText("Bun venit, <b>"+name+"</b>!");
    cardAdmin->setVisible(u->esteAdmin);
    // populate hero
    heroIdx=-1; refreshHeroBanner(); if(heroTimer && !heroTimer->isActive()) heroTimer->start();
    refreshUpcomingStrip();
    // genre combo for film page
    cbFilmGenre->blockSignals(true); cbFilmGenre->clear();
    cbFilmGenre->addItem("Toate genurile");
    std::vector<std::string> gens;
    for(auto& f:cinema.getFilme()) if(!f.getGen().empty()&&std::find(gens.begin(),gens.end(),f.getGen())==gens.end()) gens.push_back(f.getGen());
    std::sort(gens.begin(),gens.end());
    for(auto& g:gens) cbFilmGenre->addItem(QString::fromStdString(g));
    cbFilmGenre->blockSignals(false);
}

// ══════════════════════════════════════════════════════════════════════════════
// refreshSearchResults
// ══════════════════════════════════════════════════════════════════════════════
void MainWindow::refreshSearchResults(){
    std::string q=leSearch->text().toStdString();
    std::transform(q.begin(),q.end(),q.begin(),::tolower);
    QString selGen=cbFilmGenre->currentIndex()>0?cbFilmGenre->currentText():"";
    searchResults.clear(); lwFilms->clear();
    for(auto& f:cinema.getFilme()){
        if(f.getStatus()!=StatusFilm::RULAZA_ACUM) continue;
        if(!selGen.isEmpty()&&f.getGen()!=selGen.toStdString()) continue;
        if(filmFilter3D==0&&f.getEste3D()) continue;
        if(filmFilter3D==1&&!f.getEste3D()) continue;
        std::string tlc=f.getTitlu(); std::transform(tlc.begin(),tlc.end(),tlc.begin(),::tolower);
        if(!q.empty()&&tlc.find(q)==std::string::npos) continue;
        searchResults.push_back(const_cast<Film*>(&f));
        auto* item=new QListWidgetItem(lwFilms); item->setSizeHint(QSize(0,76));
        auto* card=new QWidget; card->setStyleSheet("background:transparent;");
        auto* hb=new QHBoxLayout(card); hb->setContentsMargins(4,8,20,8); hb->setSpacing(0);
        auto* bar=new QFrame; bar->setFixedWidth(3); bar->setStyleSheet("background:#e50914;border-radius:2px;");
        hb->addWidget(bar); hb->addSpacing(14);
        auto* tc=new QVBoxLayout; tc->setSpacing(4);
        auto* tl=new QLabel(QString::fromStdString(f.getTitlu()));
        tl->setStyleSheet("font-size:15px;font-weight:700;color:#f0f0f5;background:transparent;border:none;");
        tl->setAttribute(Qt::WA_TransparentForMouseEvents);
        QString info=QString::fromStdString(f.getGen());
        if(f.getDurata()>0) info+="  •  "+QString::number(f.getDurata())+" min";
        if(f.getRating()>0) info+="  •  ★ "+QString::number(f.getRating(),'f',1);
        auto* il=new QLabel(info); il->setStyleSheet("font-size:12px;color:#8b8fa8;background:transparent;border:none;");
        il->setAttribute(Qt::WA_TransparentForMouseEvents);
        tc->addWidget(tl); tc->addWidget(il); hb->addLayout(tc,1);
        auto* rc=new QVBoxLayout; rc->setSpacing(4); rc->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        auto* pl=new QLabel(QString::number((int)f.getPretBaza())+" RON");
        pl->setStyleSheet("font-size:15px;font-weight:700;color:#e50914;background:transparent;border:none;");
        pl->setAlignment(Qt::AlignRight); pl->setAttribute(Qt::WA_TransparentForMouseEvents); rc->addWidget(pl);
        if(f.getEste3D()){auto* b=new QLabel("3D");b->setStyleSheet("color:#38bdf8;font-size:11px;font-weight:700;background:transparent;border:none;");b->setAlignment(Qt::AlignRight);b->setAttribute(Qt::WA_TransparentForMouseEvents);rc->addWidget(b);}
        auto* rcw=new QWidget; rcw->setFixedWidth(82); rcw->setStyleSheet("background:transparent;"); rcw->setLayout(rc);
        hb->addWidget(rcw);
        lwFilms->setItemWidget(item,card);
    }
    filmListStack->setCurrentIndex(searchResults.empty()?1:0);
}

// ══════════════════════════════════════════════════════════════════════════════
// refreshMapPage / refreshCancelSeats
// ══════════════════════════════════════════════════════════════════════════════
void MainWindow::refreshMapPage(){
    int si=cbMapSala->currentIndex(),di=cbMapData->currentIndex(),oi=cbMapOra->currentIndex();
    if(si<0||di<0||oi<0) return;
    auto& sali=cinema.getSali(); if(si>=(int)sali.size()) return;
    auto zile=urmatoroarele7Zile(); if(di>=(int)zile.size()) return;
    auto& ore=cinema.getOre(); if(oi>=(int)ore.size()) return;
    mapWidget->load(cinema.getSalaById(sali[si].getId()),zile[di],ore[oi]);
}
void MainWindow::refreshCancelSeats(){
    int si=cbCancelSala->currentIndex(),di=cbCancelData->currentIndex(),oi=cbCancelOra->currentIndex();
    if(si<0||di<0||oi<0) return;
    auto& sali=cinema.getSali(); if(si>=(int)sali.size()) return;
    auto zile=urmatoroarele7Zile(); if(di>=(int)zile.size()) return;
    auto& ore=cinema.getOre(); if(oi>=(int)ore.size()) return;
    cancelWidget->load(cinema.getSalaById(sali[si].getId()),zile[di],ore[oi]);
    cancelWidget->setSelected(cancelRow,cancelCol);
}

// ══════════════════════════════════════════════════════════════════════════════
// Browse
// ══════════════════════════════════════════════════════════════════════════════
void MainWindow::refreshBrowse(){
    QString cur=cbGenuri->currentText();
    cbGenuri->blockSignals(true); cbGenuri->clear();
    std::vector<std::string> gens;
    for(auto& f:cinema.getFilme()) if(!f.getGen().empty()&&f.getGen()!="Necunoscut"&&std::find(gens.begin(),gens.end(),f.getGen())==gens.end()) gens.push_back(f.getGen());
    std::sort(gens.begin(),gens.end());
    for(auto& g:gens) cbGenuri->addItem(QString::fromStdString(g));
    int idx=cbGenuri->findText(cur); if(idx>=0) cbGenuri->setCurrentIndex(idx);
    cbGenuri->blockSignals(false);
    onGenChanged(cbGenuri->currentIndex());
}
void MainWindow::onGenChanged(int){
    std::string gen=cbGenuri->currentText().toStdString();
    lwBrowseFilms->blockSignals(true); lwBrowseFilms->clear();
    for(auto& f:cinema.getFilme()){
        if(f.getGen()!=gen) continue;
        if(browseActiveOnly&&f.getStatus()!=StatusFilm::RULAZA_ACUM) continue;
        if(browse3D==0&&f.getEste3D()) continue;
        if(browse3D==1&&!f.getEste3D()) continue;
        QString lbl=QString::fromStdString(f.getTitlu());
        if(f.getEste3D()) lbl+="  [3D]";
        if(f.getStatus()==StatusFilm::IN_CURAND) lbl+="  [în curând]";
        lwBrowseFilms->addItem(lbl);
    }
    lwBrowseFilms->blockSignals(false);
    if(lwBrowseFilms->count()>0){lwBrowseFilms->setCurrentRow(0);updateBrowseDetail(0);}
    else lblBrowseDetail->setText("Niciun film în acest gen.");
}
void MainWindow::updateBrowseDetail(int row){
    if(row<0){lblBrowseDetail->clear();return;}
    std::string gen=cbGenuri->currentText().toStdString();
    std::vector<Film*> vis;
    for(auto& f:cinema.getFilme()){
        if(f.getGen()!=gen) continue;
        if(browseActiveOnly&&f.getStatus()!=StatusFilm::RULAZA_ACUM) continue;
        if(browse3D==0&&f.getEste3D()) continue;
        if(browse3D==1&&!f.getEste3D()) continue;
        vis.push_back(const_cast<Film*>(&f));
    }
    if(row>=(int)vis.size()) return;
    Film* f=vis[row];
    std::ostringstream oss;
    oss<<"<span style='font-size:15px;font-weight:700;color:#f0f0f5;'>"<<f->getTitlu()<<"</span><br><br>"
       <<"<table style='font-size:13px;line-height:2;'>"
       <<"<tr><td style='color:#8b8fa8;width:70px;'>Gen</td><td>"<<f->getGen()<<"</td></tr>"
       <<"<tr><td style='color:#8b8fa8;'>Durată</td><td>"<<f->getDurata()<<" min</td></tr>"
       <<"<tr><td style='color:#8b8fa8;'>Format</td><td>"<<(f->getEste3D()?"3D":"2D")<<"</td></tr>";
    if(f->getRating()>0) oss<<"<tr><td style='color:#8b8fa8;'>Rating</td><td style='color:#f5a623;'>★ "<<std::fixed<<std::setprecision(1)<<f->getRating()<<"/10</td></tr>";
    oss<<"<tr><td style='color:#8b8fa8;'>Preț</td><td style='color:#e50914;font-weight:700;'>"<<(int)f->getPretBaza()<<" RON"<<(f->getEste3D()?" + 10 RON":"")<<"</td></tr>"
       <<"<tr><td style='color:#8b8fa8;'>Status</td><td>"<<numeStatus(f->getStatus())<<"</td></tr>"
       <<"</table>";
    lblBrowseDetail->setText(QString::fromStdString(oss.str()));
}

// ══════════════════════════════════════════════════════════════════════════════
// Admin refresh helpers
// ══════════════════════════════════════════════════════════════════════════════
void MainWindow::refreshProfile(){
    auto*u=cinema.getCurent();if(!u)return;
    teHistory->clear();
    if(u->istoricRezervari.empty()) teHistory->setPlainText("(nicio rezervare înregistrată)");
    else{int i=1;for(auto&r:u->istoricRezervari)teHistory->append(QString::number(i++)+"  •  "+QString::fromStdString(r));}
}
void MainWindow::refreshAdminFilms(){
    lwAdminFilms->clear();
    for(auto&f:cinema.getFilme()){
        QString l=QString::fromStdString(f.getTitlu());
        if(f.getEste3D())l+=" [3D]";
        l+="  •  "+QString::fromStdString(f.getGen());
        l+="  •  "+QString::number((int)f.getPretBaza())+" RON";
        if(f.getRating()>0)l+="  •  ★"+QString::number(f.getRating(),'f',1);
        if(f.getStatus()==StatusFilm::IN_CURAND)l+="  [în curând]";
        lwAdminFilms->addItem(l);
    }
}
void MainWindow::refreshAdminOre(){ lwAdminOre->clear(); for(auto&o:cinema.getOre())lwAdminOre->addItem(QString::fromStdString(o)); }
void MainWindow::refreshAdminUsers(){ lwAdminUsers->clear(); for(auto&u:cinema.getUtilizatori()){QString l=QString::fromStdString(u.username);if(u.esteAdmin)l+="  [ADMIN]";lwAdminUsers->addItem(l);} }
void MainWindow::refreshAdminReservations(){
    teAllRes->clear();
    std::ifstream f("toate_rezervarile.txt"); if(!f.is_open()){teAllRes->setPlainText("(gol)");return;}
    int nr=0; std::string ln;
    while(std::getline(f,ln)){if(!ln.empty()&&ln.back()=='\r')ln.pop_back();if(!ln.empty())teAllRes->append(QString::number(++nr)+".  "+QString::fromStdString(ln));}
    if(!nr)teAllRes->setPlainText("(nicio rezervare)");
}
void MainWindow::refreshAdminReport(){
    teReport->clear();
    std::ifstream f("toate_rezervarile.txt"); if(!f.is_open()){teReport->setPlainText("(gol)");return;}
    std::map<std::string,int>bF,bS,bO; std::map<std::string,double>vF;
    double tot=0;int totB=0;std::string ln;
    while(std::getline(f,ln)){
        if(!ln.empty()&&ln.back()=='\r')ln.pop_back(); if(ln.empty())continue;
        auto ex=[&](const std::string&tag,const std::string&end)->std::string{
            size_t p=ln.find(tag);if(p==std::string::npos)return"";
            p+=tag.size();size_t e=ln.find(end,p);return e==std::string::npos?ln.substr(p):ln.substr(p,e-p);
        };
        std::string film=ex("Film: "," | "),sala=ex("Sala: "," | "),ora=ex("Ora: "," | "),sp=ex("Pret: "," RON");
        if(film.empty())continue;
        double pret=0;try{pret=std::stod(sp);}catch(...){}
        bF[film]++;vF[film]+=pret;bS[sala]++;bO[ora]++;totB++;tot+=pret;
    }
    std::ostringstream oss;
    oss<<"=== RAPORT VÂNZĂRI ===\n\n── Pe film ──────────────────────────────────────────────────\n";
    for(auto&[k,v]:bF)oss<<"  "<<std::left<<std::setw(42)<<k<<std::setw(4)<<v<<" bilete  │  "<<std::fixed<<std::setprecision(2)<<vF[k]<<" RON\n";
    oss<<"\n── Pe sală ──────────────────────────────────────────────────\n";
    for(auto&[k,v]:bS)oss<<"  "<<std::left<<std::setw(30)<<k<<v<<" bilete\n";
    oss<<"\n── Pe oră ───────────────────────────────────────────────────\n";
    for(auto&[k,v]:bO)oss<<"  "<<k<<"  :  "<<v<<" bilete\n";
    oss<<"\n═════════════════════════════════════════════════════════════\n";
    oss<<"  TOTAL  "<<totB<<" bilete  │  "<<std::fixed<<std::setprecision(2)<<tot<<" RON\n";
    teReport->setPlainText(QString::fromStdString(oss.str()));
}

// ══════════════════════════════════════════════════════════════════════════════
// Auth
// ══════════════════════════════════════════════════════════════════════════════
void MainWindow::onLogin(){
    if(cinema.login(leUser->text().toStdString(),lePass->text().toStdString())){
        leUser->clear();lePass->clear();refreshMainMenu();goTo(P_MAIN);
    } else QMessageBox::warning(this,"Eroare","Username sau parolă incorectă.");
}
void MainWindow::onRegister(){
    bool ok;
    QString usr=QInputDialog::getText(this,"Înregistrare","Username nou:",QLineEdit::Normal,"",&ok);if(!ok||usr.isEmpty())return;
    QString par=QInputDialog::getText(this,"Înregistrare","Parolă:",QLineEdit::Password,"",&ok);if(!ok||par.isEmpty())return;
    QString par2=QInputDialog::getText(this,"Înregistrare","Confirmă parola:",QLineEdit::Password,"",&ok);if(!ok)return;
    if(par!=par2){QMessageBox::warning(this,"","Parolele nu coincid!");return;}
    if(cinema.inregistrare(usr.toStdString(),par.toStdString())){
        QMessageBox::information(this,"","Cont creat! Bun venit, "+usr+"!");refreshMainMenu();goTo(P_MAIN);
    } else QMessageBox::warning(this,"","Username deja existent.");
}
void MainWindow::onLogout(){ cinema.logout();heroTimer->stop();leUser->clear();lePass->clear();goTo(P_LOGIN); }

// ══════════════════════════════════════════════════════════════════════════════
// Main menu navigation
// ══════════════════════════════════════════════════════════════════════════════
void MainWindow::onNewReservation(){
    rezFilm=nullptr;rezSalaId=-1;rezRow=-1;rezCol=-1;
    filmFilter3D=-1; leSearch->clear();
    cbFilmGenre->setCurrentIndex(0);
    refreshSearchResults();
    goTo(P_RESERVE_FILM);
}
void MainWindow::onBrowseGenuri(){
    browse3D=-1;browseActiveOnly=false;
    btnBrowseFilter3D->setText("Format: Toate"); btnBrowseFilterActive->setText("Status: Toate");
    refreshBrowse();goTo(P_BROWSE);
}
void MainWindow::onHartaSala(){
    cbMapSala->clear();cbMapData->clear();cbMapOra->clear();
    for(auto&s:cinema.getSali())cbMapSala->addItem(QString::fromStdString(s.getNume()));
    auto zile=urmatoroarele7Zile();Data azi=dataAzi();
    for(auto&z:zile){QString l=QString::fromStdString(numeZiSaptamana(z))+"  –  "+QString::fromStdString(z.toString());if(z==azi)l+="  ★";cbMapData->addItem(l);}
    for(auto&o:cinema.getOre())cbMapOra->addItem(QString::fromStdString(o));
    refreshMapPage();goTo(P_MAP);
}
void MainWindow::onAnulareRezervare(){
    cbCancelSala->clear();cbCancelData->clear();cbCancelOra->clear();cancelRow=-1;cancelCol=-1;
    for(auto&s:cinema.getSali())cbCancelSala->addItem(QString::fromStdString(s.getNume()));
    auto zile=urmatoroarele7Zile();Data azi=dataAzi();
    for(auto&z:zile){QString l=QString::fromStdString(numeZiSaptamana(z))+"  –  "+QString::fromStdString(z.toString());if(z==azi)l+="  ★";cbCancelData->addItem(l);}
    for(auto&o:cinema.getOre())cbCancelOra->addItem(QString::fromStdString(o));
    refreshCancelSeats();goTo(P_CANCEL);
}
void MainWindow::onProfilMeu(){ refreshProfile();goTo(P_PROFILE); }
void MainWindow::onAdminPanel(){ adminTabs->setCurrentIndex(0);refreshAdminReservations();goTo(P_ADMIN); }

// ══════════════════════════════════════════════════════════════════════════════
// Reservation flow
// ══════════════════════════════════════════════════════════════════════════════
void MainWindow::onFilmSelected(){
    int idx=lwFilms->currentRow();
    if(idx<0||(int)searchResults.size()<=idx){QMessageBox::warning(this,"","Selectează un film!");return;}
    detailFromPage=P_RESERVE_FILM;
    populateFilmDetail(searchResults[idx]);
    refreshFilmMiniHeaders();
    goTo(P_FILM_DETAIL);
}
void MainWindow::onProceedFromDetail(){
    if(!rezFilm)return;
    lwSali->clear();
    for(auto&s:cinema.getSali()){
        QString l=QString::fromStdString(s.getNume());
        l+="   ("+QString::number(s.getRanduri())+" rânduri × "+QString::number(s.getColoane())+" locuri)";
        lwSali->addItem(l);
    }
    goTo(P_RESERVE_SALA);
}
void MainWindow::onSalaSelected(){
    int idx=lwSali->currentRow();
    if(idx<0||(int)cinema.getSali().size()<=idx){QMessageBox::warning(this,"","Selectează o sală!");return;}
    rezSalaId=cinema.getSali()[idx].getId();goTo(P_RESERVE_DATA);
}
void MainWindow::onDataSelected(){
    int idx=lwData->currentRow();
    if(idx<0){QMessageBox::warning(this,"","Selectează o dată!");return;}
    rezData=urmatoroarele7Zile()[idx];
    lwOra->clear();
    Sala*sala=cinema.getSalaById(rezSalaId);
    for(auto&o:cinema.getOre()){
        int lib=sala?sala->locuriLibere(rezData,o):0;
        lwOra->addItem(QString::fromStdString(o)+"   –   "+QString::number(lib)+" locuri libere");
    }
    goTo(P_RESERVE_ORA);
}
void MainWindow::onOraSelected(){
    int idx=lwOra->currentRow();
    if(idx<0){QMessageBox::warning(this,"","Selectează o oră!");return;}
    rezOra=cinema.getOre()[idx];
    Sala*sala=cinema.getSalaById(rezSalaId);
    rezRow=-1;rezCol=-1;
    seatMapWidget->load(sala,rezData,rezOra);
    if(rezFilm) seatMapWidget->setPretBaza(rezFilm->getPretBaza()+(rezFilm->getEste3D()?10.0:0.0));
    lblSeatInfo->setText("Selectează un loc liber");
    updateSeatLegendCounts();
    goTo(P_RESERVE_LOC);
}
void MainWindow::onLocSelected(int row,int col){
    Sala*sala=cinema.getSalaById(rezSalaId);if(!sala)return;
    rezRow=row;rezCol=col;seatMapWidget->setSelected(row,col);
    bool occ=sala->esteOcupat(rezData,rezOra,row,col);
    CategorieLocSeat cat=sala->getCategorieLocSeat(row,col);
    double pret=(rezFilm->getPretBaza()+(rezFilm->getEste3D()?10.0:0.0))*multiplicatorCategorie(cat);
    QString info=QString("R%1  L%2   [%3]   %4 RON").arg(row+1).arg(col+1).arg(QString::fromStdString(numeCategorie(cat))).arg(pret,0,'f',2);
    if(occ)info+="   ⛔ Loc ocupat";
    lblSeatInfo->setText(info);
}
void MainWindow::onConfirmReservation(){
    std::string email=leEmail->text().trimmed().toStdString();
    cinema.proceseazaRezervare(rezSalaId,rezData,rezRow,rezCol,rezFilm,rezOra,email);
    // populate confirm-done page
    lblDoneRef->setText(lblBookingRef->text()
        .remove(QRegularExpression("<[^>]*>")).trimmed()
        .replace("Referință rezervare:","").trimmed());
    Sala* sala=cinema.getSalaById(rezSalaId);
    CategorieLocSeat cat=sala?sala->getCategorieLocSeat(rezRow,rezCol):CategorieLocSeat::STANDARD;
    double pret=(rezFilm->getPretBaza()+(rezFilm->getEste3D()?10.0:0.0))*multiplicatorCategorie(cat);
    std::ostringstream oss;
    oss<<"<table style='font-size:13px;line-height:2;'>";
    oss<<"<tr><td style='color:#8b8fa8;width:80px;'>Film</td><td><b>"<<rezFilm->getTitlu()<<"</b></td></tr>";
    oss<<"<tr><td style='color:#8b8fa8;'>Sală</td><td>"<<(sala?sala->getNume():"—")<<"</td></tr>";
    oss<<"<tr><td style='color:#8b8fa8;'>Data</td><td>"<<rezData.toString()<<"</td></tr>";
    oss<<"<tr><td style='color:#8b8fa8;'>Ora</td><td>"<<rezOra<<"</td></tr>";
    oss<<"<tr><td style='color:#8b8fa8;'>Loc</td><td>R"<<rezRow+1<<" L"<<rezCol+1<<" ["<<numeCategorie(cat)<<"]</td></tr>";
    oss<<"<tr><td style='color:#8b8fa8;'>Preț</td><td style='color:#e50914;font-weight:700;font-size:16px;'>"<<std::fixed<<std::setprecision(2)<<pret<<" RON</td></tr>";
    if(!email.empty()) oss<<"<tr><td style='color:#8b8fa8;'>Email</td><td>"<<email<<"</td></tr>";
    oss<<"</table>";
    lblDoneSummary->setText(QString::fromStdString(oss.str()));
    goTo(P_CONFIRM_DONE);
}
void MainWindow::onCancelReservation(){
    if(cancelRow<0){QMessageBox::warning(this,"","Selectează un loc ocupat!");return;}
    int si=cbCancelSala->currentIndex(),di=cbCancelData->currentIndex(),oi=cbCancelOra->currentIndex();
    auto&sali=cinema.getSali();auto zile=urmatoroarele7Zile();auto&ore=cinema.getOre();
    if(si<0||di<0||oi<0)return;
    Sala*sala=cinema.getSalaById(sali[si].getId());
    Data data=zile[di];std::string ora=ore[oi];
    if(!sala||!sala->esteOcupat(data,ora,cancelRow,cancelCol)){QMessageBox::warning(this,"","Locul nu este ocupat!");return;}
    if(QMessageBox::question(this,"Confirmare",QString("Anulezi rezervarea R%1 L%2?").arg(cancelRow+1).arg(cancelCol+1))!=QMessageBox::Yes)return;
    cinema.anuleazaRezervare(sala->getId(),data,ora,cancelRow,cancelCol);
    cancelRow=-1;cancelCol=-1;refreshCancelSeats();
    QMessageBox::information(this,"","Rezervare anulată cu succes!");
}

// ══════════════════════════════════════════════════════════════════════════════
// Browse reserve / detail
// ══════════════════════════════════════════════════════════════════════════════
void MainWindow::onBrowseReserve(){
    int row=lwBrowseFilms->currentRow();if(row<0){QMessageBox::warning(this,"","Selectează un film!");return;}
    std::string gen=cbGenuri->currentText().toStdString();
    std::vector<Film*>vis;
    for(auto&f:cinema.getFilme()){if(f.getGen()!=gen)continue;if(browseActiveOnly&&f.getStatus()!=StatusFilm::RULAZA_ACUM)continue;if(browse3D==0&&f.getEste3D())continue;if(browse3D==1&&!f.getEste3D())continue;vis.push_back(const_cast<Film*>(&f));}
    if(row>=(int)vis.size())return;
    Film*f=vis[row];
    if(f->getStatus()==StatusFilm::IN_CURAND){QMessageBox::information(this,"","Filmul nu rulează încă.");return;}
    detailFromPage=P_BROWSE;populateFilmDetail(f);refreshFilmMiniHeaders();goTo(P_FILM_DETAIL);
}

// ══════════════════════════════════════════════════════════════════════════════
// Admin – film dialog helper
// ══════════════════════════════════════════════════════════════════════════════
bool MainWindow::showFilmDialog(Film& out, const Film* ex){
    auto* dlg=new QDialog(this); dlg->setWindowTitle(ex?"Editează Film":"Adaugă Film");
    dlg->setFixedWidth(520);
    dlg->setStyleSheet("QDialog{background:#141418;}QLabel{color:#e5e5f0;font-size:13px;}"
                       "QLineEdit,QTextEdit,QDoubleSpinBox,QSpinBox,QComboBox{background:#1e1e28;color:#f0f0f5;border:1.5px solid #2a2a38;border-radius:7px;padding:7px 12px;font-size:13px;}"
                       "QCheckBox{color:#f0f0f5;font-size:13px;}"
                       "QPushButton{background:#e50914;color:white;border:none;border-radius:7px;padding:9px 24px;font-size:13px;font-weight:700;min-width:80px;}"
                       "QPushButton:hover{background:#ff2233;}"
                       "QPushButton[flat=true]{background:#1e1e28;color:#8b8fa8;border:1.5px solid #2a2a38;}"
                       "QPushButton[flat=true]:hover{background:#28283a;color:#f0f0f5;}");
    auto* vb=new QVBoxLayout(dlg); vb->setContentsMargins(28,24,28,24); vb->setSpacing(0);
    auto* fl=new QFormLayout; fl->setSpacing(10); fl->setLabelAlignment(Qt::AlignRight);

    auto mkIn=[](const QString&ph,const QString&val="")->QLineEdit*{auto*e=new QLineEdit;e->setPlaceholderText(ph);e->setText(val);return e;};
    auto* eTitlu=mkIn("ex: Avengers", ex?QString::fromStdString(ex->getTitlu()):"");
    auto* eGen  =mkIn("ex: Acțiune",  ex?QString::fromStdString(ex->getGen()):"");
    auto* eReg  =mkIn("ex: James Cameron", ex?QString::fromStdString(ex->getRegizor()):"");
    auto* eDist =mkIn("ex: Actor A, Actor B", ex?QString::fromStdString(ex->getDistributie()):"");
    auto* eData =mkIn("ex: 05 iunie 2026",   ex?QString::fromStdString(ex->getDataPremiera()):"");
    auto* eTara =mkIn("ex: SUA 2026",         ex?QString::fromStdString(ex->getTara()):"");
    auto* eLimb =mkIn("ex: EN",               ex?QString::fromStdString(ex->getLimba()):"EN");
    auto* eClas =mkIn("ex: AG / AP12 / N15",  ex?QString::fromStdString(ex->getClasificare()):"AG");
    auto* ePost =mkIn("ex: posters/film.jpg", ex?QString::fromStdString(ex->getPosterPath()):"");
    auto* eDesc =new QTextEdit; eDesc->setPlaceholderText("Descriere film…"); eDesc->setFixedHeight(80);
    if(ex) eDesc->setPlainText(QString::fromStdString(ex->getDescriere()));

    auto* sDur=new QSpinBox; sDur->setRange(1,360); sDur->setSuffix(" min"); sDur->setValue(ex?ex->getDurata():90);
    auto* sPret=new QDoubleSpinBox; sPret->setRange(1,999); sPret->setSuffix(" RON"); sPret->setDecimals(2); sPret->setValue(ex?ex->getPretBaza():25);
    auto* sRat=new QDoubleSpinBox; sRat->setRange(0,10); sRat->setSingleStep(0.1); sRat->setDecimals(1); sRat->setValue(ex?ex->getRating():0);
    auto* cbStat=new QComboBox; cbStat->addItems({"Rulează acum","În curând"});
    if(ex&&ex->getStatus()==StatusFilm::IN_CURAND) cbStat->setCurrentIndex(1);
    auto* chk3D=new QCheckBox("Film 3D"); chk3D->setChecked(ex&&ex->getEste3D());

    fl->addRow("Titlu *",     eTitlu);
    fl->addRow("Gen *",       eGen);
    fl->addRow("Durată",      sDur);
    fl->addRow("Preț bază",   sPret);
    fl->addRow("Rating /10",  sRat);
    fl->addRow("Status",      cbStat);
    fl->addRow("",            chk3D);
    fl->addRow("Descriere",   eDesc);
    fl->addRow("Regizor",     eReg);
    fl->addRow("Distribuție", eDist);
    fl->addRow("Premieră",    eData);
    fl->addRow("Producție",   eTara);
    fl->addRow("Limbă",       eLimb);
    fl->addRow("Clasificare", eClas);
    fl->addRow("Poster path", ePost);
    vb->addLayout(fl); vb->addSpacing(20);

    auto* bRow=new QHBoxLayout; bRow->addStretch();
    auto* bOk=new QPushButton(ex?"Salvează":"Adaugă"); bOk->setDefault(true);
    auto* bCan=new QPushButton("Anulează"); bCan->setProperty("flat",true);
    bRow->addWidget(bCan); bRow->addWidget(bOk); vb->addLayout(bRow);
    connect(bOk,&QPushButton::clicked,dlg,&QDialog::accept);
    connect(bCan,&QPushButton::clicked,dlg,&QDialog::reject);

    if(dlg->exec()!=QDialog::Accepted){delete dlg;return false;}
    if(eTitlu->text().trimmed().isEmpty()){QMessageBox::warning(this,"","Titlul este obligatoriu!");delete dlg;return false;}
    out=Film(eTitlu->text().trimmed().toStdString(),
             chk3D->isChecked(), sPret->value(),
             eGen->text().trimmed().toStdString(), sDur->value(),
             cbStat->currentIndex()==1?StatusFilm::IN_CURAND:StatusFilm::RULAZA_ACUM,
             sRat->value(),
             eDesc->toPlainText().toStdString(),
             eReg->text().trimmed().toStdString(),
             eDist->text().trimmed().toStdString(),
             eData->text().trimmed().toStdString(),
             eTara->text().trimmed().toStdString(),
             eLimb->text().trimmed().toStdString(),
             eClas->text().trimmed().toStdString(),
             ePost->text().trimmed().toStdString());
    delete dlg; return true;
}

// ══════════════════════════════════════════════════════════════════════════════
// Admin slots
// ══════════════════════════════════════════════════════════════════════════════
void MainWindow::onAdminAddFilm(){
    Film f("",false,25); if(!showFilmDialog(f)) return;
    cinema.adaugaFilmManual(f.getTitlu(),f.getEste3D(),f.getPretBaza(),f.getGen(),f.getDurata(),f.getStatus(),
        f.getRating(),f.getDescriere(),f.getRegizor(),f.getDistributie(),f.getDataPremiera(),
        f.getTara(),f.getLimba(),f.getClasificare(),f.getPosterPath());
    refreshAdminFilms(); refreshUpcomingStrip();
    QMessageBox::information(this,"","Film adăugat cu succes!");
}
void MainWindow::onAdminEditFilm(){
    int idx=lwAdminFilms->currentRow();if(idx<0){QMessageBox::warning(this,"","Selectează un film!");return;}
    auto&filme=cinema.getFilme();if(idx>=(int)filme.size())return;
    Film f=filme[idx]; if(!showFilmDialog(f,&filme[idx]))return;
    cinema.editeazaFilm(idx,f.getTitlu(),f.getEste3D(),f.getPretBaza(),f.getGen(),f.getDurata(),f.getStatus(),
        f.getRating(),f.getDescriere(),f.getRegizor(),f.getDistributie(),f.getDataPremiera(),
        f.getTara(),f.getLimba(),f.getClasificare(),f.getPosterPath());
    refreshAdminFilms(); refreshUpcomingStrip();
}
void MainWindow::onAdminDeleteFilm(){
    int idx=lwAdminFilms->currentRow();if(idx<0){QMessageBox::warning(this,"","Selectează un film!");return;}
    auto&filme=cinema.getFilme();
    if(QMessageBox::question(this,"Confirmare","Ștergi '"+QString::fromStdString(filme[idx].getTitlu())+"'?")!=QMessageBox::Yes)return;
    cinema.stergeFilm(idx);refreshAdminFilms();refreshUpcomingStrip();
}
void MainWindow::onAdminAddOra(){
    bool ok;
    QString ora=QInputDialog::getText(this,"Adaugă Oră","Ora (HH:MM):",QLineEdit::Normal,"",&ok);
    if(!ok||ora.isEmpty())return;
    if(ora.length()!=5||ora[2]!=':'){QMessageBox::warning(this,"","Format invalid! Folosește HH:MM");return;}
    cinema.adaugaOra(ora.toStdString());refreshAdminOre();
}
void MainWindow::onAdminDeleteOra(){
    int idx=lwAdminOre->currentRow();if(idx<0){QMessageBox::warning(this,"","Selectează o oră!");return;}
    if(QMessageBox::question(this,"Confirmare","Ștergi ora "+QString::fromStdString(cinema.getOre()[idx])+"?")!=QMessageBox::Yes)return;
    cinema.stergeOra(idx);refreshAdminOre();
}
void MainWindow::onAdminDeleteUser(){
    int idx=lwAdminUsers->currentRow();if(idx<0){QMessageBox::warning(this,"","Selectează un utilizator!");return;}
    auto&users=cinema.getUtilizatori();if(idx>=(int)users.size())return;
    QString usr=QString::fromStdString(users[idx].username);
    if(usr=="admin"){QMessageBox::warning(this,"","Nu poți șterge adminul!");return;}
    if(QMessageBox::question(this,"Confirmare","Ștergi utilizatorul '"+usr+"'?")!=QMessageBox::Yes)return;
    cinema.stergeUtilizator(usr.toStdString());refreshAdminUsers();
}

// ══════════════════════════════════════════════════════════════════════════════
// fetchPoster  –  loads movie poster from TMDB
// ══════════════════════════════════════════════════════════════════════════════
void MainWindow::fetchPoster(Film* f){
    if(!f || QString(TMDB_API_KEY).isEmpty()) return;

    detailPoster->setText("⏳");
    detailPoster->setStyleSheet("background:#18181f;border:1px solid #1e1e2a;"
                                "border-radius:12px;font-size:32px;color:#50536a;");

    QUrl url("https://api.themoviedb.org/3/search/movie");
    QUrlQuery q;
    q.addQueryItem("api_key",  TMDB_API_KEY);
    q.addQueryItem("query",    QString::fromStdString(f->getTitlu()));
    q.addQueryItem("language", "en-US");
    url.setQuery(q);

    auto* reply = netManager->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        reply->deleteLater();
        if(reply->error() != QNetworkReply::NoError){
            detailPoster->setText("🎬");
            return;
        }
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonArray results = doc.object()["results"].toArray();
        if(results.isEmpty()){ detailPoster->setText("🎬"); return; }

        QString posterPath = results[0].toObject()["poster_path"].toString();
        if(posterPath.isEmpty()){ detailPoster->setText("🎬"); return; }

        QUrl imgUrl("https://image.tmdb.org/t/p/w500" + posterPath);
        auto* imgReply = netManager->get(QNetworkRequest(imgUrl));
        connect(imgReply, &QNetworkReply::finished, this, [this, imgReply](){
            imgReply->deleteLater();
            if(imgReply->error() != QNetworkReply::NoError){
                detailPoster->setText("🎬"); return;
            }
            QPixmap px;
            px.loadFromData(imgReply->readAll());
            if(!px.isNull()){
                detailPoster->setPixmap(
                    px.scaled(220,330,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation));
                detailPoster->setStyleSheet(
                    "border:1px solid #1e1e2a;border-radius:12px;");
            } else {
                detailPoster->setText("🎬");
            }
        });
    });
}
