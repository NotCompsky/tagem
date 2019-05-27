#include "mainwindow.h"

#include <cstdio> // for remove
#include <unistd.h> // for symlink

#include <QApplication> // for QApplication::queryKeyboardModifiers
#include <QSlider>
#include <QLayout>
#include <QInputDialog>
#include <QKeyEvent>
#include <QTimer>

#ifdef BOXABLE
  #include <QPainter>
#endif
#ifdef SCROLLABLE
  #include <QScrollBar>
#endif

#include "sql_utils.hpp" // for mysu::*, SQL_*
#include "utils.h" // for count_digits, itoa_nonstandard

#define STDIN_FILENO 0

// mysql.files.media_id is the ID of the unique image/scene, irrespective of rescaling, recolouring, etc.


constexpr int MIN_FONT_SIZE = 8;
constexpr int SCROLL_INTERVAL = 1;


char STMT[4096];
char NOTE[30000];


uint64_t get_last_insert_id(){
    SQL_RES = SQL_STMT->executeQuery("SELECT LAST_INSERT_ID() as ''");
    SQL_RES->next();
    return SQL_RES->getUInt64(1);
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
    
    constexpr const char* a = "INSERT INTO relation (master_id, slave_id) VALUES(";
    int i = 0;
    memcpy(STMT + i,  a,  strlen(a));
    i += strlen(a);
    i += itoa_nonstandard(this->id,  STMT + i);
    STMT[i++] = ',';
    i += itoa_nonstandard(iw->id,    STMT + i);
    STMT[i++] = ')';
    STMT[i] = 0;
    SQL_STMT->execute(STMT);
    
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
        sql__add_many2many(SQL_STMT, "relation", ir->id, "tag", tagid);
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

MainWindow::MainWindow(const int argc,  const char** argv,  QWidget *parent) : QWidget(parent)
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
    
    mysu::init(argv[1], "mytag");
    
    this->tagslist;
    SQL_RES = SQL_STMT->executeQuery("SELECT id, name FROM tag");
    while (SQL_RES->next()){
        const QString s = QString::fromStdString(SQL_RES->getString(2));
        this->tag_id2name[SQL_RES->getUInt64(1)] = s;
        this->tagslist << s;
    }
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
    
    media_fp[0] = 0;
    
    keyReceiver* key_receiver = new keyReceiver();
    key_receiver->window = this;
    
    this->installEventFilter(key_receiver);
    
    for (auto i=0; i<10; ++i)
        tag_preset[i] = "";
}

#ifdef VID
void MainWindow::set_player_options_for_img(){
    PRINTF("Duration: %d\n", this->m_player->duration());
    if (this->m_player->duration() == 40){
        PRINTF("Auto paused\n");
        this->m_player->pause(true);
    }
}
#endif

void MainWindow::media_next(){
    // TODO: Do not have different strings, but one fp string and lengths of dir and fname
    size_t size;
    char* dummy = nullptr;
    getline(&dummy, &size, stdin);
    memcpy(this->media_fp,  dummy,  strlen(dummy)-1); // Remove trailing newline
    this->media_fp[strlen(dummy)-1] = 0;
    
    this->media_dir_len = 0;
    for (auto i = 0;  this->media_fp[i] != 0;  ++i)
        if (this->media_fp[i] == '/')
            this->media_dir_len = i;
    
    memcpy(this->media_dir,  this->media_fp,  this->media_dir_len);
    this->media_dir[this->media_dir_len] = 0;
    
    auto media_fname_len = strlen(dummy)-1 - this->media_dir_len - 1;
    memcpy(this->media_fname,  this->media_fp + this->media_dir_len + 1,  media_fname_len);
    this->media_fname[media_fname_len] = 0;
    
    this->file_id_str_len = 0; // Tells us that file_id hasn't been cached yet
    
    this->media_open();
}

bool is_file_in_db(char* fp){
    constexpr const char* a = "SELECT id FROM file WHERE name=\"";
    int i = 0;
    
    memcpy(STMT + i,  a,  strlen(a));
    i += strlen(a);
    
    while (*fp != 0){
        if (*fp == '"'  ||  *fp == '\\')
            STMT[i++] = '\\';
        STMT[i++] = *fp;
        ++fp;
    }
    
    STMT[i++] = '"';
    
    STMT[i] = 0;
    
    
    SQL_RES = SQL_STMT->executeQuery(STMT);
    
    
    return (SQL_RES->next());
}

