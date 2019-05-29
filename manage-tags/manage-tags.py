#!/usr/bin/env python3


from   itertools import chain
import mysql.connector
from   PyQt5.QtCore import QMimeData, QModelIndex, Qt
from   PyQt5.QtWidgets import QApplication, QMainWindow, QTreeView, QWidget, QHBoxLayout, QVBoxLayout, QPushButton, QToolButton, QHeaderView
from   PyQt5.QtGui import QStandardItem, QStandardItemModel
import sys


#n_uncommited_edits:int = 0


def cursor_execute(s:str):
    print(s)
    cursor.execute(s, multi=False) # WARNING: If multi is set to False, it will automatically commit the last stored commit
    cnx.commit()
    #global n_uncommited_edits
    #n_uncommited_edits += 1
    #win.commit_btn.setText(f"Commit {n_uncommited_edits} edits")

'''
def cursor_commit(dunno:bool):
    cnx.commit()
    global n_uncommited_edits
    n_uncommited_edits = 0
    win.commit_btn.setText("Nothing to commit")
'''


class PrimaryItem(QStandardItem):
    def delete_self(self):
        # Does not call TagTreeModel::removeRows
        s:str = f"DELETE FROM tag2parent WHERE parent_id={self.parent().text() or '0'} AND tag_id={self.text()}"
        cursor_execute(s)
        self.parent().removeRow(self.row())


class TagTreeModel(QStandardItemModel):
    def dropMimeData(self, data:QMimeData, action, row:int, col:int, parent:QModelIndex):
        # Dropped items by default can be dropped onto any column; this is nonsensical for our purposes
        # NOTE: Probably platform dependant. See available formats with data.formats()
        if (not super().dropMimeData(data, action, row, 0, parent)):
            return False
        parent_tag_id:str = parent.data() or "0"
        tag_id:str = self.index(self.rowCount(parent)-1, 0, parent).data()
        s:str = f"INSERT IGNORE INTO tag2parent (parent_id, tag_id) VALUES ({parent_tag_id}, {tag_id})"
        cursor_execute(s)
        return True
    def removeRows(self, row:int, count:int, parent:QModelIndex):
        tag_id:str = self.index(row, 0, parent).data()
        if (not super().removeRows(row, count, parent)):
            return False
        parent_tag_id:str = parent.data() or "0"
        s:str = f"DELETE FROM tag2parent WHERE parent_id={parent_tag_id} AND tag_id={tag_id}"
        cursor_execute(s)
        return True
    # NOTE: moveRows is not called when moving a row between two parents


class TagTreeView(QTreeView):
    ID, NAME, COUNT, DLT_BTN = range(4)
    
    def __init__(self, use_tagtreemodel:bool, parent:QWidget):
        super().__init__(parent)
        self.header().setSectionResizeMode(QHeaderView.ResizeToContents)
        
        if (use_tagtreemodel):
            self.model = TagTreeModel(0, 4, parent)
        else:
            self.model = QStandardItemModel(0, 4, parent)
        
        self.setModel(self.model)
        
        self.init_headers()
        
    def init_headers(self):
        self.model.setHeaderData(self.ID,        Qt.Horizontal, "ID")
        self.model.setHeaderData(self.NAME,      Qt.Horizontal, "Name")
        self.model.setHeaderData(self.COUNT,     Qt.Horizontal, "Direct Occurances") # i.e. does not include the count of tags inheriting from it
        self.model.setHeaderData(self.DLT_BTN,   Qt.Horizontal, "Delete")
        
    def place_tags(self, root:int, expand:bool):
        self.tagid2entry:dict = {root: self.model}
        queue:list = []
        res:list = [x for x in cursor]
        n_results:int = len(res)
        generator = chain(res, queue)
        
        for i, (parent_id, tag_id, name, count) in enumerate(generator):
            if (i > 2*n_results):
                ls:list = [f"{p}\t{t}\t{n.decode()}" for (p,t,n,c) in generator]
                delim:str = "\n  "
                raise Exception(f"Skipped some tags at least twice, probably because heirarchy is broken. Bad tags: {delim}Parent Tag Name{delim}{delim.join(ls)}")
            print(parent_id, tag_id, name, count)
            # parent_id is actually tag_id and vice versa, but this is the view of the reverse heirarchy, parents branching from children
            if (parent_id == tag_id):
                raise ValueError("tag_id == parent_id (unfortunately, MySQL does not support checks)")
            
            entry__id:QStandardItem = PrimaryItem(str(tag_id))
            entry__id.setEditable(False)
            entry__id.setDropEnabled(True)
            
            entry__name:QStandardItem = QStandardItem(name.decode())
            entry__name.setEditable(True)
            entry__name.setDropEnabled(False)
            
            entry__count:QStandardItem = QStandardItem(str(count))
            entry__count.setEditable(False)
            entry__count.setDropEnabled(False)
            
            entry__delete:QStandardItem = QStandardItem()
            entry__delete.setEditable(False)
            entry__delete.setDropEnabled(False)
            
            try:
                self.tagid2entry[parent_id].appendRow([entry__id, entry__name, entry__count, entry__delete])
            except KeyError:
                queue.append((parent_id, tag_id, name, count))
                continue
            
            delete_btn = QToolButton(text="X")
            delete_btn.setMaximumSize(delete_btn.sizeHint())
            self.setIndexWidget(entry__delete.index(), delete_btn)
            delete_btn.clicked.connect(entry__id.delete_self)
            if (expand):
                self.expand(entry__id.index())
            
            self.tagid2entry[tag_id] = entry__id


