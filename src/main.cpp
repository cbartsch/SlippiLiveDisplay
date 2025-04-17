#include <QApplication>
#include <FelgoApplication>

#include <QQmlApplicationEngine>

#include <QDir>
#include <QFileInfo>
#include <QMutex>

#include "dolphinconnection.h"
#include "eventparser.h"
#include "enet/enet.h"

// uncomment this line to add the Live Client Module and use live reloading with your custom C++ code
//#include <FelgoLiveClient>

static QtMessageHandler origMessageHandler;
static QFileInfo logFileInfo;

void logMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  static QMutex mutex;
  QMutexLocker lock(&mutex);

  // skip useless logs from Qt bugs - TODO fix all that come from app code (some come from libraries)
  if(msg.startsWith("qrc:/qt-project.org/imports/QtQuick/Controls/macOS/") ||
      msg.contains("QML Connections: Implicitly defined onFoo") ||
      msg.contains("MeleeData is not defined")) {
    return;
  }

  QFile logFile(logFileInfo.absoluteFilePath());

  if (logFile.open(QIODevice::Append | QIODevice::Text)) {
    logFile.write(qFormatLogMessage(type, context, msg).toUtf8() + '\n');
    logFile.flush();
  }

  origMessageHandler(type, context, msg);
}

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  FelgoApplication felgo;

  // Use platform-specific fonts instead of Felgo's default font
  felgo.setPreservePlatformFonts(true);

  QQmlApplicationEngine engine;
  felgo.initialize(&engine);

  QFileInfo logFileDir(qApp->applicationFilePath());
#ifdef Q_OS_MAC
  // on Mac the executable is at Slippipedia.app/Contents/MacOS/Slippipedia -> remove those paths for the log file:
  logFileDir = QFileInfo(logFileDir.dir(), "../../../..");
#endif

  logFileInfo = QFileInfo(logFileDir.dir(), "log.txt");
  qDebug() << "Logging to external file:" << logFileInfo.absoluteFilePath();

  origMessageHandler = qInstallMessageHandler(logMessageHandler);

  qDebug().noquote() << "\n\nSlippiLiveDisplay started at" << QDateTime::currentDateTime().toString(Qt::DateFormat::ISODate);
  qDebug() << "------------------------------------------\n";

  if (enet_initialize () != 0)
  {
    qWarning() << "An error occurred while initializing ENet.";
    return EXIT_FAILURE;
  }

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
  qmlRegisterUncreatableType<GameInformation>("SlippiLive", 1, 0, "GameInformation", "Only used for EventParser.gameInfo");
  qmlRegisterUncreatableType<PlayerInformation>("SlippiLive", 1, 0, "PlayerInformation", "Only used for EventParser.gameInfo.playerN");

  engine.load(QUrl(felgo.mainQmlFileName()));

  // to start your project as Live Client, comment (remove) the lines "felgo.setMainQmlFileName ..." & "engine.load ...",
  // and uncomment the line below
  //FelgoLiveClient client (&engine);

  auto ret = app.exec();

  qDebug() << "Exiting with code" << ret;

  enet_deinitialize();

  return ret;
}
