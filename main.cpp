#include <QApplication>
#include <QIcon>
#include <QProxyStyle>

#include "main_window.h"

class FastToolTipStyle final : public QProxyStyle {
public:
    explicit FastToolTipStyle(const QString &baseStyle)
        : QProxyStyle(baseStyle) {
    }

    int styleHint(const StyleHint hint, const QStyleOption *option, const QWidget *widget,
                  QStyleHintReturn *returnData) const override {
        if (hint == SH_ToolTip_WakeUpDelay) {
            return 350;
        }
        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
};

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/resgoth.png")));
    a.setStyle(new FastToolTipStyle(a.style()->name()));

    QApplication::setOrganizationName("Resgoth");
    QApplication::setApplicationName("Resgoth");

    MainWindow window;
    window.show();
    return QApplication::exec();
}
