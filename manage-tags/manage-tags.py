#!/usr/bin/env python3

'''
TODO

Rewrite in C++
Capture key events for some shortcuts (such as DELETE key to delete all instances and relationships of a tag) (already written in C++)
Utility to add tags immediately with multiple parents (already written in C++)
'''


from   itertools import chain
import mysql.connector
from   PyQt5.QtCore import QMimeData, QModelIndex, Qt, QTimer
from   PyQt5.QtWidgets import QApplication, QMainWindow, QTreeView, QWidget, QHBoxLayout, QVBoxLayout, QPushButton, QToolButton, QHeaderView, QDialog, QDialogButtonBox, QLineEdit, QLabel
from   PyQt5.QtGui import QStandardItem, QStandardItemModel, QDragMoveEvent
from   re import sub
import sys


tagid2name:dict = {}
tagid2directoccurances:dict = {}
tagid2occurances:dict = {0: 0}


def cursor_execute(s:str):
    print(s)
    cursor.execute(s, multi=False) # WARNING: If multi is set to False, it will automatically commit the last stored commit. If True, it will not commit anything.
    cnx.commit()


class TagDialog(QDialog):
    def __init__(self, title, string, parent):
        # Exact (aside from renamed vars) equivalent of C++ in mainwindow.cpp
        super().__init__(parent)
        button_box = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        button_box.accepted.connect(self.accept)
        button_box.rejected.connect(self.reject)
        l = QVBoxLayout()
        l.addWidget(button_box)
        self.name_edit = QLineEdit(string)
        l.addWidget(self.name_edit)
        guide = QLabel("Enter blank tag to designate as root tag")
        l.addWidget(guide)
        self.setLayout(l)
        self.setWindowTitle(title)
        #QTimer.singleShot(0, self.name_edit, self.setFocus)


class PrimaryItem(QStandardItem):
    def delete_self(self):
        # The default does not call TagTreeModel::removeRows
        parent = self.parent()
        t:str = "0"
        if (parent is not None):
            t = parent.text()
        s:str = f"DELETE FROM tag2parent WHERE parent_id={t} AND tag_id={self.text()}"
        cursor_execute(s)
        if (parent is None):
            self.model.removeRow(self.row())
        else:
            parent.removeRow(self.row())
    def add_subtag(self):
        pass


class TagTreeModel(QStandardItemModel):
    tagid2entry:dict = {}
    tag2parent:dict = {}
    dragging = False
    
    def dropMimeData(self, data:QMimeData, action, row:int, col:int, dst_parent:QModelIndex):
        self.dragging = False
        
        b:bytes = sub(
            b"^"
            b"[^0-9]*"
            b"((?:[0-9]\0)+)" # Tag ID
            b"[^0-9]*" # Separators and tag name
            b"[0-9]+" # Number of direct occurances
            b"[^0-9]"
            b"(?:.|\n)*"
            b"$",
            b"\\1",
            data.data("application/x-qabstractitemmodeldatalist").data())
        # NOTE: MIME is probably platform dependant. See available formats with data.formats()
        
        tag_id:int = int(sub(b"\0", b"", b))
        
        current_parent_id:int = self.tag2parent[tag_id]
        
        parent_tag_id:int = int(dst_parent.data() or 0)
        
        if (parent_tag_id == current_parent_id):
            return False
        
        if (not super().dropMimeData(data, action, row, 0, dst_parent)):
            return False
        
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
    
    def moveRows(self, src_parent:QModelIndex, row:int, count:int, dst_parent:QModelIndex, dst_child:int):
        print(src_parent.text(), row, count, dst_parent.text(), dst_child)
    # NOTE: moveRows is not called when moving a row between two parents
    
    def item_changed(self, item:QStandardItem):
        if (self.dragging):
            # We only want to update the name if the item change is due to editing the name, not moving
            return
        tag_id_:str = item.index().siblingAtColumn(0).data()
        if (tag_id_ is None):
            return
        tag_id:int = int(tag_id_)
        txt:str = item.text()
        if (txt == ""):
            return
        if (txt == tagid2name[tag_id]):
            return
        try:
            # Avoid setting names as integers, just in case
            int(txt)
            return
        except ValueError:
            pass
        tagid2name[tag_id] = txt
        cursor_execute(f"UPDATE tag SET name=\"{txt}\" WHERE id={tag_id_}")


