#include "mainwindow.h"

#include <cstdio> // for remove
#include <unistd.h> // for symlink
#include <string.h> // for memset

#include <QApplication> // for QApplication::queryKeyboardModifiers
#include <QSlider>
#include <QLayout>
#include <QInputDialog>
#include <QKeyEvent>
#include <QTimer>

#ifdef TXT
  #include <QTextStream>
#endif
#ifdef BOXABLE
  #include <QPainter>
#endif
#ifdef SCROLLABLE
  #include <QScrollBar>
#endif

#include "utils.hpp" // for asciify
#include "mymysql.hpp" // for mymysql::*, BUF, BUF_INDX

namespace res1 {
    #include "mymysql_results.hpp" // for ROW, RES, COL, ERR
}
namespace res2 {
    #include "mymysql_results.hpp" // for ROW, RES, COL, ERR
}

#ifdef DEBUG
  #define PRINTF printf
#else
  template<typename... Args>
  void PRINTF(Args... args){};
#endif

#define STDIN_FILENO 0

// mysql.files.media_id is the ID of the unique image/scene, irrespective of rescaling, recolouring, etc.


constexpr int MIN_FONT_SIZE = 8;
constexpr int SCROLL_INTERVAL = 1;


constexpr const int BUF_SZ_INIT = 4096;
char* BUF = (char*)malloc(BUF_SZ_INIT);
int BUF_SZ = BUF_SZ_INIT;
int BUF_INDX = 0;


char* NOTE = (char*)malloc(30000);


uint64_t get_last_insert_id(){
    uint64_t n = 0;
    res1::query("SELECT LAST_INSERT_ID() as ''");
    res1::assign_next_result(&n);
    res1::free_result();
    return n;
}


TagDialog::TagDialog(QString title,  QString str,  QWidget *parent) : QDialog(parent){
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(buttonBox);
    nameEdit = new QLineEdit(str);
    mainLayout->addWidget(nameEdit);
    QLabel* guide = new QLabel(tr("Enter blank tag to designate as root tag"));
    mainLayout->addWidget(guide);
    this->setLayout(mainLayout);
    this->setWindowTitle(title);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    QTimer::singleShot(0, nameEdit, SLOT(setFocus())); // Set focus after TagDialog instance is visible
}

#ifdef BOXABLE
void InstanceWidget::start_relation_line(){
    this->win->start_relation_line(this);
}
void InstanceWidget::add_relation_line(InstanceWidget* iw){
    if (this->relations[iw])
        return;
    QPoint middle = (this->geometry.topRight() + iw->geometry.topLeft()) / 2;
    InstanceRelation* ir = new InstanceRelation(middle, this->parent);
    this->win->main_widget_overlay->do_not_update_instances = true;
    
    mymysql::exec("INSERT INTO relation (master_id, slave_id) VALUES(",  this->id,  ",",  iw->id,  ")");
    
    ir->id = get_last_insert_id();
    
    // Seems to avoid segfault - presumably because the tagdialog forces a paintEvent of the overlay
    while(true){
        bool ok;
        TagDialog* tagdialog = new TagDialog("Relation Tag", "");
        tagdialog->nameEdit->setCompleter(this->win->tagcompleter);
        if (tagdialog->exec() != QDialog::Accepted)
            break;
        QString tagstr = tagdialog->nameEdit->text();
        if (tagstr.isEmpty())
            break;
        uint64_t tagid;
        if (!win->tagslist.contains(tagstr))
            tagid = win->add_new_tag(tagstr);
        else {
            QByteArray tagstr_ba = tagstr.toLocal8Bit();
            const char* tagchars = tagstr_ba.data();
            tagid = win->get_id_from_table("tag", tagchars);
        }
        ir->tags.append(tagstr);
        mymysql::exec("INSERT IGNORE INTO relation2tag (relation_id, tag_id) VALUES(", ir->id, ",", tagid, ")");
    }
    this->win->main_widget_overlay->do_not_update_instances = false;
    this->relations[iw] = ir;
    this->win->main_widget_overlay->update();
}

