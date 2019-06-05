#ifndef MAINRWINDOW_H
#define MAINRWINDOW_H

#include <QCompleter>
#include <QLabel>
#include <QWidget>

#ifdef TXT
  #include <QPlainTextEdit>
#endif
#ifdef IMG
  #include <QImageReader>
#endif
#ifdef VID
  #include <QtAV>
#endif

#ifdef BOXABLE
  #include <vector> // for std::vector
  #include <QGridLayout>
  #include <QPaintEvent>
  #include <QPushButton>
  #include <QRubberBand>
#endif
#ifdef SCROLLABLE
  #include <QScrollArea>
#endif


constexpr const int MEDIA_FP_SZ = 4096;


QT_BEGIN_NAMESPACE
class QSlider;
QT_END_NAMESPACE


#ifdef BOXABLE
class InstanceWidget;
class MainWindow;
class Overlay;

class InstanceWidgetButton : public QPushButton{
 public:
    InstanceWidgetButton(const InstanceWidget* shouldBparent,  QWidget* parent,  QString txt = "")  :  QPushButton(txt, parent), shouldBparent(shouldBparent){};
    const InstanceWidget* shouldBparent;
};

class InstanceRelation : public QObject{
    Q_OBJECT
 public:
    InstanceRelation(QPoint middle,  QWidget* parent) : is_expanded(true){
        this->btn = new QPushButton("Relation", parent);
        this->btn->move(middle);
        this->toggle_expand();
        connect(this->btn, SIGNAL(clicked()), this, SLOT(toggle_expand()));
        this->btn->show();
    };
    ~InstanceRelation(){
        delete this->btn;
    };
    QPushButton* btn;
    QStringList tags;
    uint64_t id;
 public Q_SLOTS:
    void toggle_expand(){
        if (this->is_expanded)
            this->btn->setText("Relation");
        else
            this->btn->setText(this->tags.join("\n"));
        this->show_text();
        this->is_expanded = !this->is_expanded;
    };
 private:
    void show_text(){
        this->btn->resize(QSize(this->btn->sizeHint().width(), this->btn->sizeHint().height()));
    };
    bool is_expanded;
};

class InstanceWidget : public QRubberBand{
    Q_OBJECT
 public:
    InstanceWidget(QRubberBand::Shape shape,  MainWindow* win,  QWidget* parent)
    :
        QRubberBand(shape, parent),  win(win),  parent(parent),  is_expanded(true)
    {
        this->relation_btn = new InstanceWidgetButton(this, parent, "+Relation");
        this->relation_btn_sz = QSize(this->relation_btn->sizeHint().width(), this->relation_btn->sizeHint().height());
        connect(this->relation_btn, SIGNAL(clicked()), this, SLOT(start_relation_line()));
        this->relation_btn->show();
        
        this->btn = new InstanceWidgetButton(this, parent, "Instance");
        this->toggle_expand();
        connect(this->btn, SIGNAL(clicked()), this, SLOT(toggle_expand()));
        this->btn->show();
    };
    ~InstanceWidget(){
        for (auto iter = this->relations.begin();  iter != this->relations.end();  iter++){
            delete iter->second;
        }
        delete this->relation_btn;  // Only on exit: runtime error: member call on address  which does not point to an object of type 'InstanceWidgetButton'
        delete this->btn;  // Only on exit: runtime error: member call on address  which does not point to an object of type 'InstanceWidgetButton'
    };
    void set_colour(const QColor& cl){
        this->colour = cl;
        QPalette palette;
        palette.setBrush(QPalette::Highlight, QBrush(cl));
        this->setPalette(palette);
        this->update();
    };
    void show_text(){
        this->btn->resize(QSize(this->btn->sizeHint().width(), this->btn->sizeHint().height()));
        this->btn->show();
    };
    void setGeometry(const QRect& r){
        this->geometry = r;
        this->btn->move(this->geometry.topLeft());
        this->relation_btn->move(this->geometry.topRight()  -  QPoint(this->relation_btn_sz.width() - 1,  0));
        // Keep button entirely inside this widget - not only visually nicer, but avoids issue when widget borders the right edge of the window's main_widget.
        QRubberBand::setGeometry(r);
    };
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
    void toggle_expand(){
        if (this->is_expanded)
            this->btn->setText("Instance");
        else
            this->btn->setText(this->tags.join("\n"));
        this->show_text();
        this->is_expanded = !this->is_expanded;
    };
 private:
    MainWindow* win;
    QSize relation_btn_sz;
    bool is_expanded;
 private Q_SLOTS:
    void start_relation_line();
};







