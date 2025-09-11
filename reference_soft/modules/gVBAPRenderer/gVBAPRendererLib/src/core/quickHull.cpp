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
 
 Copyright (c) ISO/IEC 2014.
 
 ***********************************************************************************/

#include "quickHull_functions.h"
#include "quickHull_objects.h"
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>


#ifndef MIN
#define MIN(X, Y)  ((X) < (Y) ? (X) : (Y))
#endif

#ifndef MAX
#define MAX(X, Y)  ((X) > (Y) ? (X) : (Y))
#endif

#ifndef ROUND
#define ROUND(X)  ((X) < 0 ? ceil((X)-0.5) : floor((X)+0.5))
#endif

#ifndef SIGN
#define SIGN(X)  ((0 < (X)) - ((X) < 0))
#endif



const vertex vertex_DEFAULT = {
    {0.0, 0.0, 0.0},
    -1,
    0.0,
    0.0
};

/***** float list ******/
/*!
 \brief Creat a new list with initial lengths of \a l. Functions related to the struct are set dynamically via function pointers to mimic OOP design.
 \param l List limit
 */
floatList* floatList_new(int l) {
    floatList* res = (floatList*) malloc(sizeof(floatList));
    res->limit = l;
    res->size = 0;
    res->arr = (floatInd*) malloc(sizeof(floatInd) * res->limit);
    res->add = floatList_add;
    res->get = floatList_get;
    res->set = floatList_set;
    res->addFloatInd = floatList_addFloatInd;
    res->getFloatInd = floatList_getFloatInd;
    res->setFloatInd = floatList_setFloatInd;
    return res;
}
/*!
 \brief Destroy list. Free memory.
 \param v List
 */
void floatList_destroy(floatList* v) {
    if(v!=NULL && v->arr!=NULL){
        free(v->arr);
        v->arr=NULL;
        free(v);
        v=NULL;
    }
    else{
        assert(false);
    }
}

/*!
 \brief Add a float at the end of the list. If the limit is too low, new memory is allocated.
 \param v List
 \param n Float value
 */
void floatList_add(floatList* v, float n) {
    if(v->size == v->limit) {
        v->limit = v->limit + 1;
        v->arr = (floatInd*) realloc(v->arr, v->limit * sizeof(floatInd));
    }
    v->arr[v->size].f = n;
    v->arr[v->size].index = v->size;
    ++v->size;
}
/*!
 \brief Add an item at the end of the list. If the limit is too low, new memory is allocated.
 \param v List
 \param n Item
 */
void floatList_addFloatInd(floatList* v, floatInd n) {
    if(v->size == v->limit) {
        v->limit = v->limit + 1;
        v->arr = (floatInd*) realloc(v->arr, v->limit * sizeof(floatInd));
    }
    v->arr[v->size] = n;
    ++v->size;
}
/*!
 \brief Get a float value at the \a index.
 \param v List
 \param index Index
 */
float floatList_get(floatList* v, int index) {
    if(index >= 0 && index < v->size)
        return v->arr[index].f;
    
    assert(false);
    return 0.0;
}
/*!
 \brief Get an item at the \a index.
 \param v List
 \param index Index
 */
floatInd floatList_getFloatInd(floatList* v, int index) {
    if(index >= 0 && index < v->size)
        return v->arr[index];
    
    assert(false);
    return v->arr[-1];
}

/*!
 \brief Set a float value at the index \a index.
 \param v List
 \param index Index
 \param n Float value
 */
void floatList_set(floatList* v, int index, float n) {
    if(index >= 0 && index < v->size)
        v->arr[index].f = n; return;
    
    assert(false);
}
/*!
 \brief Set an item at the index \a index.
 \param v List
 \param index Index
 \param n Item
 */
void floatList_setFloatInd(floatList* v, int index, floatInd n) {
    if(index >= 0 && index < v->size)
        v->arr[index] = n; return;
    
    assert(false);
}



/***** vertex list ******/
/*!
 \brief Creat a new list with initial lengths of \a l. Functions related to the struct are set dynamically via function pointers to mimic OOP design.
 \param l List limit
 */
vertexList* qh_vertexList_new(int l) {
    vertexList* res = (vertexList*) malloc(sizeof(vertexList));
    res->limit = l;
    res->size = 0;
    res->arr = (vertex*) malloc(sizeof(vertex) * res->limit);
    res->add = vertexList_add;
    res->get = vertexList_get;
    res->set = vertexList_set;
    res->clear = vertexList_clear;
    res->copy = vertexList_copy;
    return res;
}
/*!
 \brief Destroy list. Free memory.
 \param v List
 */
void qh_vertexList_destroy(vertexList* v) {
    if(v!=NULL && v->arr!=NULL){
        free(v->arr);
        v->arr=NULL;
        free(v);
        v=NULL;
    }
    else{
        assert(false);
    }
}

/*!
 \brief Add an item at the end of the list. If the limit is too low, new memory is allocated.
 \param v List
 \param n item
 */
void vertexList_add(vertexList* v, vertex n) {
    if(v->size == v->limit) {
        v->limit = v->limit + 1;
        v->arr = (vertex*) realloc(v->arr, v->limit * sizeof(vertex));
    }
    v->arr[v->size] = n;
    ++v->size;
}
/*!
 \brief Get an item at the \a index.
 \param v List
 \param index Index
 */
vertex vertexList_get(vertexList* v, int index) {
    if(index >= 0 && index < v->size)
        return v->arr[index];
    
    assert(false);
    return v->arr[-1];
}
/*!
 \brief Set an item at the index \a index.
 \param v List
 \param index Index
 \param n Item
 */
void vertexList_set(vertexList* v, int index, vertex n) {
    if(index >= 0 && index < v->size)
        v->arr[index] = n; return;
    
    assert(false);
}

/*!
 \brief Clear list. Free memory.
 \param v List
 */
void vertexList_clear(vertexList* v) {
    v->limit = 0;
    v->size = 0;
    v->arr = (vertex*) realloc(v->arr, v->limit * sizeof(vertex));
}

/*!
 \brief Copy all items to a new list.
 \param from Original list
 \param to Destination list
 */
void vertexList_copy(vertexList* from, vertexList* to) {
    to -> clear(to);
    int it;
    for(it = 0; it < from->size; it++){
        to->add(to, from->get(from,it));
    }
}