void Overlay::paintEvent(QPaintEvent* e){
    if (this->do_not_update_instances)
        return;
    QPainter painter(this);
    QPen pen;
    pen.setStyle(Qt::DashLine); // https://doc.qt.io/qt-5/qt.html#PenStyle-enum
    pen.setWidth(3);
    pen.setBrush(Qt::green);
    painter.setPen(pen);
    painter.save();
    foreach(InstanceWidget* iw,  this->win->instance_widgets){
        for (auto iter = iw->relations.begin();  iter != iw->relations.end();  iter++){
            // TODO: Add triangular button along this line that additionally indicates the heirarchy of the relation
            QPoint master;
            QPoint slave;
            master = iw->geometry.topRight();
            slave  = iter->first->geometry.topLeft();
            painter.drawLine(master, slave);
            QPoint p = (master + slave) / 2;
            iter->second->btn->move(p);
        }
    }
    painter.restore();
}
#endif

MainWindow::~MainWindow(){
    mymysql::exit();
    //delete key_receiver;
  #ifdef BOXABLE
    delete this->main_widget_overlay;
  #endif
  #ifdef SCROLLABLE
    delete this->scrollArea;
  #endif
  #ifdef IMG
    //delete this->main_widget;  // runtime error: member call on address  which does not point to an object of type 'QLabel'
  #endif
  #ifdef TXT
    delete this->main_widget;
  #endif
  #ifdef VID
    delete this->m_slider;
    delete this->m_vo;
    delete this->m_player;
  #endif
    delete this->layout();
    delete this->tagcompleter;
}

MainWindow::MainWindow(const int argc,  const char** argv,  QWidget *parent)
:
    QWidget(parent),
    media_fp_indx(MEDIA_FP_SZ),
    reached_stdin_end(false)
{
  #ifdef BOXABLE
    this->is_mouse_down = false;
  #endif
    this->ignore_tagged = false;
    for (auto i = 2;  i < argc;  ++i){
        const char* arg = argv[i];
        if (arg[1] == 0){
            switch(arg[0]){
                case 't': this->ignore_tagged = true; break;
                default: exit(2);
            }
        } else exit(3);
    }
    
    mymysql::init(argv[1]);
    
    res1::query("SELECT id, name FROM tag");
    {
    uint64_t id;
    char* name;
    while (res1::assign_next_result(&id, &name)){
        const QString s = name;
        this->tag_id2name[id] = s;
        this->tagslist << s;
    }
    res1::free_result();
    }
    BUF_INDX = 0;
    this->tagcompleter = new QCompleter(this->tagslist);
    
    QVBoxLayout* vl = new QVBoxLayout();
    vl->setContentsMargins(0, 0, 0, 0); // Makes calculating offset easier
    
    /* Define this->main_widget */
  #ifdef VID
    m_unit = 1000;
    setWindowTitle(QString::fromLatin1("Media Tagger"));
    m_player = new QtAV::AVPlayer(this);
    m_vo = new QtAV::VideoOutput(this);
    this->main_widget = this->m_vo->widget();
    if (!this->main_widget){
        fprintf(stderr, "Cannot create QtAV renderer\n");
        return;
    }
    m_player->setRenderer(m_vo);
    m_slider = new QSlider();
    m_slider->setOrientation(Qt::Horizontal);
    connect(m_slider, SIGNAL(sliderMoved(int)), SLOT(seekBySlider(int)));
    connect(m_slider, SIGNAL(sliderPressed()), SLOT(seekBySlider()));
    connect(m_player, SIGNAL(positionChanged(qint64)), SLOT(updateSlider(qint64)));
    connect(m_player, SIGNAL(started()), SLOT(updateSlider()));
    connect(m_player, SIGNAL(notifyIntervalChanged()), SLOT(updateSliderUnit()));
    connect(m_player, SIGNAL(started()), SLOT(set_player_options_for_img()));
    
    this->volume = 0.1;
    this->m_player->audio()->setVolume(this->volume);
   #ifdef TXT
    #error "TXT and VID are mutually exclusive"
   #endif
   #ifdef IMG
    #error "IMG and VID are mutually exclusive"
   #endif
  #endif
  #ifdef TXT
    this->main_widget = new QPlainTextEdit(this);
    QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(this->main_widget->sizePolicy().hasHeightForWidth());
    this->main_widget->setSizePolicy(sizePolicy1);
    this->main_widget->viewport()->setProperty("cursor", QVariant(QCursor(Qt::IBeamCursor)));
    this->main_widget->setContextMenuPolicy(Qt::DefaultContextMenu);
    this->main_widget->setFrameShape(QFrame::NoFrame);
    this->main_widget->setFrameShadow(QFrame::Plain);
    this->main_widget->setLineWidth(1);
    this->main_widget->setLineWrapMode(QPlainTextEdit::NoWrap);
    this->main_widget->setTabStopWidth(40);
    this->is_read_only = true;
    this->main_widget->setReadOnly(true);
    
    this->connect(this->main_widget, SIGNAL(textChanged()), this, SLOT(file_modified()), Qt::UniqueConnection);
    this->is_file_modified = false;
   #ifdef IMG
    #error "IMG and TXT are mutually exclusive"
   #endif
  #endif
  #ifdef IMG
    this->main_widget = new QLabel;
    this->main_widget->setBackgroundRole(QPalette::Base);
    this->main_widget->setScaledContents(true);
    this->main_widget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  #endif
    
    
  #ifdef SCROLLABLE
    this->scrollArea = new QScrollArea(this);
    this->scrollArea->setBackgroundRole(QPalette::Dark);
    this->scrollArea->setWidget(this->main_widget);
    vl->addWidget(this->scrollArea);
  #else
    vl->addWidget(this->main_widget);
  #endif
    
  #ifdef BOXABLE
    this->main_widget_overlay = new Overlay(this, this->main_widget);
    this->main_widget_overlay->show();
    
    this->instance_widget = nullptr;
  #endif
  #ifdef VID
    vl->addWidget(m_slider);
  #endif
    
    setLayout(vl);
    
    keyReceiver* key_receiver = new keyReceiver();
    key_receiver->window = this;
    
    this->installEventFilter(key_receiver);
    
    for (auto i=0; i<10; ++i)
        tag_preset[i] = "";
}

