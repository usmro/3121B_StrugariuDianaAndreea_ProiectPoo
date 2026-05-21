#include <QApplication>
#include <QFont>
#include <QDir>
#include "mainwindow.h"

static const char* CSS = R"(
* { font-family: 'Segoe UI', 'Arial', sans-serif; outline: none; }
QScrollBar:vertical   { background:#111116; width:6px; border-radius:3px; margin:0; }
QScrollBar::handle:vertical   { background:#36364a; border-radius:3px; min-height:36px; }
QScrollBar::handle:vertical:hover   { background:#55557a; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical,
QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background:none; border:none; height:0; }
QScrollBar:horizontal { background:#111116; height:6px; border-radius:3px; margin:0; }
QScrollBar::handle:horizontal { background:#36364a; border-radius:3px; min-width:36px; }
QScrollBar::handle:horizontal:hover { background:#55557a; }
QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal,
QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background:none; border:none; width:0; }
QToolTip { background:#1e1e2a; color:#e5e5f0; border:1px solid #2a2a3a; padding:6px 12px; border-radius:6px; font-size:12px; }
QMessageBox { background:#141418; }
QMessageBox QLabel { color:#e5e5f0; font-size:14px; padding:4px; }
QMessageBox QPushButton { background:#e50914; color:white; border:none; border-radius:7px; padding:9px 24px; font-size:13px; font-weight:700; min-width:80px; }
QMessageBox QPushButton:hover   { background:#ff2233; }
QMessageBox QPushButton:pressed { background:#a0000e; }
QInputDialog { background:#141418; }
QInputDialog QLabel    { color:#e5e5f0; font-size:14px; }
QInputDialog QLineEdit { background:#1e1e28; color:#f0f0f5; border:1.5px solid #2a2a38; border-radius:8px; padding:10px 14px; font-size:14px; }
QInputDialog QLineEdit:focus { border-color:#e50914; }
QInputDialog QPushButton { background:#e50914; color:white; border:none; border-radius:7px; padding:9px 22px; font-size:13px; font-weight:700; min-width:70px; }
QInputDialog QPushButton:hover { background:#ff2233; }
QInputDialog QComboBox { background:#1e1e28; color:#f0f0f5; border:1.5px solid #2a2a38; border-radius:8px; padding:8px 14px; }
QInputDialog QComboBox QAbstractItemView { background:#1e1e28; color:#f0f0f5; selection-background-color:#e50914; border:1px solid #2a2a38; }
QTextEdit { background:#0a0a0f; color:#e0e0ea; border:1px solid #1e1e2a; border-radius:8px; selection-background-color:#e50914; }
)";

int main(int argc, char* argv[]){
    QApplication app(argc, argv);
    app.setStyle("Fusion");
    QPalette pal;
    pal.setColor(QPalette::Window,        QColor("#0a0a0f"));
    pal.setColor(QPalette::WindowText,    Qt::white);
    pal.setColor(QPalette::Base,          QColor("#111116"));
    pal.setColor(QPalette::AlternateBase, QColor("#18181f"));
    pal.setColor(QPalette::ToolTipBase,   QColor("#1e1e2a"));
    pal.setColor(QPalette::ToolTipText,   Qt::white);
    pal.setColor(QPalette::Text,          Qt::white);
    pal.setColor(QPalette::Button,        QColor("#1e1e28"));
    pal.setColor(QPalette::ButtonText,    Qt::white);
    pal.setColor(QPalette::Highlight,     QColor("#e50914"));
    pal.setColor(QPalette::HighlightedText, Qt::white);
    app.setPalette(pal);
    app.setStyleSheet(CSS);
    QFont f("Segoe UI",10);
    f.setHintingPreference(QFont::PreferFullHinting);
    app.setFont(f);
    

    QDir::setCurrent(QCoreApplication::applicationDirPath());
    MainWindow w; w.resize(1100,740); w.show();
    return app.exec();
}
