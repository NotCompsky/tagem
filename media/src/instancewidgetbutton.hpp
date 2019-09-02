#ifndef __INSTANCEWIDGETBUTTON__
#define __INSTANCEWIDGETBUTTON__

#include <QPushButton>
#include <QString>


class BoxWidget;

class BoxWidgetButton : public QPushButton{
 public:
    BoxWidgetButton(const BoxWidget* shouldBparent,  QWidget* parent,  QString txt = "");
    const BoxWidget* shouldBparent;
};

#endif