#ifdef VID
void MainWindow::set_player_options_for_img(){
    if (this->m_player->duration() == 40){
        PRINTF("Auto paused\n");
        this->m_player->pause(true);
    }
}
#endif

void MainWindow::media_next(){
    auto i = this->media_fp_indx;
    this->media_fp_len = 0;
    while(true){
        if (i == MEDIA_FP_SZ){
            if (this->reached_stdin_end)
                exit(0);
            size_t n = fread(this->media_fp_buf, 1, MEDIA_FP_SZ, stdin);
            i = 0;
            if (n != MEDIA_FP_SZ){
                this->reached_stdin_end = true;
                memset(this->media_fp_buf + n,  0,  MEDIA_FP_SZ - n);
            }
        }
        if (this->media_fp_buf[i] == '/')
            this->media_dir_len = this->media_fp_len;
        else if (this->media_fp_buf[i] == '\n')
            break;
        this->media_fp[this->media_fp_len] = this->media_fp_buf[i];
        ++i;
        ++this->media_fp_len;
    }
    ++this->media_dir_len; // Include the trailing slash
    this->media_fp_indx = i + 1;
    this->media_fp[this->media_fp_len] = '\n';
    
    write(1,  this->media_fp,  this->media_fp_len + 1);
    
    this->media_fp[this->media_fp_len] = 0;
    
    this->media_open();
}

uint64_t is_file_in_db(const char* fp){
    constexpr const char* a = "SELECT id FROM file WHERE name=\"";
    StartConcatWithCommaFlag c;
    EndConcatWithCommaFlag d;
    
    res1::query("SELECT id FROM file WHERE name='", c, fp, d, "'");
    
    uint64_t id = 0;
    res1::assign_next_result(&id);
    res1::free_result();
    
    return id;
}