/***** edge list ******/
/*!
 \brief Creat a new list with initial lengths of \a l. Functions related to the struct are set dynamically via function pointers to mimic OOP design.
 \param l List limit
 */
edgeList* edgeList_new(int l) {
    edgeList* res = (edgeList*) malloc(sizeof(edgeList));
    res->limit = l;
    res->size = 0;
    res->arr = (edge*) malloc(sizeof(edge) * res->limit);
    res->add = edgeList_add;
    res->get = edgeList_get;
    res->set = edgeList_set;
    return res;
}
/*!
 \brief Destroy list. Free memory.
 \param v List
 */
void edgeList_destroy(edgeList* v) {
    if(v!=NULL && v->arr!=NULL){
        free(v->arr);
        v->arr=NULL;
        free(v);
        v=NULL;
    }
    else{
        assert(false);
    }
}

/*!
 \brief Add an item at the end of the list. If the limit is too low, new memory is allocated.
 \param v List
 \param n item
 */
void edgeList_add(edgeList* v, edge n) {
    if(v->size == v->limit) {
        v->limit = v->limit + 1;
        v->arr = (edge*) realloc(v->arr, v->limit * sizeof(edge));
    }
    v->arr[v->size] = n;
    ++v->size;
}
/*!
 \brief Get an item at the \a index.
 \param v List
 \param index Index
 */
edge edgeList_get(edgeList* v, int index) {
    if(index >= 0 && index < v->size)
        return v->arr[index];
    
    assert(false);
    return v->arr[-1];
}
/*!
 \brief Set an item at the index \a index.
 \param v List
 \param index Index
 \param n Item
 */
void edgeList_set(edgeList* v, int index, edge n) {
    if(index >= 0 && index < v->size)
        v->arr[index] = n; return;
    
    assert(false);
}

/***** triangle list ******/
/*!
 \brief Creat a new list with initial lengths of \a l. Functions related to the struct are set dynamically via function pointers to mimic OOP design.
 \param l List limit
 */
triangleList* qh_triangleList_new(int l) {
    triangleList* res = (triangleList*) malloc(sizeof(triangleList));
    res->limit = l;
    res->size = 0;
    res->arr = (triangle*) malloc(sizeof(triangle) * res->limit);
    res->add = triangleList_add;
    res->get = triangleList_get;
    res->set = triangleList_set;
    res->clear = triangleList_clear;
    res->copy = triangleList_copy;
    
    return res;
}
/*!
 \brief Destroy list. Free memory.
 \param v List
 */
void qh_triangleList_destroy(triangleList* v) {
    if(v!=NULL && v->arr!=NULL){
        free(v->arr);
        v->arr=NULL;
        free(v);
        v=NULL;
    }
    else{
        assert(false);
    }
}

/*!
 \brief Clear list. Free memory.
 \param v List
 */
void triangleList_clear(triangleList* v) {
    v->limit = 0;
    v->size = 0;
    v->arr = (triangle*) realloc(v->arr, v->limit * sizeof(triangle));
}

/*!
 \brief Add an item at the end of the list. If the limit is too low, new memory is allocated.
 \param v List
 \param n item
 */
void triangleList_add(triangleList* v, triangle n) {
    if(v->size == v->limit) {
        v->limit = v->limit + 1;
        v->arr = (triangle*) realloc(v->arr, v->limit * sizeof(triangle));
    }
    v->arr[v->size] = n;
    ++v->size;
}
/*!
 \brief Get an item at the \a index.
 \param v List
 \param index Index
 */
triangle triangleList_get(triangleList* v, int index) {
    if(index >= 0 && index < v->size)
        return v->arr[index];
    
    assert(false);
    return v->arr[-1];
}
/*!
 \brief Set an item at the index \a index.
 \param v List
 \param index Index
 \param n Item
 */
void triangleList_set(triangleList* v, int index, triangle n) {
    if(index >= 0 && index < v->size)
        v->arr[index] = n; return;
    
    assert(false);
}

/*!
 \brief Copy all items to a new list.
 \param from Original list
 \param to Destination list
 */
void triangleList_copy(triangleList* from, triangleList* to) {
    to -> clear(to);
    int it;
    for(it = 0; it < from->size; it++){
        to->add(to, from->get(from,it));
    }
}

/***** posInd list ******/
/*!
 \brief Creat a new list with initial lengths of \a l. Functions related to the struct are set dynamically via function pointers to mimic OOP design.
 \param l List limit
 */
posIndList* posIndList_new(int l) {
    posIndList* res = (posIndList*) malloc(sizeof(posIndList));
    res->limit = l;
    res->size = 0;
    res->arr = (posInd*) malloc(sizeof(posInd) * res->limit);
    res->add = posIndList_add;
    res->get = posIndList_get;
    res->set = posIndList_set;
    res->clear = posIndList_clear;
    res->ismember = posIndList_ismember;
    return res;
}
/*!
 \brief Destroy list. Free memory.
 \param v List
 */
void posIndList_destroy(posIndList* v) {
    if(v!=NULL && v->arr!=NULL){
        free(v->arr);
        v->arr=NULL;
        free(v);
        v=NULL;
    }
    else{
        assert(false);
    }
}

/*!
 \brief Add an item at the end of the list. If the limit is too low, new memory is allocated.
 \param v List
 \param n item
 */
void posIndList_add(posIndList* v, posInd n) {
    if(v->size == v->limit) {
        v->limit = v->limit + 1;
        v->arr = (posInd*) realloc(v->arr, v->limit * sizeof(posInd));
    }
    v->arr[v->size] = n;
    ++v->size;
}
/*!
 \brief Get an item at the \a index.
 \param v List
 \param index Index
 */
posInd posIndList_get(posIndList* v, int index) {
    if(index >= 0 && index < v->size)
        return v->arr[index];
    
    assert(false);
    return v->arr[-1];
}
/*!
 \brief Set an item at the index \a index.
 \param v List
 \param index Index
 \param n Item
 */
void posIndList_set(posIndList* v, int index, posInd n) {
    if(index >= 0 && index < v->size)
        v->arr[index] = n; return;
    
    assert(false);
}
/*!
 \brief Clear list. Free memory.
 \param v List
 */
void posIndList_clear(posIndList* v) {
    v->limit = 0;
    v->size = 0;
    v->arr = (posInd*) realloc(v->arr, v->limit * sizeof(posInd));
}

/*!
 \brief Check for membership.
 \param v List
 */
