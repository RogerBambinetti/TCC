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
/*SimpleMtrx.cpp
*                   
* project:          3D_Audio - MPEG H
* functionality:    functionality:    Matrix math class: basic 2D matrix operations, singular value decomposition and pseudo inverse  
* authors:          Johannes Boehm (jb), 
* log:              created 16.12.2014, 
*/
/*
$Rev: 157 $
$Author: technicolor-kf $
$Date: 2015-01-16 15:00:35 +0100 (Fr, 16 Jan 2015) $
$Id: SimpleMtrx.cpp 157 2015-01-16 14:00:35Z technicolor-kf $
*/


#include "SimpleMtrx.h"
#include <stdexcept>

#include <iostream>    // for print method only
#include <iomanip>     // for print method only
#include <cmath>



#define CHSIGN(v1, v2) ((v2) >= 0.0 ? fabs(v1) : -fabs(v1))
#define MAX(x,y) ((x)>(y)?(x):(y))



///////////////  basics ////////////////////////          
simpleMtrx::simpleMtrx()
: m_rows(0), m_cols(0), m_ptr(0)
{
}

simpleMtrx::simpleMtrx(int nRows, int nCols)
: m_rows(nRows), m_cols(nCols)
{
 m_ptr = new double[m_rows*m_cols];
 }


 simpleMtrx::simpleMtrx(int nRows, int nCols, double *pData, bool copy)
: m_rows(nRows), m_cols(nCols)
{
 if (copy = false)
   m_ptr=pData;
 else
  {
   m_ptr = new double[m_rows*m_cols];
   copyData(pData);
   }
 }


 simpleMtrx::~simpleMtrx()
 {
    if(m_ptr)
        delete [] m_ptr;
 }

 simpleMtrx::simpleMtrx(const simpleMtrx& M)
 : m_rows(M.m_rows), m_cols(M.m_cols)
 {
    m_ptr = new double[m_rows*m_cols];
   copyData(M.m_ptr); 
 }

 simpleMtrx& simpleMtrx::operator=( const simpleMtrx& M )
 {
  if(this != &M)
  {
   if(m_ptr)
        delete [] m_ptr;
    m_ptr = new double[M.m_rows*M.m_cols];
    m_rows=M.m_rows; m_cols=M.m_cols;
    copyData(M.m_ptr);
  }
  return *this;
 }


 void simpleMtrx::resize(int nRows, int nCols)
{
    if(m_ptr)
        delete [] m_ptr;
    m_rows=nRows;
    m_cols=nCols;
    m_ptr = new double[m_rows*m_cols];
 }

 
void simpleMtrx::copyData(double *dp)
{
  double *pM1 = m_ptr;
  for( int rc=0; rc<m_rows*m_cols; rc++)
        *pM1++=*dp++;
}


void simpleMtrx::print()
{
  for(int r=0; r<m_rows; r++)
  {
    std::cout<<std::endl;
    std::cout.width(6);
   

    for(int c=0; c< m_cols; c++)
        std::cout<<" "<< std::setprecision(3)<<std::right<<m_ptr[r*m_cols+c]<<"  " <<std::resetiosflags(std::ios::showbase) ;  
   }
 std::cout<<std::endl;
}



///////////////  functionality ////////////////////////

 void simpleMtrx::zeros()
 {
  for( int rc=0; rc<m_rows*m_cols; rc++)
      m_ptr[rc]=0.0; 
 }

 void simpleMtrx::setIdentity()
 {
  zeros();
  int iM = (m_rows>m_cols) ? m_cols : m_rows ;
  for (int i=0; i<iM; i++)
    m_ptr[i*m_cols+i]=1.0;
 }


 simpleMtrx simpleMtrx::elementSquare()
 {
  simpleMtrx retM(m_rows, m_cols);
  for( int rc=0; rc<m_rows*m_cols; rc++)
     {*retM.m_ptr++=*m_ptr * *m_ptr; ++m_ptr;}
  return retM;
 }

void simpleMtrx::multScalar(double sc)
{
  for( int rc=0; rc<m_rows*m_cols; rc++)
      m_ptr[rc]*=sc; 
}

simpleMtrx simpleMtrx::transposeKeep()
{
 simpleMtrx Mt=simpleMtrx(m_cols, m_rows);
 for(int r=0; r<m_rows; r++)
    for(int c=0; c< m_cols; c++)
      Mt[c][r]=m_ptr[r*m_cols+c];
 return Mt;
}

