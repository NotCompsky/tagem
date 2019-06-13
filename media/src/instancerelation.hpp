#ifndef __INSTANCERELATION__
#define __INSTANCERELATION__

#include <QPushButton>
#include <QStringList>


class InstanceRelation : public QObject{
    Q_OBJECT
 public:
    InstanceRelation(QPoint middle,  QWidget* parent);
    ~InstanceRelation();
    QPushButton* btn;
    QStringList tags;
    uint64_t id;
 public Q_SLOTS:
    void toggle_expand();
 private:
    void show_text();
    bool is_expanded;
};


#endif
