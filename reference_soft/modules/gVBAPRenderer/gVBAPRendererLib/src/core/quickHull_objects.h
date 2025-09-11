/***********************************************************************************
 
 This software module was originally developed by 
 
 Fraunhofer IIS
 
 in the course of development of the ISO/IEC 23008-3 for reference purposes and its 
 performance may not have been optimized. This software module is an implementation
 of one or more tools as specified by the ISO/IEC 23008-3 standard. ISO/IEC gives 
 you a royalty-free, worldwide, non-exclusive, copyright license to copy, distribute, 
 and make derivative works of this software module or modifications  thereof for use
 in implementations or products claiming conformance to the ISO/IEC 23008-3 standard 
 and which satisfy any specified conformance criteria. Those intending to use this 
 software module in products are advised that its use may infringe existing patents. 
 ISO/IEC have no liability for use of this software module or modifications thereof. 
 Copyright is not released for products that do not conform to the ISO/IEC 23008-3 
 standard.
 
 Fraunhofer IIS retains full right to modify and use the code for its own purpose,
 assign or donate the code to a third party and to inhibit third parties from using 
 the code for products that do not conform to MPEG-related ITU Recommendations and/or 
 ISO/IEC International Standards.
 
 This copyright notice must be included in all copies or derivative works. 
 
 Copyright (c) ISO/IEC 2013.
 
 ***********************************************************************************/

/*!
\mainpage QuickHull C-library for 3D audio speaker setups.
 */

#ifndef QuickHullVBAP_QuickHull_objects_h
#define QuickHullVBAP_QuickHull_objects_h

#include "quickHull.h"

#define false 0
#define true 1


/***** objects ******/


/*! \brief Position Index */
typedef struct posInd {
    int pos;    /*!< position in original list */
    int index; /*!< sorting index */
} posInd;

/*! \brief Float Index */
typedef struct floatInd {
    float f;    /*!< value */
    int index; /*!< sorting index */
} floatInd;

/*!  \brief Edge struct */
/*!
 An edge defined by the index numbers of the two adjacent vertices
 */
typedef struct edge {
    int index[2]; /*! two adjacent vertices */
} edge;

/***** Object lists ******/
/*!  \brief List of \ref floatInd */
/*!
 Dynamic list of floats with sorting index. The allocated memory is given by \a limit. A set of access functions is implemented via function pointers.
 */
typedef struct floatList {
    int size; /*!< Current lenghts of the list.  */
    int limit; /*!< Memory limit of the list.  */
    floatInd* arr; /*!< Items of the list.  */
    void (*add)(struct floatList* v,float n); /*!< Add float with generic index to the list.  */
    float (*get)(struct floatList* v,int index); /*!< Get float from the list.  */
    void (*set)(struct floatList* v,int index, float n); /*!< Set float value in the list.  */
    void (*addFloatInd)(struct floatList* v, floatInd n);  /*!< Add float with generic index to the list.  */
    floatInd (*getFloatInd)(struct floatList* v, int index);  /*!< Get item from the list.  */
    void (*setFloatInd)(struct floatList* v, int index, floatInd n); /*!< Set item in the list.  */

} floatList;

static void floatList_add(floatList* v, float n);
static float floatList_get(floatList* v, int index);
static void floatList_set(floatList* v, int index, float n);
static void floatList_addFloatInd(floatList* v, floatInd n);
static floatInd floatList_getFloatInd(floatList* v, int index);
static void floatList_setFloatInd(floatList* v, int index, floatInd n);
static floatList* floatList_new(int l);
static void floatList_destroy(floatList* v);




/*! \brief List of \ref edge */
/*!
 Dynamic list of edges. The allocated memory is given by \a limit. A set of access functions is implemented via function pointers.
 */
typedef struct edgeList {
    int size; /*!< Current lenghts of the list.  */
    int limit; /*!< Memory limit of the list.  */
    edge* arr; /*!< Items of the list.  */
    void (*add)(struct edgeList* v,edge n); /*!< Add item to the list.  */
    edge (*get)(struct edgeList* v,int index); /*!< Get item from the list.  */
    void (*set)(struct edgeList* v,int index, edge n); /*!< Set item in the list.  */
} edgeList;

static void edgeList_add(edgeList* v, edge n);
static edge edgeList_get(edgeList* v, int index);
static void edgeList_set(edgeList* v, int index, edge n);
static edgeList* edgeList_new(int l);
static void edgeList_destroy(edgeList* v);



/*! \brief List of \ref posInd */
/*!
 Dynamic list of posInd. The allocated memory is given by \a limit. A set of access functions is implemented via function pointers.
 */
typedef struct posIndList {
    int size; /*!< Current lenghts of the list.  */
    int limit; /*!< Memory limit of the list.  */
    posInd* arr; /*!< Items of the list.  */
    void (*add)(struct posIndList* v,posInd n); /*!< Add item to the list.  */
    posInd (*get)(struct posIndList* v,int index); /*!< Get item from the list.  */
    void (*set)(struct posIndList* v,int index, posInd n); /*!< Set item in the list.  */
    void (*clear)(struct posIndList* v); /*!< Clear all items in the list.  */
    int (*ismember)(struct posIndList* v, int index); /*!< Check whether index is member of list  */
} posIndList;

static void posIndList_add(posIndList* v, posInd n);
static posInd posIndList_get(posIndList* v, int index);
static void posIndList_set(posIndList* v, int index, posInd n);
static void posIndList_clear(posIndList* v);
static int posIndList_ismember(posIndList* v, int index);
static posIndList* posIndList_new(int l);
static void posIndList_destroy(posIndList* v);


/* vertexList functions */
static void vertexList_add(vertexList* v, vertex n);
static vertex vertexList_get(vertexList* v, int index);
static void vertexList_set(vertexList* v, int index, vertex n);
static void vertexList_clear(vertexList* v);
static void vertexList_copy(vertexList* from, vertexList* to);


/* triangleList functions */
static void triangleList_add(triangleList* v, triangle n);
static triangle triangleList_get(triangleList* v, int index) ;
static void triangleList_set(triangleList* v, int index, triangle n);
static void triangleList_clear(triangleList* v);
static void triangleList_copy(triangleList* from, triangleList* to);



/* matrix functions */
static float matrix_get( matrix* v, int ROW, int COL);
static void matrix_set( matrix* v, int ROW, int COL, float DATA);
static void matrix_subMatrix( matrix* v, int numROW, int* ROWs, int numCOL, int* COLs, matrix* sub);
static void matrix_subLeadMatrix( matrix* v, int numROW, int numCOL, matrix* sub);
static void matrix_subTailMatrix( matrix* v, int startROW, int startCOL, struct matrix* sub);
static void matrix_clear(matrix* v);
static void matrix_reset(matrix* v, int ROW, int COL);
static void matrix_copy(matrix* from, matrix* to);

#endif