void simpleMtrx::transpose()
{
  double *pTmp = new double[m_rows*m_cols];
  for(int r=0; r<m_rows; r++)
    for(int c=0; c< m_cols; c++)
      pTmp[c*m_rows+r]=m_ptr[r*m_cols+c];
  int t=m_rows; m_rows=m_cols; m_cols=t;
  delete [] m_ptr;
  m_ptr=pTmp;
}

 simpleMtrx simpleMtrx::mult(const simpleMtrx & M2)  // slow version
 {
  if(m_cols != M2.m_rows)  throw(std::runtime_error("simpleMtrx::mult - matrix dimensions mismatch, M1.m_cols != M2.m_rows"));
  simpleMtrx M3=simpleMtrx( m_rows, M2.m_cols);
  for(int r=0; r<M3.m_rows; r++)
    for(int c=0; c< M3.m_cols; c++)
    {
       M3[r][c]=0.0;
      for(int i=0; i<m_cols; i++)
           M3[r][c]+= m_ptr[r*m_cols+i]*M2[i][c];
    }
  return M3;
 }

simpleMtrx simpleMtrx::add(const simpleMtrx & M2)
 {
  if(m_cols != M2.m_cols || m_rows != M2.m_rows )  throw(std::runtime_error("simpleMtrx::add - matrix dimensions mismatch"));
  simpleMtrx M3=simpleMtrx( m_rows, m_cols);
  double *pM3 = M3.data();
  double const *pM2 = M2.data();
  double *pM1 = m_ptr;
  for( int rc=0; rc<m_rows*m_cols; rc++)
     *pM3++= *pM1++ + *pM2++;
  return M3;
 }

simpleMtrx simpleMtrx::sub(const simpleMtrx & M2)
 {
  if(m_cols != M2.m_cols || m_rows != M2.m_rows )  throw(std::runtime_error("simpleMtrx::sub - matrix dimensions mismatch"));
  simpleMtrx M3=simpleMtrx( m_rows, m_cols);
   double *pM3 = M3.data();
  double *pM2 = M2.data();
  double *pM1 = m_ptr;
  for( int rc=0; rc<m_rows*m_cols; rc++)
     *pM3++= *pM1++ - *pM2++;
  return M3;
 }


 double simpleMtrx::normFro()
 {
  double r=0.0;
  double *pM1 = m_ptr;
  for( int rc=0; rc<m_rows*m_cols; rc++)
     { r += *pM1 * *pM1; pM1++;}
  return sqrt(r);
 }


 std::vector<double> simpleMtrx::vectorMult(std::vector<double> & v_in)
{
std::vector<double>  v_out(m_rows);
 if(v_in.size() != m_cols)
   throw(std::runtime_error("SimpleMatrix  vectorMult - vector elements and Matrix columns do not match"));
 for(int r=0; r<m_rows; r++)
 {
  v_out[r]=0.0;
  for(int c=0; c<m_cols; c++)
      v_out[r]+=  m_ptr[r*m_cols+c]*v_in[c]; 
 }
  return v_out;
}


void simpleMtrx::swapCols(int *idx)
{
  double *m2_ptr = new double[m_rows*m_cols];
   for(int r=0; r<m_rows; r++)
    for(int c=0; c< m_cols; c++)
        m2_ptr[r*m_cols+c] = m_ptr[r*m_cols+idx[c]];
  delete [] m_ptr;
  m_ptr= m2_ptr; 
}

////////////////////////////////////////////////////////////
///////////////   helpers for svd    ///////////////////////


void getSortIdx (const double *arr, int n, int *idx )
{
  int i, j;
  for (i=0; i<n; i++)
  {
    idx[i] = i;
  }
 
  for (i=0; i<n; i++)
  {
    for (j=i+1; j<n; j++)
    {
      if (arr[idx[i]] < arr[idx[j]])
      {
       int ival= idx[j];
       idx[j] = idx[i];
       idx[i] = ival;
      }
    }
  }

}

// two routines inspired by numerical recipes,incl. some tricks to prevent over & underflow
static double pythagoras(double v1, double v2)
{
 double at = fabs(v1), bt = fabs(v2);
 double ct, res;
 if (at > bt)       
    { 
     ct = bt / at; 
     res = at * sqrt(1.0 + ct * ct); 
    }
 else if (bt > 0.0)
   { 
    ct = at / bt; 
    res = bt * sqrt(1.0 + ct * ct); 
   }
 else 
    res = 0.0;
 return(res);
}

