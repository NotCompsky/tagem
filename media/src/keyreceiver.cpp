#include "keyreceiver.hpp"

#include <QApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QRubberBand>

#ifdef BOXABLE
# include "instancewidget.hpp"
#endif
#include "mainwindow.hpp"


constexpr static const short SCROLL_INTERVAL = 1;

constexpr static const int numeric_keys[20] = {
	Qt::Key_0,
	Qt::Key_1,
	Qt::Key_2,
	Qt::Key_3,
	Qt::Key_4,
	Qt::Key_5,
	Qt::Key_6,
	Qt::Key_7,
	Qt::Key_8,
	Qt::Key_9,
	Qt::Key_ParenRight, // 0
	Qt::Key_Exclam,
	Qt::Key_QuoteDbl,
	Qt::Key_sterling,
	Qt::Key_Dollar,
	Qt::Key_Percent,
	Qt::Key_AsciiCircum,
	Qt::Key_Ampersand,
	Qt::Key_Asterisk,
	Qt::Key_ParenLeft
};

int key2n(const int key){
	for (auto i = 0;  i < 20;  ++i)
		if (numeric_keys[i] == key)
			return i;
	return -1;
}


bool KeyReceiver::eventFilter(QObject* obj, QEvent* event)
// src: https://wiki.qt.io/How_to_catch_enter_key
{
    Qt::KeyboardModifiers kb_mods = QApplication::queryKeyboardModifiers();
    switch(event->type()){
        case QEvent::Wheel:{ // Mouse wheel rolled
            QWheelEvent* wheel_event = static_cast<QWheelEvent*>(event);
          #ifdef TXT
            short direction  =  (wheel_event->delta() > 0 ? SCROLL_INTERVAL : -1 * SCROLL_INTERVAL);
            return true;
          #endif
          #ifdef SCROLLABLE
            if ((kb_mods & Qt::ControlModifier) == 0)
                // Scroll (default) unless CTRL key is down
                return true;
            double factor  =  (wheel_event->delta() > 0 ? 1.25 : 0.80);
            window->rescale_main(factor);
          #endif
            return true;
        }
      #ifdef BOXABLE
        case QEvent::MouseButtonPress:{
            QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);
            window->mouse_dragged_from = mouse_event->pos();
          #ifdef SCROLLABLE
            window->mouse_dragged_from += window->get_scroll_offset();
            /*
            if (window->mouse_dragged_from.x() < 0)
                window->mouse_dragged_from.setX(0);
            else if (window->mouse_dragged_from.x() > window->main_widget->size().width())
                window->mouse_dragged_from.setX(window->main_widget->size().width());
            if (window->mouse_dragged_from.y() < 0)
                window->mouse_dragged_from.setY(0);
            else if (window->mouse_dragged_from.y() > window->main_widget->size().height())
                window->mouse_dragged_from.setY(window->main_widget->size().height());
            */
          #endif
            window->is_mouse_down = true;
            if (window->instance_widget != nullptr){
                delete window->instance_widget;
                window->instance_widget = nullptr;
                return true;
            }
            window->instance_widget = new InstanceWidget(QRubberBand::Rectangle, window, window->main_widget);
            window->boundingbox_geometry = QRect(window->mouse_dragged_from, QSize());
            QRect r = window->boundingbox_geometry;
            window->instance_widget->setGeometry(r);
            window->instance_widget->show();
            return true;
        }
        case QEvent::MouseButtonRelease:{
            window->is_mouse_down = false;
            return true;
        }
        case QEvent::MouseMove:{
            QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);
            window->mouse_dragged_to = mouse_event->pos();
          #ifdef SCROLLABLE
            window->mouse_dragged_to += window->get_scroll_offset();
          #endif
            if (!window->is_mouse_down  ||  window->instance_widget == nullptr){
                window->display_instance_mouseover();
                return true;
            }
            window->instance_widget->setGeometry(QRect(window->mouse_dragged_from, window->mouse_dragged_to).normalized());
            return true;
        }
      #endif
        case QEvent::KeyPress:{
            QKeyEvent* key = static_cast<QKeyEvent*>(event);
            switch(int keyval = key->key()){
                case Qt::Key_Enter:
                case Qt::Key_Return:
                case Qt::Key_D:
                    window->media_next(); // Causes SEGFAULT, even though clicking on "Next" button is fine.
                    break;
                case Qt::Key_I:
                  #ifdef TXT // No need for text editor to select rectangles
                    window->unset_read_only();
                   #ifdef BOXABLE
                    #error "TXT and BOXABLE are mutually exclusive"
                   #endif
                  #endif
                  #ifdef BOXABLE
					if (window->instance_widget != nullptr){
						window->create_instance();
						break;
					}
                  #endif
					window->display_info();
                    break;
                case Qt::Key_L:
                    window->media_linkfrom();
                    break;
                case Qt::Key_N:
                    window->media_note();
                    break;
				case Qt::Key_R: // Rate
					window->show_settings_dialog();
					break;
                case Qt::Key_S: // Save
                  #ifdef TXT
                    window->media_save();
                  #endif
                    break;
                case Qt::Key_T:
                    window->media_tag("");
                    break;
                case Qt::Key_O:
                    window->media_overwrite();
                    break;
				case Qt::Key_Q:
					window->media_score();
					break;
                case Qt::Key_X:
                    window->media_delete();
                    window->media_next();
                    break;
                case Qt::Key_Escape:
                  #ifdef TXT
                    window->set_read_only();
                  #endif
                    break;
                case Qt::Key_Space:
                  #ifdef VID
                    window->playPause();
                  #endif
                    break;
				case Qt::Key_AsciiTilde:
					break;
                case Qt::Key_BracketLeft:
                  #ifdef VID
                    if (window->volume > 0){
                        window->volume -= 0.05;
                        window->m_player->audio()->setVolume(window->volume);
                    }
                  #endif
                    break;
                case Qt::Key_BracketRight:
                  #ifdef VID
                    if (window->volume < 1.25){
                        window->volume += 0.05;
                        window->m_player->audio()->setVolume(window->volume);
                    }
                  #endif
                    break;
                /* Preset Tags */
                // N to open tag dialog and paste Nth preset into tag field, SHIFT+N to open tag dialog and set user input as Nth preset
                case Qt::Key_1:
                case Qt::Key_2:
                case Qt::Key_3:
                case Qt::Key_4:
                case Qt::Key_5:
                case Qt::Key_6:
                case Qt::Key_7:
                case Qt::Key_8:
                case Qt::Key_9:
                case Qt::Key_0:
                    window->media_tag(window->tag_preset[key2n(keyval)]);
                    break;
                case Qt::Key_Exclam:
                case Qt::Key_QuoteDbl:
                case Qt::Key_sterling:
                case Qt::Key_Dollar:
                case Qt::Key_Percent:
                case Qt::Key_AsciiCircum:
                case Qt::Key_Ampersand:
                case Qt::Key_Asterisk:
                case Qt::Key_ParenLeft:
                case Qt::Key_ParenRight:
                    window->media_tag_new_preset(key2n(keyval) - 10);
                    break;
                
                default: return QObject::eventFilter(obj, event);
            }
            return true;
        }
        default: break;
    }
    return QObject::eventFilter(obj, event);
}