void MainWindow::init_file_from_db(){
  #ifdef BOXABLE
    constexpr const int prelen_a = strlen("SELECT id, frame_n, x, y, w, h FROM instance WHERE file_id=");
    char a[prelen_a + 20 + 1]  =  "SELECT id, frame_n, x, y, w, h FROM instance WHERE file_id=";
    a[prelen_a  +  itoa_nonstandard(this->file_id,  a + prelen_a)] = 0;
    
    const double W = this->main_widget_orig_size.width();
    const double H = this->main_widget_orig_size.height();
    
    sql::ResultSet* sql_res;
    
    SQL_RES = SQL_STMT->executeQuery(a);
    while(SQL_RES->next()){
        const uint64_t instance_id = SQL_RES->getUInt64(1);
        const uint64_t frame_n     = SQL_RES->getUInt64(2);
        const double x = SQL_RES->getDouble(3);
        const double y = SQL_RES->getDouble(4);
        const double w = SQL_RES->getDouble(5);
        const double h = SQL_RES->getDouble(6);
        
        InstanceWidget* iw = new InstanceWidget(QRubberBand::Rectangle, this, this->main_widget);
        iw->id = instance_id;
        iw->setGeometry(QRect(QPoint(x*W, y*H),  QSize(w*W, h*H)));
        
        
        this->instanceid2pointer[instance_id] = iw;
        
        
        constexpr const int prelen_b = strlen("SELECT tag_id FROM instance2tag WHERE instance_id=");
        char b[prelen_b + 20 + 1] = "SELECT tag_id FROM instance2tag WHERE instance_id=";
        b[prelen_b  +  itoa_nonstandard(instance_id,  b + prelen_b)] = 0;
        
        sql_res = SQL_STMT->executeQuery(b);
        
        while(sql_res->next())
            iw->tags.append(this->tag_id2name[sql_res->getUInt64(1)]);
        
        
        iw->set_colour(QColor(255,0,255,100));
      #ifdef SCROLLABLE
        iw->orig_scale_factor = this->scale_factor;
      #endif
        iw->orig_geometry = iw->geometry;
        this->instance_widgets.push_back(iw);
        
        iw->frame_n = frame_n;
        
        iw->show();
    }
    
    for (auto iter = this->instanceid2pointer.begin();  iter != this->instanceid2pointer.end();  iter++){
        InstanceWidget* master = iter->second;
        
        
        constexpr const int prelen_c = strlen("SELECT id, slave_id FROM relation WHERE master_id=");
        char c[prelen_c + 20 + 1] = "SELECT id, slave_id FROM relation WHERE master_id=";
        c[prelen_c  +  itoa_nonstandard(master->id,  c + prelen_c)] = 0;
        
        sql_res = SQL_STMT->executeQuery(c);
        
        while(sql_res->next()){
            const uint64_t relation_id = sql_res->getUInt64(1);
            const uint64_t slave_id    = sql_res->getUInt64(2);
            InstanceWidget* slave  = this->instanceid2pointer[slave_id];
            
            QPoint middle = (master->geometry.topRight() + slave->geometry.topLeft()) / 2;
            InstanceRelation* ir = new InstanceRelation(middle, this->main_widget);
            // NOTE: master destruction destroys ir
            
            ir->id = relation_id;
            
            constexpr const int prelen_d = strlen("SELECT tag_id FROM relation2tag WHERE relation_id=");
            char d[prelen_d + 20 + 1] = "SELECT tag_id FROM relation2tag WHERE relation_id=";
            d[prelen_d  +  itoa_nonstandard(relation_id,  d + prelen_d)] = 0;
            
            sql_res = SQL_STMT->executeQuery(d);
            
            while(sql_res->next())
                ir->tags.append(this->tag_id2name[sql_res->getUInt64(1)]);
            
            master->relations[slave] = ir;
        }
    }
    this->main_widget_overlay->do_not_update_instances = false;
    this->main_widget_overlay->update();
  #endif
}

