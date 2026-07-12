#include "WindowAbout.h"

WindowAbout::WindowAbout(QWidget* parent) :
    QDialog(parent)
{
    setFixedSize(420, 300);
    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* label = new QLabel(
        QString("<table cellspacing=\"6\">"
                "<tr><td colspan=\"2\"><b>MHY_Scanner</b></td></tr>"
                "<tr><td>版本</td><td>%1</td></tr>"
                "<tr><td>Qt Version</td><td>%2</td></tr>"
                "<tr><td>项目地址</td><td><a href=\"https://github.com/xiaomaimainp/MHY_Scanner\">https://github.com/xiaomaimainp/MHY_Scanner</a></td></tr>"
                "</table><br><br>"
                "本程序修改自开源项目 MHY_Scanner，感谢原作者的贡献：<br>"
                "DSVVA：<a href=\"https://github.com/DSVVA/MHY_Scanner\">https://github.com/DSVVA/MHY_Scanner</a><br>"
                "loqwe：<a href=\"https://github.com/loqwe/MHY_Scanner2\">https://github.com/loqwe/MHY_Scanner2</a>")
            .arg(MHY_Scanner_VERSION, QT_VERSION_STR),
        this);
    label->setOpenExternalLinks(true);
    label->setWordWrap(true);
    layout->addWidget(label, 0, Qt::AlignCenter);
    setLayout(layout);
}