// 
int dsvd(simpleMtrx &A, int rows, int cols, double *w,simpleMtrx &V)
{
    int flag, i, its, j, jj, k, l, nm;
    double c, f, h, s, x, y, z;
    double anorm = 0.0, g = 0.0, scale = 0.0;
    double *rv1 = new double [cols] ;

// Householder reduction to bidiagonal form 
    for (i = 0; i < cols; i++) 
    {
       // left-hand reduction 
        l = i + 1;
        rv1[i] = scale * g;
        g = s = scale = 0.0;
        if (i < rows) 
        {
            for (k = i; k < rows; k++) 
                scale += fabs(A[k][i]);
            if (scale) 
            {
                for (k = i; k < rows; k++) 
                {
                    A[k][i] = (A[k][i]/scale);
                    s += (A[k][i] * A[k][i]);
                }
                f = A[i][i];
                g = -CHSIGN(sqrt(s), f);
                h = f * g - s;
                A[i][i] = (f - g);
                if (i != cols - 1) 
                {
                    for (j = l; j < cols; j++) 
                    {
                        for (s = 0.0, k = i; k < rows; k++) 
                            s += (A[k][i] * A[k][j]);
                        f = s / h;
                        for (k = i; k < rows; k++) 
                            A[k][j] += (f * A[k][i]);
                    }
                }
                for (k = i; k < rows; k++) 
                    A[k][i] =(A[k][i]*scale);
            }
        }
        w[i] = (scale * g);
    
        // right-hand reduction 
        g = s = scale = 0.0;
        if (i < rows && i != cols - 1) 
        {
            for (k = l; k < cols; k++) 
                scale += fabs(A[i][k]);
            if (scale) 
            {
                for (k = l; k < cols; k++) 
                {
                    A[i][k] = (A[i][k]/scale);
                    s += (A[i][k] * A[i][k]);
                }
                f = A[i][l];
                g = -CHSIGN(sqrt(s), f);
                h = f * g - s;
                A[i][l] = (f - g);
                for (k = l; k < cols; k++) 
                    rv1[k] = A[i][k] / h;
                if (i != rows - 1) 
                {
                    for (j = l; j < rows; j++) 
                    {
                        for (s = 0.0, k = l; k < cols; k++) 
                            s += (A[j][k] * A[i][k]);
                        for (k = l; k < cols; k++) 
                            A[j][k] += (s * rv1[k]);
                    }
                }
                for (k = l; k < cols; k++) 
                    A[i][k] = (A[i][k]*scale);
            }
        }
        anorm = MAX(anorm, (fabs(w[i]) + fabs(rv1[i])));
    }
  
    // accumulate the right-hand transformation 
    for (i = cols - 1; i >= 0; i--) 
    {
        if (i < cols - 1) 
        {
            if (g) 
            {
                for (j = l; j < cols; j++)
                    V[j][i] = ((A[i][j] / A[i][l]) / g);
                    // double division to avoid underflow 
                for (j = l; j < cols; j++) 
                {
                    for (s = 0.0, k = l; k < cols; k++) 
                        s += (A[i][k] * V[k][j]);
                    for (k = l; k < cols; k++) 
                        V[k][j] += (s * V[k][i]);
                }
            }
            for (j = l; j < cols; j++) 
                V[i][j] = V[j][i] = 0.0;
        }
        V[i][i] = 1.0;
        g = rv1[i];
        l = i;
    }
  
    // accumulate the left-hand transformation 
    for (i = cols - 1; i >= 0; i--) 
    {
        l = i + 1;
        g = w[i];
        if (i < cols - 1) 
            for (j = l; j < cols; j++) 
                A[i][j] = 0.0;
        if (g) 
        {
            g = 1.0 / g;
            if (i != cols - 1) 
            {
                for (j = l; j < cols; j++) 
                {
                    for (s = 0.0, k = l; k < rows; k++) 
                        s += (A[k][i] * A[k][j]);
                    f = (s / A[i][i]) * g;
                    for (k = i; k < rows; k++) 
                        A[k][j] += (f * A[k][i]);
                }
            }
            for (j = i; j < rows; j++) 
                A[j][i] = (A[j][i]*g);
        }
        else 
        {
            for (j = i; j < rows; j++) 
                A[j][i] = 0.0;
        }
        ++A[i][i];
    }

    // diagonalize the bidiagonal form 
    for (k = cols - 1; k >= 0; k--) 
    {                             // loop over singular values 
        for (its = 0; its < 30; its++) 
        {                         // loop over allowed iterations 
            flag = 1;
            for (l = k; l >= 0; l--) 
            {                     // test for splitting 
                nm = l - 1;
                if (fabs(rv1[l]) + anorm == anorm) 
                {
                    flag = 0;
                    break;
                }
                if (fabs(w[nm]) + anorm == anorm) 
                    break;
            }
            if (flag) 
            {
                c = 0.0;
                s = 1.0;
                for (i = l; i <= k; i++) 
                {
                    f = s * rv1[i];
                    if (fabs(f) + anorm != anorm) 
                    {
                        g = w[i];
                        h = pythagoras(f, g);
                        w[i] = h; 
                        h = 1.0 / h;
                        c = g * h;
                        s = (- f * h);
                        for (j = 0; j < rows; j++) 
                        {
                            y = A[j][nm];
                            z = A[j][i];
                            A[j][nm] = (y * c + z * s);
                            A[j][i] = (z * c - y * s);
                        }
                    }
                }
            }
            z = w[k];
            if (l == k) 
            {                  // convergence 
                if (z < 0.0) 
                {              // make singular value nonnegative 
                    w[k] = (-z);
                    for (j = 0; j < cols; j++) 
                        V[j][k] = (-V[j][k]);
                }
                break;
            }
            if (its >= 30) {
                delete [] rv1;
                //No convergence !!!!!!!!!!!!!!!!!
                return(0);
            }
    
            //shift from bottom 2 x 2 minor 
            x = w[l];
            nm = k - 1;
            y = w[nm];
            g = rv1[nm];
            h = rv1[k];
            f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0 * h * y);
            g = pythagoras(f, 1.0);
            f = ((x - z) * (x + z) + h * ((y / (f + CHSIGN(g, f))) - h)) / x;
          
            // next QR transformation 
            c = s = 1.0;
            for (j = l; j <= nm; j++) 
            {
                i = j + 1;
                g = rv1[i];
                y = w[i];
                h = s * g;
                g = c * g;
                z = pythagoras(f, h);
                rv1[j] = z;
                c = f / z;
                s = h / z;
                f = x * c + g * s;
                g = g * c - x * s;
                h = y * s;
                y = y * c;
                for (jj = 0; jj < cols; jj++) 
                {
                    x = V[jj][j];
                    z = V[jj][i];
                    V[jj][j] = (x * c + z * s);
                    V[jj][i] = (z * c - x * s);
                }
                z = pythagoras(f, h);
                w[j] = z;
                if (z) 
                {
                    z = 1.0 / z;
                    c = f * z;
                    s = h * z;
                }
                f = (c * g) + (s * y);
                x = (c * y) - (s * g);
                for (jj = 0; jj < rows; jj++) 
                {
                    y = A[jj][j];
                    z = A[jj][i];
                    A[jj][j] = (y * c + z * s);
                    A[jj][i] = (z * c - y * s);
                }
            }
            rv1[l] = 0.0;
            rv1[k] = f;
            w[k] = x;
        }
    }
    delete [] rv1;
    return(1);
}






