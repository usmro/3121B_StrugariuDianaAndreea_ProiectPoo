#include "mainwindow.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <map>
#include <set>
#include <algorithm>
#include <QFile>
#include <QDir>

#define C_BG   "#0a0a0f"
#define C_CARD "#18181f"
#define C_BOR2 "#1e1e2a"
#define C_RED  "#e50914"
#define C_GOLD "#f5a623"
#define C_BLUE "#38bdf8"
#define C_TXT  "#f0f0f5"
#define C_MUT  "#8b8fa8"
#define C_DIM  "#50536a"
#define C_GRN  "#22c55e"


static std::vector<std::string> oredinProgram(Cinematograf& c, int salaId, const Data& d){
    std::vector<std::string> ore;
    auto* loc=c.getLocatieCurenta();if(!loc)return ore;
    for(auto& p:loc->program)
        if(p.salaId==salaId&&p.data==d){
            if(std::find(ore.begin(),ore.end(),p.ora)==ore.end())
                ore.push_back(p.ora);
        }
    std::sort(ore.begin(),ore.end());
    return ore;
}


void MainWindow::showToast(const QString& msg, bool isError){
    if(!toastLabel){
        toastLabel=new QLabel(this);
        toastLabel->setAlignment(Qt::AlignCenter);
        toastLabel->setFixedHeight(42);
        toastTimer=new QTimer(this);
        toastTimer->setSingleShot(true);
        connect(toastTimer,&QTimer::timeout,toastLabel,&QLabel::hide);
    }
    toastLabel->setText(msg);
    toastLabel->setStyleSheet(isError
        ?"background:#7f1d1d;color:white;border-radius:8px;font-size:13px;padding:0 20px;font-weight:600;"
        :"background:#166534;color:white;border-radius:8px;font-size:13px;padding:0 20px;font-weight:600;");
    int w=qMin(420,qMax(240,(int)msg.length()*9));
    toastLabel->setFixedWidth(w);
    toastLabel->move((width()-w)/2, height()-62);
    toastLabel->show();toastLabel->raise();
    toastTimer->start(2500);
}


void MainWindow::refreshMapOra(){
    int si=cbMapSala->currentIndex(),di=cbMapData->currentIndex();
    if(si<0||di<0) return;
    auto* sali=cinema.getSaliCurente();
    if(!sali||si>=(int)sali->size()) return;
    auto zile=urmatoroarele7Zile();
    if(di>=(int)zile.size()) return;
    int salaId=(*sali)[si].getId();
    auto ore=oredinProgram(cinema,salaId,zile[di]);
    cbMapOra->blockSignals(true);
    cbMapOra->clear();
    for(auto& o:ore) cbMapOra->addItem(QString::fromStdString(o));
    cbMapOra->blockSignals(false);
    refreshMapPage();
}
void MainWindow::refreshCancelOra(){
    int si=cbCancelSala->currentIndex(),di=cbCancelData->currentIndex();
    if(si<0||di<0) return;
    auto* sali=cinema.getSaliCurente();
    if(!sali||si>=(int)sali->size()) return;
    auto zile=urmatoroarele7Zile();
    if(di>=(int)zile.size()) return;
    int salaId=(*sali)[si].getId();
    auto ore=oredinProgram(cinema,salaId,zile[di]);
    cbCancelOra->blockSignals(true);
    cbCancelOra->clear();
    for(auto& o:ore) cbCancelOra->addItem(QString::fromStdString(o));
    cbCancelOra->blockSignals(false);
    refreshCancelSeats();
}


void MainWindow::onLocationDetected(){
    lblDetectedCity->setText("Se detectează locația...");
    btnDetectLocation->setEnabled(false);
    auto* reply=netManager->get(QNetworkRequest(QUrl("http://ip-api.com/json/?fields=city,regionName,status")));
    connect(reply,&QNetworkReply::finished,this,[this,reply]{
        reply->deleteLater();
        btnDetectLocation->setEnabled(true);
        QString city="Suceava"; 

        if(reply->error()==QNetworkReply::NoError){
            QJsonObject obj=QJsonDocument::fromJson(reply->readAll()).object();
            if(obj["status"].toString()=="success")
                city=obj["city"].toString();
        }
        detectedCity=city;
        refreshLocationPage(city);
    });
}

void MainWindow::refreshLocationPage(const QString& city){
    lblDetectedCity->setText("Locație detectată: <b style='color:#22c55e;'>"+city+"</b>"
                             " · Se afișează cinematografele din zona ta");
    lblDetectedCity->setTextFormat(Qt::RichText);

    

    QLayout* old=locationContainer->layout();
    if(old){QLayoutItem* item;while((item=old->takeAt(0))!=nullptr){delete item->widget();delete item;}delete old;}
    auto* vb=new QVBoxLayout(locationContainer);vb->setContentsMargins(0,0,0,0);vb->setSpacing(10);

    auto locs=cinema.getLocatiiPentruOras(city.toStdString());

    

    QString lastCity;
    for(auto* loc:locs){
        QString locCity=QString::fromStdString(loc->oras);
        if(locCity!=lastCity){
            QString isDetected=locCity.toLower().contains(city.toLower())?" 📍":"";
            auto* cityHdr=new QLabel(locCity+isDetected);
            cityHdr->setStyleSheet(locCity==lastCity||lastCity.isEmpty()
                ?"font-size:16px;font-weight:800;color:#22c55e;margin-top:8px;"
                :"font-size:14px;font-weight:700;color:#8b8fa8;margin-top:16px;");
            if(lastCity.isEmpty())
                cityHdr->setStyleSheet("font-size:16px;font-weight:800;color:#22c55e;margin-top:0;");
            vb->addWidget(cityHdr);
            lastCity=locCity;
        }
        bool isCurrent=locCity.toLower().contains(city.toLower());
        auto* card=new LocationCard(*loc,isCurrent,locationContainer);
        connect(card,&LocationCard::clicked,this,&MainWindow::onLocationSelected);
        vb->addWidget(card);
    }

    if(locs.empty()){
        auto* noL=new QLabel("Nu s-au găsit cinematografe în zona ta.\nSe afișează toate locațiile.");
        noL->setStyleSheet("font-size:14px;color:#8b8fa8;");vb->addWidget(noL);
        

        for(auto& loc:cinema.getLocatii()){
            auto* card=new LocationCard(loc,false,locationContainer);
            connect(card,&LocationCard::clicked,this,&MainWindow::onLocationSelected);
            vb->addWidget(card);
        }
    }
    vb->addStretch();
}

void MainWindow::onLocationSelected(const QString& id){
    cinema.setLocatieCurenta(id.toStdString());
    cinema.incarcaRezervari();
    cinema.finalizeazaLocatie();
    auto* loc=cinema.getLocatieCurenta();
    if(!loc) return;
    QString tierStr = loc->tier==CinemaTier::PREMIUM ? " ⭐" : loc->tier==CinemaTier::BASIC ? " 🎟" : "";
    lblNavLocation->setText("📍 "+QString::fromStdString(loc->oras)+" · "+QString::fromStdString(loc->nume)+tierStr);
    refreshMainMenu();
    goTo(P_MAIN);
}


