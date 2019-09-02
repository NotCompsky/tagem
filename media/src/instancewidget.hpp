#ifndef __INSTANCEWIDGET__
#define __INSTANCEWIDGET__

#include <QRubberBand>
#include <QStringList>


class BoxRelation;
class BoxWidgetButton;
class MainWindow;


class BoxWidget : public QRubberBand{
 public:
    BoxWidget(QRubberBand::Shape shape,  MainWindow* win,  QWidget* parent);
    ~BoxWidget();
    void set_colour(const QColor& cl);
    void show_text();
    void setGeometry(const QRect& r);
    void add_relation_line(BoxWidget* iw);
	void toggle_expand(); // SLOT
    QStringList tags;
    std::map<BoxWidget*, BoxRelation*> relations;
    QRect geometry;
    double orig_scale_factor;
    QRect orig_geometry;
    BoxWidgetButton* btn;
    BoxWidgetButton* relation_btn;
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