int posIndList_ismember(posIndList* v, int index) {
    int it;
    for(it = 0; it < v->size; it++){
        if(index == v->get(v, it).index){
            return 1;
        }
    }
   
    return 0;
}

/***** matrix ******/
/*!
 \brief Creat a new matrix with initial size of \a COL and \a ROW. Functions related to the struct are set dynamically via function pointers to mimic OOP design. All values are zero by default.
 \param ROW Number of rows
 \param COL Number of columns
 */
matrix* qh_matrix_new(int ROW, int COL) {
    matrix* res = (matrix*) malloc(sizeof(matrix));
    res->col = COL;
    res->row = ROW;
    res->arr = (float*) malloc(sizeof(float) * COL * ROW);
    res->set = matrix_set;
    res->get = matrix_get;
    res->subMatrix = matrix_subMatrix;
    res->subLeadMatrix = matrix_subLeadMatrix;
    res->subTailMatrix = matrix_subTailMatrix;
    res->clear = matrix_clear;
    res->reset = matrix_reset;
    res->copy = matrix_copy;
    
    /* Reset to zero values */
    int i, j;
    for (i = 0; i < COL; i++){
        for (j = 0; j < ROW; j++){
            res->set(res,j,i,0.0);
        }
    }
    return res;
}
/*!
 \brief Destroy matrix. Free memory.
 \param v Matrix
 */
void qh_matrix_destroy(matrix* v) {
    free(v->arr);
    v->arr=NULL;
    free(v);
    v=NULL;
}
/*!
 \brief Get a value at the index \a COL and \a ROW.
 \param v Matrix
 \param ROW Row index
 \param COL Column index
 */
float matrix_get(struct matrix* v, int ROW, int COL){
    if(COL < v->col && COL >= 0 && ROW < v->row && ROW >= 0){
        return v->arr[COL + ROW * v->col];
    }
    assert(false);
    return 0.0;
}

/*!
 \brief Set a value at the index \a COL and \a ROW.
 \param v Matrix
 \param ROW Row index
 \param COL Column index
 \param DATA Value to set
 */
void matrix_set(struct matrix* v, int ROW, int COL, float DATA){
    if(COL < v->col && COL >= 0 && ROW < v->row && ROW >= 0){
        v->arr[COL + ROW * v->col] = DATA;
        return;
    }
    assert(false);
}
/*!
 \brief Extract a submatrix. The rows and columns to extract are indicated by the
 arrays ROWs and COLs
 \param v Matrix
 \param numROW Number of rows to extract
 \param ROWs Array with row indices
 \param numCOL Number of columns to extract
 \param COLs Array with column indices
 \param sub Return submatrix
 */
void matrix_subMatrix(matrix* v, int numROW, int* ROWs, int numCOL, int* COLs, matrix* sub){
    sub->reset(sub,numROW, numCOL);
    int i, j;
    for (i = 0; i < numROW; i++){
        for (j = 0; j < numCOL; j++){
            sub->set(sub, i, j, v->get(v, ROWs[i], COLs[j] ) );
        }
    }
}
/*!
 \brief Extract a leading submatrix. The rows and columns to extract are the first
 \a numROW rows and \a numCOL columns.
 \param v Matrix
 \param numROW Number of leading rows to extract
 \param numCOL Number of leading columns to extract
 \param sub Return submatrix
 */
void matrix_subLeadMatrix(matrix* v, int numROW, int numCOL, matrix* sub){
    int *ROWs = (int*)malloc(sizeof(int)*numROW);
    int *COLs = (int*)malloc(sizeof(int)*numCOL);
    
    int i, j;
    for (i = 0; i < numROW; i++){
        ROWs[i] = i;
    }
    for (j = 0; j < numCOL; j++){
        COLs[j] = j;
    }
    
    v -> subMatrix( v, numROW, ROWs, numCOL, COLs, sub);
    
    free( ROWs );
    free( COLs );
}
/*!
 \brief Extract a tail submatrix. The rows and columns to extract are the rest
        starting at the start indices.
 \param v Matrix
 \param startROW Index of the first row where extraction will start and go through the end.
 \param startCOL Index of the first column where extraction will start and go through the end.
 \param sub Return submatrix
 */
void matrix_subTailMatrix(matrix* v, int startROW, int startCOL, matrix* sub){
    int numROW = v->row - startROW;
    int numCOL = v->col - startCOL;
    int *ROWs = (int*)malloc(sizeof(int)*numROW);
    int *COLs = (int*)malloc(sizeof(int)*numCOL);
    
    int i, j;
    for (i = 0; i < numROW; i++){
        ROWs[i] = i + startROW;
    }
    for (j = 0; j < numCOL; j++){
        COLs[j] = j + startCOL;
    }
    
    v -> subMatrix( v, numROW, ROWs, numCOL, COLs, sub);
    
    free( ROWs );
    free( COLs );
}
/*!
 \brief Clear matrix. Free array memory.
 \param v Matrix
 */
void matrix_clear(matrix* v) {
    v->col = 0;
    v->row = 0;
    v->arr = (float*) realloc(v->arr, 0 * sizeof(float));
}
/*!
 \brief Reset matrix. Allocate new memory for size given by \a ROW and \a COL.
 \param v Matrix
 \param ROW Number of rows
 \param COL Number of columns
 */
void matrix_reset(matrix* v,int ROW, int COL) {
    v->clear(v);
    v->col = COL;
    v->row = ROW;
    v->arr = (float*) realloc(v->arr, COL * ROW * sizeof(float));
    /* Reset to zero values */
    int i, j;
    for (i = 0; i < COL; i++){
        for (j = 0; j < ROW; j++){
            v->set(v,j,i,0.0);
        }
    }
}
/*!
 \brief Copy matrix to a new matrix. Allocate new memory for the destination matrix.
 \param from Original matrix
 \param to Destination matrix
 */
void matrix_copy(matrix* from, matrix* to) {
    to->reset(to, from->row, from->col);
    int i, j;
    for (i = 0; i < from->col; i++){
        for (j = 0; j < from->row; j++){
            to->set(to,j,i,from->get(from,j,i));
        }
    }
}
/*!
 * For debugging. Prints contents of a matrix
 * @param v
 * @param numRealSpkr The number of real speakers inside the matrix, used for row/column labeling
 */
