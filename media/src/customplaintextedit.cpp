#include "customplaintextedit.h"

CustomPlainTextEdit::CustomPlainTextEdit(QWidget *parent) : QPlainTextEdit(parent)
{
    // No special customization here
}

/**
 * @brief CustomPlainTextEdit::wheelEvent
 * @param inEvent wheel event
 *
 * This method overrides the default wheelEvent event to enable CTRL-Scroll
 * functionality to increase/decrease the font size.
 */
void CustomPlainTextEdit::wheelEvent(QWheelEvent *inEvent)
{
    static int MIN_FONT_SIZE = 6;
    static short SCROLL_INTERVAL = 1;

    // Check to see if the Ctrl key is pressed
    Qt::KeyboardModifiers result = QApplication::queryKeyboardModifiers();
    if ((result & Qt::ControlModifier) == Qt::ControlModifier) {

        // Get the current font
        QFont font = QFont(this->font());

        // Are we scrolling up or down?
        short direction = (inEvent->delta() > 0 ? SCROLL_INTERVAL : -1 * SCROLL_INTERVAL);

        // Set the new font size
        int new_size = font.pointSize() + direction;
        new_size = new_size >= MIN_FONT_SIZE ? new_size : MIN_FONT_SIZE;
        font.setPointSize(new_size);
        this->setFont(font);

        // Don't actually scroll the window
        inEvent->ignore();
    } else {
        // Handle event normally
        QPlainTextEdit::wheelEvent(inEvent);
    }
}