///////////////   svd constructor    ///////////////////////

SVD::SVD(simpleMtrx A)
{
 bool bTransposeFlag=false;
 if (A.getRows() < A.getCols())
 { 
   bTransposeFlag=true;
   A.transpose(); 
 }
 int rows=A.getRows(), cols=A.getCols();
 
 std::vector<double> wsx(cols);
 w_s.resize(cols);  //= new double[cols];
 V.resize(cols, cols);
  if (  dsvd(A, rows, cols, wsx.data(), V) ==0)
          throw(std::runtime_error("SimpleMatrix  / SVD did not converge"));

 // get index to sort singular values and matrix columns
std::vector<int> idx(cols);
getSortIdx (wsx.data(), cols, idx.data() );
S.resize(cols, cols);
S.zeros();
for(int i=0; i<cols; i++)
{   
   w_s[i]=wsx[idx[i]];
   S[i][i]=w_s[i];
}
 V.swapCols(idx.data());
 A.swapCols(idx.data());
 if(bTransposeFlag)
 {
   U = V;
   V = A;
  }
  else
    U=A;

}



simpleMtrx SVD::pinv(double threshold)
{
 
 simpleMtrx Ut= U.transposeKeep();
 simpleMtrx Si=S;
  double cmp=Si[1][1]*threshold;
 for(int i=0; i<Si.getCols(); i++)
{   
   double si= Si[i][i];
   Si[i][i]=(si>cmp)? (1.0/si) : 0; 
}
 simpleMtrx retM=V*Si*Ut;
 return retM;
}