void matrix_print(matrix* v, int numRealSpkr) {
    int r, c;
    printf("\n         ");
    /* upper header */
    for (c = 0; c < v->col; c++) {
        if (c >= numRealSpkr) {
            printf("___g%02i_", c + 1);
        } else {
            printf("___S%02i_", c + 1);
        }
    }

    for (r = 0; r < v->row; r++) {
        /* left header */
        if (r >= numRealSpkr) {
            printf("\n    g%02i |", r + 1);
        } else {
            printf("\n    S%02i |", r + 1);
        }
        /* matrix */
        for (c = 0; c < v->col; c++) {
            printf(" %4.4f", v->get(v, r, c));
        }
    }
    printf("\n\n");
}



/***** Internal functions *****/
/*!
 \brief Convert spherical to cartesian coordinates.
 \param azi Azimuth value.
 \param ele Elevation value.
 \param r Radius
 \return cart[3] 3D-Vector in cartesian coordinates.
 */
void sph2cart(float azi, float ele, float r, float cart[3]){
    cart[0] = r * cos(ele) * cos(azi);
    cart[1] = r * cos(ele) * sin(azi);
    cart[2] = r * sin(ele);
}

/*!
 \brief posInd comparision function for sorting the indices.
 \param a First item.
 \param b Second item.
 \return c Item difference.
 */
int posIndCmp(const void* a, const void* b){
    return ( (*(const posInd*) a).index - (*(const posInd*) b).index );
}

/*!
 \brief floatIndCmp comparision function for sorting the floats.
 \param a First item.
 \param b Second item.
 \return c Item difference.
 */
int floatIndCmp(const void* a, const void* b){
    return (int) ( (*(const floatInd*) a).f - (*(const floatInd*) b).f );
}

/*!
 \brief Check whether the subset defined by subsetRange exists and is empty. If yes, then add vertex in the mean angular position.
 \param vL Vertex list with coordinates.
 \param subsetRange Tolerance azimuth and elevation range of every subset vertex.
 \param numSubset Number of subset vertices.
 \param offset Angular azimuth offset for subsets in the back
 */
int addSubsetVertex(vertexList* vL, float* subsetRange, int numSubset, float offset)
{
    int addedGhost = 0;
    posIndList* subsetIndex = posIndList_new(0);
    findSubset(vL, subsetRange, numSubset, subsetIndex);
    int numVertex = subsetIndex->size;
    if(numVertex ==  numSubset)                                     /* complete subset was found */
    {
        if(isSubsetEmpty(vL, subsetIndex, offset))                          /* subset is empty */
        {
            /* calculate mean position */
            float mean_azi = 0.0;
            float mean_ele = 0.0;
            
            int it;
            for(it = 0; it < numVertex; it++)
            {
                int index = subsetIndex->get(subsetIndex, it).index;
                mean_azi += vL->get(vL, index).azi;
                mean_ele += vL->get(vL, index).ele;
            }
            mean_azi = mean_azi / (float) numVertex + offset;
            mean_ele = mean_ele / (float) numVertex;
            vL->add(vL,init_vertex(mean_azi, mean_ele,0));
            addedGhost = 1;
        }
    }
    posIndList_destroy(subsetIndex);
    return addedGhost;
}

/*!
 \brief Search for the subset defined by the ranges.
 \param vL Vertex list with coordinates.
 \param ranges Tolerance azimuth and elevation range of every subset vertex.
 \param numRanges Number of subset vertices.
 \return subset Indices of the subset vertices.
 */
void findSubset(vertexList* vL, float* ranges, int numRanges, posIndList* subset)
{
    int it;
    for(it = 0; it < numRanges; it++)
    {
        int found = vertexInRange(vL, &ranges[it*4]);
        if(found > -1)
        {
            posInd foundInd = {0,found};
            subset->add(subset, foundInd);
        }
    }
}

/*!
 \brief Check whether a vertex exits within the range.
 \param vL Vertex list with coordinates.
 \param range Tolerance azimuth and elevation range of a vertex.
 \return vertexExist Index of the vertex, otherwise -1.
 */
int vertexInRange(vertexList* vL, float* range)
{
    int vertexExist = -1;
    
    int n;
    for(n = 0; n < vL->size; n++)
    {
        float azi = vL->get(vL, n).azi;
        float ele = vL->get(vL, n).ele;
        if (azi >= range[0] && azi <= range[1] && ele >= range[2] && ele <= range[3])
        {
            vertexExist = n;
        }
    }
    return vertexExist;
}

/*!
 \brief Check whether the subset contains no other vertex.
 \param vL Vertex list with coordinates.
 \param subsetIndex Indices of the subset vertices.
 \return isEmpty It it is empty 1, otherwise 0.
 */
int isSubsetEmpty(vertexList* vL, posIndList* subsetIndex,float offset)
{
    int isEmpty = 1;
    int numVertex = subsetIndex->size;
    int it;
    
    /* collect subset vertices position into polygon array */
    float* polygon = (float*) malloc(2*numVertex*sizeof(float));
    for(it = 0; it < numVertex; it++)
    {
        int index = subsetIndex->get(subsetIndex, it).index;
        polygon[2*it + 0] = vL->get(vL, index).azi;
        polygon[2*it + 1] = vL->get(vL, index).ele;
    }
    
    /* check every vertex to be within the polygon */
    int isInPolygon;
    for(it = 0; it < vL->size; it++)
    {
        if(!subsetIndex->ismember(subsetIndex,it))
        {
            vertex v = vL->get(vL,it);
            float point[2] = {v.azi,  v.ele};
            isInPolygon = pointInPolygon(numVertex,polygon,point,offset);
            isEmpty = isEmpty && !isInPolygon;
        }
    }
    
    free(polygon);
    return isEmpty;
}

/*!
 \brief Check whether the point lies within the vertices polygon.  
 \param numVertices Number of polygon vertices
 \param polygon Coordinates of 2D polygon.
 \param point Testing 2D point.
 \return isInPolygon If the point is in the polygon then 1, otherwise 0.
 */
