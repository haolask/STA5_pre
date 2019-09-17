/***********************************************************************************/
/*!
*
*  \file      list.c
*
*  \brief     <i><b> STAudioLib internal list implementation </b></i>
*
*  \details   Implement a tailored 'list' for STAudioLib
*
*
*  \author    Quarre Christophe
*
*  \author    (original version) Quarre Christophe
*
*  \version
*
*  \date      2013/04/22
*
*  \bug       see Readme.txt
*
*  \warning
*
*  This file is part of STAudioLib and is dual licensed,
*  ST Proprietary License or GNU General Public License version 2.
*
********************************************************************************
*
* Copyright (c) 2014 STMicroelectronics - All Rights Reserved
* Reproduction and Communication of this document is strictly prohibited
* unless specifically authorized in writing by STMicroelectronics.
* FOR MORE INFORMATION PLEASE READ CAREFULLY THE LICENSE AGREEMENT LOCATED
* IN THE ROOT DIRECTORY OF THIS SOFTWARE PACKAGE.
*
********************************************************************************
*
* ALTERNATIVELY, this software may be distributed under the terms of the
* GNU General Public License ("GPL") version 2, in which case the following
* provisions apply instead of the ones mentioned above :
*
********************************************************************************
*
* STAudioLib is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* STAudioLib is distributed in the hope that it will be
* useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* Please refer to <http://www.gnu.org/licenses/>.
*
*/
/***********************************************************************************/

#include "internal.h"	//for STA_MALLOC
#include "list.h"

/*
typedef struct sListElmt {
	void*				m_elmt;
	struct sListElmt*	m_next;
} tListElmt;


typedef struct sList {
	tListElmt*			m_first;
	tListElmt*			m_last;
	unsigned int		m_size;
} tList;
*/

//----------------------------------------------------------------------------
/*
void LIST_PushBack( tList* list, tListElmt* elmt )
{
	//push_back to list
	if ( !list->m_first )
		list->m_first = list->m_last = elmt;		//first
	else {
		list->m_last->m_next = elmt;				//add
		list->m_last = elmt;
	}
	list->m_size++;
}
*/
//----------------------------------------------------------------------------
void* LIST_AddNew( tList* list, unsigned int sizeofElmt )
{
	void* ret = 0;
	tListElmt* lelmt;

	if (!list) {goto _ret;}

	//create
	lelmt = (tListElmt*) STA_MALLOC(sizeof(tListElmt) + sizeofElmt);
	if (!lelmt) {goto _ret;} //out of memory

	lelmt->m_elmt 		= (void*)((u32)lelmt + sizeof(tListElmt));
	lelmt->m_sizeofElmt = sizeofElmt;
	lelmt->m_next 		= 0;
	memset(lelmt->m_elmt, 0, sizeofElmt);

	//push_back to list
	if ( !list->m_first ) {
		list->m_first = list->m_last = lelmt;		//first
	}
	else {
		list->m_last->m_next = lelmt;				//add
		list->m_last = lelmt;
	}
	list->m_size++;

	ret = lelmt->m_elmt;

_ret: return ret;
}
//----------------------------------------------------------------------------
/*
void LIST_Remove( tList* list, tListElmt* elmt )
{
	//is it the first element?
	if (list->m_first == elmt) {
		list->m_first = elmt->m_next;		//this was the first object

		//update m_last if it was also this one
		if (list->m_last == elmt)
			list->m_last = 0;
	}
	else {
		//find the element
		tListElmt* prev = list->m_first;
		while ( prev && prev->m_next != elmt )
			prev = prev->m_next;
		if ( !prev ) return; //can't find the element to remove
		prev->m_next = elmt->m_next;			//remove

		//update m_last if it was this one
		if (list->m_last == elmt)
			list->m_last = prev;
	}
	list->m_size--;
}
*/
//----------------------------------------------------------------------------
void LIST_DelElmt( tList* list, const void* elmt )
{
	tListElmt *lelmt = 0, *prev = 0;

	if (!list || !list->m_first) {
		goto _ret; //empty list
	}

	//is it the first element?
	if (elmt == list->m_first->m_elmt) {
		lelmt = list->m_first;
		list->m_first = lelmt->m_next;		//remove the first object

		//update m_last if it was also the first
		if (list->m_last == lelmt) {
			list->m_last = 0;
		}
	}
	else {

		if (list->m_size == 1) {
			goto _ret; //can't find the element to remove
		}

		//find the previous element, and thus the tListElmt *lelmt to remove
		prev = list->m_first;
		while (prev) {
			if (prev->m_next && prev->m_next->m_elmt == elmt) {break;}
			prev = prev->m_next;
		}
		if ( !prev ) {goto _ret;} //can't find the element to remove
		lelmt = prev->m_next;

		//disconnect the element
		prev->m_next = prev->m_next->m_next;

		//update m_last if it was this element
		if (list->m_last == lelmt) {
			list->m_last = prev;
		}
	}

	//remove and free up
	list->m_size--;
	memset(lelmt, 0, sizeof(tListElmt) + lelmt->m_sizeofElmt);
	STA_FREE(lelmt);

_ret: return;
}
//----------------------------------------------------------------------------
void LIST_DelElmts( tList* list, tListElmt* start, const tListElmt* end )
{
	tListElmt *lelmt, *prev = 0, *next = 0;

	if (!list || !list->m_first) {
		goto _ret; //empty list
	}

	//find the previous element
	if (start != list->m_first) {
		prev = list->m_first;
		while ( prev && prev->m_next != start ) {
			prev = prev->m_next;
		}
		if ( !prev ) {
			goto _ret; //can't find the first element to remove
		}
	}

	//delete the range
	lelmt = start;
	while (lelmt) {
		next = lelmt->m_next; //save next ptr before delete
		memset(lelmt, 0, sizeof(tListElmt) + lelmt->m_sizeofElmt);
		list->m_size--;
		if (lelmt == end) {		//lint !e449 misra2004 1.2 "pointer previously deallocated" (deallocated ptr set to NULL)
			STA_FREE(lelmt);
			break;
		}
		else
		{
			STA_FREE(lelmt);
		}
		lelmt = next;
	}
	//reconnect
	if (prev) {
		prev->m_next = next;
	}

	//update m_first
	if (list->m_first == start) {
		list->m_first = next; //was the last next
	}

	//update m_last
	if (list->m_last == lelmt || !lelmt) {
		list->m_last = prev;
	}

_ret: return;
}
//----------------------------------------------------------------------------
void LIST_DelAll( tList* list )
{
	tListElmt* lelmt;

	if (!list) {goto _ret;}

	lelmt = list->m_first;
	while (lelmt) {
		tListElmt* next = lelmt->m_next; //save next ptr before delete
		memset(lelmt, 0, sizeof(tListElmt) + lelmt->m_sizeofElmt);
		STA_FREE(lelmt);
		lelmt = next;
	}
	list->m_first = list->m_last = 0;
	list->m_size = 0;

_ret: return;
}

