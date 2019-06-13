#include "instancewidgetbutton.hpp"


InstanceWidgetButton::InstanceWidgetButton(const InstanceWidget* shouldBparent,  QWidget* parent,  QString txt)  :  QPushButton(txt, parent), shouldBparent(shouldBparent) {
}