void MainWindow::init_file_from_db(){
  #ifdef SCROLLABLE
    const double W = this->main_widget_orig_size.width();
    const double H = this->main_widget_orig_size.height();
  #else
    const double W = this->main_widget->size().width();
    const double H = this->main_widget->size().height();
  #endif
  #ifdef BOXABLE
    res1::query("SELECT id,frame_n,x,y,w,h FROM instance WHERE file_id=", this->file_id);
    {
    uint64_t instance_id;
    uint64_t frame_n;
    DoubleBetweenZeroAndOne zao_x(0.0), zao_y(0.0), zao_w(0.0), zao_h(0.0);
    while(res1::assign_next_result(&instance_id, &frame_n, &zao_x, &zao_y, &zao_w, &zao_h)){
        InstanceWidget* iw = new InstanceWidget(QRubberBand::Rectangle, this, this->main_widget);
        iw->id = instance_id;
        
        const double x = zao_x.value;
        const double y = zao_y.value;
        const double w = zao_w.value;
        const double h = zao_h.value;
        
        iw->setGeometry(QRect(QPoint(x*W, y*H),  QSize(w*W, h*H)));
        
        
        this->instanceid2pointer[instance_id] = iw;
        
        
        res2::query("SELECT tag_id FROM instance2tag WHERE instance_id=", instance_id);
        
        uint64_t tag_id;
        while(res2::assign_next_result(&tag_id))
            iw->tags.append(this->tag_id2name[tag_id]);
        res2::free_result();
        
        iw->set_colour(QColor(255,0,255,100));
      #ifdef SCROLLABLE
        iw->orig_scale_factor = this->scale_factor;
      #endif
        iw->orig_geometry = iw->geometry;
        this->instance_widgets.push_back(iw);
        
        iw->frame_n = frame_n;
        
        iw->show();
    }
    res1::free_result();
    }
    
    for (auto iter = this->instanceid2pointer.begin();  iter != this->instanceid2pointer.end();  iter++){
        InstanceWidget* master = iter->second;
        
        
        res1::query("SELECT id, slave_id FROM relation WHERE master_id=", master->id);
        
        uint64_t relation_id;
        uint64_t slave_id;
        while(res1::assign_next_result(&relation_id, &slave_id)){
            InstanceWidget* slave  = this->instanceid2pointer[slave_id];
            
            QPoint middle = (master->geometry.topRight() + slave->geometry.topLeft()) / 2;
            InstanceRelation* ir = new InstanceRelation(middle, this->main_widget);
            // NOTE: master destruction destroys ir
            
            ir->id = relation_id;
            
            res2::query("SELECT tag_id FROM relation2tag WHERE relation_id=", relation_id);
            uint64_t tag_id;
            while(res2::assign_next_result(&tag_id))
                ir->tags.append(this->tag_id2name[tag_id]);
            res2::free_result();
            
            master->relations[slave] = ir;
        }
        res1::free_result();
    }
    this->main_widget_overlay->do_not_update_instances = false;
    this->main_widget_overlay->update();
  #endif
}

const char* MainWindow::get_media_fp(){
    return this->media_fp;
}

void MainWindow::media_open(){
  #ifdef BOXABLE
    this->clear_instances();
  #endif
    
    this->file_id = is_file_in_db(this->get_media_fp());
    if (!this->file_id){
        if (this->ignore_tagged){
            PRINTF("Skipped previously tagged: %s\n", this->get_media_fp()); // TODO: Look into possible issues around this
            return this->media_next();
        }
    }
    
    // WARNING: fp MUST be initialised, unless called via signal button press
    QString file = this->get_media_fp();
    
    /* Set window title */
    QString fname = this->get_media_fp() + this->media_dir_len;
    this->setWindowTitle(fname);
    
  #ifdef VID
    m_player->play(file);
    PRINTF("Duration: %d\n", this->m_player->duration());
    m_player->setRepeat(-1); // Repeat infinitely
  #elif (defined TXT)
    QFile f(file);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)){
        fprintf(stderr,  "Cannot load file: %s\n",  file);
        this->media_next();
        return;
    }

    // Read from the file
    QTextStream in(&f);
    QTextDocument* document = this->main_widget->document();
    document->setPlainText(in.readAll());
    f.close();

    // Scroll to top
    this->main_widget->scroll(0, 0);
  #elif (defined IMG)
    QImageReader imgreader(file);
    imgreader.setAutoTransform(true);
    this->image = imgreader.read();
    if (this->image.isNull()){
        QByteArray bstr = imgreader.errorString().toLocal8Bit();
        fprintf(stderr,  "%s while loading %s\n",  bstr.data(),  this->get_media_fp());
        this->media_next();
        return;
    }
    this->main_widget->setPixmap(QPixmap::fromImage(this->image));
    this->main_widget->adjustSize();
  #endif
  #ifdef SCROLLABLE
    this->main_widget_orig_size = this->main_widget->size();
    this->scale_factor = 1.0;
  #endif
  #ifdef BOXABLE
    this->main_widget_overlay->setGeometry(this->main_widget->geometry());
  #endif
    
    if (this->file_id != 0)
        this->init_file_from_db();
}

