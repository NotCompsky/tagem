#include "mainwindow.hpp"
#include "info_dialog.hpp"
#include "relation_add_instance_tags.hpp"

#include <cstdio> // for remove
#ifndef _WIN32
# include <unistd.h> // for symlink
#endif
#include <string.h> // for memset

#include <QApplication> // for QApplication::queryKeyboardModifiers
#include <QSlider>
#include <QLayout>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMessageBox>
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

#include <compsky/asciify/asciify.hpp> // for compsky::asciify::(flag|fake_type)
#include <compsky/mysql/query.hpp> // for compsky::mysql::(exec|query)(_buffer)?
#include "name_dialog.hpp"

#include "keyreceiver.hpp"

#ifdef BOXABLE
# include <vector> // for std::vector
# include "instancerelation.hpp"
# include "instancewidget.hpp"
# include "instancewidgetbutton.hpp"
# include "overlay.hpp"
#endif

#include "utils.hpp"

#ifdef DEBUG
# define PRINTF printf
#else
# define PRINTF(...)
#endif

#define STDIN_FILENO 0

// mysql.files.media_id is the ID of the unique image/scene, irrespective of rescaling, recolouring, etc.


namespace _mysql {
	extern MYSQL* obj;
}

MYSQL_RES* RES1;
MYSQL_ROW ROW1;
MYSQL_RES* RES2;
MYSQL_ROW ROW2;


constexpr int MIN_FONT_SIZE = 8;

constexpr static const compsky::asciify::flag::Escape f_esc;
constexpr static const compsky::asciify::flag::guarantee::BetweenZeroAndOne f_g;
constexpr static const compsky::asciify::flag::concat::Start f_start;
constexpr static const compsky::asciify::flag::concat::End f_end;

#define BUF_SZ (2 * 4096)
char BUF[BUF_SZ];
char* ITR = BUF;
char NOTE[30000];