class Overlay : public QWidget{
    Q_OBJECT
 public:
    Overlay(MainWindow* win,  QWidget* parent)  :  win(win), QWidget(parent){
        this->setAttribute(Qt::WA_NoSystemBackground);
        this->setAttribute(Qt::WA_TransparentForMouseEvents);
    };
    bool do_not_update_instances;
 protected:
    void paintEvent(QPaintEvent* e) override;
 private:
    MainWindow* win;
};
#endif


#ifdef BOXABLE
bool operator<(const QRect& a, const QRect& b);
#endif


class MainWindow : public QWidget{
    Q_OBJECT
 public:
    ~MainWindow();
    explicit MainWindow(const int argc,  const char** argv,  QWidget *parent = 0);
    QString media_tag(QString str);
    void media_tag_new_preset(int n);
    void media_overwrite();
    void media_next();
    void media_open();
    void init_file_from_db();
    void media_replace_w_link(const char* src);
    void media_delete();
    void media_linkfrom();
    void media_score();
    void media_note();
    uint64_t get_id_from_table(const char* table_name, const char* entry_name);
    uint64_t file_attr_id(const char* attr,  uint64_t attr_id_int,  const char* file_id_str,  const int file_id_str_len);
    double volume;
    QString tag_preset[10];
    bool reached_stdin_end;
  #ifdef BOXABLE
    InstanceWidget* instance_widget; // Selection box
    bool is_mouse_down;
    QPoint mouse_dragged_from;
    QPoint mouse_dragged_to;
    std::vector<InstanceWidget*> instance_widgets; // TODO: Replace this with instanceid2pointer
    std::map<uint64_t, InstanceWidget*> instanceid2pointer;
    QRect boundingbox_geometry;
    void display_instance_mouseover();
    void create_instance();
    Overlay* main_widget_overlay;
    void start_relation_line(InstanceWidget* iw);
    void add_instance_to_table(const uint64_t frame_n);
  #endif
  #ifdef VID
    QtAV::VideoOutput* m_vo;
    QtAV::AVPlayer* m_player;
    QWidget* main_widget;
  #endif
  #ifdef TXT
    QPlainTextEdit* main_widget;
    void media_save();
    void set_read_only();
    void unset_read_only();
    bool is_file_modified;
    bool is_read_only;
  #endif
  #ifdef SCROLLABLE
    void rescale_main(double factor);
    QPoint get_scroll_offset();
    QScrollArea* scrollArea;
    QLabel* main_widget;
    QSize main_widget_orig_size;
    double scale_factor;
  #endif
    uint64_t add_new_tag(QString tagstr,  uint64_t tagid = 0);
    QStringList tagslist;
    std::map<uint64_t, QString> tag_id2name;
    QCompleter* tagcompleter;
 public Q_SLOTS:
  #ifdef VID
    void seekBySlider(int value);
    void seekBySlider();
    void playPause();
  #endif
 private Q_SLOTS:
  #ifdef VID
    void updateSlider(qint64 value);
    void updateSlider();
    void updateSliderUnit();
    void set_player_options_for_img();
  #endif
  #ifdef TXT
    void file_modified();
  #endif
 private:
  #ifdef VID
    QSlider *m_slider;
    int m_unit;
  #elif (defined IMG)
    QImage image;
  #endif
  #ifdef BOXABLE
    void create_relation_line_to(InstanceWidget* iw);
    void clear_instances();
    InstanceWidget* relation_line_from;
  #endif
  #ifdef SCROLLABLE
    template<typename T>
    void scale__correcting_for_offset(QRect& rect,  T scale){
        const QPoint d = (rect.bottomRight() - rect.topLeft())  *  scale;
        const QPoint p = ((rect.topLeft()) * scale);
        const QPoint q = QPoint(p.x() + d.x(),  p.y() + d.y());
        rect.setTopLeft(p);
        rect.setBottomRight(q);
    };
  #endif
    const char* get_media_fp();
    bool ignore_tagged;
    char media_fp[MEDIA_FP_SZ];
    char media_fp_buf[MEDIA_FP_SZ];
    int media_fp_indx;
    int media_fp_len;
    char media_fname[1024];
    int media_dir_len;
    int file_id_str_len;
    char file_id_str[16]; // Cache database ID of file. NOT an integer, but rather the string that is inserted into SQL query statements.
    uint64_t file_id;
    FILE* inf;
    
    void ensure_fileID_set();
    void set_table_attr_by_id(const char* tbl,  const char* id,  const int id_len,  const char* col,  const char* val);
    void tag2parent(uint64_t tag_id,  uint64_t parent_id);
};

class keyReceiver : public QObject{
// src: https://wiki.qt.io/How_to_catch_enter_key
    Q_OBJECT
 public:
    MainWindow* window;
 protected:
    bool eventFilter(QObject* obj,  QEvent* event);
};


#endif // MAINRWINDOW_H
