/*
Based on Python Qt5 code by Dmitry from https://github.com/d1vanov/PyQt5-reorderable-list-model/blob/master/reorderable_list_model.py
*/

#ifndef MAINRWINDOW_H
#define MAINRWINDOW_H

#include <QAbstractItemModel>
#include <QCompleter>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QTreeView>
#include <QWidget>
#include <QMimeData>
#include <QStandardItem>


class Node;


class Node {
 public:
    Node(const QString& txt,  Node* parent)
    :
        text(txt),
        parent(parent)
    {
        
    };
    ~Node(){};
    QVariant data() const{
        return QVariant("placeholder_data");
    };
    
    QString text;
    Node* parent;
    QList<Node*> children;
};


class ReadOnlyModel : public QAbstractItemModel {
    Q_OBJECT
 public:
    ReadOnlyModel(Node* root){
        this->m_root = root;
    };
    QVariant data(const QModelIndex& index,  int role) const{
        if (index.isValid()  &&  role == Qt::DisplayRole){
            Node* node = this->nodeForIndex(index);
            return node->data();
        }
        return QVariant();
    };
    QModelIndex index(int row,  int column,  const QModelIndex& parent) const{
        if (this->hasIndex(row, column, parent))
            return this->createIndex(row, column, this->nodeForIndex(parent)->children.at(row));
        return QModelIndex();
    };
    QModelIndex parent(const QModelIndex& child_index) const{
        Node* child  = this->nodeForIndex(child_index);
        Node* parent = child->parent;
        if (parent == this->m_root)
            return QModelIndex();
        return createIndex(this->rowForNode(parent), 0, parent);
    };
    int rowCount(const QModelIndex& parent) const{
        return this->nodeForIndex(parent)->children.count();
    };
    int columnCount(const QModelIndex& parent) const{
        Q_UNUSED(parent);
        return 1;
    };
 private:
    Node* m_root;
 protected:
    QModelIndex indexForNode(Node* node) const {
        if (node == this->m_root)
            return QModelIndex();
        return this->createIndex(this->rowForNode(node), 0, node);
    };
    Node* nodeForIndex(const QModelIndex& index) const{
        if (index.isValid())
            return static_cast<Node*>(index.internalPointer());
        return this->m_root;
    };
    int rowForNode(Node* node) const{
        return node->parent->children.indexOf(node);
    };
};


class EditableModel : public ReadOnlyModel {
    Q_OBJECT
 public:
    EditableModel(Node* root) : ReadOnlyModel(root) {};
    bool setData(const QModelIndex& index,  const QVariant& value,  int role){
        if (index.isValid()  &&  role == Qt::EditRole){
            this->nodeForIndex(index)->text = value.toString();
            emit dataChanged(index, index);
            return true;
        }
        return false;
    };
    Qt::ItemFlags flags(const QModelIndex& index) const{
        Qt::ItemFlags default_flags = ReadOnlyModel::flags(index);
        if (index.isValid())
            return Qt::ItemIsEditable | default_flags;
        return default_flags;
    };
};


class InsertRemoveModel : public EditableModel {
    Q_OBJECT
 public:
    InsertRemoveModel(Node* root) : EditableModel(root) {};
    ~InsertRemoveModel(){};
    void insertNode(Node* parent,  int pos,  Node* node){
        this->beginInsertRows(this->indexForNode(parent), pos, pos);
        parent->children.insert(pos, node);
        this->endInsertRows();
    };
    void removeNode(Node* node){
        Node* parent = node->parent;
        int pos = this->rowForNode(node);
        this->beginRemoveRows(this->indexForNode(parent), pos, pos);
        parent->children.removeAt(pos);
        this->endRemoveRows();
    };
    void removeAllNodes();
};

/*
class LazyModel : public ReadOnlyModel {
    // see https://www.youtube.com/watch?v=d2OFjACLgOg
public:
    bool hasChildren(parent) const;
    bool canFetchMore(parent) const;
    void fetchMore();
};*/



class DraggableModel : public InsertRemoveModel {
    Q_OBJECT
 public:
    DraggableModel(Node* root) : InsertRemoveModel(root) {};
    Qt::ItemFlags flags(const QModelIndex& index) const{
        if (index.isValid())
            return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | InsertRemoveModel::flags(index);
        return Qt::ItemIsDropEnabled | InsertRemoveModel::flags(index); // Empty index
    };
    Qt::DropActions supportedDropActions() const{
        return Qt::CopyAction | Qt::MoveAction;
    };
    QStringList mimeTypes() const{
        QStringList types = {"application/vnd.text.list"};
        return types;
    };
    QMimeData* mimeData(const QModelIndexList& indexes) const{
        QByteArray encodedData;
        QDataStream stream(&encodedData, QIODevice::WriteOnly);
        foreach(const QModelIndex& index,  indexes)
            if (index.isValid())
                stream << this->data(index, Qt::DisplayRole).toString();
        QMimeData* mimeData = new QMimeData();
        mimeData->setData("application/vnd.text.list", encodedData);
        return mimeData;
    };
    bool dropMimeData(const QMimeData* data,  Qt::DropAction action,  int row,  int column,  const QModelIndex& parent_index){
        if (action == Qt::IgnoreAction)
            return true;
        if (!data->hasFormat("application/vnd.text.list"))
            return false;
        if (column >= this->columnCount(parent_index))
            return false;
        Node* parent = this->nodeForIndex(parent_index);
        QByteArray encodedData = data->data("application/vnd.text.list");
        QDataStream stream(&encodedData, QIODevice::ReadOnly);
        while(!stream.atEnd()){
            QString text;
            stream >> text;
            Node* node = new Node(text, parent);
            this->insertNode(parent, row, node);
        }
        return true;
    };
    //bool removeRows(int row,  int count,  const QModelIndex& parent);
    //bool insertRows(int row,  int count,  const QModelIndex& parent);
};


class MainWindow : public QWidget {
    Q_OBJECT
 public:
    MainWindow(const int argc,  const char** argv,  QWidget *parent = 0) : QWidget(parent) {
        this->root_node = new Node("ROOT", nullptr);
        
        this->model = new DraggableModel(this->root_node);
        QStandardItem* item = new QStandardItem("Draggable Item");
        item->setDragEnabled(true);
        item->setDropEnabled(true);
        
        this->model->insertNode(this->root_node,  0,  new Node("First", this->root_node));
        this->model->insertNode(this->root_node,  0,  new Node("Second", this->root_node));
        this->model->insertNode(this->root_node,  0,  new Node("Third", this->root_node));
    };
    ~MainWindow(){
        delete this->model;
        delete this->root_node;
    };
    DraggableModel* model;
    Node* root_node;
};


#endif // MAINRWINDOW_H
