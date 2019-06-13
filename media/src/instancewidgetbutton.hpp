#ifndef __INSTANCEWIDGETBUTTON__
#define __INSTANCEWIDGETBUTTON__

#include <QPushButton>
#include <QString>


class InstanceWidget;

class InstanceWidgetButton : public QPushButton{
 public:
    InstanceWidgetButton(const InstanceWidget* shouldBparent,  QWidget* parent,  QString txt = "");
    const InstanceWidget* shouldBparent;
};

#endif
