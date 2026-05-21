#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QListWidget>
#include <QComboBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QInputDialog>
#include <QFrame>
#include <QTextEdit>
#include <QScrollArea>
#include <QTabWidget>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QTimer>
#include <QFormLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QToolTip>
#include <QDesktopServices>
#include <QUrl>
#include "Cinema.h"
#include "config.h"


class SeatMapWidget : public QWidget {
    Q_OBJECT
public:
    bool readOnly = false;
    explicit SeatMapWidget(QWidget* parent = nullptr);
    void load(Sala* sala, const Data& d, const std::string& ora);
    void setSelected(const std::vector<std::pair<int,int>>& seats);
    void setPretBaza(double p){ pretBaza_=p; }
    void setPreferinta(const PreferintaLoc& p){ preferinta_=p; update(); }
    std::vector<std::pair<int,int>> getSelected() const { return selected_; }
signals:
    void seatClicked(int row, int col);
protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void leaveEvent(QEvent*) override;
    QSize sizeHint() const override;
private:
    Sala*       sala = nullptr;
    Data        data = {1,1,2025};
    std::string ora;
    std::vector<std::pair<int,int>> selected_;
    int hoverRow=-1, hoverCol=-1;
    double pretBaza_ = 0.0;
    PreferintaLoc preferinta_;
    static constexpr int S=52,G=10,ML=72,MT=120;
    QRect              seatRect(int r,int c) const;
    std::pair<int,int> seatAt(QPoint p)     const;
};


class MenuCard : public QFrame {
    Q_OBJECT
public:
    MenuCard(const QString& emoji,const QString& title,const QString& desc,bool gold=false,QWidget* parent=nullptr);
signals:
    void clicked();
protected:
    void mousePressEvent(QMouseEvent* e) override { if(e->button()==Qt::LeftButton) emit clicked(); QFrame::mousePressEvent(e); }
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    void enterEvent(QEnterEvent*) override { setStyleSheet(hov_); }
#else
    void enterEvent(QEvent*)     override { setStyleSheet(hov_); }
#endif
    void leaveEvent(QEvent*)     override { setStyleSheet(nor_); }
private:
    QString nor_, hov_;
};


class StepBar : public QWidget {
    Q_OBJECT
public:
    explicit StepBar(QWidget* parent=nullptr);
    void setStep(int step);
protected:
    void paintEvent(QPaintEvent*) override;
    QSize sizeHint() const override { return {0,52}; }
private:
    int current_=0;
    static const QStringList STEPS;
};


class LocationCard : public QFrame {
    Q_OBJECT
public:
    LocationCard(const CinemaLocation& loc, bool isCurrentCity, QWidget* parent=nullptr);
    QString locationId() const { return id_; }
signals:
    void clicked(const QString& id);
protected:
    void mousePressEvent(QMouseEvent* e) override { if(e->button()==Qt::LeftButton) emit clicked(id_); QFrame::mousePressEvent(e); }
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    void enterEvent(QEnterEvent*) override { setStyleSheet(hov_); }
#else
    void enterEvent(QEvent*)     override { setStyleSheet(hov_); }
#endif
    void leaveEvent(QEvent*)     override { setStyleSheet(nor_); }
private:
    QString id_, nor_, hov_;
};


enum Page {
    P_LOGIN=0, P_LOCATION, P_MAIN,
    P_RESERVE_FILM, P_RESERVE_SALA, P_RESERVE_DATA, P_RESERVE_ORA,
    P_RESERVE_LOC, P_RESERVE_FOOD, P_RESERVE_CONFIRM,
    P_BROWSE, P_MAP, P_CANCEL, P_PROFILE, P_ADMIN,
    P_FILM_DETAIL, P_CONFIRM_DONE
};


class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent=nullptr);
    ~MainWindow();

private slots:
    

    void onLogin(); void onRegister(); void onLogout();
    

    void onLocationDetected(); void onLocationSelected(const QString& id);
    

    void onNewReservation(); void onBrowseGenuri(); void onHartaSala();
    void onAnulareRezervare(); void onProfilMeu(); void onAdminPanel();
    

    void onFilmSelected(); void onProceedFromDetail();
    void onDataSelected();
    void onShowingSelected(const Programare* p);
    void onLocSelected(int row, int col);
    void onFoodConfirmed(); void onFoodSkipped();
    void onConfirmReservation(); void onCancelReservation();
    void onRateFilm(const QString& titlu, int stele);
    void onExportCalendar();
    

    void onGenChanged(int idx); void updateBrowseDetail(int row); void onBrowseReserve();
    

    void onAdminAddFilm(); void onAdminEditFilm(); void onAdminDeleteFilm();
    void onAdminDeleteUser();
    

    void refreshHeroBanner(); void refreshUpcomingStrip();
    void updateSeatLegendCounts(); void fetchPoster(Film* f);
    void showToast(const QString& msg, bool isError=true);
    void refreshMapOra();   

    void refreshCancelOra();