#ifdef VID
void MainWindow::seekBySlider(int value)
{
    if (!m_player->isPlaying())
        return;
    m_player->seek(qint64(value*m_unit));
}

void MainWindow::seekBySlider()
{
    seekBySlider(m_slider->value());
}

void MainWindow::playPause()
{
    if (!m_player->isPlaying()) {
        m_player->play();
        return;
    }
    m_player->pause(!m_player->isPaused());
}

void MainWindow::updateSlider(qint64 value)
{
    m_slider->setRange(0, int(m_player->duration()/m_unit));
    m_slider->setValue(int(value/m_unit));
}

void MainWindow::updateSlider()
{
    updateSlider(m_player->position());
}

void MainWindow::updateSliderUnit()
{
    m_unit = m_player->notifyInterval();
    updateSlider();
}
#endif


void MainWindow::media_score(){
    /*
    Rating of the unique media object itself. Not aspects unique to the file (such as resolution, or compresssion quality).
    
    It is applied to the 'file' table because there is no sense in having multiple files for the same unique media object. These perceptual duplicates will be listed themselves elsewhere.
    */
    bool ok;
    int score_int = QInputDialog::getInt(this, tr("Rating"), tr("Rating"), 0, -100, 100, 1, &ok);
    if (!ok)
        return;
    
    this->ensure_fileID_set();
    
    mymysql::exec("UPDATE file SET score=", score_int, " WHERE id=", this->file_id);
}

void MainWindow::ensure_fileID_set(){
    if (this->file_id == 0){
        StartConcatWithApostrapheAndCommaFlag c;
        EndConcatWithApostrapheAndCommaFlag d;
        mymysql::exec("INSERT INTO file (name) VALUES(", c, this->get_media_fp(), d, ")");
        this->file_id = get_last_insert_id();
    }
}

void MainWindow::media_note(){
    this->ensure_fileID_set();
    
    res1::query("SELECT note FROM file WHERE id=", this->file_id);
    
    NOTE[0] = 0;
    res1::assign_next_result(&NOTE);
    
    bool ok;
    QString str = QInputDialog::getMultiLineText(this, tr("Note"), tr("Note"), NOTE, &ok);
    if (!ok || str.isEmpty())
        return;
    QByteArray  bstr = str.toLocal8Bit();
    const char* cstr = bstr.data();
    auto j = 0;
    for (auto i = 0;  i < cstr[i] != 0;  ++i){
        if (cstr[i] == '"'  ||  cstr[i] == '\\')
            NOTE[j++] = '\\';
        NOTE[j++] = cstr[i];
    }
    NOTE[j] = 0;
    StartConcatWithCommaFlag c;
    EndConcatWithCommaFlag d;
    mymysql::exec("UPDATE file SET note=", c, NOTE, d, " WHERE id=", this->file_id);
    res1::free_result();
}

void MainWindow::tag2parent(uint64_t tagid,  uint64_t parid){
    mymysql::exec("INSERT IGNORE INTO tag2parent (tag_id, parent_id) VALUES (", tagid, ",", parid, ")");
}