int pointInPolygon(int numVertices, float* polygon, float* point, float offset)
{
    int it, next;
    
    /* shift the polygon by the testing point. V are the relative coordinates. */
    float* V = (float*) malloc(2*numVertices*sizeof(float));
    for(it=0; it<numVertices; it++)
    {
        V[2*it + 0] = revertOrientationForBack(polygon[2*it + 0],offset) - revertOrientationForBack(point[0],offset);
        V[2*it + 1] = polygon[2*it + 1] - point[1];
    }
    
    /* a indicates the 'orientation' of the line defined by a polygonal boundary. If all orientations are equal the point is inside the polygon. */
    /* calculate first and last point of polygon */
    int isInPolygon = 1;
    
    /* calculate succesive pairs in the polygon */
    for(it=0; it<numVertices; it++)
    {
        next = (it+1)%numVertices;
        isInPolygon = isInPolygon && ((V[2*next + 0]*V[2*it + 1] - V[2*next + 1]*V[2*it + 0]) >= 0);
    }
    
    free(V);
    return isInPolygon;
}
/*!
 \brief Revert Front and Back with offset 180. For offset 0, the angles do not change. This is important to cope with the azimuth wrap around at -180/180.
 \param back Back side azimuth value.
 \param offset For 180: Reverse orientation. For 0: keep orientation.
 \return front Transformed azimuth value.
 */
float revertOrientationForBack(float back, float offset)
{
    float front = -(fmod(offset - back + 180, 360) - 180);
    return front;
}

/*!
 \brief Add all imaginary speakers according to the five rules. And choose a valid initial polyhedron. 
 \param vL Vertex list with coordinates.
 \return idx Indices of the initial polyhedron.
*/
void add_imaginary_speakers(vertexList* vL, floatList* idx)
{
    int Nv = vL->size;
    float thresh = 45.0; 
    float max_angle = 160.0;
    
    /**-- find highest and lowest speakers --**/
    int VoG = 0;
    int VoH = 0;
    
    int n, k;
    for(n = 0; n < Nv; n++){
        if(vL->get(vL,n).ele > vL->get(vL,VoG).ele){
            VoG = n;
        }
        if(vL->get(vL,n).ele < vL->get(vL,VoH).ele){
            VoH = n;
        }
    }
    
    /* add VoG if necessary */
    if(vL->get(vL,VoG).ele < thresh){
        VoG = vL->size;
        vL->add(vL,init_vertex(0.0, 90.0,0));
        /*printf("\n  Adding God Ghost");*/
    }
    
    /* add VoH if necessary */
    if(vL->get(vL,VoH).ele > -thresh){
        VoH = vL->size;
        vL->add(vL,init_vertex(0.0, -90.0,0));
        /*printf("\n  Adding Hell Ghost");*/
    }
    
    /**-- find quadrilateral front and rear subsets --**/
    float subsetArange[16] =
    {+23,+37,-9,+20, /* CH_M_L030 */
        -37,-23,-9,+20, /* CH_M_R030 */
        -37,-11,+21,+60,/* CH_U_R030 */
        +11,+37,+21,+60};/* CH_U_L030 */
    if (addSubsetVertex(vL, subsetArange, 4, 0.0)) {
        /*printf("\n  Added ghost for subset A! ");*/
    }
    else {
        float subsetBrange[16] =
        {+38,+52,-9,+20, /*CH_M_L045*/
            -52,-38,-9,+20, /*CH_M_R045*/
            -66,-38,+21,+60, /*CH_U_R045*/
            +38,+66,+21,+60};/*CH_U_L045*/
        if (addSubsetVertex(vL, subsetBrange, 4, 0.0)){
            /*printf("\n  Added ghost for subset B! ");*/        
        }
    }
       
    
    
    float subsetCrange[16] =
    {+101,+124,-45,+20,/*CH_M_L110*/
        +101,+124,+21,+60,/*CH_U_L110*/
        -124,-101,+21,+60,/*CH_U_R110*/
        -124,-101,-45,+20};/*CH_M_R110*/
    if (addSubsetVertex(vL, subsetCrange, 4, 180.0)) {
        /*printf("\n  Added ghost for subset C! ");*/        
    }
    else {
        float subsetDrange[16] =
        {+125,+142,-45,+20,/*CH_M_L135*/
            +125,+157,+21,+60,/*CH_U_R135*/
            -157,-125,+21,+60,/*CH_U_L135*/
            -142,-125,-45,+20};/*CH_M_R135*/
        if (addSubsetVertex(vL, subsetDrange, 4, 180.0)) {
            /*printf("\n  Added ghost for subset D! "); */       
        }        
    }
    
    
    
    /**-- extend surrounding speakers if necessary --**/
    floatList* azi = floatList_new(0);
    for(n = 0; n < Nv; n++){
        if(fabs(vL->get(vL,n).ele) < thresh){
            floatInd newAzi = {vL->get(vL,n).azi, n};
            azi->addFloatInd(azi, newAzi);
        }
    }
    qsort(azi->arr, azi->size, sizeof(floatInd), floatIndCmp);
    
    floatList* angle_diff = floatList_new(0);
    floatList* sectors = floatList_new(0);
    for(n = 1; n < azi->size; n++){
        angle_diff->add(angle_diff, azi->get(azi, n) - azi->get(azi, n-1)); /* angle_diff = [azi(2:end), azi(1) + 360] - azi; */
        sectors->add(sectors, ceil(angle_diff->get(angle_diff, n-1) / max_angle)); /*sectors = ceil(angle_diff / max_angle);*/
    }
    n=0;
    angle_diff->add(angle_diff, azi->get(azi, n) + 360.0f - azi->get(azi, azi->size-1));
    sectors->add(sectors, ceil(angle_diff->get(angle_diff, angle_diff->size-1) / max_angle)); /*sectors = ceil(angle_diff / max_angle);*/
    
    /*printf("\n  Fillup Surrounds: ");*/
    for(n = 0; n < sectors->size; n++){
        if(sectors->get(sectors,n) > 1){
            float new_diff = angle_diff->get(angle_diff,n) / sectors->get(sectors,n);
            float num_new = sectors->get(sectors,n) - 1;
            for(k = 1; k <= num_new; k++){
                float new_azi = azi -> get(azi, n) + k * new_diff;
                int new_chan = vL->size;
                floatInd newAzi = {new_azi, new_chan};
                azi->addFloatInd(azi, newAzi);
                vL->add(vL, init_vertex(new_azi, 0.0, 0)); /*vertex(new_chan) = init_vertex(new_azi, 0);*/
                /*printf("%f°, ", new_azi);*/
            }
        }
    }
    
    
    
    /**-- find indices of the initial polyeder --**/
    qsort(azi->arr, azi->size, sizeof(floatInd), floatIndCmp);
    
    floatInd fI_VoG = {0.0, VoG};
    idx->addFloatInd(idx,fI_VoG);
    floatInd fI_VoH = {0.0, VoH};
    idx->addFloatInd(idx,fI_VoH);
    
    int v1 = 0;
    int v2 = 0;
    int v3 = 0;
    
    for(n = 1; n < azi->size; n++){
        if(azi->get(azi, n) <= azi->get(azi, v1) + 160){
            v2 = n;
        }
        
        if(azi->get(azi, n) >= azi->get(azi, v1) + 200){
            v3 = n;
            break;
        }
    }
    
    v1 = azi->getFloatInd(azi,v1).index;
    v2 = azi->getFloatInd(azi,v2).index;
    v3 = azi->getFloatInd(azi,v3).index;
    
    floatInd fI_v1 = {0.0, v1};
    idx->addFloatInd(idx,fI_v1);
    floatInd fI_v2 = {0.0, v2};
    idx->addFloatInd(idx,fI_v2);
    floatInd fI_v3 = {0.0, v3};
    idx->addFloatInd(idx,fI_v3);
    
    
    floatList_destroy(angle_diff);
    floatList_destroy(sectors);
    floatList_destroy(azi);
}