class TagChildTreeView(TagTreeView):
    def __init__(self, parent:QWidget):
        super().__init__(True, parent)
        self.setSelectionBehavior(self.SelectRows)
        self.setSelectionMode(self.SingleSelection)
        self.setDragDropMode(self.InternalMove)
        self.setDragDropOverwriteMode(False)
        
        cursor.execute("SELECT parent_id, B.id, B.name, B.c FROM tag2parent t2p LEFT JOIN (SELECT id, name, COUNT(A.tag_id) as c FROM tag LEFT JOIN(SELECT tag_id FROM file2tag) A ON A.tag_id = id GROUP BY id, name) B ON B.id = t2p.tag_id ORDER BY t2p.parent_id ASC", multi=False)
        
        self.place_tags(0, False)


class TagParentTreeView(TagTreeView):
    def __init__(self, tag_child_tree_view:TagChildTreeView, parent:QWidget):
        super().__init__(False, parent)
        
        tag_child_tree_view.selectionModel().selectionChanged.connect(self.set_root)
        self.tag_child_tree_view = tag_child_tree_view
        
        self.setSelectionBehavior(self.SelectRows)
        self.setSelectionMode(self.SingleSelection)
        self.setDragDropOverwriteMode(False)
    
    def set_root(self):
        self.tagid2entry = {} # Not unnecessary
        self.init_headers()
        
        for _ in range(self.model.rowCount()):
            self.model.removeRow(0)
        
        tag_id:str = self.tag_child_tree_view.selectionModel().currentIndex().siblingAtColumn(0).data()
        cursor.execute(f"CALL ancestor_tags_id_rooted_from_id(\"tmp_tag_parents\", {tag_id})", multi=False)
        cursor.execute("SELECT node, parent, name, 0 FROM tmp_tag_parents JOIN tag ON id=parent WHERE node")
        
        self.place_tags(int(tag_id), False)


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("MyTag Tag Manager")
        w = QWidget()
        l:QVBoxLayout = QVBoxLayout()
        hl:QHBoxLayout = QHBoxLayout()
        self.commit_btn = QPushButton("Commit")
        tag_child_tree_view:TagChildTreeView = TagChildTreeView(self)
        hl.addWidget(tag_child_tree_view)
        tag_parent_tree_view:TagParentTreeView = TagParentTreeView(tag_child_tree_view, self)
        hl.addWidget(tag_parent_tree_view)
        #self.commit_btn.clicked.connect(cursor_commit)
        l.addLayout(hl)
        l.addWidget(self.commit_btn)
        w.setLayout(l)
        self.setCentralWidget(w)
        self.show()


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
    cnx.autocommit = False
    cursor = cnx.cursor()
    
    app:QApplication = QApplication([])
    win:MainWindow = MainWindow()
    sys.exit(app.exec_())
