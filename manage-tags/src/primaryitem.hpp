#ifndef __PRIMARYITEM__
#define __PRIMARYITEM__

#include <QStandardItem>

// Assumes asciify.hpp already included

#include "asciify_flags.hpp"

#include "tagtreemodel.hpp"

namespace detail {
    extern char* BUF;
    extern size_t BUF_INDX;
    
    extern void enlarge_buf();
}

namespace res1 {
    template<typename... Args>void exec(Args... args);
}

inline char* uint64_to_str(uint64_t n){
    // Inlined to avoid multiple definition error
    if (detail::BUF_INDX <= 20)
        detail::enlarge_buf();
    detail::BUF[--detail::BUF_INDX] = 0;
    do {
        --detail::BUF_INDX;
        detail::BUF[detail::BUF_INDX] = '0' + (n % 10);
        n /= 10;
    } while(n != 0);
    return detail::BUF + detail::BUF_INDX;
}

class StandardItem : public QStandardItem {
 public:
    StandardItem(const QString& s) : QStandardItem(s) {};
    StandardItem(uint64_t n) : QStandardItem(uint64_to_str(n)) {};
};

class PrimaryItem : public QObject, public StandardItem {
    Q_OBJECT
 public:
    PrimaryItem(const QString& s) : StandardItem(s) {};
    PrimaryItem(uint64_t n) : StandardItem(uint64_to_str(n)) {};
    QStandardItem* parent(){
        // Avoid ambiguous definition from multiple inheritances
        return QStandardItem::parent();
    };
 public Q_SLOTS:
    void delete_self(){
        QStandardItem* prnt = this->parent();
        
        const QString qs = (prnt) ? prnt->text() : "0";
        
        res1::exec("DELETE FROM tag2parent WHERE parent_id=",  qs,  " AND tag_id=", this->text());
        if (prnt)
            this->model()->removeRow(this->row());
        else
            prnt->removeRow(this->row());
    };
    void add_subtag(){
    };
};

class NameItem : public QObject, public StandardItem {
 public:
    const uint64_t tag_id;
    
    NameItem(const uint64_t id,  QString s) : tag_id(id), StandardItem(s) {}
    
    void setData(const QVariant& value,  int role = Qt::UserRole + 1){
        QString a = this->data().toString();
        QByteArray b = a.toLocal8Bit();
        
        const char* s = b.data(); // Equivalent to siblingAtColumn, but that is introduced in Qt 5.11
        
        if (!s)
            return;
        
        const QString qs = value.toString();
        const QByteArray ba = qs.toLocal8Bit();
        const char* neue = ba.data();
        if (neue[0] == 0)
            return;
        
        TagTreeModel* mdl = (TagTreeModel*)this->model();
        if (!mdl)
            // Probably during construction of NameItem during construction of TagTreeModel
            return;
        
        if (qs == mdl->tag2name[this->tag_id])
            return;
        
        res1::exec("UPDATE tag SET name=\"",  compsky::asciify::flag::escape,  '"',  neue,  "\" WHERE id=",  this->tag_id);
        
        QStandardItem::setData(value, role);
    };
};

#endif
