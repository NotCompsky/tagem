#!/usr/bin/env python3


from   itertools import chain
import mysql.connector
from   PyQt5.QtCore import QMimeData, QModelIndex, Qt
from   PyQt5.QtWidgets import QApplication, QMainWindow, QTreeView, QWidget, QHBoxLayout, QVBoxLayout, QPushButton
from   PyQt5.QtGui import QStandardItem, QStandardItemModel
import sys


class TagTreeModel(QStandardItemModel):
    def dropMimeData(self, data:QMimeData, action, row:int, col:int, parent:QModelIndex):
        # Dropped items by default can be dropped onto any column; this is nonsensical for our purposes
        # NOTE: Probably platform dependant. See available formats with data.formats()
        if (not super().dropMimeData(data, action, row, 0, parent)):
            return False
        parent_tag_id:str = parent.data() or "0"
        tag_id:str = self.index(self.rowCount(parent)-1, 0, parent).data()
        s:str = f"INSERT INTO tag2parent (parent_id, tag_id) VALUES ({parent_tag_id}, {tag_id})"
        print(s)
        cursor.execute(s, multi=False)
        self.update_tag_parent_tree(tag_id)
        return True
    def removeRows(self, row:int, count:int, parent:QModelIndex):
        tag_id:str = self.index(row, 0, parent).data()
        if (not super().removeRows(row, count, parent)):
            return False
        parent_tag_id:str = parent.data() or "0"
        s:str = f"DELETE FROM tag2parent WHERE parent_id={parent_tag_id} AND tag_id={tag_id}"
        print(s)
        cursor.execute(s, multi=False)
        return True
    # NOTE: moveRows is not called when moving a row between two parents


class TagTreeView(QTreeView):
    def __init__(self, parent:QWidget):
        super().__init__(parent)
        
    def place_tags(self):
        self.tagid2entry:dict = {0: self.model}
        queue:list = []
        generator = chain(cursor, queue)
        for (parent_id, tag_id, name, count) in generator:
            # parent_id is actually tag_id and vice versa, but this is the view of the reverse heirarchy, parents branching from children
            if (parent_id == tag_id):
                raise ValueError("tag_id == parent_id (unfortunately, MySQL does not support checks)")
            entry__id:QStandardItem = QStandardItem(str(tag_id))
            entry__id.setEditable(False)
            entry__id.setDropEnabled(True)
            
            entry__name:QStandardItem = QStandardItem(name.decode())
            entry__name.setEditable(True)
            entry__name.setDropEnabled(False)
            
            entry__count:QStandardItem = QStandardItem(str(count))
            entry__count.setEditable(False)
            entry__count.setDropEnabled(False)
            
            try:
                self.tagid2entry[parent_id].appendRow([entry__id, entry__name, entry__count])
            except KeyError:
                queue.append((parent_id, tag_id, name, count))
                continue
            
            self.tagid2entry[tag_id] = entry__id


class TagChildTreeView(TagTreeView):
    ID, NAME, COUNT = range(3)
    
    def __init__(self, parent:QWidget):
        super().__init__(parent)
        self.setSelectionBehavior(self.SelectRows)
        self.setSelectionMode(self.SingleSelection)
        self.setDragDropMode(self.InternalMove)
        self.setDragDropOverwriteMode(False)
        
        self.model:TagTreeModel = TagTreeModel(0, 3, parent)
        
        self.model.setHeaderData(self.ID,    Qt.Horizontal, "ID")
        self.model.setHeaderData(self.NAME,  Qt.Horizontal, "Name")
        self.model.setHeaderData(self.COUNT, Qt.Horizontal, "Direct Occurances") # i.e. does not include the count of tags inheriting from it
        
        self.setModel(self.model)
        
        cursor.execute("SELECT parent_id, B.id, B.name, B.c FROM tag2parent t2p LEFT JOIN (SELECT id, name, COUNT(A.tag_id) as c FROM tag LEFT JOIN(SELECT tag_id FROM file2tag) A ON A.tag_id = id GROUP BY id, name) B ON B.id = t2p.tag_id ORDER BY t2p.parent_id ASC", multi=False)
        
        self.place_tags()


class TagParentTreeView(TagTreeView):
    ID, NAME = range(2)
    
    def __init__(self, tag_child_tree_view:TagChildTreeView, parent:QWidget):
        super().__init__(parent)
        
        tag_child_tree_view.selectionModel().selectionChanged.connect(self.set_root)
        self.tag_child_tree_view = tag_child_tree_view
        
        self.setSelectionBehavior(self.SelectRows)
        self.setSelectionMode(self.SingleSelection)
        self.setDragDropOverwriteMode(False)
        
        self.model:TagTreeModel = TagTreeModel(0, 2, parent)
        
        self.model.setHeaderData(self.ID,    Qt.Horizontal, "ID")
        self.model.setHeaderData(self.NAME,  Qt.Horizontal, "Name")
        
        self.setModel(self.model)
    
    def set_root(self):
        self.model.clear(); #removeRows(0, self.countRows())
        
        tag_id:str = self.tag_child_tree_view.selectionModel().currentIndex().siblingAtColumn(0).data()
        cursor.execute(f"CALL ancestor_tags_id_rooted_from_id(\"tmp_tag_parents\", {tag_id})", multi=False)
        cursor.execute("SELECT parent, node, name, 0 FROM tmp_tag_parents JOIN tag ON id=node")
        
        self.place_tags()


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("MyTag Tag Manager")
        w = QWidget()
        l:QVBoxLayout = QVBoxLayout()
        hl:QHBoxLayout = QHBoxLayout()
        commit_btn = QPushButton("Commit")
        tag_child_tree_view:TagChildTreeView = TagChildTreeView(self)
        hl.addWidget(tag_child_tree_view)
        tag_parent_tree_view:TagParentTreeView = TagParentTreeView(tag_child_tree_view, self)
        hl.addWidget(tag_parent_tree_view)
        commit_btn.clicked.connect(self.commit)
        l.addLayout(hl)
        l.addWidget(commit_btn)
        w.setLayout(l)
        self.setCentralWidget(w)
        self.show()
    def commit(self, dunno:bool):
        cnx.commit()


if __name__ == "__main__":
    fp:str = sys.argv[1]
    credentials:list = open(fp).read().split("\n")
    
    kwargs:dict = {"user": credentials[1],  "password": credentials[2],  "database": "mytag"}
    
    kwargs["get_warnings"] = True
    kwargs["raise_on_warnings"] = True
    kwargs["buffered"] = True
    
    mysql_url:str = credentials[0]
    if (mysql_url.startswith("unix://")):
        kwargs["unix_socket"] = credentials[0][7:]
    else:
        raise Exception("Not imnplemented yet, and to prove it this message has typos")
    
    cnx = mysql.connector.connect(**kwargs)
    cursor = cnx.cursor()
    
    app:QApplication = QApplication([])
    win:MainWindow = MainWindow()
    sys.exit(app.exec_())
