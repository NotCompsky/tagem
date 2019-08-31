#ifndef __PRIMARYITEM__
#define __PRIMARYITEM__

#include <QStandardItem>


class TagTreeView;


// Assumes asciify.hpp already included


inline char* uint64_to_str(uint64_t n){
	static char* buf = (char*)malloc(4096);
	static char* itr = nullptr;
	static size_t buf_sz = 4096;
	
	if (itr  <  buf + buf_sz + 19 + 1){
		buf_sz *= 2;
		char* _dummy = (char*)realloc(buf, buf_sz);
		if (_dummy == nullptr)
			; // TODO: 
		if (_dummy > buf)
			itr += _dummy - buf;
		else
			itr -= buf - _dummy;
		buf = _dummy;
	}
	size_t n_digits = 0;
	uint64_t m = n;
	do {
		++n_digits;
		m /= 10;
	} while(m != 0);
	itr += n_digits + 1; // Terminating null byte
	*itr = 0;
    do {
		*(--itr) = '0' + (n % 10);
        n /= 10;
    } while(n != 0);
	return itr;
}


class StandardItem : public QStandardItem {
 public:
    StandardItem(const QString& s) : QStandardItem(s) {};
    StandardItem(const uint64_t n) : QStandardItem(uint64_to_str(n)) {};
};

class PrimaryItem : public QObject, public StandardItem {
 private:
	const uint64_t tag_id;
	TagTreeView* const view;
 public:
	PrimaryItem(TagTreeView* const _view,  const uint64_t n,  const QString& s)
	: view(_view)
	, tag_id(n)
	, StandardItem(s)
	{};
	
	PrimaryItem(TagTreeView* const _view,  const uint64_t n)
	: view(_view)
	, tag_id(n)
	, StandardItem(uint64_to_str(n))
	{};
	
	void delete_self(); // SLOT
	void add_subtag(); // SLOT
	void add_parent(); // SLOT
    QStandardItem* parent();
};

class NameItem : public QObject, public StandardItem {
 public:
    const uint64_t tag_id;
    
    NameItem(const uint64_t id,  const QString& s) : tag_id(id), StandardItem(s) {}
    
    void setData(const QVariant& value,  const int role = Qt::UserRole + 1);
};

#endif
