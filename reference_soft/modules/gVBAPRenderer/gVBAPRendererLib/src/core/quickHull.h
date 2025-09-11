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

#ifndef QuickHullVBAP_QuickHull_h
#define QuickHullVBAP_QuickHull_h

/**
 * \defgroup api API
 */
/* @{
 */

#ifdef __cplusplus
extern "C"
{
#endif

/*! \brief Vertex struct */
/*!
 A vertex defined by its three cartesian coordinates
 */
typedef struct vertex {
    float xyz[3]; /*!< 3D cartesian coordinates  */
    int index; /*!< sorting index */
    float ele; /*!< elevation */
    float azi; /*!< azimuth */
} vertex;
/*!  Vertex default values */
/*!
 struct to initialise the default values
 */
extern const vertex vertex_DEFAULT;

/*!  \brief Triangle struct */
/*!
 A triangle defined by the index numbers of the three corner vertices
 */
typedef struct triangle {
    int index[3]; /*! three corner vertices */
} triangle;

/*!  \brief List of \ref vertex */
/*!
 Dynamic list of vertices. The allocated memory is given by \a limit. A set of access functions is implemented via function pointers.
 */
typedef struct vertexList {
    int size; /*!< Current lenghts of the list.  */
    int limit; /*!< Memory limit of the list.  */
    vertex* arr; /*!< Items of the list.  */
    void (*add)(struct vertexList* v,vertex n); /*!< Add item to the list.  */
    vertex (*get)(struct vertexList* v,int index); /*!< Get item from the list.  */
    void (*set)(struct vertexList* v,int index, vertex n); /*!< Set item in the list.  */
    void (*clear)(struct vertexList* v); /*!< Clear all items in the list.  */
    void (*copy)(struct vertexList* from, struct vertexList* to); /*!< Copy all items to another list.  */
} vertexList;
vertexList* qh_vertexList_new(int l);
void qh_vertexList_destroy(vertexList* v);

/*! \brief List of \ref triangle */
/*!
 Dynamic list of triangles. The allocated memory is given by \a limit. A set of access functions is implemented via function pointers.
 */
typedef struct triangleList {
    int size; /*!< Current lenghts of the list.  */
    int limit; /*!< Memory limit of the list.  */
    triangle* arr; /*!< Items of the list.  */
    void (*add)(struct triangleList* v,triangle n); /*!< Add item to the list.  */
    triangle (*get)(struct triangleList* v,int index); /*!< Get item from the list.  */
    void (*set)(struct triangleList* v,int index, triangle n); /*!< Set item in the list.  */
    void (*clear)(struct triangleList* v); /*!< Clear all items in the list.  */
    void (*copy)(struct triangleList* from, struct triangleList* to); /*!< Copy all items to another list.  */
} triangleList;
triangleList* qh_triangleList_new(int l);
void qh_triangleList_destroy(triangleList* v);

/*! \brief Matrix struct */
/*!
 2-dimensional matrix of floats. The allocated memory is given by \a col and \a row. A set of access functions is implemented via function pointers.
 */
typedef struct matrix {
    int col; /*!< Number of columns  */
    int row; /*!< Number of rows  */
    float* arr; /*!< Matrix entries  */
    float (*get)(struct matrix* v, int ROW, int COL); /*!< Get item in the matrix.  */
    void (*set)(struct matrix* v, int ROW, int COL, float DATA); /*!< Set item in the matrix.  */
    void (*subMatrix)(struct matrix* v, int numROW, int* ROWs, int numCOL, int* COLs, struct matrix* sub); /* Extract submatrix */
    void (*subLeadMatrix)(struct matrix* v, int numROW, int numCOL, struct matrix* sub); /* Extract leading submatrix */
    void (*subTailMatrix)(struct matrix* v, int startROW, int startCOL, struct matrix* sub); /* Extract tail submatrix */
    void (*clear)(struct matrix* v); /*!< Clear the matrix.  */
    void (*reset)(struct matrix* v, int ROW, int COL); /*!< Reset the matrix to a new size.  */
    void (*copy)(struct matrix* from, struct matrix* to); /*!< Copy to another matrix. */
} matrix;
matrix* qh_matrix_new(int ROW, int COL);
void qh_matrix_destroy(matrix* v);

/*!
 \brief Generate vertex list for further processing
 \param numSpk Number of speakers
 \param az Array of azimuth values in degree
 \param el Array of elevation values in degree
 \param cone 0: orthogonal to cone of confusion; 1: along cone of confusion
 \param Return initialiased vertexList
 */
void qh_gen_VertexList(int numSpk, float* az, float* el, int cone, vertexList* vL);


/*!
 \brief Delaunay triangulation on a sphere surface.
 \param vL List of vertices
 \param tL List with Delaunay triangles
 \param downmix_mat Downmix matrix
 */
void qh_sphere_triangulation(vertexList* vL, triangleList* tL, matrix* downmix_mat);

/*!
 \brief Create downmix matrix
 \param ch_in Number of in channels
 \param ch_out Number of out channels
 \param dmx Return downmix matrix
 */
void qh_dmx_gen(int ch_in, int ch_out, matrix* dmx);

/*!
 \brief Add neighbors to downmix matrix
 \param dmx Downmix matrix
 \param in Index of input neighbors in a row vector
 \param out Index of output neighbors in a row vector
 */
void qh_dmx_add(matrix* dmx, matrix* in, matrix* out);

/**@}*/
#ifdef __cplusplus
}
#endif

#endif