void MainWindow::media_open(){
  #ifdef BOXABLE
    this->clear_instances();
  #endif
    
    if (is_file_in_db(this->media_fp)){
        this->file_id = SQL_RES->getUInt64(1);
        this->file_id_str_len = count_digits(this->file_id);
        itoa_nonstandard(this->file_id, this->file_id_str_len, this->file_id_str);
        this->file_id_str[this->file_id_str_len] = 0;
    } else {
        if (this->ignore_tagged){
            PRINTF("Skipped previously tagged: %s\n", this->media_fp);
            return this->media_next();
        }
    }
    
    // WARNING: fp MUST be initialised, unless called via signal button press
    QString file = this->media_fp;
    
    /* Set window title */
    QString fname = this->media_fname;
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
        fprintf(stderr,  "%s while loading %s\n",  bstr.data(),  file);
        this->media_next();
        return;
    }
    this->main_widget->setPixmap(QPixmap::fromImage(this->image));
    this->main_widget->adjustSize();
    this->main_widget_orig_size = this->main_widget->size();
    this->scale_factor = 1.0d;
  #endif
  #ifdef BOXABLE
    this->main_widget_overlay->setGeometry(this->main_widget->geometry());
  #endif
    
    if (this->file_id_str_len != 0)
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


void MainWindow::set_table_attr_by_id(const char* tbl, const char* id, const int id_len, const char* col, const char* val){
    int i;
    
    i = 0;
    
    constexpr const char* a = "UPDATE ";
    memcpy(STMT + i,  a,  strlen(a));
    i += strlen(a);
    
    memcpy(STMT + i,  tbl,  strlen(tbl));
    i += strlen(tbl);
    
    constexpr const char* b = " SET ";
    memcpy(STMT + i,  b,  strlen(b));
    i += strlen(b);
    
    memcpy(STMT + i,  col,  strlen(col));
    i += strlen(col);
    
    constexpr const char* c = " = ";
    memcpy(STMT + i,  c,  strlen(c));
    i += strlen(c);
    
    memcpy(STMT + i,  val,  strlen(val));
    i += strlen(val);
    
    constexpr const char* d = " WHERE id = ";
    memcpy(STMT + i,  d,  strlen(d));
    i += strlen(d);
    
    memcpy(STMT + i,  id,  id_len);
    i += id_len;
    
    STMT[i++] = ';';
    STMT[i] = 0;
    
    
    PRINTF("%s\n", STMT);
    SQL_STMT->execute(STMT);
}

uint64_t MainWindow::file_attr_id(const char* attr, uint64_t attr_id_int, const char* file_id, const int file_id_len){
    return sql__file_attr_id(SQL_STMT, SQL_RES, attr, attr_id_int, file_id, file_id_len);
}

void MainWindow::ensure_fileID_set(){
    if (this->file_id_str_len == 0){
        this->file_id = this->get_id_from_table("file", this->media_fp);
        this->file_id_str_len = count_digits(this->file_id);
        itoa_nonstandard(this->file_id, this->file_id_str_len, this->file_id_str);
        this->file_id_str[this->file_id_str_len] = 0;
    }
}

void MainWindow::media_score(){
    /*
    Rating of the unique media object itself. Not aspects unique to the file (such as resolution, or compresssion quality).
    
    It is applied to the 'file' table because there is no sense in having multiple files for the same unique media object. These perceptual duplicates will be listed themselves elsewhere.
    */
    bool ok;
    int score_int = QInputDialog::getInt(this, tr("Rating"), tr("Rating"), 0, -100, 100, 1, &ok);
    if (!ok)
        return;
    
    int n = count_digits(score_int);
    char score[n+1];
    itoa_nonstandard(score_int, n, score);
    score[n] = 0;
    
    this->ensure_fileID_set();
    
    this->set_table_attr_by_id("file", this->file_id_str, this->file_id_str_len, "score", score);
}