/***** Operation functions *****/
/*!
 \brief Multiply matrices A and B and return C.
 \param A A matrix.
 \param B A matrix.
 \return C The resulting matrix.
 */
void m_mult(matrix* A, matrix* B, matrix* C){
    assert(A->col == B->row);
    C -> reset(C, A->row, B->col);
    int i, j, k;
    for (i = 0; i < A->row; i++)
        for (j = 0; j < B->col; j++) {
            C->set(C,i,j,0);
            for (k = 0; k < A->col; k++)
                C->set(C,i,j, C->get(C,i,j) + A->get(A,i,k) * B->get(B,k,j));
        }
}

/*!
 \brief Absolute norm of a matrix. 
 \param A A matrix.
 \return norm Sum of all absolute value matrix entries.
 */
float m_norm1(matrix* A){
    float norm = 0.0;
    int i, j;
    for (i = 0; i < A->row; i++)
        for (j = 0; j < A->col; j++) {
            norm = norm + fabs(A->get(A,i,j));
        }
    return norm;
}

/*!
 \brief The maximum element value in a matrix. 
 \param A A matrix.
 \return max The maximum of all value matrix entries.
 */
float m_max(matrix* A) {
    if (A->row <= 0 || A->col <= 0) {
        assert(false);
        return 0.f;
    }
    float max = A->arr[0];
    int i, j;
    for (i = 0; i < A->row; i++)
        for (j = 0; j < A->col; j++) {
            float w = A->get(A, i, j);
            if (w > max) {
                max = w;
            }
        }
    return max;
}

/*!
 \brief Subtract cartesian coordinates of vertex \a b from vertex \a a.
 \param a A vertex.
 \param b A vertex.
 \return c The difference vector.
 */
vertex vSub(const vertex a, const vertex b){
    vertex c = vertex_DEFAULT;
    int it;
    for(it = 0; it<3; it++){
        c.xyz[it] = a.xyz[it] - b.xyz[it];
    }
    return c;
}
/*!
 \brief Dot product of vertices \a a and \a b.
 \param a A vertex.
 \param b A vertex.
 \return c Dot product.
 */
float vDot(const vertex a, const vertex b){
    float c = (float) 0.0;
    int it;
    for(it = 0; it<3; it++){
        c += a.xyz[it] * b.xyz[it];
    }
    return c;
}

/*!
 \brief Cross product of vertices \a a and \a b.
 \param a A vertex.
 \param b A vertex.
 \return c Cross product vertex.
 */
vertex vCross(const vertex a, const vertex b){
    vertex c = vertex_DEFAULT;
    c.xyz[0] = a.xyz[1] * b.xyz[2] - a.xyz[2] * b.xyz[1];
    c.xyz[1] = a.xyz[2] * b.xyz[0] - a.xyz[0] * b.xyz[2];
    c.xyz[2] = a.xyz[0] * b.xyz[1] - a.xyz[1] * b.xyz[0];
    return c;
}

/*!
 \brief Normalise the length of vertex \a a to 1.
 \param a A vertex.
 \return c Normalised vertex.
 */
vertex vNormalize(const vertex a){
    float norm = (float)sqrt(a.xyz[0]*a.xyz[0] + a.xyz[1]*a.xyz[1] + a.xyz[2]*a.xyz[2]);
    vertex c = vertex_DEFAULT;
    c.xyz[0] = a.xyz[0] / norm;
    c.xyz[1] = a.xyz[1] / norm;
    c.xyz[2] = a.xyz[2] / norm;
    return c;
}
/*!
 \brief Sort indices of an edge in an ascending fashion.
 \param e An edge.
 */
void eSort(edge* e){
    if(e->index[0] > e->index[1]){
        int tmp = e->index[1];
        e->index[1] = e->index[0];
        e->index[0] = tmp;
    }
}

/*!
 \brief Signed shortest distance of point \a X to the plane defined by vertices \a P1, \a P2 and \a P3.
 Distance is positive if the plane separates \a X and the origin. The vertices \a P1, \a P2 and \a P3
 have to be ordered in counter-clockwise fashion.
 \see flip_plane
 \param P1 First plane point.
 \param P2 Second plane point.
 \param P3 Third plane point.
 \param X Distant point.
 */
float point_plane_dist(const vertex P1, const vertex P2, const vertex P3, const vertex X){
    vertex N = vCross(vSub(P1,P2), vSub(P1,P3));
    float dist = vDot(vSub(X,P1), vNormalize(N));
    return dist;
}

/*!
 \brief Wrapper function for \a point_plane_dist.
 \see point_plane_dist
 \param vL Vertex list with coordinates.
 \param surface Indices of plane vertices.
 \param vtx Index of distant point.
 \return dist Distance of the point \a vtx to plane \a surface.
 */
float vertex_distance(vertexList* vL, triangle surface, int vtx){
    vertex P1 = vL->get(vL,surface.index[0]);
    vertex P2 = vL->get(vL,surface.index[1]);
    vertex P3 = vL->get(vL,surface.index[2]);
    vertex X  = vL->get(vL,vtx);
    
    float dist = point_plane_dist(P1, P2, P3, X);
    return dist;
}

/*!
 \brief Extracts visible edges for a given set of triangles. Essentially, all edges which
 occures exactly once in the set of triangles are visible.
 \param T List of the triangles.
 \param E Return structure for visible edges.
 */
