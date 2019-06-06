#ifndef __PRIMARYITEM__
#define __PRIMARYITEM__

#include <QStandardItem>

// Assumes asciify.hpp already included


namespace detail {
    extern char* BUF;
    extern size_t BUF_INDX;
    
    extern void enlarge_buf();
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
    QStandardItem* parent();
 public Q_SLOTS:
    void delete_self();
    void add_subtag();
};

class NameItem : public QObject, public StandardItem {
 public:
    const uint64_t tag_id;
    
    NameItem(const uint64_t id,  QString s) : tag_id(id), StandardItem(s) {}
    
    void setData(const QVariant& value,  int role = Qt::UserRole + 1);
};

#endif
