//!
//!  \file 		doxytemplate.c
//!  \brief 	<i><b> 'brief description' </b></i>
//!  \details   'This files presents example of Doxygen comments.'
//!	 \details	'It should not be compiled with the ETAL project!.'
//!  $Author$
//!  \author 	(original version) 'author of first commit'
//!  $Revision$
//!  $Date$
//!  \see		'optional'
//!

/*
 * Lines starting with NOTE are for explanation only and should be removed from the implementation
 *
 * NOTE: the filename specified after \file must match exactly this file's name,
 * othewise doxygen will ignore the file completely
 *
 * NOTE: the tag \see is a way to include a cross-reference, Doxygen will search 
 * the annotated sources for the corresponding name and insert an hyperlink
 *
 * NOTE: all entries enclosed in '' denote stuff you should customize
 *
 */

#include "osal.h"

/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/
/*
 * NOTE: \def introduces a macro definition block: it must be specified
 * only for the first macro in the block, for the others it is sufficient
 * to insert the description (introduced by the Doxygen special comment markings "/*!")
 */

/*!
 * \def		MACRO1
 * 			'description of MACRO1, possibly spanning multiple lines'
 */
#define MACRO1      (0x01)
/*!
 * 			'description of MACRO2; it is not necessary to repeat def'
 * \see		MACRO1 'optional'
 */
#define MACRO2      (0x02)
/*!
 * 			'description of MACRO3; it is not necessary to repeat def'
 * \see		'optional'
 */
#define MACRO3(_x_) (((_x_) & MACRO1) == MACRO1)
/*!
 * 			'description of MACRO4; it is not necessary to repeat def'
 * \see		'optional'
 */
#define MACRO4(_x_)    (((_x_) & MACRO2) == MACRO2)

/*****************************************************************
| Local types
|----------------------------------------------------------------*/

/*!
 * \enum	myEnum
 * 			'enum description'
 */
typedef enum
{
	val1,
	val2
} myEnum;

/*!
 * \struct	myStruct
 * 			'struct description'
 */
typedef struct
{
	/*! description of val3
	 *  details
	 *  details
	 */
	int val3;
	/*! description of val4
	 *  details
	 *  details
	 */
	int val4;
} myStruct;

/*****************************************************************
| prototypes
|----------------------------------------------------------------*/

/*****************************************************************
| variable definition
|----------------------------------------------------------------*/
/*
 * NOTE: \var introduces a variable definition block; the same rules as the \def hold
 */
/*!
 * \var		pippo
 * 			'description of pippo'
 */
int pippo;
/*!
 * 			'description of pluto, it is not necessary to repeat var'
 */
int pluto;

/***************************
 *
 * func1 (NOTE: this header is not required by Doxygen and has no influence on the output)
 *
 **************************/
/*!
 * \brief		'Brief description of the function (one line at most)'
 * \details		'Long description of the function, multiple lines if needed'
 *              'Doxygen merges this and the previous line in a single paragraph with no newlines in between'
 * \details		'This line, instead, is separated by a newline from the previous paragraph. The same could
 * 				be obtained with an HTML <BR> tag but using the Doxygen tag seems more readable'
 * 				
 * 				'the Doxygen Markdown syntax should allow introducing a new paragraph simply by
 * 				leaving an empty line above it; in my experience sometimes it works, sometimes
 * 				it doesn't. Repeating the Doxygen tag works always (but it is not usable for
 * 				the \param tag, unfortunatly.'
 * \remark		'Optional, if present this paragraph is introduced by a 'Remark' heading'
 * \param[in]	par1 - 'description of par1; this is an input to the function'
 * 				       'Continues the description of par1, this is merged with the previous line in a single paragraph'
 * 				       '[in],[out],[in,out] are optional'
 * 				       'The first string after param MUST be a valid parameter name or Doxygen will complain'
 * \param[out]	par2 - 'this is an output of the function'
 * 				       'Doxygen checks if all parameters of the function have a description'
 * \param[in,out] par3 - 'you guess this'
 * 				       'If a function takes no argument, don't put any param entry or Doxygen will complain'
 * \return		'Description of one possible return value'
 * 				'The description may continue on this line and is merged with the previous line'
 * 				'Unlike the param tag, you can put whatever value here, Doxygen does not validate it:'
 * 				'For example for a function returning void you can put 'void' here'
 * 				'The current documentaion convention is not to put the line if the function returns void.'
 * \return		'Description of another return value; this is placed in a separate paragraph'
 * \see			'optional: one or more references; Doxygen searches the name(s) in the documented sources and creates hyperlinks'
 * \callgraph
 * \callergraph
 * \todo		'All todo tags are collected in a dedicated page, with reference to this function'
 */
/*
 * CALLGRAPH, CALLERGRAPH: the tags 'callgraph' and 'callergraph' instruct Doxygen to create a graphical
 * representation of the callers and callee of this function; the tags require no parameter.
 *
 * HYPERLINKS: Doxygen automaticaly inserts hyperlinks in the documentation, when the text respects the
 * following conventions:
 * A link to the variable #var.
 * A link to the global typedef ::B.
 * A link to the global enumeration type #GlobEnum.
 * A link to the define ABS(x).
 * A link to a variable \link #var using another text\endlink as a link.
 * And last but not least a link to a file: autolink.cpp.
 * Inside a see also ("\sa" or "\see") section any word is checked
 * For details: https://www.stack.nl/~dimitri/doxygen/manual/autolink.html
 *
 * FUNCTION PARAMETERS: to refer to a function parameter in the function description use the "*" doxygen
 * tag to make it be rendered in italics: for example given *receiverHandle*, the word
 * receiverHandle will be rendered in italics. This is equivalent to using the "\a" tag
 * but seems more readable in the code. For bold you can use **.
 *
 * BULLETED LISTS: Doxygen recognizes the Markdown syntax and creates e.g. bulleted lists.
 * For example:
 *  - item1
 *  - item2
 * Will be rendered as two bullets items. Just remember to put at least a space before the '-',
 * tabs do not work.
 *
 * PARAGRAPH HEADINGS: may be rendered with Markdown syntax:
 * heading
 * -------
 * Will be rendered as paragraph heading. See function ETAL_cmdSeekStart_CMOST in etalcmd_cmost.cfor
 * a less trivial example.
 *
 * RESERVED WORDS: don't use <> in doxygen comments, it will complain that the HTML tag is not
 * recognized; don't use e.g. 'param' in doxygen comments, it will confuse it
 *
 */
int func1(int par1, int *par2, int *par3)
{
	/* we don't put doxygen comments in the function body (by default they are ignored) */
	return 0;
}

/*
 * NOTE: the function template is repeated below without comments for easy copy paste
 * The template includes all possible Doxygen tags; remove the unused ones 
 * otherwise Doxygen may issue a warning when creating the documentation
 */

/***************************
 *
 * 
 *
 **************************/
/*!
 * \brief		
 * \details		
 * \remark		
 * \param[in]	
 * \param[out]	
 * \param[in,out] 
 * \return		
 * \see			
 * \callgraph
 * \callergraph
 * \todo		
 */