void visible_edges(triangleList* T, edgeList* E){
    /* create all possible edges */
    edgeList* tmpE = edgeList_new(0);
    edge e;
    
    int it1, it2;
    for(it1 = 0; it1<T->size; it1++){
        e.index[0] = T->arr[it1].index[0];
        e.index[1] = T->arr[it1].index[1];
        eSort(&e);
        tmpE->add(tmpE,e);
        e.index[0] = T->arr[it1].index[0];
        e.index[1] = T->arr[it1].index[2];
        eSort(&e);
        tmpE->add(tmpE,e);
        e.index[0] = T->arr[it1].index[1];
        e.index[1] = T->arr[it1].index[2];
        eSort(&e);
        tmpE->add(tmpE,e);
    }
    
    edge e1, e2;
    /* delete those which are doubles */
    for(it1 = 0; it1<T->size*3; it1++){
        e1 = tmpE->get(tmpE,it1);
        int duplicate = 0;
        for(it2 = 0; it2<T->size*3; it2++){
            if(it1!=it2){
                e2 = tmpE->get(tmpE,it2);
                if(e1.index[0] == e2.index[0] && e1.index[1] == e2.index[1]){
                    /* delete both edges */
                    duplicate = 1;
                }
            }
        }
        if(!duplicate){
            /* copy to output */
            E->add(E,e1);
        }
    }
    
    edgeList_destroy(tmpE);
}


/*!
 \brief Flips the orientation of the plane to ensure counter clockwise order of the defining vertices. As a result, the distance of the plane to the origin is negativ.
 \param vL Vertex list with coordinates.
 \param surface Indices of plane vertices.
 */
void flip_plane(vertexList* vL, triangle* surface){
    vertex P1 = vL->get(vL,surface->index[0]);
    vertex P2 = vL->get(vL,surface->index[1]);
    vertex P3 = vL->get(vL,surface->index[2]);
    vertex origin = vertex_DEFAULT;
    
    if(point_plane_dist(P1, P2, P3, origin) > 0){
        int tmp = surface->index[0];
        surface->index[0] = surface->index[2];
        surface->index[2] = tmp;
    }
}


/*!
 \brief Add new vertex \a vtx to a convex hull. The convex hull is given by the triangles \a tL. The plane orientation has to be counter clockwise and the result is guaranteed to be counter clockwise as well. The given convex hull has to cover at least a hemisphere.
 \param vL Vertex list with coordinates.
 \param tL List of indices of plane vertices.
 \param vtx New vertex.
 */
void add_vertex(vertexList* vL, triangleList* tL, int vtx){
    triangleList* tLnew = qh_triangleList_new(0);
    triangleList* tLvis = qh_triangleList_new(0);
    
    int it1;
    for(it1 = 0; it1<tL->size; it1++){
        if(vertex_distance(vL, tL->get(tL,it1), vtx) > -0.00001){
            tLvis->add(tLvis, tL->get(tL,it1));
        }
        else{
            tLnew->add(tLnew, tL->get(tL,it1));
        }
    }
    edgeList* eL = edgeList_new(0);
    visible_edges(tLvis, eL);
    
    triangle tmp;
    for(it1 = 0; it1<eL->size; it1++){
        tmp.index[0] = eL->get(eL,it1).index[0];
        tmp.index[1] = eL->get(eL,it1).index[1];
        tmp.index[2] = vtx;
        flip_plane(vL, &tmp);
        tLnew->add(tLnew, tmp);
    }
    tLnew->copy(tLnew, tL);
    
    edgeList_destroy(eL);
    qh_triangleList_destroy(tLnew);
    qh_triangleList_destroy(tLvis);
}
/*!
 \brief Sorting the order of the vertices. The order is saved in \a iL.
 \param vL Vertex list with coordinates.
 \param iL List of vertex indices.
 */
void sort_vertices(vertexList* vL, posIndList* iL){
    int it;
    for(it = 0; it<vL->size; it++){
        posInd newItem;
        newItem.pos = it;
        newItem.index = vL->get(vL,it).index;
        iL->add(iL, newItem);
    }
    qsort(iL->arr, iL->size, sizeof(posInd), posIndCmp);
}

/*!
 \brief Create the initial polyeder and return it in \a tL.
 \param vL Vertex list with coordinates.
 \param tL List of triangles.
 */
void initial_polyeder(vertexList* vL, triangleList* tL){
    floatList* idx = floatList_new(0);
    add_imaginary_speakers(vL, idx);
    int N = idx -> size - 2;
    int n;
    for(n = 0; n < N-1; n++){
        triangle newT = {{idx->getFloatInd(idx,0).index, idx->getFloatInd(idx,n+2).index, idx->getFloatInd(idx,n+3).index}};
        tL->add(tL, newT);
    }
    triangle newT = {{idx->getFloatInd(idx,0).index, idx->getFloatInd(idx,N+1).index, idx->getFloatInd(idx,2).index}};
    tL->add(tL, newT);
    
    for(n = 0; n < N-1; n++){
        triangle newT = {{idx->getFloatInd(idx,1).index, idx->getFloatInd(idx,n+3).index, idx->getFloatInd(idx,n+2).index}};
        tL->add(tL, newT);
    }
    {
        triangle newT = {{idx->getFloatInd(idx,1).index, idx->getFloatInd(idx,2).index, idx->getFloatInd(idx,N+1).index}};
        tL->add(tL, newT);
    }
    floatList_destroy(idx);
}

/*!
 \brief Initialise vertex including the sorting index.
 \param azimuth Azimuth angle in degree
 \param elevation Elevation angle in degree
 \param 0: orthogonal to cone of confusion; 1: along cone of confusion
 */
vertex init_vertex(float azimuth, float elevation, int cone){
    vertex v = vertex_DEFAULT;
    v.azi = (float) ( 180.0 - fmod(180.0 - azimuth, 360.0) );
    if (v.azi >= 180.0f) {
      v.azi = v.azi - 360.0f;
    }
    v.ele = (float) ( MAX(-90.0, MIN(90., elevation)) );
    
    float deg2rad = (float) (PI/180.0);
    sph2cart(v.azi * deg2rad, v.ele * deg2rad, 1.0, v.xyz);
    
    float idx_azi = (float) ROUND(fabs(90.0 - fabs(v.azi)));         /* along cone of confusion */
    float idx_ele;
    if(cone == 1){
        idx_ele = (float) (90.0 - ROUND(fabs(v.ele)) );
    }
    else{
        idx_ele = (float) ROUND(fabs(v.ele));
    }
    
    v.index = (int) (idx_azi + 181.0 * idx_ele);   /* vertices on the median plane have lowest index */
    return v;
}


