/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
** 
** Trolltech ASA (c) 2007
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTREEVIEW_P_H
#define QTREEVIEW_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "private/qabstractitemview_p.h"

#ifndef QT_NO_TREEVIEW

struct QTreeViewItem
{
    QTreeViewItem() : expanded(false), spanning(false), total(0), level(0), height(0) {}
    QModelIndex index; // we remove items whenever the indexes are invalidated
    uint expanded : 1;
    uint spanning : 1;
    uint total : 30; // total number of children visible
    uint level : 16; // indentation
    int height : 16; // row height
};

class QTreeViewPrivate : public QAbstractItemViewPrivate
{
    Q_DECLARE_PUBLIC(QTreeView)
public:

    QTreeViewPrivate()
        : QAbstractItemViewPrivate(),
          header(0), indent(20), lastViewedItem(0), defaultItemHeight(-1),
          uniformRowHeights(false), rootDecoration(true),
          itemsExpandable(true), sortingEnabled(false),
          allColumnsShowFocus(false),
          animationsEnabled(false), columnResizeTimerID(0),
          autoExpandDelay(-1), hoverBranch(-1) {}

    ~QTreeViewPrivate() {}
    void initialize();

    struct AnimatedOperation
    {
        enum Type { Expand, Collapse };
        int item;
        int top;
        int duration;
        Type type;
        QPixmap before;
        QPixmap after;
    };

    void expand(int item, bool emitSignal);
    void collapse(int item, bool emitSignal);

    void prepareAnimatedOperation(int item, AnimatedOperation::Type type);
    void beginAnimatedOperation();
    void _q_endAnimatedOperation();
    void drawAnimatedOperation(QPainter *painter) const;
    QPixmap renderTreeToPixmap(const QRect &rect) const;

    inline QRect animationRect() const
        { return QRect(0, animatedOperation.top, viewport->width(),
                       viewport->height() - animatedOperation.top); }

    void _q_currentChanged(const QModelIndex&, const QModelIndex&);
    void _q_columnsAboutToBeRemoved(const QModelIndex &, int, int);
    void _q_columnsRemoved(const QModelIndex &, int, int);
    void _q_modelAboutToBeReset();

    void layout(int item);

    int pageUp(int item) const;
    int pageDown(int item) const;

    inline int above(int item) const
        { return (--item < 0 ? 0 : item); }
    inline int below(int item) const
        { return (++item >= viewItems.count() ? viewItems.count() - 1 : item); }
    inline void invalidateHeightCache(int item) const
        { viewItems[item].height = 0; }

    int itemHeight(int item) const;
    int indentationForItem(int item) const;
    int coordinateForItem(int item) const;
    int itemAtCoordinate(int coordinate) const;

    int viewIndex(const QModelIndex &index) const;
    QModelIndex modelIndex(int i) const;

    int firstVisibleItem(int *offset = 0) const;
    int columnAt(int x) const;
    bool hasVisibleChildren( const QModelIndex& parent) const;

    void relayout(const QModelIndex &parent);
    void reexpandChildren(const QModelIndex &parent);

    void updateScrollBars();

    int itemDecorationAt(const QPoint &pos) const;

    void select(int start, int stop, QItemSelectionModel::SelectionFlags command);

    QPair<int,int> startAndEndColumns(const QRect &rect) const;

    void updateChildCount(const int parentItem, const int delta);
    void rowsRemoved(const QModelIndex &parent,
                     int start, int end, bool before);

    QHeaderView *header;
    int indent;

    mutable QVector<QTreeViewItem> viewItems;
    mutable int lastViewedItem;
    int defaultItemHeight; // this is just a number; contentsHeight() / numItems
    bool uniformRowHeights; // used when all rows have the same height
    bool rootDecoration;
    bool itemsExpandable;
    bool sortingEnabled;
    bool allColumnsShowFocus;

    // used for drawing
    mutable QPair<int,int> leftAndRight;
    mutable int current;
    mutable bool spanning;

    // used when expanding and collapsing items
    QVector<QPersistentModelIndex> expandedIndexes;
    QStack<bool> expandParent;
    AnimatedOperation animatedOperation;
    bool animationsEnabled;

    // used when hiding and showing items
    QVector<QPersistentModelIndex> hiddenIndexes;

    // used for spanning rows
    QVector<QPersistentModelIndex> spanningIndexes;

    // used for updating resized columns
    int columnResizeTimerID;
    QList<int> columnsToUpdate;

    // used for the automatic opening of nodes during DND
    int autoExpandDelay;
    QBasicTimer openTimer;

    // used for drawing hilighted expand/collapse indicators
    int hoverBranch;

};

#endif // QT_NO_TREEVIEW

#endif // QTREEVIEW_P_H
