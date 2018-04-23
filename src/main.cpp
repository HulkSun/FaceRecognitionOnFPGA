#include <QCoreApplication>
#include "main_widget.h"
#include "log_system.h"
extern LogSystem* logSystem;
extern QString QstrLogDir;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QFont font = app.font();
    font.setPointSize(13);
    app.setFont(font);

    ReadConfig("../config.ini");
    logSystem = new LogSystem(QstrLogDir);
//    qInstallMessageHandler(MessageOutput);
    if (!HistoryDbConnect()) {
        return -1;
    }

    MainWidget main_widget;
    main_widget.show();

    return app.exec();
}