private:
    Cinematograf cinema;

    

    Film*       rezFilm=nullptr;
    int         rezSalaId=-1;
    Data        rezData={1,1,2025};
    std::string rezOra;
    const Programare* rezProgramare=nullptr;       

    std::vector<std::pair<int,int>> rezLocuri;     

    int         maxLocuri=8;
    FoodOrder   rezFood;
    int         cancelRow=-1, cancelCol=-1;
    int         browse3D=-1;
    bool        browseActiveOnly=false;
    Page        detailFromPage=P_RESERVE_FILM;
    int         filmFilter3D=-1;

    

    QString     detectedCity;
    QNetworkAccessManager* netManager=nullptr;

    

    QStackedWidget* stack;
    QWidget*        navbar;
    QLabel*         lblNavUser, *lblNavLocation;
    StepBar*        stepBar;

    

    QLineEdit *leUser, *lePass;

    

    QWidget*    locationContainer;
    QLabel*     lblDetectedCity;
    QPushButton* btnDetectLocation;

    

    QLabel*   lblWelcome;
    MenuCard* cardAdmin;
    QLabel    *lblHeroTitle,*lblHeroInfo,*lblHeroBadge,*lblHeroPoster;
    QFrame    *heroFrame;
    QTimer    *heroTimer;
    int        heroIdx=0;
    QWidget   *upcomingContainer;
    QFrame    *upcomingSection;

    

    QLabel      *detailTitle,*detailGenreRating,*detailDesc,
                *detailPoster,*detailMeta,*detailPrice;
    QPushButton *btnDetailBook;
    QLabel      *detailUnavail;

    

    QLineEdit*      leSearch;
    QComboBox*      cbFilmGenre;
    QPushButton*    btnFilm3D;
    QListWidget*    lwFilms;
    QLabel*         lblFilmsEmpty;
    QStackedWidget* filmListStack;
    std::vector<Film*> searchResults;

    

    QScrollArea* showingsScroll;
    QWidget*     showingsContainer;
    QLabel*      lblNoShowings;
    const Programare* hoveredProgramare=nullptr;

    

    QListWidget *lwData;

    QLabel      *miniData,*miniShow,*miniLoc,*miniFood,*miniConf;
    SeatMapWidget* seatMapWidget;
    QLabel*        lblSeatInfo,*lblSeatCount;
    QLabel        *lblLegStd,*lblLegVip,*lblLegCpl;
    QPushButton*   btnClearSeats;

    

    std::vector<QPushButton*> foodQtyBtns;
    std::vector<QLabel*>      foodQtyLabels;
    QLabel*                   lblFoodTotal;

    

    QLabel*    lblConfirmDetails;
    QLineEdit* leEmail;
    QLabel*    lblBookingRef;

    

    QLabel      *lblDoneRef,*lblDoneSummary;
    QPushButton *btnExportCal;

    

    QScrollArea* profileScroll;
    QWidget*     profileContainer;

    

    QComboBox*   cbGenuri;
    QListWidget* lwBrowseFilms;
    QLabel*      lblBrowseDetail;
    QPushButton *btnBrowseFilter3D,*btnBrowseFilterActive;

    

    SeatMapWidget* mapWidget;
    QComboBox     *cbMapSala,*cbMapData,*cbMapOra;

    

    SeatMapWidget* cancelWidget;
    QComboBox     *cbCancelSala,*cbCancelData,*cbCancelOra;

    

    QTextEdit* teHistory;

    

    QTabWidget*  adminTabs;
    QTextEdit   *teAllRes,*teReport;
    QListWidget *lwAdminFilms,*lwAdminOre,*lwAdminUsers;

    

    QLabel*  toastLabel = nullptr;
    QTimer*  toastTimer = nullptr;
    

    QWidget* buildNavbar();
    QWidget* buildLoginPage();
    QWidget* buildLocationPage();
    QWidget* buildMainPage();
    QWidget* buildFilmDetailPage();
    QWidget* buildReserveFilmPage();
    QWidget* buildReserveDataPage();
    QWidget* buildReserveShowingPage();   

    QWidget* buildReserveLocPage();
    QWidget* buildReserveFoodPage();
    QWidget* buildReserveConfirmPage();
    QWidget* buildBrowsePage();
    QWidget* buildMapPage();
    QWidget* buildCancelPage();
    QWidget* buildProfilePage();
    QWidget* buildAdminPage();
    QWidget* buildConfirmDonePage();

    

    void goTo(Page p);
    void refreshMainMenu();
    void refreshLocationPage(const QString& city);
    void refreshSearchResults();
    void refreshMapPage();
    void refreshCancelSeats();
    void refreshAdminFilms();
    void refreshAdminUsers();
    void refreshAdminReservations();
    void refreshAdminReport();
    void refreshAdminStatsCards();
    void refreshBrowse();
    void refreshProfile();
    void refreshProfileRatings();
    void refreshFilmMiniHeaders();
    void refreshFoodPage();
    void buildConfirmPage();
    void populateFilmDetail(Film* f);
    QFrame* filmMiniCard(QLabel*& lbl);
    bool    showFilmDialog(Film& out,const Film* existing=nullptr);
};
