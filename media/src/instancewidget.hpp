#ifndef __INSTANCEWIDGET__
#define __INSTANCEWIDGET__

#include <QRubberBand>
#include <QStringList>


class InstanceRelation;
class InstanceWidgetButton;
class MainWindow;


class InstanceWidget : public QRubberBand{
 public:
    InstanceWidget(QRubberBand::Shape shape,  MainWindow* win,  QWidget* parent);
    ~InstanceWidget();
    void set_colour(const QColor& cl);
    void show_text();
    void setGeometry(const QRect& r);
    void add_relation_line(InstanceWidget* iw);
	void toggle_expand(); // SLOT
    QStringList tags;
    std::map<InstanceWidget*, InstanceRelation*> relations;
    QRect geometry;
    double orig_scale_factor;
    QRect orig_geometry;
    InstanceWidgetButton* btn;
    InstanceWidgetButton* relation_btn;
    QColor colour;
    QWidget* parent;
    uint64_t id;
    uint64_t frame_n;
 private:
	void start_relation_line(); // SLOT
    MainWindow* win;
    QSize relation_btn_sz;
    bool is_expanded;
};


#endif
