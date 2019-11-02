#include "keyreceiver.hpp"
#include "tag-manager/mainwindow.hpp"

#include <QApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QRubberBand>

#ifdef BOXABLE
# include "boxes/box_widget.hpp"
#endif
#include "mainwindow.hpp"


extern TagManager* tag_manager;


constexpr static const short SCROLL_INTERVAL = 1;


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
            if (window->box_widget != nullptr){
                delete window->box_widget;
                window->box_widget = nullptr;
                return true;
            }
            window->box_widget = new BoxWidget(QRubberBand::Rectangle, window, window->main_widget);
            window->boundingbox_geometry = QRect(window->mouse_dragged_from, QSize());
            QRect r = window->boundingbox_geometry;
            window->box_widget->setGeometry(r);
            window->box_widget->show();
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
            if (!window->is_mouse_down  ||  window->box_widget == nullptr){
                window->display_box_mouseover();
                return true;
            }
            window->box_widget->setGeometry(QRect(window->mouse_dragged_from, window->mouse_dragged_to).normalized());
            return true;
        }
      #endif
        case QEvent::KeyPress:{
            QKeyEvent* key = static_cast<QKeyEvent*>(event);
			const bool is_shift_key_down = (key->modifiers() & Qt::ShiftModifier);
			// For some keys, presumably depending on keyboard layout, the shift key is already accounted for.
			unsigned int n = 1;
			int sign = 1;
            switch(int keyval = key->key()){
                case Qt::Key_Enter:
                case Qt::Key_Return:
                case Qt::Key_D:
                    window->media_next(); // Causes SEGFAULT, even though clicking on "Next" button is fine.
                    break;
				case Qt::Key_E:
					if (window->readonly)
						break;
				  #ifdef ERA
					if (is_shift_key_down)
						window->display_eras();
					else
						window->create_era();
				  #endif
					break;
                case Qt::Key_I:
					if (window->readonly)
						break;
					
					if (is_shift_key_down){
						window->display_info();
						break;
					}
                  #ifdef TXT // No need for text editor to select rectangles
                    window->unset_read_only();
                   #ifdef BOXABLE
                    #error "TXT and BOXABLE are mutually exclusive"
                   #endif
                  #endif
                  #ifdef BOXABLE
					if (window->box_widget != nullptr){
						window->create_box();
						break;
					}
                  #endif
                    break;
                case Qt::Key_L:
					if (window->readonly)
						break;
                    window->media_linkfrom();
                    break;
				case Qt::Key_M:
				{
					if (window->readonly)
						break;
					if (tag_manager == nullptr)
						tag_manager = new TagManager(window);
					tag_manager->show();
					break;
				}
				case Qt::Key_R: // Rate
					if (window->readonly)
						break;
					window->show_settings_dialog();
					break;
                case Qt::Key_S: // Save
                  #ifdef TXT
					if (window->readonly)
						break;
                    window->media_save();
                  #endif
                    break;
                case Qt::Key_T:
					if (window->readonly)
						break;
                    window->media_tag("");
                    break;
                case Qt::Key_O:
					if (window->readonly)
						break;
                    window->media_overwrite();
                    break;
				case Qt::Key_P:
					// Plugin loading dialog
					break;
				case Qt::Key_V:
					if (window->readonly)
						break;
					window->assign_value();
					break;
                case Qt::Key_X:
					if (window->readonly)
						break;
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
					if (window->readonly)
						break;
				  #ifdef BOXABLE
					window->display_relation_hub();
				  #endif
					break;
                case Qt::Key_BracketLeft:
                  #ifdef AUDIO
					window->set_volume(window->volume - 0.05);
                  #endif
                    break;
                case Qt::Key_BracketRight:
                  #ifdef AUDIO
					window->set_volume(window->volume + 0.05);
                  #endif
                    break;
				case Qt::Key_F5:
				  #if (defined VID || defined AUDIO)
					window->seekBySlider(0);
					break;
				  #endif
                /* Preset Tags */
                // N to open tag dialog and paste Nth preset into tag field, SHIFT+N to open tag dialog and set user input as Nth preset
				
				case Qt::Key_0:
					++n;
				case Qt::Key_9:
					++n;
				case Qt::Key_8:
					++n;
				case Qt::Key_7:
					++n;
				case Qt::Key_6:
					++n;
				case Qt::Key_5:
					++n;
				case Qt::Key_4:
					++n;
				case Qt::Key_3:
					++n;
				case Qt::Key_2:
					++n;
				case Qt::Key_1:
					if (window->readonly)
						break;
					window->media_tag(window->tag_preset[n]);
					break;
				
				case Qt::Key_ParenRight:
					++n;
				case Qt::Key_ParenLeft:
					++n;
				case Qt::Key_Asterisk:
					++n;
				case Qt::Key_Ampersand:
					++n;
				case Qt::Key_AsciiCircum:
					++n;
				case Qt::Key_Percent:
					++n;
				case Qt::Key_Dollar:
					++n;
				case Qt::Key_sterling:
					++n;
				case Qt::Key_QuoteDbl:
					++n;
				case Qt::Key_Exclam:
					if (window->readonly)
						break;
					window->media_tag_new_preset(n);
					break;
				
				case Qt::Key_Left:
					sign = -1;
				case Qt::Key_Right:
				  #ifdef VID
					window->jump(sign * 1000);
				  #endif
					break;
                
                default: return QObject::eventFilter(obj, event);
            }
            return true;
        }
        default: break;
    }
    return QObject::eventFilter(obj, event);
}
