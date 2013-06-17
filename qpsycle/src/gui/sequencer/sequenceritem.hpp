/***************************************************************************
*   Copyright (C) 2007 Psycledelics Community   *
*   psycle.sourceforge.net   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#ifndef SEQUENCERITEM_H
#define SEQUENCERITEM_H

namespace psycle { namespace core {
	class SequenceEntry;
	class Pattern;
}}

#include <QGraphicsItem>
#include <QObject>
#include <QAction>

class QKeyEvent;
class QGraphicsSceneMouseEvent;

namespace qpsycle {

	class SequencerView;
	class SequencerLine;
	class SequencerDraw;

	class SequencerItem : public QObject, public QGraphicsItem
	{
		Q_OBJECT

		public:
		SequencerItem( SequencerDraw *seqDraw );
		~SequencerItem();

		QRectF boundingRect() const;
		void paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );

		void setSequenceEntry( psycle::core::SequenceEntry *sequenceEntry );
		psycle::core::SequenceEntry *sequenceEntry() const; 

		// Enable the use of qgraphicsitem_cast with this item.
		enum { Type = UserType + 5 };
		int type() const { return Type; }

	protected: 
		void mouseMoveEvent( QGraphicsSceneMouseEvent *event );
		void mousePressEvent( QGraphicsSceneMouseEvent *event );
		void contextMenuEvent( QGraphicsSceneContextMenuEvent *event );
		void keyPressEvent( QKeyEvent *event );
		QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    private Q_SLOTS :
//		void onReplaceWithCloneActionTriggered();
//		void onLoopEntryActionTriggered();
		void onDeleteEntryActionTriggered();

    Q_SIGNALS:
		void clicked(SequencerItem*);
		void deleteRequest( SequencerItem* );
		void moved( SequencerItem*, QPointF diff );
		void changedLine( SequencerItem*, int );
		void newPatternCreated( psycle::core::Pattern * );

	private:
		void setNewPattern( psycle::core::Pattern *newPattern );

		psycle::core::SequenceEntry *sequenceEntry_;

		QAction *replaceWithCloneAction_;
		QAction *deleteEntryAction_;
		QAction *loopEntryAction_;
		const QColor QColorFromLongColor( long longCol ) const;

		SequencerDraw *seqDraw_;
	};

} // namespace qpsycle

#endif
