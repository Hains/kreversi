/*
    SPDX-FileCopyrightText: 2006 Dmitry Suzdalev <dimsuz@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>

#include <KAboutData>
#include <KLocalizedString>
#include <KCrash>
#include <KDBusService>
#define HAVE_KICONTHEME __has_include(<KIconTheme>)
#if HAVE_KICONTHEME
#include <KIconTheme>
#endif

#define HAVE_STYLE_MANAGER __has_include(<KStyleManager>)
#if HAVE_STYLE_MANAGER
#include <KStyleManager>
#endif
#include "highscores.h"
#include "mainwindow.h"
#include "kreversi_version.h"

int main(int argc, char **argv)
{
    QApplication application(argc, argv);
#if HAVE_STYLE_MANAGER
    KStyleManager::initStyle();
#else // !HAVE_STYLE_MANAGER
#if defined(Q_OS_MACOS) || defined(Q_OS_WIN)
    QApplication::setStyle(QStringLiteral("breeze"));
#endif // defined(Q_OS_MACOS) || defined(Q_OS_WIN)
#endif // HAVE_STYLE_MANAGER
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("kreversi"));

    KAboutData aboutData(QStringLiteral("kreversi"), i18n("KReversi"),
                         QStringLiteral(KREVERSI_VERSION_STRING),
                         i18n("KDE Reversi Board Game"),
                         KAboutLicense::GPL,
                         i18n("(c) 1997-2000, Mario Weilguni\n(c) 2004-2006, Inge Wallin\n(c) 2006, Dmitry Suzdalev"),
                         QString(),
                         QStringLiteral("https://apps.kde.org/kreversi"));
    aboutData.addAuthor(i18n("Mario Weilguni"), i18n("Original author"), QStringLiteral("mweilguni@sime.com"));
    aboutData.addAuthor(i18n("Inge Wallin"), i18n("Original author"), QStringLiteral("inge@lysator.liu.se"));
    aboutData.addAuthor(i18n("Dmitry Suzdalev"), i18n("Game rewrite for KDE4. Current maintainer."), QStringLiteral("dimsuz@gmail.com"));
    aboutData.addCredit(i18n("Simon Hürlimann"), i18n("Action refactoring"));
    aboutData.addCredit(i18n("Mats Luthman"), i18n("Game engine, ported from his JAVA applet."));
    aboutData.addCredit(i18n("Arne Klaassen"), i18n("Original raytraced chips."));
    aboutData.addCredit(i18n("Mauricio Piacentini"), i18n("Vector chips and background for KDE4."));
    aboutData.addCredit(i18n("Brian Croom"), i18n("Port rendering code to KGameRenderer"), QStringLiteral("brian.s.croom@gmail.com"));
    aboutData.addCredit(i18n("Denis Kuplyakov"), i18n("Port rendering code to QML, redesign and a lot of improvements"), QStringLiteral("dener.kup@gmail.com"));

    KAboutData::setApplicationData(aboutData);
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("kreversi")));

    KCrash::initialize();

    QCommandLineParser parser;
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("demo"), i18n("Start with demo game playing")));
    aboutData.setupCommandLine(&parser);
    parser.process(application);
    aboutData.processCommandLine(&parser);

    KDBusService service;

    if (application.isSessionRestored()) {
        kRestoreMainWindows<KReversiMainWindow>();
    } else {
        KReversiMainWindow *mainWin = new KReversiMainWindow(nullptr, parser.isSet(QStringLiteral("demo")));
        mainWin->show();
    }

    KExtHighscore::ExtManager highscoresManager;

    return application.exec();
}
