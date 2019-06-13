#ifndef __INSTANCEWIDGET__
#define __INSTANCEWIDGET__

#include <QRubberBand>
#include <QStringList>


class InstanceRelation;
class InstanceWidgetButton;
class MainWindow;


class InstanceWidget : public QRubberBand{
    Q_OBJECT
 public:
    InstanceWidget(QRubberBand::Shape shape,  MainWindow* win,  QWidget* parent);
    ~InstanceWidget();
    void set_colour(const QColor& cl);
    void show_text();
    void setGeometry(const QRect& r);
    void add_relation_line(InstanceWidget* iw);
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
 public Q_SLOTS:
    void toggle_expand();
 private:
    MainWindow* win;
    QSize relation_btn_sz;
    bool is_expanded;
 private Q_SLOTS:
    void start_relation_line();
};


#endif
