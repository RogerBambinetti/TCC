/**************************************************************************************
  This software module was originally developed by  
 Deutsche Thomson OHG (DTO)
  in the course of development of the ISO/IEC 23008-3 for reference purposes and its 
 performance may not have been optimized. This software module is an implementation
 of one or more tools as specified by the ISO/IEC 23008-3 standard. ISO/IEC gives 
 you a royalty-free, worldwide, non-exclusive, copyright license to copy, distribute, 
 and make derivative works of this software module or modifications  thereof for use
 in implementations or products claiming conformance to the ISO/IEC 23008-3 standard 
 and which satisfy any specified conformance criteria. 
  In no event shall the above provision be qualified, deemed, or construed as granting 
 to use and any third party, either expressly, by implication or by way of estoppel,
 any license or any authorization or other right to license, sell, distribute, under 
 any patent or patent application and any intellectual property rights other than the 
 copyrights owned by Company which are embodied in such software module and expressly 
 licensed hereunder. 
 Those intending to use this software module in products are advised that its use 
 may implement third party intellectual property rights, and in particular existing 
 patents or patent application which licenses to use are not of Company or ISO/IEC 
 responsibility, which hereby fully disclaim any warranty and liability of infringement 
 of free enjoyment with respect to the software module and its use.
 The software modules is provided as is, without warranty of any kind. 
 DTO and ISO/IEC have no liability for use of this software module or modifications
 thereof.
 Copyright hereunder is not released licensed for products that do not conform to the 
 ISO/IEC 23008-3 standard.
 DTO retains full right to modify and use the code for its own  purpose, assign or 
 donate the code to a third party and to inhibit third parties from  using the code 
 for products that do not conform to MPEG-related ITU Recommendations  and/or 
 ISO/IEC International Standards.
 This copyright notice must be included in all copies or derivative works. 
 This copyright license shall be construed according to the laws of Germany. 
  Copyright (c) ISO/IEC 2015.
*****************************************************************************************/
/*SimpleMtrx.h
* project:          3D_Audio - MPEG H
* functionality:    Matrix math class: basic 2D matrix operations, singular value decomposition and pseudo inverse  
* authors:          Johannes Boehm (jb), 
* log:              created 16.12.2014, 
*/
/*
$Rev: 157 $
$Author: technicolor-kf $
$Date: 2015-01-16 15:00:35 +0100 (Fr, 16 Jan 2015) $
$Id: SimpleMtrx.h 157 2015-01-16 14:00:35Z technicolor-kf $
*/

#ifndef __SIMPLEMATRIX_H__
#define __SIMPLEMATRIX_H__



#include <stdexcept>
#include <vector>                           



class simpleMtrx
{
 public:
    simpleMtrx();                       // construct an empty object
    simpleMtrx(int nRows, int nCols);   // construct and allocate storage on heap
    simpleMtrx(int nRows, int nCols, double *pData, bool copy=true);  //construct and copy data from pData (copy true) or use pData as storage (ownership changes, be careful...)
    ~simpleMtrx();                      // destructor

    simpleMtrx(const simpleMtrx& M);    // Copy constructor 
    simpleMtrx& operator=( const simpleMtrx& rhs );  
    
    void resize(int nRows, int nCols);  // resize Matrix, data is lost 

    // operator overload for direct access
    double * operator[](int row)  { return & m_ptr[row * m_cols];}   // allows access as C array: M[2][0]
    const double* operator[](int row) const { return & m_ptr[row * m_cols];}   // allows read-only access as C array: M[2][0]
    
    void setIdentity();                 // value one diagonal elements
    void zeros();                       // element zeros
    void multScalar(double sc);         // multiplication by scalar, no operator 
    simpleMtrx transposeKeep();         // returns a transposed matrix of argument 
    void transpose();                   // matrix transpose
    simpleMtrx elementSquare();         // returns a new matrix with squared elements;
    simpleMtrx mult( const simpleMtrx & M2);  // multiplication, use operator overload
    simpleMtrx add( const simpleMtrx & M2);   // addition, use operator overload
    simpleMtrx sub( const simpleMtrx & M2);   // use operator overload
    double normFro();                   // Frobenius norm
    std::vector<double> vectorMult(std::vector<double>  & v_in); // v_out[]= M*v_in[]
    void copyData(double *dp);          // copy data from *dp, be careful 
    simpleMtrx operator*(  const simpleMtrx & M2 )  { return mult(M2); }; // multiplication
    simpleMtrx operator+(  const simpleMtrx & M2 )  { return add(M2); }; // addition
    simpleMtrx operator-(  const simpleMtrx & M2 )  { return sub(M2); }; // subtraction
    void print(); 
    double * data(){return m_ptr;};      // get raw pointer
    double * const data() const {return m_ptr;};      // get read-only raw pointer
    int getRows(){return m_rows;};       // get number of rows  
    int getCols(){return m_cols;};       // get number of columns
    void swapCols(int *idx);             // interchange columns indicated by values in idx
 private:
 int m_rows, m_cols;    
 double *m_ptr;
 };

 // class to decompose a double 2 D matrix
 class SVD
{
public:
    SVD(simpleMtrx A);
    simpleMtrx U,S,V;
    std::vector<double> w_s;
    simpleMtrx pinv(double threshold=1e-5);
private:
    SVD();
};

#endif // __SIMPLEMATRIX_H__