MainWindow::~MainWindow(){
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

MainWindow::MainWindow(QWidget *parent)
:
    QWidget(parent),
    media_fp_indx(MEDIA_FP_SZ),
    reached_stdin_end(false)
{
  #ifdef BOXABLE
    this->is_mouse_down = false;
  #endif
    
    compsky::mysql::query_buffer(_mysql::obj,  RES1,  "SELECT id, name FROM tag");
    {
    uint64_t id;
	const char* name;
    while (compsky::mysql::assign_next_row(RES1,  &ROW1,  &id, &name)){
        const QString s = name;
        this->tag_id2name[id] = s;
        this->tagslist << s;
    }
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
    connect(m_slider,  &QSlider::sliderMoved,     this,  &MainWindow::seekBySlider);
    connect(m_slider,  &QSlider::sliderPressed,   this,  [=]() { this->seekBySlider(m_slider->value()); });
    connect(m_player,  &QtAV::AVPlayer::positionChanged, this, &MainWindow::updateSlider);
    connect(m_player,  &QtAV::AVPlayer::started,  this,  [=](){ this->updateSlider(m_player->position()); });
    connect(m_player,  &QtAV::AVPlayer::notifyIntervalChanged, this, &MainWindow::updateSliderUnit);
    connect(m_player,  &QtAV::AVPlayer::started,  this,  &MainWindow::set_player_options_for_img);
    
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
    this->main_widget->setTabStopWidth(40);
    this->is_read_only = true;
    this->main_widget->setReadOnly(true);
    
    this->connect(this->main_widget, &QPlainTextEdit::textChanged, this, &MainWindow::file_modified, Qt::UniqueConnection);
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
    
    KeyReceiver* key_receiver = new KeyReceiver();
    key_receiver->window = this;
    
    this->installEventFilter(key_receiver);
    
    for (auto i=0; i<10; ++i)
        tag_preset[i] = "";
	
	this->inlist_filter_dialog = new InlistFilterDialog(this);
}

#ifdef AUDIO
void MainWindow::set_volume(const double _volume){
	this->volume = _volume;
	this->m_player->audio()->setVolume(this->volume);
}
#endif

#ifdef VID
void MainWindow::set_player_options_for_img(){
    if (this->m_player->duration() == 40){
        PRINTF("Auto paused\n");
        this->m_player->pause(true);
    }
}
#endif

void MainWindow::set_media_dir_len(){
	this->media_dir_len = 0;
	for(size_t i = 0;  this->media_fp[i] != 0;  ++i)
		if(this->media_fp[i] == '/')
			this->media_dir_len = i + 1;
}

void MainWindow::media_next(){
	// TODO: Implement in inlist_filter_dialog
	const InlistFilterRules r = this->inlist_filter_dialog->rules;
	
	switch(r.files_from_which){
		case files_from_which::sql:
		{
			const char* _media_fp;
			if(likely(compsky::mysql::assign_next_row__no_free(this->inlist_filter_dialog->files_from_sql__res,  &(this->inlist_filter_dialog->files_from_sql__row),  &_media_fp))){
				printf("%s\n", _media_fp);
				memcpy(this->media_fp,  _media_fp,  strlen(_media_fp) + 1);
				this->set_media_dir_len();
				this->media_open();
				return;
			}
			QMessageBox::warning(this,  "No more results",  "SQL query results exhausted - looping to beginning");
			mysql_data_seek(this->inlist_filter_dialog->files_from_sql__res, 0);
			return;
		}
		case files_from_which::directory:
			QMessageBox::warning(this,  "Files from directory",  "Not implemented");
			return;
		case files_from_which::url:
			QMessageBox::warning(this,  "Files from URL",  "Not implemented");
			return;
		case files_from_which::bash:
		{
			const qint64 _sz = this->inlist_filter_dialog->files_from_bash.readLine(this->media_fp, MEDIA_FP_SZ);
			if (_sz == -1){
				// Errored
				QMessageBox::warning(this,  "No more results",  "Bash command output exhausted.");
				return;
			}
			this->media_fp[_sz - 1] = 0; // Remove trailing newline
			this->set_media_dir_len();
			printf("media_fp: %s\n", this->media_fp);
			this->media_open();
			return;
		}
		case files_from_which::stdin:
			break;
	}
	
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

void MainWindow::init_file_from_db(){
  #ifdef SCROLLABLE
    const double W = this->main_widget_orig_size.width();
    const double H = this->main_widget_orig_size.height();
  #else
    const double W = this->main_widget->size().width();
    const double H = this->main_widget->size().height();
  #endif
  #ifdef BOXABLE
    compsky::mysql::query(_mysql::obj,  RES1,  BUF,  "SELECT id,frame_n,x,y,w,h FROM instance WHERE file_id=", this->file_id);
    {
    uint64_t instance_id;
    uint64_t frame_n;
    double x = 0.0;
    double y = 0.0;
    double w = 0.0;
    double h = 0.0;
    
    while(compsky::mysql::assign_next_row(RES1,  &ROW1,  &instance_id, &frame_n, f_g, &x, f_g, &y, f_g, &w, f_g, &h)){
        InstanceWidget* iw = new InstanceWidget(QRubberBand::Rectangle, this, this->main_widget);
        iw->id = instance_id;
        
        iw->setGeometry(QRect(QPoint(x*W, y*H),  QSize(w*W, h*H)));
        
        
        this->instanceid2pointer[instance_id] = iw;
        
        
        compsky::mysql::query(_mysql::obj,  RES2,  BUF,  "SELECT tag_id FROM instance2tag WHERE instance_id=", instance_id);
        
        uint64_t tag_id;
        while(compsky::mysql::assign_next_row(RES2, &ROW2, &tag_id))
            iw->tags.append(this->tag_id2name[tag_id]);
        
        iw->set_colour(QColor(255,0,255,100));
      #ifdef SCROLLABLE
        iw->orig_scale_factor = this->scale_factor;
      #endif
        iw->orig_geometry = iw->geometry;
        this->instance_widgets.push_back(iw);
        
        iw->frame_n = frame_n;
        
        iw->show();
    }
    }
    
    for (auto iter = this->instanceid2pointer.begin();  iter != this->instanceid2pointer.end();  iter++){
        InstanceWidget* master = iter->second;
        
        
        compsky::mysql::query(_mysql::obj,  RES1,  BUF,  "SELECT id, slave_id FROM relation WHERE master_id=", master->id);
        
        uint64_t relation_id;
        uint64_t slave_id;
        while(compsky::mysql::assign_next_row(RES1,  &ROW1,  &relation_id, &slave_id)){
            InstanceWidget* slave  = this->instanceid2pointer[slave_id];
            
            QPoint middle = (master->geometry.topRight() + slave->geometry.topLeft()) / 2;
            InstanceRelation* ir = new InstanceRelation(relation_id, middle, this, this->main_widget);
            // NOTE: master destruction destroys ir
            
            ir->id = relation_id;
            
            master->relations[slave] = ir;
        }
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
	const InlistFilterRules r = this->inlist_filter_dialog->rules;
	
    if (r.skip_tagged  &&  this->file_id){
		PRINTF("Skipped previously tagged: %s\n", this->get_media_fp()); // TODO: Look into possible issues around this
		return this->media_next();
	}
	if (!r.filename_regexp.pattern().isEmpty()){
		const QRegularExpressionMatch m = r.filename_regexp.match(this->get_media_fp());
		if (m.captured().isEmpty())
			return this->media_next();
	}
    
    // WARNING: fp MUST be initialised, unless called via signal button press
    QString file = this->get_media_fp();
	
	QFile f(file);
	this->file_sz = f.size();
	
	if (
		(r.file_sz_min && this->file_sz < r.file_sz_min) ||
		(r.file_sz_max && this->file_sz > r.file_sz_max)
	)
		return this->media_next();
	
    /* Set window title */
    QString fname = this->get_media_fp() + this->media_dir_len;
    this->setWindowTitle(fname);
    
  #ifdef VID
    m_player->play(file);
    PRINTF("Duration: %d\n", this->m_player->duration());
    m_player->setRepeat(-1); // Repeat infinitely
  #elif (defined TXT)
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
    if (r.skip_trans){
		
	}
	const int w = this->image.width();
	const int h = this->image.height();
	if (
		(r.w_min && w < r.w_min) ||
		(r.w_max && w > r.w_max) ||
		(r.h_min && h < r.h_min) ||
		(r.h_max && h > r.h_max) ||
		(r.skip_grey && this->image.allGray()) ||
		(r.skip_trans && this->image.hasAlphaChannel() && false /* && this->image_has_transparent_pixel() : hasAlpha only detects whether an alpha channel exists, not whether it is used. */ )
	)
		return this->media_next();
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

void MainWindow::updateSliderUnit()
{
    m_unit = m_player->notifyInterval();
	this->updateSlider(m_player->position());
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
    
    compsky::mysql::exec(_mysql::obj,  BUF,  "UPDATE file SET score=", score_int, " WHERE id=", this->file_id);
}

void MainWindow::ensure_fileID_set(){
    if (this->file_id == 0){
        compsky::mysql::exec(_mysql::obj,  BUF,  "INSERT INTO file (name) VALUES(\"", f_esc, '"', this->get_media_fp(), "\")");
        this->file_id = get_last_insert_id();
    }
}

void MainWindow::media_note(){
    this->ensure_fileID_set();
    
    compsky::mysql::query(_mysql::obj,  RES1,  BUF,  "SELECT note FROM file WHERE id=", this->file_id);
    
	const char* previous_note;
    compsky::mysql::assign_next_row(RES1,  &ROW1,  &previous_note);
    
    bool ok;
    QString str = QInputDialog::getMultiLineText(this, tr("Note"), tr("Note"), previous_note, &ok);
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
    compsky::mysql::exec(_mysql::obj,  BUF,  "UPDATE file SET note=\"", f_esc, '"', NOTE, "\" WHERE id=", this->file_id);
}

void MainWindow::tag2parent(uint64_t tagid,  uint64_t parid){
    compsky::mysql::exec(_mysql::obj,  BUF,  "INSERT IGNORE INTO tag2parent (tag_id, parent_id) VALUES (", tagid, ",", parid, ")");
}

uint64_t MainWindow::ask_for_tag(const QString str){
	NameDialog* tagdialog = new NameDialog("Tag", str);
	tagdialog->name_edit->setCompleter(this->tagcompleter);
	const auto rc = tagdialog->exec();
	const QString tagstr = tagdialog->name_edit->text();

	if (rc != QDialog::Accepted  ||  tagstr.isEmpty())
		return 0;

	const QByteArray tagstr_ba = tagstr.toLocal8Bit();
	const char* tagchars = tagstr_ba.data();
	
	return  (this->tagslist.contains(tagstr))  ?  this->get_id_from_table("tag", tagchars)  :  this->add_new_tag(tagstr);
}

uint64_t MainWindow::add_new_tag(const QString tagstr,  uint64_t tagid){
	const QByteArray tagstr_ba = tagstr.toLocal8Bit();
    const char* const tagchars = tagstr_ba.data();
    
    this->tagslist.append(tagstr);
    delete this->tagcompleter;
    this->tagcompleter = new QCompleter(this->tagslist);
    
    if (tagid == 0)
        tagid = this->get_id_from_table("tag", tagchars);
    
    this->tag_id2name[tagid] = tagstr;
    
    /* Get parent tag */
    NameDialog* tagdialog = new NameDialog("Parent Tag of", tagstr);
    tagdialog->name_edit->setCompleter(this->tagcompleter);
    goto__cannotcancelparenttag:
    if (tagdialog->exec() != QDialog::Accepted)
        goto goto__cannotcancelparenttag;
    
    QString parent_tagstr = tagdialog->name_edit->text();
    
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

const QString MainWindow::media_tag(const QString str){
    if (media_fp[0] == 0)
        return "";
    
	const uint64_t tagid = this->ask_for_tag(str);
	
	if (tagid == 0)
		return "";
    
    this->ensure_fileID_set();
    
    compsky::mysql::exec(_mysql::obj,  BUF,  "INSERT IGNORE INTO file2tag (file_id, tag_id) VALUES(", this->file_id, ',', tagid, ")");
    
    return this->tag_id2name[tagid];
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

#ifndef _WIN32
void MainWindow::media_replace_w_link(const char* src){
    PRINTF("Deleting: %s\n", this->get_media_fp());
    if (remove(this->get_media_fp()) != 0)
        fprintf(stderr, "Failed to delete: %s\n", this->get_media_fp());
    if (symlink(src, this->get_media_fp()) != 0)
        fprintf(stderr, "Failed to create ln2del symlink: %s\n", this->get_media_fp());
}
#endif

void MainWindow::media_delete(){
    this->media_replace_w_link("/home/compsky/bin/ln2del_ln");
}

#ifndef _WIN32
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
#endif

uint64_t MainWindow::get_id_from_table(const char* const table_name,  const char* const entry_name){
    uint64_t value = 0;
    
    goto__returnresultsofthegetidfromtablequery:
    compsky::mysql::query(_mysql::obj,  RES1,  BUF,  "SELECT id FROM ", table_name, " WHERE name=\"", f_esc, '"', entry_name, '"');
    
    while(compsky::mysql::assign_next_row(RES1,  &ROW1,  &value));
    
    if (value)
        return value;
    
    compsky::mysql::exec(_mysql::obj,  BUF,  "INSERT INTO ", table_name, " (name) VALUES (\"", f_esc, '"', entry_name, "\")");
    goto goto__returnresultsofthegetidfromtablequery;
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
    
    this->ensure_fileID_set();
    
    compsky::mysql::exec(_mysql::obj,  BUF,  "INSERT INTO instance (file_id, x, y, w, h, frame_n) VALUES(", f_start, ',', this->file_id, f_g, x, 17, f_g, y, 17, f_g, w, 17, f_g, h, 17, frame_n, f_end, ')');
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
		const uint64_t tagid = this->ask_for_tag();
		if (tagid == 0)
			break;
		this->instance_widget->tags.append(this->tag_id2name[tagid]);
        compsky::mysql::exec(_mysql::obj,  BUF,  "INSERT IGNORE INTO instance2tag (instance_id, tag_id) VALUES(", this->instance_widget->id, ',', tagid, ')');
    }
    
    this->instance_widget->set_colour(QColor(255,0,255,100));
  #ifdef SCROLLABLE
    this->instance_widget->orig_scale_factor = this->scale_factor;
  #endif
    this->instance_widget->orig_geometry = this->instance_widget->geometry;
    this->instance_widgets.push_back(this->instance_widget);
    
    this->instance_widget->frame_n = frame_n;
    
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
#endif // end ifdef BOXABLE

void MainWindow::show_settings_dialog(){
	this->inlist_filter_dialog->show();
}

void MainWindow::display_info(){
	InfoDialog* info_dialog = new InfoDialog(this->file_id, this->file_sz, this);
	info_dialog->exec();
	delete info_dialog;
}

void MainWindow::display_relation_hub(){
	RelationAddInstanceTags* hub = new RelationAddInstanceTags(this);
	hub->exec();
	delete hub;
}
