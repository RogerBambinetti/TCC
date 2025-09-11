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

#ifndef QuickHullVBAP_QuickHull_functions_h
#define QuickHullVBAP_QuickHull_functions_h

#include "quickHull_objects.h"

#define PI 3.14159265358979323846264338327

/***** Internal functions *****/
/**
 * \defgroup fun Internal Functions
 * @{
 */


static vertex vSub(const vertex a, const vertex b);
static float vDot(const vertex a, const vertex b);
static vertex vCross(const vertex a, const vertex b);
static vertex vNormalize(const vertex a);
static void eSort(edge* e);
static float point_plane_dist(const vertex P1, const vertex P2, const vertex P3, const vertex X);
static float vertex_distance(vertexList* vL, triangle surface, int vtx);
static void visible_edges(triangleList* T, edgeList* E);
static void flip_plane(vertexList* vL, triangle* surface);
// static void flip_all_planes(vertexList* vL, triangleList* tL);
static void add_vertex(vertexList* vL, triangleList* tL, int vtx);
static void sort_vertices(vertexList* vL, posIndList* indexArray);
static void initial_polyeder(vertexList* vL, triangleList* tL);
static int addSubsetVertex(vertexList* vL, float* subsetRange, int numSubset, float offset);
static void findSubset(vertexList* vertex, float* ranges, int numRanges, posIndList* subset);
static int vertexInRange(vertexList* vL, float* range);
static int isSubsetEmpty(vertexList* vL, posIndList* subsetIndex, float offset);
static int pointInPolygon(int numVertices, float* V, float* point, float offset);
static float revertOrientationForBack(float back, float offset);
static vertex init_vertex(float azimuth, float elevation, int cone);
static void remap_ghost(vertexList* vL, triangleList* tL, int Nv, matrix* downmix_mat);

/**@}*/

#endif