class TagTreeView(QTreeView):
    ID, NAME, COUNT, DLT_BTN, ADDCHLD = range(5)
    editable = False
    
    def __init__(self, editable:bool, parent:QWidget):
        super().__init__(parent)
        self.header().setSectionResizeMode(QHeaderView.ResizeToContents)
        
        if (editable):
            self.model = TagTreeModel(0, 5, parent)
            self.model.itemChanged.connect(self.model.item_changed)
            self.editable = True
        else:
            self.model = QStandardItemModel(0, 3, parent)
        
        self.setModel(self.model)
        
        self.init_headers() 
        
    def init_headers(self):
        self.model.setHeaderData(self.ID,        Qt.Horizontal, "ID")
        self.model.setHeaderData(self.NAME,      Qt.Horizontal, "Name")
        self.model.setHeaderData(self.COUNT,     Qt.Horizontal, "Occurances") # Either direct - i.e. does not include the count of tags inheriting from it - or inherited
        if (self.editable):
            self.model.setHeaderData(self.DLT_BTN,   Qt.Horizontal, "Unchild")
            self.model.setHeaderData(self.ADDCHLD,   Qt.Horizontal, "+Child")
        
    def place_tags(self, root:int, expand:bool):
        self.model.tagid2entry = {root: self.model}
        queue:list = []
        res:list = [x for x in cursor]
        n_results:int = len(res)
        generator = chain(res, queue)
        
        for i, (parent_id, tag_id, name, count) in enumerate(generator):
            if (i > 2*n_results):
                ls:list = [f"{p}\t{t}\t{n.decode()}" for (p,t,n,c) in generator]
                delim:str = "\n  "
                print(f"Skipped some tags at least twice, probably because heirarchy is broken. Bad tags: {delim}Parent Tag Name{delim}{delim.join(ls)}")
                break
            print(parent_id, tag_id, name, count)
            # parent_id is actually tag_id and vice versa, but this is the view of the reverse heirarchy, parents branching from children
            if (parent_id == tag_id):
                raise ValueError(f"{name}: tag_id == parent_id == {tag_id} (unfortunately, MySQL does not support checks)")
            
            entry__id:QStandardItem = PrimaryItem(str(tag_id))
            entry__id.setEditable(False)
            entry__id.setDropEnabled(True)
            
            entry__name:QStandardItem = QStandardItem("[NONE]" if name is None else name.decode())
            entry__name.setEditable(True)
            entry__name.setDropEnabled(False)
            
            entry__count:QStandardItem = QStandardItem(str(count))
            entry__count.setEditable(False)
            entry__count.setDropEnabled(False)
            
            ls:list = [entry__id, entry__name, entry__count]
            
            if (self.editable):
                entry__addchild:QStandardItem = QStandardItem()
                entry__addchild.setEditable(False)
                entry__addchild.setDropEnabled(False)
                ls.append(entry__addchild)
                
                entry__delete:QStandardItem = QStandardItem()
                entry__delete.setEditable(False)
                entry__delete.setDropEnabled(False)
                ls.append(entry__delete)
            
            try:
                self.model.tagid2entry[parent_id].appendRow(ls)
            except KeyError:
                queue.append((parent_id, tag_id, name, count))
                continue
            
            if (self.editable):
                addchild_btn = QToolButton(text="+")
                addchild_btn.setMaximumSize(addchild_btn.sizeHint())
                self.setIndexWidget(entry__addchild.index(), addchild_btn)
                addchild_btn.clicked.connect(entry__id.add_subtag)
                
                delete_btn = QToolButton(text="X")
                delete_btn.setMaximumSize(delete_btn.sizeHint())
                self.setIndexWidget(entry__delete.index(), delete_btn)
                delete_btn.clicked.connect(entry__id.delete_self)
            
            if (expand):
                self.expand(entry__id.index())
            
            self.model.tag2parent[tag_id] = parent_id
            self.model.tagid2entry[tag_id] = entry__id
            tagid2name[tag_id] = name
            tagid2directoccurances[tag_id] = tagid2occurances[tag_id] = 0 if count is None else count
            tagid2occurances[parent_id] += 0 if count is None else count
    
    def dragMoveEvent(self, e:QDragMoveEvent):
        self.model.is_dragging = True
        super().dragMoveEvent(e)


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
        self.model.tagid2entry = {} # Not unnecessary
        self.model.tag2parent = {}
        self.init_headers()
        
        for _ in range(self.model.rowCount()):
            self.model.removeRow(0)
        
        tag_id:str = self.tag_child_tree_view.selectionModel().currentIndex().siblingAtColumn(0).data()
        cursor.execute(f"CALL ancestor_tags_id_rooted_from_id(\"tmp_tag_parents\", {tag_id})", multi=False)
        cursor.execute("SELECT node, parent, name, -1 FROM tmp_tag_parents JOIN tag ON id=parent WHERE node")
        
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
        self.add_new_tag()
    def add_new_tag(self):
        tagdialog = TagDialog("Title", "Str", self)
        if (tagdialog.exec_() == 0):
            return
        s:str = tagdialog.name_edit.text()
        if (s == ""):
            return 


if __name__ == "__main__":
    import re
    
    fp:str = sys.argv[1]
    
    srch = re.search("HOST: (?P<host>.*)\nPATH: (?P<path>.*)\nUSER: (?P<user>.*)\nPWD:  (?P<password>.*)\nDB:   (?P<database>.*)\nPORT: (?P<port>.*)\n",  open(fp).read())
    
    kwargs:dict = srch.groupdict()
    
    kwargs["get_warnings"] = True
    kwargs["raise_on_warnings"] = True
    kwargs["buffered"] = True
    
    if (kwargs["host"] == "localhost"):
        kwargs["unix_socket"] = kwargs["path"]
        del kwargs["host"]
        del kwargs["path"]
        del kwargs["port"]
    else:
        raise Exception("Not imnplemented yet, and to prove it this message has typos")
    
    cnx = mysql.connector.connect(**kwargs)
    cnx.autocommit = False
    cursor = cnx.cursor()
    
    app:QApplication = QApplication([])
    win:MainWindow = MainWindow()
    sys.exit(app.exec_())
