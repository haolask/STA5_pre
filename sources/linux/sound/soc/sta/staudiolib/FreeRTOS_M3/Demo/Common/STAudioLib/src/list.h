/***********************************************************************************/
/*!
*
*  \file      list.h
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

#ifndef LIST_H_
#define LIST_H_


typedef struct sListElmt {
	void*				m_elmt;
	unsigned int		m_sizeofElmt;
	struct sListElmt*	m_next;
} tListElmt;


typedef struct sList {
	tListElmt*			m_first;
	tListElmt*			m_last;
	unsigned int		m_size;
} tList;

/*
typedef struct sListIterator {
	tList*				list;
	tListElmt*			cur;
} tListIterator;
*/

void* LIST_AddNew( tList* list, unsigned int sizeofElmt );
void  LIST_DelElmt( tList* list, const void* elmt );
void  LIST_DelElmts( tList* list, tListElmt* start, const tListElmt* end );
void  LIST_DelAll( tList* list );

//#define LIST_FIRST(it, type)		((it).cur = (it).list->m_first), (type)(it).cur->m_elmt
//#define LIST_NEXT(it, type)		((it).cur = (it).cur->m_next),   (type)(it).cur->m_elmt

//#define LIST_FOR 	for(e = m_modules->m_first, mod = MOD(e); e; e = e->m_next, mod = MOD(e) )


//lint -estring(960, LIST_ITERATOR)		misra2004 19.4  allow this macro definition used for Code readibility
//lint -estring(9022, LIST_ITERATOR)	misra2004 19.10 allow this macro definition used for Code readibility
#define LIST_ITERATOR(type, it)		tListElmt* _##it; type it = 0


//lint -estring(960, LIST_FOR_EACH, LIST_FOR_RANGE)		misra2004 19.12: allow macro definition using #/## (Code readibility)
//lint -emacro(960, LIST_FOR_EACH, LIST_FOR_RANGE)		misra2004 12.10: allow macro definition using comma operator in the for() (Code readibility)
//lint -estring(9022, LIST_FOR_EACH, LIST_FOR_RANGE)	misra2004 19.10: allow macro definition having unparenthesized param (Code readibility)
#define LIST_FOR_EACH(it, list, type)	\
	for(_##it = (list)->m_first, (_##it) ? it = (type)_##it->m_elmt : 0; _##it; _##it = _##it->m_next, (_##it) ? it = (type)_##it->m_elmt : 0 )

#define LIST_FOR_RANGE(it, first, lastExcl, type)	\
	for(_##it = (first), (_##it)? it = (type)_##it->m_elmt : 0; (_##it) && (_##it) != (lastExcl); _##it = _##it->m_next, (_##it) ? it = (type)_##it->m_elmt : 0 )


#endif /* LIST_H_ */
