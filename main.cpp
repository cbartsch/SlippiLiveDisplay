#include <QApplication>
#include <FelgoApplication>

#include <QQmlApplicationEngine>

#include "dolphinconnection.h"
#include "eventparser.h"
#include "enet/enet.h"

// uncomment this line to add the Live Client Module and use live reloading with your custom C++ code
//#include <FelgoLiveClient>

int main(int argc, char *argv[])
{
    if (enet_initialize () != 0)
    {
        qWarning() << "An error occurred while initializing ENet.";
        return EXIT_FAILURE;
    }

    QApplication app(argc, argv);

    FelgoApplication felgo;

    // Use platform-specific fonts instead of Felgo's default font
    felgo.setPreservePlatformFonts(true);

    QQmlApplicationEngine engine;
    felgo.initialize(&engine);

    // Set an optional license key from project file
    // This does not work if using Felgo Live, only for Felgo Cloud Builds and local builds
    felgo.setLicenseKey(PRODUCT_LICENSE_KEY);

    // use this during development
    // for PUBLISHING, use the entry point below
    felgo.setMainQmlFileName(QStringLiteral("qml/Main.qml"));

    // use this instead of the above call to avoid deployment of the qml files and compile them into the binary with qt's resource system qrc
    // this is the preferred deployment option for publishing games to the app stores, because then your qml files and js files are protected
    // to avoid deployment of your qml files and images, also comment the DEPLOYMENTFOLDERS command in the .pro file
    // also see the .pro file for more details
    //felgo.setMainQmlFileName(QStringLiteral("qrc:/qml/Main.qml"));

    qmlRegisterType<DolphinConnection>("SlippiLive", 1, 0, "DolphinConnection");
    qmlRegisterType<EventParser>("SlippiLive", 1, 0, "SlippiEventParser");
    qmlRegisterUncreatableType<GameInformation>("SlippiLive", 1, 0, "GameInfo", "Only used for EventParser.gameInfo");

    engine.load(QUrl(felgo.mainQmlFileName()));

    // to start your project as Live Client, comment (remove) the lines "felgo.setMainQmlFileName ..." & "engine.load ...",
    // and uncomment the line below
    //FelgoLiveClient client (&engine);

    auto ret = app.exec();

    qDebug() << "Exiting with code" << ret;

    enet_deinitialize();

    return ret;
}