uint64_t MainWindow::add_new_tag(QString tagstr,  uint64_t tagid){
    QByteArray tagstr_ba = tagstr.toLocal8Bit();
    const char* tagchars = tagstr_ba.data();
    
    this->tagslist.append(tagstr);
    delete this->tagcompleter;
    this->tagcompleter = new QCompleter(this->tagslist);
    
    if (tagid == 0)
        tagid = this->get_id_from_table("tag", tagchars);
    
    this->tag_id2name[tagid] = tagstr;
    
    /* Get parent tag */
    TagDialog* tagdialog = new TagDialog("Parent Tag of", tagstr);
    tagdialog->nameEdit->setCompleter(this->tagcompleter);
    goto__cannotcancelparenttag:
    if (tagdialog->exec() != QDialog::Accepted)
        goto goto__cannotcancelparenttag;
    
    QString parent_tagstr = tagdialog->nameEdit->text();
    
    if (parent_tagstr.isEmpty()){
        this->tag2parent(tagid, 0);
        return tagid;
    }
    
    QByteArray parent_tagstr_ba = parent_tagstr.toLocal8Bit();
    char* parent_tagchars = parent_tagstr_ba.data();
    
    /* Insert parent-child relations */
    int lastindx = 0;
    for (auto i = 0;  ;  ++i)
        if (parent_tagchars[i] == '|'  ||  parent_tagchars[i] == 0){
            char iszero = parent_tagchars[i];
            parent_tagchars[i] = 0;
            
            PRINTF("parent_tagchars: %s\n",  parent_tagchars + lastindx);
            auto parid = this->get_id_from_table("tag",  parent_tagchars + lastindx);
            
            this->tag2parent(tagid, parid);
            
            QString parstr = QString::fromLocal8Bit(parent_tagchars + lastindx);
            if (!this->tagslist.contains(parstr))
                this->add_new_tag(parstr, parid);
            
            if (iszero == 0)
                break;
            
            lastindx = i + 1;
        }
    
    return tagid;
}

QString MainWindow::media_tag(QString str){
    if (media_fp[0] == 0)
        return "";
    
    // Triggered on key press
    bool ok;
    TagDialog* tagdialog = new TagDialog("Tag", str);
    tagdialog->nameEdit->setCompleter(this->tagcompleter);
    if (tagdialog->exec() != QDialog::Accepted)
        return "";
    
    QString tagstr = tagdialog->nameEdit->text();
    
    if (tagstr.isEmpty())
        return "";
    
    uint64_t tagid;
    QByteArray tagstr_ba = tagstr.toLocal8Bit();
    const char* tagchars = tagstr_ba.data();
    if (!this->tagslist.contains(tagstr)){
        tagid = this->add_new_tag(tagstr);
    } else tagid = this->get_id_from_table("tag", tagchars);
    
    this->ensure_fileID_set();
    
    mymysql::exec("INSERT IGNORE INTO file2tag (file_id, tag_id) VALUES(", tagid, ",", this->file_id,  ")");
    
    return tagstr;
}

void MainWindow::media_tag_new_preset(int n){
    tag_preset[n] = media_tag(tag_preset[n]);
}

void MainWindow::media_overwrite(){
    // Triggered on key press
    bool ok;
    QString str = QInputDialog::getText(this, tr("Overwrite"), tr("Value"), QLineEdit::Normal, "Manually deleted", &ok);
    if (ok && !str.isEmpty())
        PRINTF("Overwritten with: %s\n", str.toLocal8Bit().data());
}

void MainWindow::media_replace_w_link(const char* src){
    PRINTF("Deleting: %s\n", this->get_media_fp());
    if (remove(this->get_media_fp()) != 0)
        fprintf(stderr, "Failed to delete: %s\n", this->get_media_fp());
    if (symlink(src, this->get_media_fp()) != 0)
        fprintf(stderr, "Failed to create ln2del symlink: %s\n", this->get_media_fp());
}

void MainWindow::media_delete(){
    this->media_replace_w_link("/home/compsky/bin/ln2del_ln");
}

void MainWindow::media_linkfrom(){
    bool ok;
    QString str = QInputDialog::getText(this, tr("Overwrite with link"), tr("Source path"), QLineEdit::Normal, "", &ok);
    if (!ok || str.isEmpty()){
        PRINTF("Cancelling media_linkfrom\n");
        return;
    }
    
    const char* src = str.toLocal8Bit().data();
    this->media_replace_w_link(src);
}

uint64_t MainWindow::get_id_from_table(const char* table_name, const char* entry_name){
    uint64_t value = 0;
    StartConcatWithApostrapheAndCommaFlag c;
    EndConcatWithApostrapheAndCommaFlag d;
    res1::query("SELECT id FROM ", table_name, " WHERE name=", c, entry_name, d);
    res1::assign_next_result(&value);
    res1::free_result();
    return value;
}


#ifdef TXT
void MainWindow::unset_read_only(){
    this->is_read_only = false;
    this->main_widget->setReadOnly(this->is_read_only);
}