void MainWindow::media_note(){
    this->ensure_fileID_set();
    
    constexpr const char* a = "SELECT note FROM file WHERE id=";
    int i = 0;
    memcpy(STMT,  a,  strlen(a));
    i += strlen(a);
    memcpy(STMT + i,  this->file_id_str,  this->file_id_str_len);
    i += this->file_id_str_len;
    STMT[i] = 0;
    
    PRINTF("%s\n", STMT);
    SQL_RES = SQL_STMT->executeQuery(STMT);
    
    if (SQL_RES->next()){
        std::string s = SQL_RES->getString(1);
        memcpy(NOTE,  s.c_str(),  strlen(s.c_str()));
        NOTE[strlen(s.c_str())] = 0;
    } else NOTE[0] = 0;
    
    
    bool ok;
    QString str = QInputDialog::getMultiLineText(this, tr("Note"), tr("Note"), NOTE, &ok);
    if (!ok || str.isEmpty())
        return;
    QByteArray  bstr = str.toLocal8Bit();
    const char* cstr = bstr.data();
    auto j = 0;
    for (auto i = 0;  i < strlen(cstr);  ++i){
        if (cstr[i] == '"'  ||  cstr[i] == '\\')
            NOTE[j++] = '\\';
        NOTE[j++] = cstr[i];
    }
    NOTE[j] = 0;
    sql__update(SQL_STMT, SQL_RES, "file", "note", NOTE, this->file_id);
}

void MainWindow::tag2parent(uint64_t tagid,  uint64_t parid){
    constexpr const char* a = "INSERT IGNORE INTO tag2parent (tag_id, parent_id) VALUES (";
    int i = 0;
    memcpy(STMT,  a,  strlen(a));
    i += strlen(a);
    i += itoa_nonstandard(tagid,  STMT + i);
    STMT[i++] = ',';
    i += itoa_nonstandard(parid,  STMT + i);
    STMT[i++] = ')';
    STMT[i] = 0;
    
    printf("%s\n", STMT);
    SQL_STMT->execute(STMT);
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
            
            printf("parent_tagchars: %s\n",  parent_tagchars + lastindx);
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
    
    this->file_attr_id("tag",  tagid,  this->file_id_str,  this->file_id_str_len);
    
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
        PRINTF("Overwritten with: %s\n", str);
}

void MainWindow::media_replace_w_link(const char* src){
    PRINTF("Deleting: %s\n", this->media_fp);
    if (remove(this->media_fp) != 0)
        fprintf(stderr, "Failed to delete: %s\n", this->media_fp);
    if (symlink(src, this->media_fp) != 0)
        fprintf(stderr, "Failed to create ln2del symlink: %s\n", this->media_fp);
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
    uint64_t value;
    return sql__get_id_from_table(SQL_STMT, SQL_RES, table_name, entry_name, value);
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
    
    QString filename = this->media_fname;
    
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
    const double W = this->main_widget_orig_size.width();
    const double H = this->main_widget_orig_size.height();
    const double x  =  (topL.x() / W)   /  this->scale_factor;
    const double y  =  (topL.y() / H)   /  this->scale_factor;
    const double w  =  ((botR.x() / W)  /  this->scale_factor) - x;
    const double h  =  ((botR.y() / H)  /  this->scale_factor) - y;
    const double xywh[4] = {x, y, w, h};
    
    
    this->ensure_fileID_set(); // Modifies STMT, so must be done first
    
    int i = 0;
    
    constexpr const char* a = "INSERT INTO instance (file_id, x, y, w, h, frame_n) VALUES(";
    
    memcpy(STMT + i,  a,  strlen(a));
    i += strlen(a);
    
    /*
    const QByteArray bname = qname.toLocal8Bit();
    const char* cname = bname.data();
    
    STMT[i++] = '"';
    while (*cname != 0){
        if (*cname == '\\'  ||  *cname == '"')
            STMT[i++] = '\\';
        STMT[i++] = *cname;
        ++cname;
    }
    STMT[i++] = '"';
    STMT[i++] = ',';
    */
    
    itoa_nonstandard(this->file_id,  this->file_id_str_len,  STMT + i);
    i += this->file_id_str_len;
    STMT[i++] = ',';
    
    for (auto j = 0;  j < 4;  ++j){
        if (xywh[j] == 1.0d){
            STMT[i++] = '1';
        } else {
            STMT[i++] = '0';
            STMT[i++] = '.';
            ftoa_nonstandard__gt_zero__lt_one(xywh[j],  16,  STMT + i);
            i += 16;
        }
        STMT[i++] = ',';
    }
    
    i += itoa_nonstandard(frame_n,  STMT + i);
    STMT[i++] = ')';
    STMT[i] = 0;
    
    printf("%s\n", STMT);
    SQL_STMT->execute(STMT);
}

void MainWindow::create_instance(){
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
            break;
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
        sql__add_many2many(SQL_STMT, "instance", this->instance_widget->id, "tag", tagid);
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