void MainWindow::populateFilmDetail(Film* f){
    rezFilm=f;
    detailTitle->setText(QString::fromStdString(f->getTitlu()));
    QString gr=QString::fromStdString(f->getGen());
    if(f->getDurata()>0) gr+="  •  "+QString::number(f->getDurata())+" min";
    if(f->getEste3D()) gr+="  •  <span style='color:#38bdf8;'>3D</span>";
    if(f->getRating()>0){
        QString stars;int s=qRound(f->getRating()/2.0);
        for(int i=0;i<s;i++)stars+="★";for(int i=s;i<5;i++)stars+="☆";
        gr+="  •  <span style='color:#f5a623;'>"+stars+"</span>"
           +"  <span style='color:#8b8fa8;font-size:12px;'>"+QString::number(f->getRating(),'f',1)+"/10</span>";
    }
    detailGenreRating->setText(gr);
    QString desc=QString::fromStdString(f->getDescriere());
    detailDesc->setText(desc.isEmpty()?"Nicio descriere disponibilă.":desc);
    auto row=[](const QString& k,const QString& v)->QString{
        return "<tr><td style='color:#8b8fa8;padding-right:20px;white-space:nowrap;'>"+k+"</td><td>"+v+"</td></tr>";
    };
    QString meta="<table style='font-size:13px;line-height:2;'>";
    if(!f->getRegizor().empty())      meta+=row("REGIZOR",    QString::fromStdString(f->getRegizor()));
    if(!f->getDistributie().empty())  meta+=row("DISTRIBUȚIE", QString::fromStdString(f->getDistributie()));
    if(!f->getDataPremiera().empty()) meta+=row("PREMIERĂ",    QString::fromStdString(f->getDataPremiera()));
    if(!f->getTara().empty())         meta+=row("PRODUCȚIE",   QString::fromStdString(f->getTara()));
    meta+=row("LIMBĂ",      QString::fromStdString(f->getLimba().empty()?"—":f->getLimba()));
    meta+=row("CLASIFICARE",QString::fromStdString(f->getClasificare().empty()?"AG":f->getClasificare()));
    meta+=row("FORMAT",     f->getEste3D()?"3D":"2D");
    meta+="</table>";detailMeta->setText(meta);
    double pret=(f->getPretBaza()+(f->getEste3D()?10.0:0.0))*cinema.getLocatieMultiplicator();
    QString tierBadge = QString::fromStdString(cinema.getTierBadge());
    QString pretStr = "de la  " + QString::number((int)pret) + " RON";
    if(cinema.getLocatieMultiplicator() != 1.0)
        pretStr += "  <span style='font-size:13px;color:#8b8fa8;'>[" + tierBadge + "]</span>";
    detailPrice->setText(pretStr);
    if(!f->getPosterPath().empty()){
        QPixmap px(QString::fromStdString(f->getPosterPath()));
        if(!px.isNull()){detailPoster->setPixmap(px.scaled(220,330,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation));detailPoster->setStyleSheet("border:1px solid #1e1e2a;border-radius:12px;");}
        else{detailPoster->setText("🎬");detailPoster->setStyleSheet("background:#18181f;border:1px solid #1e1e2a;border-radius:12px;font-size:48px;color:#50536a;");}
    } else {
        detailPoster->setText("🎬");detailPoster->setStyleSheet("background:#18181f;border:1px solid #1e1e2a;border-radius:12px;font-size:48px;color:#50536a;");
    }
    bool avail=(f->getStatus()==StatusFilm::RULAZA_ACUM);
    bool hasTrailer=!f->getTrailerYtId().empty();
    btnDetailBook->setEnabled(avail);btnDetailBook->setVisible(avail);
    detailUnavail->setText(avail?"":"Disponibil curand");
    if(f->getPosterPath().empty()) fetchPoster(f);
}


void MainWindow::fetchPoster(Film* f){
    if(!f||QString(TMDB_API_KEY).isEmpty()) return;
    detailPoster->setText("⏳");
    detailPoster->setStyleSheet("background:#18181f;border:1px solid #1e1e2a;border-radius:12px;font-size:32px;color:#50536a;");
    QUrl url("https://api.themoviedb.org/3/search/movie");
    QUrlQuery q;q.addQueryItem("api_key",TMDB_API_KEY);q.addQueryItem("query",QString::fromStdString(f->getTitlu()));q.addQueryItem("language","en-US");url.setQuery(q);
    auto* reply=netManager->get(QNetworkRequest(url));
    connect(reply,&QNetworkReply::finished,this,[this,reply]{
        reply->deleteLater();
        if(reply->error()!=QNetworkReply::NoError){detailPoster->setText("🎬");return;}
        QJsonArray results=QJsonDocument::fromJson(reply->readAll()).object()["results"].toArray();
        if(results.isEmpty()){detailPoster->setText("🎬");return;}
        QString posterPath=results[0].toObject()["poster_path"].toString();
        if(posterPath.isEmpty()){detailPoster->setText("🎬");return;}
        auto* imgReply=netManager->get(QNetworkRequest(QUrl("https://image.tmdb.org/t/p/w500"+posterPath)));
        connect(imgReply,&QNetworkReply::finished,this,[this,imgReply]{
            imgReply->deleteLater();
            if(imgReply->error()!=QNetworkReply::NoError){detailPoster->setText("🎬");return;}
            QPixmap px;px.loadFromData(imgReply->readAll());
            if(!px.isNull()){
                detailPoster->setPixmap(px.scaled(220,330,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation));
                detailPoster->setStyleSheet("border:1px solid #1e1e2a;border-radius:12px;");
                

                if(lblHeroPoster){
                    lblHeroPoster->setPixmap(px.scaled(82,122,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation));
                    lblHeroPoster->setStyleSheet("border-radius:8px;border:1px solid #5a1a1a;");
                }
            }
            else detailPoster->setText("🎬");
        });
    });
}


void MainWindow::refreshFilmMiniHeaders(){
    if(!rezFilm) return;
    QString t=QString::fromStdString(rezFilm->getTitlu());
    if(rezFilm->getEste3D()) t+="  [3D]";
    if(miniData) miniData->setText(t);
    if(miniShow) miniShow->setText(t);
}


void MainWindow::refreshHeroBanner(){
    std::vector<Film*>active;
    for(auto& f:cinema.getFilme()) if(f.getStatus()==StatusFilm::RULAZA_ACUM) active.push_back(const_cast<Film*>(&f));
    if(active.empty()){heroFrame->hide();return;}
    heroFrame->show();heroIdx=(heroIdx+1)%(int)active.size();
    Film* f=active[heroIdx];
    lblHeroTitle->setText(QString::fromStdString(f->getTitlu()));
    QString info=QString::fromStdString(f->getGen());
    if(f->getDurata()>0) info+="  •  "+QString::number(f->getDurata())+" min";
    if(f->getEste3D()) info+="  •  3D";
    if(f->getRating()>0){int s=qRound(f->getRating()/2.0);QString st;for(int i=0;i<s;i++)st+="★";for(int i=s;i<5;i++)st+="☆";info+="  •  "+st;}
    lblHeroInfo->setText(info);
    

    if(!f->getPosterPath().empty()){
        QPixmap px(QString::fromStdString(f->getPosterPath()));
        if(!px.isNull()){
            lblHeroPoster->setPixmap(px.scaled(82,122,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation));
            lblHeroPoster->setStyleSheet("border-radius:8px;border:1px solid #5a1a1a;");
            return;
        }
    }
    

    lblHeroPoster->setText("🎬");
    lblHeroPoster->setStyleSheet("background:#1a0505;border-radius:8px;border:1px solid #3a1010;font-size:24px;color:#3a1010;");
    if(!f->getPosterPath().empty()||QString(TMDB_API_KEY).isEmpty()) return;
    fetchPoster(f);
}
void MainWindow::refreshUpcomingStrip(){
    QLayout* old=upcomingContainer->layout();
    if(old){QLayoutItem* item;while((item=old->takeAt(0))!=nullptr){delete item->widget();delete item;}delete old;}
    std::vector<Film*>upcoming;
    for(auto& f:cinema.getFilme()) if(f.getStatus()==StatusFilm::IN_CURAND) upcoming.push_back(const_cast<Film*>(&f));
    upcomingSection->setVisible(!upcoming.empty());if(upcoming.empty())return;
    auto* hb=new QHBoxLayout(upcomingContainer);hb->setContentsMargins(0,0,0,0);hb->setSpacing(12);
    for(auto* f:upcoming){
        auto* card=new QFrame;card->setFixedSize(160,88);card->setStyleSheet("QFrame{background:#16120a;border:1px solid #3a2a0a;border-radius:10px;}");
        auto* cv=new QVBoxLayout(card);cv->setContentsMargins(14,12,14,12);cv->setSpacing(4);
        auto* tl=new QLabel(QString::fromStdString(f->getTitlu()));tl->setStyleSheet("font-size:12px;font-weight:700;color:#f0f0f5;background:transparent;border:none;");tl->setWordWrap(true);
        auto* dl=new QLabel(QString::fromStdString(f->getDataPremiera().empty()?"În curând":f->getDataPremiera()));dl->setStyleSheet("font-size:11px;color:#f5a623;background:transparent;border:none;");
        cv->addWidget(tl);cv->addWidget(dl);hb->addWidget(card);
    }
    hb->addStretch();
}


void MainWindow::refreshMainMenu(){
    auto*u=cinema.getCurent();if(!u)return;
    QString name=QString::fromStdString(u->username);
    lblNavUser->setText(u->esteAdmin?name+" [ADMIN]":name);
    lblWelcome->setText("Bun venit, <b>"+name+"</b>!");
    cardAdmin->setVisible(u->esteAdmin);
    heroIdx=-1;refreshHeroBanner();if(heroTimer&&!heroTimer->isActive())heroTimer->start();
    refreshUpcomingStrip();
    cbFilmGenre->blockSignals(true);cbFilmGenre->clear();cbFilmGenre->addItem("Toate genurile");
    std::vector<std::string>gens;
    auto filmeLocatie = cinema.getFilmePentruLocatie();
    for(auto* f : filmeLocatie)
        if(!f->getGen().empty()&&std::find(gens.begin(),gens.end(),f->getGen())==gens.end())
            gens.push_back(f->getGen());
    std::sort(gens.begin(),gens.end());
    for(auto& g:gens) cbFilmGenre->addItem(QString::fromStdString(g));
    cbFilmGenre->blockSignals(false);
}