void MainWindow::set_read_only(){
    this->is_read_only = true; //!(this->main_widget->isReadOnly());
    this->main_widget->setReadOnly(this->is_read_only);
}

void MainWindow::file_modified(){
    this->is_file_modified = true;
}

void MainWindow::media_save(){
    if (!this->is_file_modified)
        return;
    
    QString filename = this->get_media_fp();
    
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    
    QTextStream out(&file);
    out << this->main_widget->document()->toPlainText();
    out.flush();
    file.close();
    
    this->is_file_modified = false;
}
#endif


#ifdef BOXABLE
bool operator<(const QRect& a, const QRect& b){
    // To allow their use as keys
    return (&a < &b);
}

template<typename T>
void scale(QRect& rect,  T scale){
    const QPoint d = (rect.bottomRight() - rect.topLeft())  *  scale;
    const QPoint p = rect.topLeft() * scale;
    const QPoint q = QPoint(p.x() + d.x(),  p.y() + d.y());
    rect.setTopLeft(p);
    rect.setBottomRight(q);
};

#ifdef SCROLLABLE
void MainWindow::rescale_main(double factor){
    this->scale_factor *= factor;
  #ifdef IMG
    Q_ASSERT(this->main_widget->pixmap());
  #endif
    this->main_widget->resize(this->main_widget_orig_size * this->scale_factor);
    foreach(InstanceWidget* iw,  instance_widgets){
        QRect g = iw->orig_geometry;
        scale(g,  this->scale_factor / iw->orig_scale_factor);
        iw->setGeometry(g);
    }
    this->main_widget_overlay->setGeometry(this->main_widget->geometry());
}
#endif


#ifdef SCROLLABLE
QPoint MainWindow::get_scroll_offset(){
    return QPoint(this->scrollArea->horizontalScrollBar()->value(), this->scrollArea->verticalScrollBar()->value());
}
#endif


void MainWindow::display_instance_mouseover(){
}


void MainWindow::add_instance_to_table(const uint64_t frame_n){
    const QPoint topL = this->instance_widget->geometry.topLeft();
    const QPoint botR = this->instance_widget->geometry.bottomRight();
  #ifdef SCROLLABLE
    const double W = this->main_widget_orig_size.width();
    const double H = this->main_widget_orig_size.height();
    double x  =  (topL.x() / W)   /  this->scale_factor;
    double y  =  (topL.y() / H)   /  this->scale_factor;
    double w  =  ((botR.x() / W)  /  this->scale_factor) - x;
    double h  =  ((botR.y() / H)  /  this->scale_factor) - y;
  #elif (defined BOXABLE)
    const QPoint p = this->m_vo->widget()->pos();
    const double W = this->m_vo->widget()->size().width();
    const double H = this->m_vo->widget()->size().height();
    double x  =  (topL.x() / W) + p.x();
    double y  =  (topL.y() / H) + p.y();
    double w  =  (botR.x() / W);
    double h  =  (botR.y() / H);
  #endif
    
    
    
    EnsureDoubleBetweenZeroAndOne a(x);
    EnsureDoubleBetweenZeroAndOne b(y);
    EnsureDoubleBetweenZeroAndOne c(w);
    EnsureDoubleBetweenZeroAndOne d(h);
    
    
    this->ensure_fileID_set();
    
    StartConcatWithCommaFlag comma0;
    EndConcatWithCommaFlag comma1;
    
    mymysql::exec("INSERT INTO instance (file_id, x, y, w, h, frame_n) VALUES(", comma0, this->file_id, a, 17, b, 17, c, 17, d, 17, frame_n, comma1, ')');
}

