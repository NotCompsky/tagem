#include "instancewidget.hpp"

#include <QBrush>
#include <QColor>
#include <QDialog>
#include <QPalette>
#include <QRect>

#include <compsky/mysql/query.hpp>

#include "instancerelation.hpp"
#include "instancewidgetbutton.hpp"
#include "mainwindow.hpp"
#include "name_dialog.hpp"
#include "overlay.hpp"

#include "utils.hpp"


namespace _mysql {
	extern MYSQL* obj;
}

extern char BUF[];


InstanceWidget::InstanceWidget(QRubberBand::Shape shape,  MainWindow* win,  QWidget* parent)
:
    QRubberBand(shape, parent),  win(win),  parent(parent),  is_expanded(true)
{
    this->relation_btn = new InstanceWidgetButton(this, parent, "+Relation");
    this->relation_btn_sz = QSize(this->relation_btn->sizeHint().width(), this->relation_btn->sizeHint().height());
    connect(this->relation_btn, &QPushButton::clicked, this, &InstanceWidget::start_relation_line);
    this->relation_btn->show();
    
    this->btn = new InstanceWidgetButton(this, parent, "Instance");
    this->toggle_expand();
    connect(this->btn, &QPushButton::clicked, this, &InstanceWidget::toggle_expand);
    this->btn->show();
}

InstanceWidget::~InstanceWidget(){
    for (auto iter = this->relations.begin();  iter != this->relations.end();  iter++){
        delete iter->second;
    }
    delete this->relation_btn;  // Only on exit: runtime error: member call on address  which does not point to an object of type 'InstanceWidgetButton'
    delete this->btn;  // Only on exit: runtime error: member call on address  which does not point to an object of type 'InstanceWidgetButton'
}

void InstanceWidget::set_colour(const QColor& cl){
    this->colour = cl;
    QPalette palette;
    palette.setBrush(QPalette::Highlight, QBrush(cl));
    this->setPalette(palette);
    this->update();
}

void InstanceWidget::show_text(){
    this->btn->resize(QSize(this->btn->sizeHint().width(), this->btn->sizeHint().height()));
    this->btn->show();
}

void InstanceWidget::setGeometry(const QRect& r){
    this->geometry = r;
    this->btn->move(this->geometry.topLeft());
    this->relation_btn->move(this->geometry.topRight()  -  QPoint(this->relation_btn_sz.width() - 1,  0));
    // Keep button entirely inside this widget - not only visually nicer, but avoids issue when widget borders the right edge of the window's main_widget.
    QRubberBand::setGeometry(r);
}

void InstanceWidget::toggle_expand(){
    if (this->is_expanded)
        this->btn->setText("Instance");
    else
        this->btn->setText(this->tags.join("\n"));
    this->show_text();
    this->is_expanded = !this->is_expanded;
};

void InstanceWidget::start_relation_line(){
    this->win->start_relation_line(this);
}
void InstanceWidget::add_relation_line(InstanceWidget* iw){
    if (this->relations[iw])
        return;
    QPoint middle = (this->geometry.topRight() + iw->geometry.topLeft()) / 2;
    
    compsky::mysql::exec(_mysql::obj,  BUF,  "INSERT INTO relation (master_id, slave_id) VALUES(",  this->id,  ',',  iw->id,  ')');
    const uint64_t _relation_id = get_last_insert_id();
	
	InstanceRelation* ir = new InstanceRelation(_relation_id, middle, this->parent);
    this->win->main_widget_overlay->do_not_update_instances = true;
    
    // Seems to avoid segfault - presumably because the tagdialog forces a paintEvent of the overlay
    while(true){
        NameDialog* tagdialog = new NameDialog("Relation Tag", "");
        tagdialog->name_edit->setCompleter(this->win->tagcompleter);
		const auto rc = tagdialog->exec();
		const QString tagstr = tagdialog->name_edit->text();
		delete tagdialog;
		
        if (rc != QDialog::Accepted)
            break;
        if (tagstr.isEmpty())
            break;
		
        uint64_t tagid;
        if (!win->tagslist.contains(tagstr))
            tagid = win->add_new_tag(tagstr);
        else {
			const QByteArray tagstr_ba = tagstr.toLocal8Bit();
            const char* tagchars = tagstr_ba.data();
            tagid = win->get_id_from_table("tag", tagchars);
        }
        compsky::mysql::exec(_mysql::obj,  BUF,  "INSERT IGNORE INTO relation2tag (relation_id, tag_id) VALUES(", ir->id, ',', tagid, ')');
    }
    this->win->main_widget_overlay->do_not_update_instances = false;
    this->relations[iw] = ir;
    this->win->main_widget_overlay->update();
}