/*!
 \brief Create downmix matrix
 \param[in] vL List of vertices containing real and ghost speaker vertices
 \param[in] tL List with Delaunay triangles
 \param[in] Nv Number of physical loudspakers
 \param[out] downmix_mat Downmix matrix
 */
void remap_ghost(vertexList* vL, triangleList* tL, int Nv, matrix* downmix_mat){
    
    matrix* M = qh_matrix_new(vL->size, vL->size);
    /* Create adjacency matrix */
    int it;
    for(it = 0; it < vL->size; it++){ /* iterate through real+ghost vertices */
        if(it < Nv){ /* real -> real speakers */
            M->set(M,it,it,1.0);
        }
        else{ /* ghost -> (ghost|real) speakers,  Matrix: COLUMN: ghost --> ROWS: (ghost|real) neighbor, */
            int itT;
            for(itT = 0; itT < tL->size; itT++){ /* iterate through triangles */
                triangle T = tL->get(tL, itT);
                int itI;
                int isNeighbor = 0;
                for(itI = 0; itI < 3; itI++){ /* iterate through indices of a triangle, check if we have neighbors*/
                    if( T.index[itI] == it ){
                        isNeighbor = 1;
                    }
                }
                if(isNeighbor == 1){
                    for(itI = 0; itI < 3; itI++){ /* iterate through indices of a triangle, collect neighbors */
                        if( T.index[itI] != it ){
                            M->set(M,T.index[itI],it,1.0);
                        }
                    }
                }
            }
            /* Normalize ghost columns*/
            int it2;
            float numOfNeighbors = 0.0;
            for(it2 = 0; it2 < vL->size; it2++){
                numOfNeighbors += M->get(M,it2,it);
            }
            for(it2 = 0; it2 < vL->size; it2++){
                M->set(M,it2,it, M->get(M,it2,it) / numOfNeighbors);
            }
        }
    }
    
    /*printf("\nReducing Matrix: ");*/
    /* Reduce Matrix */
    matrix* MM = qh_matrix_new(M->row, M->col);
    matrix* ghostMatrix = qh_matrix_new(vL->size - Nv, vL->size- Nv);
    for(it = 0; it < 20; it++){
        /*printf("\nIteration %i: ", it);*/
        /*matrix_print(M, Nv);*/
        m_mult(M,M,MM);
        MM->copy(MM,M);
        M->subTailMatrix(M, Nv, Nv, ghostMatrix);        
        /*
        printf("\nGhost_mat: ");
        matrix_print(ghostMatrix, 0);
         */
        if(m_max(ghostMatrix) <= 0.0001f){
            /*printf("\n Matrix is converged!!!!\n\n\n");*/
            break; /* converged */
        }
    }
    qh_matrix_destroy(ghostMatrix);
    qh_matrix_destroy(MM);
    
    /* Power Normalize */
    for(it = 0; it < vL->size; it++){
        int it2;
        for(it2 = 0; it2 < vL->size; it2++){
            M->set(M,it,it2, sqrt(M->get(M,it,it2)));
        }
    }
    
    /* Take Submatrix */
    M->subLeadMatrix(M, Nv, vL->size, downmix_mat);
    
    /*printf("\nSubmatrix: ");*/
	  /*matrix_print(downmix_mat,Nv);*/

    qh_matrix_destroy(M);
}

/***********************************************************************************/
/************* API FUNCTIONS *******************************************************/
/***********************************************************************************/

void qh_sphere_triangulation(vertexList* vL, triangleList* tL, matrix* downmix_mat){
    
    int Nv = vL -> size;
    
    initial_polyeder(vL, tL);
    
    posIndList* vertex_order = posIndList_new(0);
    sort_vertices(vL, vertex_order);
    
    int vtx, it;
    for(it = 0; it < vertex_order->size; it++){
        vtx = vertex_order->get(vertex_order,it).pos;
        add_vertex(vL, tL, vtx);
    }
    
    /* if initial_polyeder() adds some imaginary loudspeakers, use remap_ghost() to calculate downmix_mat */
    if(vL->size > Nv){
        remap_ghost(vL, tL, Nv, downmix_mat);
    }
    posIndList_destroy(vertex_order);
}

void qh_dmx_gen(int ch_in, int ch_out, matrix* dmx){
    if(ch_out < ch_in)
        ch_out = ch_in;
    
    dmx->reset(dmx, ch_in, ch_out);
    
    int it1;
    for(it1 = 0; it1 < ch_in; it1++){
        dmx->set(dmx, it1, it1, 1.0);
    }
}

/*!
 \brief Add neighbors to downmix matrix
        Step 1: All downmix matrix collumns specified by the "in" vector are set to zero
        step 2: All downmix matrix elements specified by the "in" x "out" indices (submatrix 
                in the downmixmatrix) are set to 1/(size of out).
 \param dmx Downmix matrix
 \param in Index of input neighbors in a row vector. 
           The elements selecting the column of the downmix matrix.
 \param out Index of output neighbors in a row vector.
           The elements selecting the rows, which will
 */
void qh_dmx_add(matrix* dmx, matrix* in, matrix* out){
    int N = out->row;
    
    int it1, it2;
    /* Loop through in channels elements which selects the row in the downmix matrix */
    for(it1 = 0; it1 < in->row; it1++){
        int inCh = (int) in->get(in, it1, 0);
        /* Sets the columns (given by in) of dmx to zero*/
        for(it2 = 0; it2 < dmx->row; it2++){ /* set to zero */
            dmx->set(dmx, it2, inCh, 0.0);
        }
        
        /* Sets the rows (given by out) of dmx to 1/N*/
        for(it2 = 0; it2 < out->row; it2++){
            int outCh = (int) out->get(out, it2, 0);
            dmx->set(dmx, outCh, inCh, 1.0f / sqrt((float)N));
        }
    }
}

void qh_gen_VertexList(int numSpk, float* az, float* el, int cone, vertexList* vL){
    int it;
    for(it = 0; it < numSpk; it++){
        vL->add(vL, init_vertex(az[it], el[it], cone));
    }
}