void MainWindow::create_instance(){
    if (this->instance_widget == nullptr)
        return;
    
    const uint64_t frame_n = 
  #ifdef VID
    this->m_player->duration();
  #else
    0;
  #endif
    
    this->add_instance_to_table(frame_n);
    this->instance_widget->id = get_last_insert_id();
    
    while(true){
        bool ok;
        TagDialog* tagdialog = new TagDialog("Instance Tag", "");
        tagdialog->nameEdit->setCompleter(this->tagcompleter);
        if (tagdialog->exec() != QDialog::Accepted)
            break; // TODO: Create a way to cancel ALL past operations on this instance_widget if the cancel button is pressed for any one of the tags
        QString tagstr = tagdialog->nameEdit->text();
        if (tagstr.isEmpty())
            break;
        uint64_t tagid;
        if (!this->tagslist.contains(tagstr))
            tagid = this->add_new_tag(tagstr);
        else {
            QByteArray tagstr_ba = tagstr.toLocal8Bit();
            const char* tagchars = tagstr_ba.data();
            tagid = this->get_id_from_table("tag", tagchars);
        }
        this->instance_widget->tags.append(tagstr);
        mymysql::exec("INSERT IGNORE INTO instance2tag (instance_id, tag_id) VALUES(", this->instance_widget->id, ",", tagid, ")");
    }
    
    this->instance_widget->set_colour(QColor(255,0,255,100));
  #ifdef SCROLLABLE
    this->instance_widget->orig_scale_factor = this->scale_factor;
  #endif
    this->instance_widget->orig_geometry = this->instance_widget->geometry;
    this->instance_widgets.push_back(this->instance_widget);
    
    this->instance_widget->frame_n = frame_n;
    
    goto__clearrubberband:
    this->instance_widget = nullptr;
}

void MainWindow::clear_instances(){
    for (auto i = 0;  i < this->instance_widgets.size();  ++i)
        delete this->instance_widgets[i];
    this->instance_widgets.clear();
    this->relation_line_from = nullptr;
    this->instanceid2pointer.clear();
}

void MainWindow::start_relation_line(InstanceWidget* iw){
    if (this->relation_line_from == nullptr){
        this->relation_line_from = iw;
        return;
    }
    this->create_relation_line_to(iw);
    this->relation_line_from = nullptr;
}

void MainWindow::create_relation_line_to(InstanceWidget* iw){
    if (iw == this->relation_line_from)
        return;
    this->relation_line_from->add_relation_line(iw);
}
#endif




std::map<const int, const int> key2n = {
    {Qt::Key_1, 1},
    {Qt::Key_2, 2},
    {Qt::Key_3, 3},
    {Qt::Key_4, 4},
    {Qt::Key_5, 5},
    {Qt::Key_6, 6},
    {Qt::Key_7, 7},
    {Qt::Key_8, 8},
    {Qt::Key_9, 9},
    {Qt::Key_0, 0},
    {Qt::Key_Exclam, 1},
    {Qt::Key_QuoteDbl, 2},
    {Qt::Key_sterling, 3},
    {Qt::Key_Dollar, 4},
    {Qt::Key_Percent, 5},
    {Qt::Key_AsciiCircum, 6},
    {Qt::Key_Ampersand, 7},
    {Qt::Key_Asterisk, 8},
    {Qt::Key_ParenLeft, 9},
    {Qt::Key_ParenRight, 0},
};


bool keyReceiver::eventFilter(QObject* obj, QEvent* event)
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
                case Qt::Key_L:
                    window->media_linkfrom();
                    break;
                case Qt::Key_R: // Rate
                    window->media_score();
                    break;
                case Qt::Key_I:
                  #ifdef TXT // No need for text editor to select rectangles
                    window->unset_read_only();
                   #ifdef BOXABLE
                    #error "TXT and BOXABLE are mutually exclusive"
                   #endif
                  #endif
                  #ifdef BOXABLE
                    window->create_instance();
                  #endif
                    break;
                case Qt::Key_Escape:
                  #ifdef TXT
                    window->set_read_only();
                  #endif
                    break;
                case Qt::Key_S: // Save
                  #ifdef TXT
                    window->media_save();
                  #endif
                    break;
                case Qt::Key_N:
                    window->media_note();
                    break;
                case Qt::Key_T:
                    window->media_tag("");
                    break;
                case Qt::Key_O:
                    window->media_overwrite();
                    break;
                case Qt::Key_Q:
                    window->close();
                    break;
                case Qt::Key_X:
                    window->media_delete();
                    window->media_next();
                    break;
                case Qt::Key_Space:
                  #ifdef VID
                    window->playPause();
                  #endif
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
                    window->media_tag(window->tag_preset[key2n[keyval]]);
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
                    window->media_tag_new_preset(key2n[keyval]);
                    break;
                
                default: return QObject::eventFilter(obj, event);
            }
            return true;
        }
        default: break;
    }
    return QObject::eventFilter(obj, event);
}