void MainWindow::refreshSearchResults(){
    std::string q=leSearch->text().toStdString();
    std::transform(q.begin(),q.end(),q.begin(),::tolower);
    QString selGen=cbFilmGenre->currentIndex()>0?cbFilmGenre->currentText():"";
    searchResults.clear();lwFilms->clear();
    

    auto filmeLocatie = cinema.getFilmePentruLocatie();
    for(auto* fp : filmeLocatie){
        auto& f = *fp;
        if(f.getStatus()!=StatusFilm::RULAZA_ACUM) continue;
        if(!selGen.isEmpty()&&f.getGen()!=selGen.toStdString()) continue;
        if(filmFilter3D==0&&f.getEste3D()) continue;
        if(filmFilter3D==1&&!f.getEste3D()) continue;
        std::string tlc=f.getTitlu();std::transform(tlc.begin(),tlc.end(),tlc.begin(),::tolower);
        if(!q.empty()&&tlc.find(q)==std::string::npos) continue;
        searchResults.push_back(fp);
        auto* item=new QListWidgetItem(lwFilms);item->setSizeHint(QSize(0,76));
        auto* card=new QWidget;card->setStyleSheet("background:transparent;");
        auto* hb=new QHBoxLayout(card);hb->setContentsMargins(4,8,20,8);hb->setSpacing(0);
        auto* bar=new QFrame;bar->setFixedWidth(3);bar->setStyleSheet("background:#e50914;border-radius:2px;");hb->addWidget(bar);hb->addSpacing(14);
        auto* tc=new QVBoxLayout;tc->setSpacing(4);
        auto* tl=new QLabel(QString::fromStdString(f.getTitlu()));tl->setStyleSheet("font-size:15px;font-weight:700;color:#f0f0f5;background:transparent;border:none;");tl->setAttribute(Qt::WA_TransparentForMouseEvents);
        QString info=QString::fromStdString(f.getGen());
        if(f.getDurata()>0) info+="  •  "+QString::number(f.getDurata())+" min";
        if(f.getRating()>0) info+="  •  ★ "+QString::number(f.getRating(),'f',1);
        auto* il=new QLabel(info);il->setStyleSheet("font-size:12px;color:#8b8fa8;background:transparent;border:none;");il->setAttribute(Qt::WA_TransparentForMouseEvents);
        tc->addWidget(tl);tc->addWidget(il);hb->addLayout(tc,1);
        auto* rcw=new QWidget;rcw->setFixedWidth(82);rcw->setStyleSheet("background:transparent;");
        auto* rc=new QVBoxLayout(rcw);rc->setSpacing(4);rc->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        auto* pl=new QLabel(QString::number((int)f.getPretBaza())+" RON");pl->setStyleSheet("font-size:15px;font-weight:700;color:#e50914;background:transparent;border:none;");pl->setAlignment(Qt::AlignRight);pl->setAttribute(Qt::WA_TransparentForMouseEvents);rc->addWidget(pl);
        if(f.getEste3D()){auto* b=new QLabel("3D");b->setStyleSheet("color:#38bdf8;font-size:11px;font-weight:700;background:transparent;border:none;");b->setAlignment(Qt::AlignRight);b->setAttribute(Qt::WA_TransparentForMouseEvents);rc->addWidget(b);}
        hb->addWidget(rcw);lwFilms->setItemWidget(item,card);
    }
    filmListStack->setCurrentIndex(searchResults.empty()?1:0);
}


void MainWindow::refreshMapPage(){
    int si=cbMapSala->currentIndex(),di=cbMapData->currentIndex(),oi=cbMapOra->currentIndex();
    if(si<0||di<0) return;
    auto* sali=cinema.getSaliCurente();if(!sali||si>=(int)sali->size())return;
    auto zile=urmatoroarele7Zile();if(di>=(int)zile.size())return;
    int salaId=(*sali)[si].getId();
    auto ore=oredinProgram(cinema,salaId,zile[di]);
    if(oi<0||oi>=(int)ore.size()) return;
    mapWidget->load(cinema.getSalaByIdSafe(salaId),zile[di],ore[oi]);
}
void MainWindow::refreshCancelSeats(){
    int si=cbCancelSala->currentIndex(),di=cbCancelData->currentIndex(),oi=cbCancelOra->currentIndex();
    if(si<0||di<0) return;
    auto* sali=cinema.getSaliCurente();if(!sali||si>=(int)sali->size())return;
    auto zile=urmatoroarele7Zile();if(di>=(int)zile.size())return;
    int salaId=(*sali)[si].getId();
    auto ore=oredinProgram(cinema,salaId,zile[di]);
    if(oi<0||oi>=(int)ore.size()) return;
    cancelWidget->load(cinema.getSalaByIdSafe(salaId),zile[di],ore[oi]);
    if(cancelRow>=0&&cancelCol>=0) cancelWidget->setSelected({{cancelRow,cancelCol}});
}


void MainWindow::updateSeatLegendCounts(){
    Sala* sala=cinema.getSalaByIdSafe(rezSalaId);if(!sala)return;
    int std_=0,vip=0,cpl=0;
    for(int r=0;r<sala->getRanduri();r++) for(int c=0;c<sala->getColoane();c++){
        if(sala->esteOcupat(rezData,rezOra,r,c)) continue;
        switch(sala->getCategorieLocSeat(r,c)){
            case CategorieLocSeat::VIP:   vip++;break;
            case CategorieLocSeat::CUPLU: cpl++;break;
            default:                      std_++;break;
        }
    }
    if(lblLegStd) lblLegStd->setText("("+QString::number(std_)+" libere)");
    if(lblLegVip) lblLegVip->setText("("+QString::number(vip)+" libere)");
    if(lblLegCpl) lblLegCpl->setText("("+QString::number(cpl)+" libere)");
}


