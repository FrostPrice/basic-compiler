#include <QApplication>
#include <QDebug>
#include <QFontDatabase>
#include "./ui/main_window.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    int fontId = QFontDatabase::addApplicationFont(":/src/assets/fonts/Monocraft.ttc");
    if (fontId == -1)
    {
        qWarning() << "Failed to load custom font!";
    }
    else
    {
        QString fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
        QFont customFont(fontFamily);
        app.setFont(customFont);
    }

    MainWindow w;
    w.show();
    return app.exec();
}
