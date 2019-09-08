#include "box_widget_btn.hpp"


BoxWidgetButton::BoxWidgetButton(const BoxWidget* shouldBparent,  QWidget* parent,  QString txt)  :  QPushButton(txt, parent), shouldBparent(shouldBparent) {
}
