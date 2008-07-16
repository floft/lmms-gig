#ifndef SINGLE_SOURCE_COMPILE

/*
 * automation_track.cpp - automationTrack handles automation of objects without
 *                        a track
 *
 * Copyright (c) 2008 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 * Copyright (c) 2006-2008 Javier Serrano Polo <jasp00/at/users.sourceforge.net>
 * 
 * This file is part of Linux MultiMedia Studio - http://lmms.sourceforge.net
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 */


#include "automation_track.h"
#include "automation_pattern.h"
#include "embed.h"
#include "name_label.h"
#include "string_pair_drag.h"
#include "project_journal.h"
#include "track_container_view.h"


automationTrack::automationTrack( trackContainer * _tc, bool _hidden ) :
	track( _hidden ? HiddenAutomationTrack : AutomationTrack, _tc )
{
	setName( tr( "Automation track" ) );
}




automationTrack::~automationTrack()
{
}




bool automationTrack::play( const midiTime & _start, const fpp_t _frames,
				const f_cnt_t _frame_base, Sint16 _tco_num )
{
	if( isMuted() )
	{
		return( FALSE );
	}

	tcoVector tcos;
	if( _tco_num >= 0 )
	{
		trackContentObject * tco = getTCO( _tco_num );
		tcos.push_back( tco );
	}
	else
	{
		getTCOsInRange( tcos, _start, _start + static_cast<int>(
					_frames / engine::framesPerTick()) );
	}

	for( tcoVector::iterator it = tcos.begin(); it != tcos.end(); ++it )
	{
		automationPattern * p =
				dynamic_cast<automationPattern *>( *it );
		if( p == NULL || ( *it )->isMuted() )
		{
			continue;
		}
		midiTime cur_start = _start;
		if( _tco_num < 0 )
		{
			cur_start -= p->startPosition();
		}
		p->processMidiTime( cur_start );
	}
	return( FALSE );
}




trackView * automationTrack::createView( trackContainerView * _tcv )
{
	return( new automationTrackView( this, _tcv ) );
}




trackContentObject * automationTrack::createTCO( const midiTime & )
{
	return( new automationPattern( this ) );
}




void automationTrack::saveTrackSpecificSettings( QDomDocument & _doc,
							QDomElement & _this )
{
}




void automationTrack::loadTrackSpecificSettings( const QDomElement & _this )
{
}





automationTrackView::automationTrackView( automationTrack * _at,
						trackContainerView * _tcv ) :
	trackView( _at, _tcv )
{
	setFixedHeight( 32 );
	trackLabelButton * tlb = new trackLabelButton( this,
						getTrackSettingsWidget() );
	tlb->setPixmap( embed::getIconPixmap( "automation_track" ) );
	tlb->move( 3, 1 );
	tlb->show();
	setModel( _at );
}




automationTrackView::~automationTrackView()
{
}




void automationTrackView::dragEnterEvent( QDragEnterEvent * _dee )
{
	stringPairDrag::processDragEnterEvent( _dee, "automatable_model" );
}




void automationTrackView::dropEvent( QDropEvent * _de )
{
	QString type = stringPairDrag::decodeKey( _de );
	QString val = stringPairDrag::decodeValue( _de );
	if( type == "automatable_model" )
	{
		automatableModel * mod = dynamic_cast<automatableModel *>(
				engine::getProjectJournal()->
					getJournallingObject( val.toInt() ) );
		if( mod != NULL )
		{
			midiTime pos = midiTime( getTrackContainerView()->
							currentPosition() +
				( _de->pos().x() -
					getTrackContentWidget()->x() ) *
						midiTime::ticksPerTact() /
		static_cast<int>( getTrackContainerView()->pixelsPerTact() ) )
				.toNearestTact();

			if( pos.getTicks() < 0 )
			{
				pos.setTicks( 0 );
			}

			trackContentObject * tco = getTrack()->createTCO( pos );
			automationPattern * pat =
				dynamic_cast<automationPattern *>( tco );
			pat->addObject( mod );
			pat->movePosition( pos );
		}
	}

	update();
}



#endif
