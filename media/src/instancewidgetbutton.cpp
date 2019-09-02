#include "instancewidgetbutton.hpp"


BoxWidgetButton::BoxWidgetButton(const BoxWidget* shouldBparent,  QWidget* parent,  QString txt)  :  QPushButton(txt, parent), shouldBparent(shouldBparent) {
}
