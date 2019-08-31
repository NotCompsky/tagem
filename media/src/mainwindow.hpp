#ifndef MAINRWINDOW_H
#define MAINRWINDOW_H

#include "inlist_filter_dialog.hpp"

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
# include <vector> // for std::vector
class InstanceWidget;
class Overlay;
#endif
#ifdef SCROLLABLE
  #include <QScrollArea>
#endif


constexpr const int MEDIA_FP_SZ = 4096;


QT_BEGIN_NAMESPACE
class QSlider;
QT_END_NAMESPACE


#ifdef BOXABLE
bool operator<(const QRect& a, const QRect& b);

template<typename T>
void scale(QRect& rect,  T scale){
    const QPoint d = (rect.bottomRight() - rect.topLeft())  *  scale;
    const QPoint p = rect.topLeft() * scale;
    const QPoint q = QPoint(p.x() + d.x(),  p.y() + d.y());
    rect.setTopLeft(p);
    rect.setBottomRight(q);
};
#endif


class MainWindow : public QWidget{
 public:
    ~MainWindow();
    explicit MainWindow(QWidget *parent = 0);
	const QString media_tag(const QString str);
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
	void show_settings_dialog();
	void display_info();
	void display_relation_hub(); // SLOT
# ifdef AUDIO
	void set_volume(const double _volume);
# endif
  #ifdef VID
	void seekBySlider(int value); // SLOT
	void playPause(); // SLOT
  #endif
# ifdef AUDIO
    double volume;
# endif
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
	InlistFilterDialog* inlist_filter_dialog;
	bool auto_next;
 private:
  #ifdef VID
	void updateSlider(qint64 value); // SLOT
	void updateSliderUnit(); // SLOT
	void set_player_options_for_img(); // SLOT
	void parse_mediaStatusChanged(int status); // SLOT
  #endif
  #ifdef TXT
	void file_modified(); // SLOT
  #endif
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
	void set_media_dir_len();
    const char* get_media_fp();
    char media_fp[MEDIA_FP_SZ];
    char media_fp_buf[MEDIA_FP_SZ];
    int media_fp_indx;
    int media_fp_len;
    char media_fname[1024];
	qint64 file_sz;
    int media_dir_len;
    int file_id_str_len;
    char file_id_str[16]; // Cache database ID of file. NOT an integer, but rather the string that is inserted into SQL query statements.
    uint64_t file_id;
    FILE* inf;
    
    void ensure_fileID_set();
    void set_table_attr_by_id(const char* tbl,  const char* id,  const int id_len,  const char* col,  const char* val);
};


#endif // MAINRWINDOW_H