void MainWindow::refreshBrowse(){
    QString cur=cbGenuri->currentText();
    cbGenuri->blockSignals(true);cbGenuri->clear();
    std::vector<std::string>gens;
    for(auto& f:cinema.getFilme()) if(!f.getGen().empty()&&std::find(gens.begin(),gens.end(),f.getGen())==gens.end()) gens.push_back(f.getGen());
    std::sort(gens.begin(),gens.end());for(auto& g:gens) cbGenuri->addItem(QString::fromStdString(g));
    int idx=cbGenuri->findText(cur);if(idx>=0)cbGenuri->setCurrentIndex(idx);
    cbGenuri->blockSignals(false);onGenChanged(cbGenuri->currentIndex());
}
void MainWindow::onGenChanged(int){
    std::string gen=cbGenuri->currentText().toStdString();
    lwBrowseFilms->blockSignals(true);lwBrowseFilms->clear();
    for(auto& f:cinema.getFilme()){
        if(f.getGen()!=gen)continue;
        if(browseActiveOnly&&f.getStatus()!=StatusFilm::RULAZA_ACUM)continue;
        if(browse3D==0&&f.getEste3D())continue;if(browse3D==1&&!f.getEste3D())continue;
        QString lbl=QString::fromStdString(f.getTitlu());
        if(f.getEste3D())lbl+="  [3D]";if(f.getStatus()==StatusFilm::IN_CURAND)lbl+="  [în curând]";
        lwBrowseFilms->addItem(lbl);
    }
    lwBrowseFilms->blockSignals(false);
    if(lwBrowseFilms->count()>0){lwBrowseFilms->setCurrentRow(0);updateBrowseDetail(0);}
    else lblBrowseDetail->setText("Niciun film în acest gen.");
}
void MainWindow::updateBrowseDetail(int row){
    if(row<0){lblBrowseDetail->clear();return;}
    std::string gen=cbGenuri->currentText().toStdString();
    std::vector<Film*>vis;
    for(auto& f:cinema.getFilme()){if(f.getGen()!=gen)continue;if(browseActiveOnly&&f.getStatus()!=StatusFilm::RULAZA_ACUM)continue;if(browse3D==0&&f.getEste3D())continue;if(browse3D==1&&!f.getEste3D())continue;vis.push_back(const_cast<Film*>(&f));}
    if(row>=(int)vis.size())return;Film* f=vis[row];
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


void MainWindow::refreshProfile(){
    auto* u=cinema.getCurent(); if(!u) return;
    

    QLayout* old=profileContainer->layout();
    if(old){QLayoutItem* it;while((it=old->takeAt(0))!=nullptr){delete it->widget();delete it;}delete old;}
    auto* vb=new QVBoxLayout(profileContainer);
    vb->setContentsMargins(8,8,8,8); vb->setSpacing(12);

    

    auto* histHdr=new QLabel("ISTORIC REZERVĂRI");
    histHdr->setStyleSheet("font-size:11px;font-weight:700;color:#50536a;letter-spacing:2px;");
    vb->addWidget(histHdr);

    if(u->istoricRezervari.empty()){
        auto* empty=new QLabel("Nicio rezervare înregistrată.");
        empty->setStyleSheet("font-size:13px;color:#8b8fa8;padding:8px;");
        vb->addWidget(empty);
    } else {
        int i=1;
        for(auto& r:u->istoricRezervari){
            auto* card=new QFrame;
            card->setStyleSheet("QFrame{background:#18181f;border:1px solid #1e1e2a;border-radius:8px;}");
            auto* hb=new QHBoxLayout(card);hb->setContentsMargins(14,10,14,10);hb->setSpacing(8);
            auto* num=new QLabel(QString::number(i++));
            num->setStyleSheet("font-size:12px;font-weight:700;color:#50536a;background:transparent;border:none;min-width:20px;");
            auto* txt=new QLabel(QString::fromStdString(r));
            txt->setStyleSheet("font-size:12px;color:#c8c8d8;background:transparent;border:none;");
            txt->setWordWrap(true);
            hb->addWidget(num);hb->addWidget(txt,1);
            vb->addWidget(card);
        }
    }

    vb->addSpacing(8);
    auto* ratHdr=new QLabel("EVALUEAZĂ FILME");
    ratHdr->setStyleSheet("font-size:11px;font-weight:700;color:#50536a;letter-spacing:2px;");
    vb->addWidget(ratHdr);
    refreshProfileRatings();
    vb->addStretch();
}

void MainWindow::refreshProfileRatings(){
    auto* u=cinema.getCurent(); if(!u||!profileContainer) return;
    auto* vb=qobject_cast<QVBoxLayout*>(profileContainer->layout());
    if(!vb) return;

    

    

    

    std::set<std::string> filmeVazute;
    for(auto& r:u->istoricRezervari){
        

        size_t p=r.find(" | "); if(p!=std::string::npos) filmeVazute.insert(r.substr(0,p));
    }

    for(auto& titlu:filmeVazute){
        auto* card=new QFrame;
        card->setStyleSheet("QFrame{background:#16120a;border:1px solid #3a2a0a;border-radius:8px;}");
        auto* hb=new QHBoxLayout(card);hb->setContentsMargins(14,10,14,10);hb->setSpacing(12);
        auto* tl=new QLabel(QString::fromStdString(titlu));
        tl->setStyleSheet("font-size:13px;font-weight:600;color:#f0f0f5;background:transparent;border:none;");
        hb->addWidget(tl,1);
        

        int curRating=cinema.getRatingUtilizator(titlu);
        for(int s=1;s<=5;s++){
            auto* btn=new QPushButton(s<=curRating?"★":"☆");
            btn->setFixedSize(28,28);btn->setCursor(Qt::PointingHandCursor);
            btn->setStyleSheet(s<=curRating
                ?"QPushButton{background:transparent;border:none;color:#f5a623;font-size:18px;}"
                 "QPushButton:hover{color:#f5a623;}"
                :"QPushButton{background:transparent;border:none;color:#3a3a55;font-size:18px;}"
                 "QPushButton:hover{color:#f5a623;}");
            QString t=QString::fromStdString(titlu); int stars=s;
            connect(btn,&QPushButton::clicked,this,[this,t,stars]{
                onRateFilm(t,stars);
            });
            hb->addWidget(btn);
        }
        if(curRating>0){
            auto* rl=new QLabel(QString::number(curRating)+"/5");
            rl->setStyleSheet("font-size:12px;color:#f5a623;background:transparent;border:none;");
            hb->addWidget(rl);
        }
        vb->addWidget(card);
    }
    if(filmeVazute.empty()){
        auto* noF=new QLabel("Rezervă filme pentru a le putea evalua.");
        noF->setStyleSheet("font-size:13px;color:#8b8fa8;padding:4px;");
        vb->addWidget(noF);
    }
}

void MainWindow::onRateFilm(const QString& titlu, int stele){
    cinema.evalueazaFilm(titlu.toStdString(), stele);
    showToast("Ai notat '"+titlu+"' cu "+QString::number(stele)+" stele!",false);
    refreshProfile();
}

void MainWindow::onExportCalendar(){
    if(!rezFilm||rezOra.empty()) return;
    auto* loc=cinema.getLocatieCurenta();
    Sala* sala=cinema.getSalaByIdSafe(rezSalaId);

    

    auto toIcal=[](const Data& d,const std::string& ora)->QString{
        QString dt=QString("%1%2%3T%4%5%6")
            .arg(d.an,4,10,QChar('0')).arg(d.luna,2,10,QChar('0')).arg(d.zi,2,10,QChar('0'))
            .arg(QString::fromStdString(ora).left(2))
            .arg(QString::fromStdString(ora).right(2))
            .arg("00");
        return dt;
    };

    int durMin=rezFilm->getDurata()>0?rezFilm->getDurata():120;
    

    int startMin=timeToMin(rezOra);
    int endMin=startMin+durMin;
    std::string endOra=minToTime(endMin);

    QString ics="BEGIN:VCALENDAR\r\nVERSION:2.0\r\nPRODID:-//CinemaApp//RO\r\n";
    ics+="BEGIN:VEVENT\r\n";
    ics+="DTSTART:"+toIcal(rezData,rezOra)+"\r\n";
    ics+="DTEND:"+toIcal(rezData,endOra)+"\r\n";
    ics+="SUMMARY:"+QString::fromStdString(rezFilm->getTitlu())
        +(loc?" - "+QString::fromStdString(loc->nume):"")+"\r\n";
    if(sala) ics+="DESCRIPTION:"+QString::fromStdString(sala->getNume())+"\r\n";
    if(loc)  ics+="LOCATION:"+QString::fromStdString(loc->adresa)+"\r\n";
    ics+="END:VEVENT\r\nEND:VCALENDAR\r\n";

    QString path=QDir::currentPath()+"/data/rezervare.ics";
    QFile f(path); if(f.open(QIODevice::WriteOnly|QIODevice::Text)){
        f.write(ics.toUtf8()); f.close();
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
        showToast("Eveniment exportat în calendar!",false);
    } else showToast("Nu s-a putut crea fișierul .ics");
}

void MainWindow::refreshAdminStatsCards(){
    

    auto* adminWidget=findChild<QWidget*>("adminStatsWidget");
    if(!adminWidget) return;
    auto* hb=qobject_cast<QHBoxLayout*>(adminWidget->layout());
    if(!hb) return;
    

    QLayoutItem* item;
    while((item=hb->takeAt(0))!=nullptr){delete item->widget();delete item;}

    

    std::ifstream f("data/toate_rezervarile.txt");
    double totalVenit=0; int totalRez=0;
    std::map<std::string,int> perFilm,perOra,perLoc;
    std::string ln;
    while(std::getline(f,ln)){
        if(ln.empty()||ln.find("Film: ")==std::string::npos) continue;
        totalRez++;
        auto ex=[&](const std::string& tag,const std::string& end)->std::string{
            size_t p=ln.find(tag); if(p==std::string::npos) return "";
            p+=tag.size(); size_t e=ln.find(end,p);
            return e==std::string::npos?ln.substr(p):ln.substr(p,e-p);
        };
        perFilm[ex("Film: "," | ")]++;
        perOra[ex("Ora: "," | ")]++;
        perLoc[ex("Locatie: "," | ")]++;
        std::string sp=ex("Pret: "," RON");
        try{totalVenit+=std::stod(sp);}catch(...){}
    }

    auto topKey=[](const std::map<std::string,int>& m)->std::string{
        if(m.empty()) return "—";
        return std::max_element(m.begin(),m.end(),[](auto&a,auto&b){return a.second<b.second;})->first;
    };

    struct Card{QString ico,val,lbl;QString color;};
    std::vector<Card> cards={
        {"🎟",QString::number(totalRez),"Rezervări","#38bdf8"},
        {"💰",QString::number((int)totalVenit)+" RON","Venituri totale","#22c55e"},
        {"🎬",QString::fromStdString(topKey(perFilm)),"Film popular","#f5a623"},
        {"🕐",QString::fromStdString(topKey(perOra)),"Ora de vârf","#8b5cf6"},
    };
    for(auto& c:cards){
        auto* card=new QFrame;card->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
        card->setStyleSheet("QFrame{background:#18181f;border:1px solid #1e1e2a;border-radius:10px;}");
        auto* cv=new QVBoxLayout(card);cv->setContentsMargins(14,10,14,10);cv->setSpacing(2);
        auto* il=new QLabel(c.ico+" "+c.val);
        il->setStyleSheet(QString("font-size:16px;font-weight:800;color:%1;background:transparent;border:none;").arg(c.color));
        auto* ll=new QLabel(c.lbl);
        ll->setStyleSheet("font-size:11px;color:#8b8fa8;background:transparent;border:none;");
        ll->setWordWrap(true);
        cv->addWidget(il);cv->addWidget(ll);
        hb->addWidget(card,1);
    }
}
void MainWindow::refreshAdminFilms(){
    lwAdminFilms->clear();
    for(auto&f:cinema.getFilme()){
        QString l=QString::fromStdString(f.getTitlu());if(f.getEste3D())l+=" [3D]";
        l+="  •  "+QString::fromStdString(f.getGen())+"  •  "+QString::number((int)f.getPretBaza())+" RON";
        if(f.getRating()>0)l+="  •  ★"+QString::number(f.getRating(),'f',1);
        if(f.getStatus()==StatusFilm::IN_CURAND)l+="  [în curând]";lwAdminFilms->addItem(l);
    }
}

void MainWindow::refreshAdminUsers(){
    lwAdminUsers->clear();
    for(auto&u:cinema.getUtilizatori()){QString l=QString::fromStdString(u.username);if(u.esteAdmin)l+="  [ADMIN]";lwAdminUsers->addItem(l);}
}
void MainWindow::refreshAdminReservations(){
    teAllRes->clear();
    std::ifstream f("data/toate_rezervarile.txt");if(!f.is_open()){teAllRes->setPlainText("(gol)");return;}
    int nr=0;std::string ln;
    while(std::getline(f,ln)){if(!ln.empty()&&ln.back()=='\r')ln.pop_back();if(!ln.empty())teAllRes->append(QString::number(++nr)+".  "+QString::fromStdString(ln));}
    if(!nr)teAllRes->setPlainText("(nicio rezervare)");
}
void MainWindow::refreshAdminReport(){
    teReport->clear();
    std::ifstream f("data/toate_rezervarile.txt");if(!f.is_open()){teReport->setPlainText("(gol)");return;}
    std::map<std::string,int>bF,bS,bO,bL;std::map<std::string,double>vF;
    double tot=0;int totB=0;std::string ln;
    while(std::getline(f,ln)){
        if(!ln.empty()&&ln.back()=='\r')ln.pop_back();if(ln.empty())continue;
        auto ex=[&](const std::string&tag,const std::string&end)->std::string{
            size_t p=ln.find(tag);if(p==std::string::npos)return"";
            p+=tag.size();size_t e=ln.find(end,p);return e==std::string::npos?ln.substr(p):ln.substr(p,e-p);
        };
        std::string film=ex("Film: "," | "),loc=ex("Locatie: "," | "),sala=ex("Sala: "," | "),ora=ex("Ora: "," | "),sp=ex("Pret: "," RON");
        if(film.empty())continue;double pret=0;try{pret=std::stod(sp);}catch(...){}
        bF[film]++;vF[film]+=pret;bS[sala]++;bO[ora]++;bL[loc]++;totB++;tot+=pret;
    }
    std::ostringstream oss;
    oss<<"=== RAPORT VÂNZĂRI ===\n\n── Pe locație ────────────────────────────────\n";
    for(auto&[k,v]:bL) oss<<"  "<<std::left<<std::setw(35)<<k<<v<<" bilete\n";
    oss<<"\n── Pe film ───────────────────────────────────\n";
    for(auto&[k,v]:bF) oss<<"  "<<std::left<<std::setw(40)<<k<<std::setw(4)<<v<<" bilete  │  "<<std::fixed<<std::setprecision(2)<<vF[k]<<" RON\n";
    oss<<"\n── Pe oră ────────────────────────────────────\n";
    for(auto&[k,v]:bO) oss<<"  "<<k<<"  :  "<<v<<" bilete\n";
    oss<<"\n═════════════════════════════════════════════\n";
    oss<<"  TOTAL  "<<totB<<" bilete  │  "<<std::fixed<<std::setprecision(2)<<tot<<" RON\n";
    teReport->setPlainText(QString::fromStdString(oss.str()));
}
void MainWindow::refreshFoodPage(){
    

    rezFood=FoodOrder{};
    for(auto* lbl:foodQtyLabels) lbl->setText("0");
    if(lblFoodTotal) lblFoodTotal->setText("Total food: 0 RON");
}


void MainWindow::onLogin(){
    if(cinema.login(leUser->text().toStdString(),lePass->text().toStdString())){
        leUser->clear();lePass->clear();
        

        onLocationDetected();
        goTo(P_LOCATION);
    } else showToast("Username sau parola incorecta.");
}
void MainWindow::onRegister(){
    bool ok;
    QString usr=QInputDialog::getText(this,"Înregistrare","Username nou:",QLineEdit::Normal,"",&ok);if(!ok||usr.isEmpty())return;
    QString par=QInputDialog::getText(this,"Înregistrare","Parolă:",QLineEdit::Password,"",&ok);if(!ok||par.isEmpty())return;
    QString par2=QInputDialog::getText(this,"Înregistrare","Confirmă parola:",QLineEdit::Password,"",&ok);if(!ok)return;
    if(par!=par2){showToast("Parolele nu coincid!");return;}
    if(cinema.inregistrare(usr.toStdString(),par.toStdString())){
        showToast("Cont creat! Bun venit, "+usr+"!",false);
        onLocationDetected();goTo(P_LOCATION);
    } else showToast("Username deja existent.");
}
void MainWindow::onLogout(){cinema.logout();heroTimer->stop();leUser->clear();lePass->clear();goTo(P_LOGIN);}


void MainWindow::onNewReservation(){
    rezFilm=nullptr;rezSalaId=-1;rezLocuri.clear();
    filmFilter3D=-1;leSearch->clear();cbFilmGenre->setCurrentIndex(0);
    refreshSearchResults();goTo(P_RESERVE_FILM);
}
void MainWindow::onBrowseGenuri(){
    browse3D=-1;browseActiveOnly=false;
    btnBrowseFilter3D->setText("Format: Toate");btnBrowseFilterActive->setText("Status: Toate");
    refreshBrowse();goTo(P_BROWSE);
}
void MainWindow::onHartaSala(){
    cbMapSala->clear();cbMapData->clear();cbMapOra->clear();
    auto* sali=cinema.getSaliCurente();if(sali) for(auto&s:*sali) cbMapSala->addItem(QString::fromStdString(s.getNume()));
    auto zile=urmatoroarele7Zile();Data azi=dataAzi();
    for(auto&z:zile){QString l=QString::fromStdString(numeZiSaptamana(z))+"  –  "+QString::fromStdString(z.toString());if(z==azi)l+="  ★";cbMapData->addItem(l);}
    

    refreshMapOra();goTo(P_MAP);
}
void MainWindow::onAnulareRezervare(){
    cbCancelSala->clear();cbCancelData->clear();cbCancelOra->clear();cancelRow=-1;cancelCol=-1;
    auto* sali=cinema.getSaliCurente();if(sali) for(auto&s:*sali) cbCancelSala->addItem(QString::fromStdString(s.getNume()));
    auto zile=urmatoroarele7Zile();Data azi=dataAzi();
    for(auto&z:zile){QString l=QString::fromStdString(numeZiSaptamana(z))+"  –  "+QString::fromStdString(z.toString());if(z==azi)l+="  ★";cbCancelData->addItem(l);}
    

    refreshCancelOra();goTo(P_CANCEL);
}
void MainWindow::onProfilMeu(){refreshProfile();refreshProfileRatings();goTo(P_PROFILE);}
void MainWindow::onAdminPanel(){refreshAdminStatsCards();adminTabs->setCurrentIndex(0);refreshAdminReservations();goTo(P_ADMIN);}


void MainWindow::onFilmSelected(){
    int idx=lwFilms->currentRow();
    if(idx<0||(int)searchResults.size()<=idx){showToast("Selectează un film!");return;}
    detailFromPage=P_RESERVE_FILM;populateFilmDetail(searchResults[idx]);refreshFilmMiniHeaders();goTo(P_FILM_DETAIL);
}
void MainWindow::onProceedFromDetail(){
    if(!rezFilm)return;
    rezLocuri.clear();rezProgramare=nullptr;
    refreshFilmMiniHeaders();
    goTo(P_RESERVE_DATA);
}

void MainWindow::onDataSelected(){
    int idx=lwData->currentRow();
    if(idx<0){showToast("Selecteaza o data!");return;}
    rezData=urmatoroarele7Zile()[idx];
    if(!rezFilm) return;

    auto programari=cinema.getProgrameFilm(rezFilm->getTitlu(),rezData);

    

    QLayout* oldL=showingsContainer->layout();
    if(oldL){
        QLayoutItem* it;
        while((it=oldL->takeAt(0))!=nullptr){delete it->widget();delete it;}
        delete oldL;
    }
    QVBoxLayout* vb=new QVBoxLayout(showingsContainer);
    vb->setContentsMargins(0,0,8,8);vb->setSpacing(10);

    if(programari.empty()){
        lblNoShowings->show();
        showingsScroll->hide();
        goTo(P_RESERVE_SALA);
        return;
    }
    lblNoShowings->hide();
    showingsScroll->show();

    double locMult  = cinema.getLocatieMultiplicator();
    double pretBaza = rezFilm->getPretBaza()+(rezFilm->getEste3D()?10.0:0.0);

    for(const Programare* p : programari){
        Sala* sala  = cinema.getSalaByIdSafe(p->salaId);
        int total   = sala?(sala->getRanduri()*sala->getColoane()):50;
        int libere  = sala?sala->locuriLibere(rezData,p->ora):total;
        int pretMin = (int)(pretBaza*locMult);
        int pretMax = (int)(pretBaza*1.8*locMult);

        

        QPushButton* card = new QPushButton;
        card->setFixedHeight(90);
        card->setCursor(Qt::PointingHandCursor);
        card->setFlat(true);
        card->setStyleSheet(
            "QPushButton{background:#18181f;border:1.5px solid #1e1e2a;border-radius:12px;text-align:left;padding:0;}"
            "QPushButton:hover{background:#1e1e28;border:2px solid #e50914;}"
            "QPushButton:pressed{background:#141420;}");

        

        QHBoxLayout* hb = new QHBoxLayout(card);
        hb->setContentsMargins(20,8,20,8);hb->setSpacing(16);

        

        QWidget* tb = new QWidget;tb->setFixedWidth(90);
        tb->setStyleSheet("background:#0d1a10;border-radius:8px;border:1px solid #166534;");
        QVBoxLayout* tv = new QVBoxLayout(tb);tv->setContentsMargins(0,6,0,6);tv->setSpacing(2);
        QLabel* tL = new QLabel(QString::fromStdString(p->ora));
        tL->setStyleSheet("font-size:22px;font-weight:800;color:#22c55e;background:transparent;border:none;");
        tL->setAlignment(Qt::AlignCenter);tL->setAttribute(Qt::WA_TransparentForMouseEvents);
        

        bool isWeekend=false;
        {tm t2{};t2.tm_mday=rezData.zi;t2.tm_mon=rezData.luna-1;t2.tm_year=rezData.an-1900;mktime(&t2);isWeekend=(t2.tm_wday==0||t2.tm_wday==6);}
        if(isWeekend){
            QLabel* wknd=new QLabel("Weekend +20%");
            wknd->setStyleSheet("font-size:9px;color:#f5a623;background:#2d1f08;border-radius:4px;padding:1px 5px;background:transparent;border:none;");
            wknd->setAttribute(Qt::WA_TransparentForMouseEvents);
            tv->addWidget(wknd);
        }
        QLabel* tE = new QLabel("pana la "+QString::fromStdString(p->oraFinal));
        tE->setStyleSheet("font-size:10px;color:#4ade80;background:transparent;border:none;");
        tE->setAlignment(Qt::AlignCenter);tE->setAttribute(Qt::WA_TransparentForMouseEvents);
        tv->addWidget(tL);tv->addWidget(tE);
        hb->addWidget(tb);

        

        QVBoxLayout* iv = new QVBoxLayout;iv->setSpacing(3);
        QLabel* sL = new QLabel(QString::fromStdString(p->salaNume));
        sL->setStyleSheet("font-size:15px;font-weight:700;color:#f0f0f5;background:transparent;border:none;");
        sL->setAttribute(Qt::WA_TransparentForMouseEvents);

        

        QString seatsColor = libere>10?"#22c55e":libere>3?"#f5a623":"#e50914";
        QString seatsText  = "<span style='color:"+seatsColor+";font-weight:700;'>"+
                             QString::number(libere)+"</span>"
                             "<span style='color:#8b8fa8;'> / "+QString::number(total)+" locuri libere</span>";
        QLabel* seL = new QLabel(seatsText);seL->setTextFormat(Qt::RichText);
        seL->setStyleSheet("font-size:12px;background:transparent;border:none;");
        seL->setAttribute(Qt::WA_TransparentForMouseEvents);
        iv->addWidget(sL);iv->addWidget(seL);
        hb->addLayout(iv,1);

        

        QVBoxLayout* pv = new QVBoxLayout;pv->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        QLabel* pL = new QLabel(QString::number(pretMin)+"-"+QString::number(pretMax)+" RON");
        pL->setStyleSheet("font-size:14px;font-weight:700;color:#e50914;background:transparent;border:none;");
        pL->setAttribute(Qt::WA_TransparentForMouseEvents);
        QLabel* pS = new QLabel(rezFilm->getEste3D()?"3D":"2D");
        pS->setStyleSheet("font-size:11px;color:#8b8fa8;background:transparent;border:none;");
        pS->setAttribute(Qt::WA_TransparentForMouseEvents);
        pv->addWidget(pL);pv->addWidget(pS);
        hb->addLayout(pv);

        QLabel* ar = new QLabel(">");
        ar->setStyleSheet("font-size:22px;color:#50536a;background:transparent;border:none;");
        ar->setAttribute(Qt::WA_TransparentForMouseEvents);
        hb->addWidget(ar);

        const Programare* pCopy = p;
        connect(card,&QPushButton::clicked,this,[this,pCopy]{onShowingSelected(pCopy);});
        vb->addWidget(card);
    }
    vb->addStretch();
    goTo(P_RESERVE_SALA);
}

void MainWindow::onShowingSelected(const Programare* p){
    if(!p||!rezFilm)return;
    rezProgramare=p;rezSalaId=p->salaId;rezOra=p->ora;
    Sala* sala=cinema.getSalaByIdSafe(rezSalaId);
    rezLocuri.clear();
    seatMapWidget->load(sala,rezData,rezOra);
    seatMapWidget->setPretBaza(rezFilm->getPretBaza()+(rezFilm->getEste3D()?10.0:0.0));
    lblSeatInfo->setText("Apasa pe un loc pentru a-l selecta");
    lblSeatCount->setText("0 / 8 locuri selectate");
    QString hdr="  "+QString::fromStdString(rezFilm->getTitlu())
               +"  .  "+QString::fromStdString(p->ora)
               +"  .  "+QString::fromStdString(p->salaNume);
    if(miniLoc) miniLoc->setText(hdr);
    if(miniFood) miniFood->setText(hdr);
    if(miniConf) miniConf->setText(hdr);
    

    PreferintaLoc pref=cinema.getPreferintaCurenta();
    seatMapWidget->setPreferinta(pref);
    updateSeatLegendCounts();goTo(P_RESERVE_LOC);
}
void MainWindow::onLocSelected(int row,int col){
    Sala* sala=cinema.getSalaByIdSafe(rezSalaId);if(!sala)return;
    if(sala->esteOcupat(rezData,rezOra,row,col)){lblSeatInfo->setText("⛔ Loc ocupat!");return;}
    

    for(int i=0;i<(int)rezLocuri.size();i++){
        if(rezLocuri[i].first==row&&rezLocuri[i].second==col){
            rezLocuri.erase(rezLocuri.begin()+i);
            seatMapWidget->setSelected(rezLocuri);
            lblSeatCount->setText(QString::number(rezLocuri.size())+" / 8 locuri selectate");
            updateSeatLegendCounts();return;
        }
    }
    if((int)rezLocuri.size()>=maxLocuri){lblSeatInfo->setText("Maximum 8 locuri per rezervare!");return;}
    rezLocuri.push_back({row,col});
    seatMapWidget->setSelected(rezLocuri);
    CategorieLocSeat cat=sala->getCategorieLocSeat(row,col);
    double pret=(rezFilm->getPretBaza()+(rezFilm->getEste3D()?10.0:0.0))*multiplicatorCategorie(cat);
    lblSeatInfo->setText(QString("R%1  L%2   [%3]   %4 RON").arg(row+1).arg(col+1).arg(QString::fromStdString(numeCategorie(cat))).arg(pret,0,'f',2));
    lblSeatCount->setText(QString::number(rezLocuri.size())+" / 8 locuri selectate");
    updateSeatLegendCounts();
}
void MainWindow::onFoodConfirmed(){ buildConfirmPage(); }
void MainWindow::onFoodSkipped()  { rezFood=FoodOrder{}; buildConfirmPage(); }

void MainWindow::buildConfirmPage(){
    Sala* sala=cinema.getSalaByIdSafe(rezSalaId);
    std::ostringstream oss;
    oss<<"<table style='font-size:14px;line-height:2;'>"
       <<"<tr><td style='color:#8b8fa8;width:80px;'>Film</td><td><b>"<<rezFilm->getTitlu()<<"</b></td></tr>"
       <<"<tr><td style='color:#8b8fa8;'>Locație</td><td>"<<(cinema.getLocatieCurenta()?cinema.getLocatieCurenta()->nume:"—")<<"</td></tr>"
       <<"<tr><td style='color:#8b8fa8;'>Sală</td><td>"<<(sala?sala->getNume():"—")<<"</td></tr>"
       <<"<tr><td style='color:#8b8fa8;'>Data</td><td>"<<rezData.toString()<<"</td></tr>"
       <<"<tr><td style='color:#8b8fa8;'>Ora</td><td>"<<rezOra<<"</td></tr>"
       <<"<tr><td style='color:#8b8fa8;'>Locuri</td><td>";
    double totalBilete=0;
    for(int i=0;i<(int)rezLocuri.size();i++){
        auto [r,c]=rezLocuri[i];
        CategorieLocSeat cat=sala?sala->getCategorieLocSeat(r,c):CategorieLocSeat::STANDARD;
        double pret=(rezFilm->getPretBaza()+(rezFilm->getEste3D()?10.0:0.0))*multiplicatorCategorie(cat);
        totalBilete+=pret;
        if(i>0)oss<<", ";
        oss<<"R"<<r+1<<" L"<<c+1<<" ["<<numeCategorie(cat)<<"]";
    }
    oss<<"</td></tr>"
       <<"<tr><td style='color:#8b8fa8;'>Bilete</td><td style='color:#e50914;font-weight:700;'>"<<std::fixed<<std::setprecision(2)<<totalBilete<<" RON</td></tr>";
    double foodTotal=rezFood.total(cinema.getMeniu());
    if(!rezFood.empty()){
        oss<<"<tr><td style='color:#8b8fa8;'>Food</td><td>"<<rezFood.toStr(cinema.getMeniu())<<"  ("<<std::fixed<<std::setprecision(2)<<foodTotal<<" RON)</td></tr>";
        oss<<"<tr><td style='color:#8b8fa8;'>TOTAL</td><td style='color:#e50914;font-size:18px;font-weight:800;'>"<<std::fixed<<std::setprecision(2)<<totalBilete+foodTotal<<" RON</td></tr>";
    } else {
        oss<<"<tr><td style='color:#8b8fa8;'>TOTAL</td><td style='color:#e50914;font-size:18px;font-weight:800;'>"<<std::fixed<<std::setprecision(2)<<totalBilete<<" RON</td></tr>";
    }
    oss<<"</table>";
    lblConfirmDetails->setText(QString::fromStdString(oss.str()));
    const char ch[]="ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
    std::string ref="CIN-";for(int i=0;i<8;i++)ref+=ch[rand()%32];
    lblBookingRef->setText("Referință:  <b style='color:#38bdf8;font-size:16px;'>"+QString::fromStdString(ref)+"</b>");
    leEmail->clear();goTo(P_RESERVE_CONFIRM);
}

void MainWindow::onConfirmReservation(){
    std::string email=leEmail->text().trimmed().toStdString();
    try {
        cinema.proceseazaRezervare(rezSalaId,rezData,rezLocuri,rezFilm,rezOra,email,rezFood);
    

    if(!rezLocuri.empty()){
        auto [r,c]=rezLocuri[0];
        Sala* sala=cinema.getSalaByIdSafe(rezSalaId);
        if(sala){
            PreferintaLoc pref;
            pref.tip=sala->getCategorieLocSeat(r,c);
            pref.rand_=r; pref.coloana=c; pref.setat=true;
            cinema.salveazaPreferinta(pref);
        }
    }
    } catch(const LocOcupatException& e){
        showToast(QString::fromStdString(e.what()));return;
    } catch(const SalaNotFoundException& e){
        showToast(QString::fromStdString(e.what()));return;
    } catch(const std::exception& e){
        showToast(QString::fromStdString(e.what()));return;
    }
    

    QString refText=lblBookingRef->text().remove(QRegularExpression("<[^>]*>")).trimmed();
    refText=refText.mid(refText.indexOf("CIN-"));
    lblDoneRef->setText(refText);
    

    Sala* sala=cinema.getSalaByIdSafe(rezSalaId);
    std::ostringstream oss;
    oss<<"<table style='font-size:13px;line-height:2;'>"
       <<"<tr><td style='color:#8b8fa8;width:80px;'>Film</td><td><b>"<<rezFilm->getTitlu()<<"</b></td></tr>"
       <<"<tr><td style='color:#8b8fa8;'>Locație</td><td>"<<(cinema.getLocatieCurenta()?cinema.getLocatieCurenta()->nume:"—")<<"</td></tr>"
       <<"<tr><td style='color:#8b8fa8;'>Sală</td><td>"<<(sala?sala->getNume():"—")<<"</td></tr>"
       <<"<tr><td style='color:#8b8fa8;'>Data</td><td>"<<rezData.toString()<<" · "<<rezOra<<"</td></tr>"
       <<"<tr><td style='color:#8b8fa8;'>Locuri</td><td>"<<(int)rezLocuri.size()<<" loc"<<(rezLocuri.size()==1?"":"uri")<<"</td></tr>";
    double foodTotal=rezFood.total(cinema.getMeniu());
    if(!rezFood.empty()) oss<<"<tr><td style='color:#8b8fa8;'>Food</td><td>"<<rezFood.toStr(cinema.getMeniu())<<"</td></tr>";
    oss<<"</table>";
    lblDoneSummary->setText(QString::fromStdString(oss.str()));
    goTo(P_CONFIRM_DONE);
}
void MainWindow::onCancelReservation(){
    if(cancelRow<0){showToast("Selectează un loc ocupat!");return;}
    int si=cbCancelSala->currentIndex(),di=cbCancelData->currentIndex(),oi=cbCancelOra->currentIndex();
    auto* sali=cinema.getSaliCurente();
    if(!sali||si<0||di<0||oi<0)return;
    auto zile=urmatoroarele7Zile();Sala* sala=cinema.getSalaByIdSafe((*sali)[si].getId());
    auto oreL=oredinProgram(cinema,sala?sala->getId():-1,zile[di]);
    if(oi>=(int)oreL.size())return;
    Data data=zile[di];std::string ora=oreL[oi];
    if(!sala||!sala->esteOcupat(data,ora,cancelRow,cancelCol)){showToast("Locul nu este ocupat!");return;}
    if(QMessageBox::question(this,"Confirmare",QString("Anulezi rezervarea R%1 L%2?").arg(cancelRow+1).arg(cancelCol+1))!=QMessageBox::Yes)return;
    cinema.anuleazaRezervare(sala->getId(),data,ora,cancelRow,cancelCol);
    cancelRow=-1;cancelCol=-1;refreshCancelSeats();showToast("Rezervare anulată cu succes!",false);
}


void MainWindow::onBrowseReserve(){
    int row=lwBrowseFilms->currentRow();if(row<0){showToast("Selectează un film!");return;}
    std::string gen=cbGenuri->currentText().toStdString();
    std::vector<Film*>vis;
    for(auto& f:cinema.getFilme()){if(f.getGen()!=gen)continue;if(browseActiveOnly&&f.getStatus()!=StatusFilm::RULAZA_ACUM)continue;if(browse3D==0&&f.getEste3D())continue;if(browse3D==1&&!f.getEste3D())continue;vis.push_back(const_cast<Film*>(&f));}
    if(row>=(int)vis.size())return;Film* f=vis[row];
    if(f->getStatus()==StatusFilm::IN_CURAND){showToast("Filmul nu rulează încă.",false);return;}
    detailFromPage=P_BROWSE;populateFilmDetail(f);refreshFilmMiniHeaders();goTo(P_FILM_DETAIL);
}


bool MainWindow::showFilmDialog(Film& out,const Film* ex){
    auto* dlg=new QDialog(this);dlg->setWindowTitle(ex?"Editează Film":"Adaugă Film");dlg->setFixedWidth(520);
    dlg->setStyleSheet("QDialog{background:#141418;}QLabel{color:#e5e5f0;font-size:13px;}"
                       "QLineEdit,QTextEdit,QDoubleSpinBox,QSpinBox,QComboBox{background:#1e1e28;color:#f0f0f5;border:1.5px solid #2a2a38;border-radius:7px;padding:7px 12px;font-size:13px;}"
                       "QCheckBox{color:#f0f0f5;font-size:13px;}"
                       "QPushButton{background:#e50914;color:white;border:none;border-radius:7px;padding:9px 24px;font-size:13px;font-weight:700;min-width:80px;}"
                       "QPushButton:hover{background:#ff2233;}"
                       "QPushButton[flat=true]{background:#1e1e28;color:#8b8fa8;border:1.5px solid #2a2a38;}"
                       "QPushButton[flat=true]:hover{background:#28283a;color:#f0f0f5;}");
    auto* vb=new QVBoxLayout(dlg);vb->setContentsMargins(28,24,28,24);vb->setSpacing(0);
    auto* fl=new QFormLayout;fl->setSpacing(10);fl->setLabelAlignment(Qt::AlignRight);
    auto mkIn=[](const QString&ph,const QString&val="")->QLineEdit*{auto*e=new QLineEdit;e->setPlaceholderText(ph);e->setText(val);return e;};
    auto* eTitlu=mkIn("ex: Avengers",ex?QString::fromStdString(ex->getTitlu()):"");
    auto* eGen  =mkIn("ex: Actiune", ex?QString::fromStdString(ex->getGen()):"");
    auto* eReg  =mkIn("ex: James Cameron",ex?QString::fromStdString(ex->getRegizor()):"");
    auto* eDist =mkIn("ex: Actor A, Actor B",ex?QString::fromStdString(ex->getDistributie()):"");
    auto* eData =mkIn("ex: 05 iunie 2026",ex?QString::fromStdString(ex->getDataPremiera()):"");
    auto* eTara =mkIn("ex: SUA 2026",ex?QString::fromStdString(ex->getTara()):"");
    auto* eLimb =mkIn("ex: EN",ex?QString::fromStdString(ex->getLimba()):"EN");
    auto* eClas =mkIn("ex: AG / AP12",ex?QString::fromStdString(ex->getClasificare()):"AG");
    auto* ePost =mkIn("ex: posters/film.jpg",ex?QString::fromStdString(ex->getPosterPath()):"");
    auto* eDesc=new QTextEdit;eDesc->setPlaceholderText("Descriere film…");eDesc->setFixedHeight(80);if(ex)eDesc->setPlainText(QString::fromStdString(ex->getDescriere()));
    auto* sDur=new QSpinBox;sDur->setRange(1,360);sDur->setSuffix(" min");sDur->setValue(ex?ex->getDurata():90);
    auto* sPret=new QDoubleSpinBox;sPret->setRange(1,999);sPret->setSuffix(" RON");sPret->setDecimals(2);sPret->setValue(ex?ex->getPretBaza():25);
    auto* sRat=new QDoubleSpinBox;sRat->setRange(0,10);sRat->setSingleStep(0.1);sRat->setDecimals(1);sRat->setValue(ex?ex->getRating():0);
    auto* cbStat=new QComboBox;cbStat->addItems({"Rulează acum","În curând"});if(ex&&ex->getStatus()==StatusFilm::IN_CURAND)cbStat->setCurrentIndex(1);
    auto* chk3D=new QCheckBox("Film 3D");chk3D->setChecked(ex&&ex->getEste3D());
    fl->addRow("Titlu *",eTitlu);fl->addRow("Gen *",eGen);fl->addRow("Durată",sDur);fl->addRow("Preț bază",sPret);
    fl->addRow("Rating /10",sRat);fl->addRow("Status",cbStat);fl->addRow("",chk3D);fl->addRow("Descriere",eDesc);
    fl->addRow("Regizor",eReg);fl->addRow("Distribuție",eDist);fl->addRow("Premieră",eData);
    fl->addRow("Producție",eTara);fl->addRow("Limbă",eLimb);fl->addRow("Clasificare",eClas);fl->addRow("Poster path",ePost);
    vb->addLayout(fl);vb->addSpacing(20);
    auto* bRow=new QHBoxLayout;bRow->addStretch();
    auto* bOk=new QPushButton(ex?"Salvează":"Adaugă");bOk->setDefault(true);
    auto* bCan=new QPushButton("Anulează");bCan->setProperty("flat",true);
    bRow->addWidget(bCan);bRow->addWidget(bOk);vb->addLayout(bRow);
    connect(bOk,&QPushButton::clicked,dlg,&QDialog::accept);
    connect(bCan,&QPushButton::clicked,dlg,&QDialog::reject);
    if(dlg->exec()!=QDialog::Accepted){delete dlg;return false;}
    if(eTitlu->text().trimmed().isEmpty()){showToast("Titlul este obligatoriu!");delete dlg;return false;}
    out=Film(eTitlu->text().trimmed().toStdString(),chk3D->isChecked(),sPret->value(),
             eGen->text().trimmed().toStdString(),sDur->value(),
             cbStat->currentIndex()==1?StatusFilm::IN_CURAND:StatusFilm::RULAZA_ACUM,
             sRat->value(),eDesc->toPlainText().toStdString(),eReg->text().trimmed().toStdString(),
             eDist->text().trimmed().toStdString(),eData->text().trimmed().toStdString(),
             eTara->text().trimmed().toStdString(),eLimb->text().trimmed().toStdString(),
             eClas->text().trimmed().toStdString(),ePost->text().trimmed().toStdString());
    delete dlg;return true;
}
void MainWindow::onAdminAddFilm(){
    Film f("",false,25);if(!showFilmDialog(f))return;
    cinema.adaugaFilmManual(f.getTitlu(),f.getEste3D(),f.getPretBaza(),f.getGen(),f.getDurata(),f.getStatus(),
        f.getRating(),f.getDescriere(),f.getRegizor(),f.getDistributie(),f.getDataPremiera(),
        f.getTara(),f.getLimba(),f.getClasificare(),f.getPosterPath());
    refreshAdminFilms();refreshUpcomingStrip();showToast("Film adăugat!",false);
}
void MainWindow::onAdminEditFilm(){
    int idx=lwAdminFilms->currentRow();if(idx<0){showToast("Selectează un film!");return;}
    auto&filme=cinema.getFilme();if(idx>=(int)filme.size())return;
    Film f=filme[idx];if(!showFilmDialog(f,&filme[idx]))return;
    cinema.editeazaFilm(idx,f.getTitlu(),f.getEste3D(),f.getPretBaza(),f.getGen(),f.getDurata(),f.getStatus(),
        f.getRating(),f.getDescriere(),f.getRegizor(),f.getDistributie(),f.getDataPremiera(),
        f.getTara(),f.getLimba(),f.getClasificare(),f.getPosterPath());
    refreshAdminFilms();refreshUpcomingStrip();
}
void MainWindow::onAdminDeleteFilm(){
    int idx=lwAdminFilms->currentRow();if(idx<0){showToast("Selectează un film!");return;}
    if(QMessageBox::question(this,"Confirmare","Ștergi '"+QString::fromStdString(cinema.getFilme()[idx].getTitlu())+"'?")!=QMessageBox::Yes)return;
    cinema.stergeFilm(idx);refreshAdminFilms();refreshUpcomingStrip();
}

void MainWindow::onAdminDeleteUser(){
    int idx=lwAdminUsers->currentRow();if(idx<0){showToast("Selectează un utilizator!");return;}
    auto&users=cinema.getUtilizatori();if(idx>=(int)users.size())return;
    QString usr=QString::fromStdString(users[idx].username);
    if(usr=="admin"){showToast("Nu poți șterge adminul!");return;}
    if(QMessageBox::question(this,"Confirmare","Ștergi utilizatorul '"+usr+"'?")!=QMessageBox::Yes)return;
    cinema.stergeUtilizator(usr.toStdString());refreshAdminUsers();
}
