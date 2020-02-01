/*
     * Licensed to the Apache Software Foundation (ASF) under one or more
     * contributor license agreements.  See the NOTICE file distributed with
     * this work for additional information regarding copyright ownership.
     * The ASF licenses this file to You under the Apache License, Version 2.0
     * (the "License"); you may not use this file except in compliance with
     * the License.  You may obtain a copy of the License at
     *
     *      http://www.apache.org/licenses/LICENSE-2.0
     *
     * Unless required by applicable law or agreed to in writing, software
     * distributed under the License is distributed on an "AS IS" BASIS,
     * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
     * See the License for the specific language governing permissions and
     * limitations under the License.
     */


/**
     * This class solves a least squares problem using the Levenberg-Marquardt algorithm.
     *
     * <p>This implementation <em>should</em> work even for over-determined systems
     * (i.e. systems having more point than equations). Over-determined systems
     * are solved by ignoring the point which have the smallest impact according
     * to their jacobian column norm. Only the rank of the matrix and some loop bounds
     * are changed to implement this.</p>
     *
     * <p>The resolution engine is a simple translation of the MINPACK <a
     * href="http://www.netlib.org/minpack/lmder.f">lmder</a> routine with minor
     * changes. The changes include the over-determined resolution, the use of
     * inherited convergence checker and the Q.R. decomposition which has been
     * rewritten following the algorithm described in the
     * P. Lascaux and R. Theodor book <i>Analyse num&eacute;rique matricielle
     * appliqu&eacute;e &agrave; l'art de l'ing&eacute;nieur</i>, Masson 1986.</p>
     * <p>The authors of the original fortran version are:
     * <ul>
     * <li>Argonne National Laboratory. MINPACK project. March 1980</li>
     * <li>Burton S. Garbow</li>
     * <li>Kenneth E. Hillstrom</li>
     * <li>Jorge J. More</li>
     * </ul>
     * The redistribution policy for MINPACK is available <a
     * href="http://www.netlib.org/minpack/disclaimer">here</a>, for convenience, it
     * is reproduced below.</p>
     *
     * <table border="0" width="80%" cellpadding="10" align="center" bgcolor="#E0E0E0">
     * <tr><td>
     *    Minpack Copyright Notice (1999) University of Chicago.
     *    All rights reserved
     * </td></tr>
     * <tr><td>
     * Redistribution and use in source and binary forms, with or without
     * modification, are permitted provided that the following conditions
     * are met:
     * <ol>
     *  <li>Redistributions of source code must retain the above copyright
     *      notice, this list of conditions and the following disclaimer.</li>
     * <li>Redistributions in binary form must reproduce the above
     *     copyright notice, this list of conditions and the following
     *     disclaimer in the documentation and/or other materials provided
     *     with the distribution.</li>
     * <li>The end-user documentation included with the redistribution, if any,
     *     must include the following acknowledgment:
     *     <code>This product includes software developed by the University of
     *           Chicago, as Operator of Argonne National Laboratory.</code>
     *     Alternately, this acknowledgment may appear in the software itself,
     *     if and wherever such third-party acknowledgments normally appear.</li>
     * <li><strong>WARRANTY DISCLAIMER. THE SOFTWARE IS SUPPLIED "AS IS"
     *     WITHOUT WARRANTY OF ANY KIND. THE COPYRIGHT HOLDER, THE
     *     UNITED STATES, THE UNITED STATES DEPARTMENT OF ENERGY, AND
     *     THEIR EMPLOYEES: (1) DISCLAIM ANY WARRANTIES, EXPRESS OR
     *     IMPLIED, INCLUDING BUT NOT LIMITED TO ANY IMPLIED WARRANTIES
     *     OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE
     *     OR NON-INFRINGEMENT, (2) DO NOT ASSUME ANY LEGAL LIABILITY
     *     OR RESPONSIBILITY FOR THE ACCURACY, COMPLETENESS, OR
     *     USEFULNESS OF THE SOFTWARE, (3) DO NOT REPRESENT THAT USE OF
     *     THE SOFTWARE WOULD NOT INFRINGE PRIVATELY OWNED RIGHTS, (4)
     *     DO NOT WARRANT THAT THE SOFTWARE WILL FUNCTION
     *     UNINTERRUPTED, THAT IT IS ERROR-FREE OR THAT ANY ERRORS WILL
     *     BE CORRECTED.</strong></li>
     * <li><strong>LIMITATION OF LIABILITY. IN NO EVENT WILL THE COPYRIGHT
     *     HOLDER, THE UNITED STATES, THE UNITED STATES DEPARTMENT OF
     *     ENERGY, OR THEIR EMPLOYEES: BE LIABLE FOR ANY INDIRECT,
     *     INCIDENTAL, CONSEQUENTIAL, SPECIAL OR PUNITIVE DAMAGES OF
     *     ANY KIND OR NATURE, INCLUDING BUT NOT LIMITED TO LOSS OF
     *     PROFITS OR LOSS OF DATA, FOR ANY REASON WHATSOEVER, WHETHER
     *     SUCH LIABILITY IS ASSERTED ON THE BASIS OF CONTRACT, TORT
     *     (INCLUDING NEGLIGENCE OR STRICT LIABILITY), OR OTHERWISE,
     *     EVEN IF ANY OF SAID PARTIES HAS BEEN WARNED OF THE
     *     POSSIBILITY OF SUCH LOSS OR DAMAGES.</strong></li>
     * <ol></td></tr>
     * </table>
     * @deprecated As of 3.1 (to be removed in 4.0).
     * @since 2.0
     *
     */


#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <vector>

#include <math/levenberg_marquardt.h>

#define NEGATIVE_INFINITY ( -INFINITY )

template <class T = double> class Array1D
{
public:
    Array1D( int n = 0 )
    {
        m_array = new T[n];
        m_size = n;
    }

    ~Array1D()
    {
        delete[] m_array;
    }

    Array1D( const Array1D<T>& other )
    {
        this->m_size = other.m_size;
        this->m_array = new T[this->m_size];
        memcpy( this->m_array, other.m_array, sizeof( T ) * this->m_size );
    }

    const Array1D<T>& operator=( const Array1D<T>& other )
    {
        this->m_size = other.m_size;
        this->m_array = new T[this->m_size];
        memcpy( this->m_array, other.m_array, sizeof( T ) * this->m_size );
        return *this;
    }

    void fill( int start, int end, T value )
    {
        for( int i = start; i < end; i++ )
            ( *this )[i] = value;
    }

    T& operator[]( size_t idx )
    {
        return m_array[idx];
    }
    const T& operator[]( size_t idx ) const
    {
        return m_array[idx];
    }

    int size() const
    {
        return m_size;
    }

    T* getData()
    {
        return m_array;
    }

private:
    int m_size;
    T*  m_array;
};

template <class T = double> class Array2D
{
public:
    Array2D( int n_rows = 0, int n_cols = 0 ) : m_rows( n_rows ), m_cols( n_cols )
    {
        m_data.resize( m_rows );
        for( int i = 0; i < m_rows; i++ )
            m_data[i] = new Array1D<double>( m_cols );
    }

    Array2D( const Array2D<T>& other )
    {
        m_rows = other.m_rows;
        m_cols = other.m_cols;
        m_data.resize( other.m_rows );
        for( int i = 0; i < m_rows; i++ )
            m_data[i] = new Array1D<double>( other[i] );
    }

    ~Array2D()
    {
        for( int i = 0; i < m_rows; i++ )
            delete m_data[i];
    }

    const Array2D<T>& operator=( const Array2D<T>& other )
    {
        for( int i = 0; i < m_rows; i++ )
            delete m_data[i];
        m_rows = other.m_rows;
        m_cols = other.m_cols;
        m_data.resize( other.m_rows );
        for( int i = 0; i < m_rows; i++ )
            m_data[i] = new Array1D<double>( other[i] );
        return *this;
    }

    Array1D<T>& operator[]( size_t idx )
    {
        return *m_data[idx];
    }
    const Array1D<T>& operator[]( size_t idx ) const
    {
        return *m_data[idx];
    }

    int size() const
    {
        return m_rows;
    }

protected:
    int                      m_rows, m_cols;
    std::vector<Array1D<T>*> m_data;
};

template <class T = double> class PointVectorValuePair
{
public:
    PointVectorValuePair( const Array1D<T>& p, const Array1D<T>& v )
    {
        m_point = p;
        m_value = v;
    }
    
    PointVectorValuePair( )
    {

    }

    PointVectorValuePair( const PointVectorValuePair<T>& other )
    {
        this->m_point = other.m_point;
        this->m_value = other.m_value;
    }

    PointVectorValuePair<T>& operator=( const PointVectorValuePair<T>& other )
    {
        this->m_point = other.m_point;
        this->m_value = other.m_value;
        return *this;
    }


    const Array1D<T>& getPoint() const
    {
        return m_point;
    };
    const Array1D<T>& getValue() const
    {
        return m_value;
    };
    Array1D<T>& point()
    {
        return m_point;
    };
    Array1D<T>& value()
    {
        return m_value;
    };

private:
    Array1D<T> m_point, m_value;
};

template <class T> class Matrix : public Array2D<T>
{
public:
    Matrix( int n_rows = 0, int n_cols = 0 ) : Array2D<T>( n_rows, n_cols )
    {
        zero();
    }

    void zero()
    {
        for( int i = 0; i < this->m_rows; i++ )
            for( int j = 0; j < this->m_cols; j++ )
                ( *this )[i][j] = 0.0;
    }

    static Matrix<T> identity( int n )
    {
        Matrix<T> rv( n, n );
        for( int i = 0; i < n; i++ )
            for( int j = 0; j < n; j++ )
                rv[i][j] = 0.0;

        for( int i = 0; i < n; i++ )
            rv[i][i] = 1.0;

        return rv;
    }

    int rows() const
    {
        return this->m_rows;
    }
    int cols() const
    {
        return this->m_cols;
    }

    const Array1D<double> operate( const Array1D<double>& v ) const
    {
        auto nRows = this->size();
        auto nCols = ( *this )[0].size();
        
        assert( v.size() == nCols );
        
        Array1D<double> out( nRows );
        for( int row = 0; row < nRows; row++ )
        {
            auto   dataRow = ( *this )[row];
            double sum = 0;
            for( int i = 0; i < nCols; i++ )
            {
                sum += dataRow[i] * v[i];
            }
            out[row] = sum;
        }
        return out;
    }

    const Matrix<double> scalarMultiply( double s ) const
    {
        int rowCount = this->size();
        int columnCount = ( *this )[0].size();

        Matrix<double> out( rowCount, columnCount );

        for( int row = 0; row < rowCount; ++row )
        {
            for( int col = 0; col < columnCount; ++col )
            {
                out[row][col] = ( *this )[row][col] * s;
            }
        }

        return out;
    }
};

class LevenbergMarquardtOptimizer
{
protected:
    int evalCount = 0;
    int evalMaximalCount = 100000;

    /** Number of solved point. */
    int solvedCols;
    /** Diagonal elements of the R matrix in the Q.R. decomposition. */
    Array1D<double> diagR;
    /** Norms of the columns of the jacobian matrix. */
    Array1D<double> jacNorm;
    /** Coefficients of the Householder transforms vectors. */
    Array1D<double> beta;
    /** Columns permutation array. */
    Array1D<int> permutation;
    /** Rank of the jacobian matrix. */
    int rank;
    /** Levenberg-Marquardt parameter. */
    double lmPar;
    /** Parameters evolution direction associated with lmPar. */
    Array1D<double> lmDir;
    /** Positive input variable used in determining the initial step bound. */
    double initialStepBoundFactor;
    /** Desired relative error in the sum of squares. */
    double costRelativeTolerance;
    /**  Desired relative error in the approximate solution parameters. */
    double parRelativeTolerance;
    /** Desired max cosine on the orthogonality between the function vector
         * and the columns of the jacobian. */
    double orthoTolerance;
    /** Threshold for QR ranking. */
    double qrRankingThreshold;
    /** Weighted residuals. */
    Array1D<double> weightedResidual;
    /** Weighted Jacobian. */
    Array2D<double> weightedJacobian;

    double cost;

    PointVectorValuePair<double> lastFit;

    std::string m_failureReason;

public:
    LevenbergMarquardtOptimizer()
    {
        initialStepBoundFactor = 100;
        costRelativeTolerance = 1e-10;
        parRelativeTolerance = 1e-10;
        orthoTolerance = 1e-10;
        qrRankingThreshold = 1e-30;
    }

    virtual ~LevenbergMarquardtOptimizer()
    {

    }

    virtual const Matrix<double> computeJacobian( const Array1D<double>& params ) const = 0;

    virtual const Matrix<double> computeWeightedJacobian( const Array1D<double>& params ) const
    {
        return computeJacobian( params );
    }

    virtual Array1D<double> getTarget() const = 0;
    virtual Array1D<double> getStartPoint() const = 0;

protected:
    const Matrix<double> getWeightSquareRoot() const
    {
        auto target = getTarget();
        return Matrix<double>::identity( target.size() );
    }

    const Matrix<double> getWeight() const
    {
        auto target = getTarget();
        return Matrix<double>::identity( target.size() );
    }

    const Array1D<double> computeResiduals( const Array1D<double>& objectiveValue )
    {
        const auto target = getTarget();

        assert( objectiveValue.size() == target.size() );

        Array1D<double> residuals( target.size() );
        for( int i = 0; i < target.size(); i++ )
        {
            residuals[i] = target[i] - objectiveValue[i];
        }

        return residuals;
    }

    virtual const Array1D<double> computeObjectiveValue( const Array1D<double>& params ) const = 0;

    double computeCost( const Array1D<double>& residuals )
    {
        return sqrt( dotProduct( residuals, getWeight().operate( residuals ) ) );
    }

    double dotProduct( const Array1D<double>& v1, const Array1D<double>& v2 ) const
    {
        double dot = 0;
        for( int i = 0; i < v1.size(); i++ )
        {
            dot += v1[i] * v2[i];
        }
        return dot;
    }

    /**
         * Decompose a matrix A as A.P = Q.R using Householder transforms.
         * <p>As suggested in the P. Lascaux and R. Theodor book
         * <i>Analyse num&eacute;rique matricielle appliqu&eacute;e &agrave;
         * l'art de l'ing&eacute;nieur</i> (Masson, 1986), instead of representing
         * the Householder transforms with u<sub>k</sub> unit vectors such that:
         * <pre>
         * H<sub>k</sub> = I - 2u<sub>k</sub>.u<sub>k</sub><sup>t</sup>
         * </pre>
         * we use <sub>k</sub> non-unit vectors such that:
         * <pre>
         * H<sub>k</sub> = I - beta<sub>k</sub>v<sub>k</sub>.v<sub>k</sub><sup>t</sup>
         * </pre>
         * where v<sub>k</sub> = a<sub>k</sub> - alpha<sub>k</sub> e<sub>k</sub>.
         * The beta<sub>k</sub> coefficients are provided upon exit as recomputing
         * them from the v<sub>k</sub> vectors would be costly.</p>
         * <p>This decomposition handles rank deficient cases since the tranformations
         * are performed in non-increasing columns norms order thanks to columns
         * pivoting. The diagonal elements of the R matrix are therefore also in
         * non-increasing absolute values order.</p>
         *
         * @param jacobian Weighted Jacobian matrix at the current point.
         * @exception ConvergenceException if the decomposition cannot be performed
         */
    bool qrDecomposition( const Matrix<double>& jacobian )
    {
        // Code in this class assumes that the weighted Jacobian is -(W^(1/2) J),
        // hence the multiplication by -1.
        weightedJacobian = jacobian.scalarMultiply( -1 );

        const int nR = weightedJacobian.size();
        const int nC = weightedJacobian[0].size();

        // initializations
        for( int k = 0; k < nC; ++k )
        {
            permutation[k] = k;
            double norm2 = 0;
            for( int i = 0; i < nR; ++i )
            {
                double akk = weightedJacobian[i][k];
                norm2 += akk * akk;
            }
            jacNorm[k] = sqrt( norm2 );
        }

        // transform the matrix column after column
        for( int k = 0; k < nC; ++k )
        {

            // select the column with the greatest norm on active components
            int    nextColumn = -1;
            double ak2 = NEGATIVE_INFINITY;
            for( int i = k; i < nC; ++i )
            {
                double norm2 = 0;
                for( int j = k; j < nR; ++j )
                {
                    double aki = weightedJacobian[j][permutation[i]];
                    norm2 += aki * aki;
                }
                if( std::isinf( norm2 ) || std::isnan( norm2 ) )
                {
                    m_failureReason = "Unable to perform QR decomposition on Jacobian";
                    return false;
                }
                if( norm2 > ak2 )
                {
                    nextColumn = i;
                    ak2 = norm2;
                }
            }
            if( ak2 <= qrRankingThreshold )
            {
                rank = k;
                return true;
            }
            int pk = permutation[nextColumn];
            permutation[nextColumn] = permutation[k];
            permutation[k] = pk;

            // choose alpha such that Hk.u = alpha ek
            double akk = weightedJacobian[k][pk];
            double alpha = ( akk > 0 ) ? -sqrt( ak2 ) : sqrt( ak2 );
            double betak = 1.0 / ( ak2 - akk * alpha );
            beta[pk] = betak;

            // transform the current column
            diagR[pk] = alpha;
            weightedJacobian[k][pk] -= alpha;

            // transform the remaining columns
            for( int dk = nC - 1 - k; dk > 0; --dk )
            {
                double gamma = 0;
                for( int j = k; j < nR; ++j )
                {
                    gamma += weightedJacobian[j][pk] * weightedJacobian[j][permutation[k + dk]];
                }
                gamma *= betak;
                for( int j = k; j < nR; ++j )
                {
                    weightedJacobian[j][permutation[k + dk]] -= gamma * weightedJacobian[j][pk];
                }
            }
        }
        rank = solvedCols;
        return true;
    }

    /**
         * Compute the product Qt.y for some Q.R. decomposition.
         *
         * @param y vector to multiply (will be overwritten with the result)
         */
    void qTy( Array1D<double>& y )
    {
        const int nR = weightedJacobian.size();
        const int nC = weightedJacobian[0].size();

        for( int k = 0; k < nC; ++k )
        {
            int    pk = permutation[k];
            double gamma = 0;
            for( int i = k; i < nR; ++i )
            {
                gamma += weightedJacobian[i][pk] * y[i];
            }
            gamma *= beta[pk];
            for( int i = k; i < nR; ++i )
            {
                y[i] -= gamma * weightedJacobian[i][pk];
            }
        }
    }

    void setCost( double c )
    {
        cost = c;
    }

public:

    bool doOptimize()
    {
        int             nR = getTarget().size(); // Number of observed data.
        Array1D<double> currentPoint = getStartPoint();
        int             nC = currentPoint.size(); // Number of parameters.

        // arrays shared with the other private methods
        solvedCols = std::min( nR, nC );
        diagR = Array1D<double>( nC );
        jacNorm = Array1D<double>( nC );
        beta = Array1D<double>( nC );
        permutation = Array1D<int>( nC );
        lmDir = Array1D<double>( nC );

        // local point
        double delta = 0;
        double xNorm = 0;
        auto   diag = Array1D<double>( nC );
        auto   oldX = Array1D<double>( nC );
        auto   oldRes = Array1D<double>( nR );
        auto   oldObj = Array1D<double>( nR );
        auto   qtf = Array1D<double>( nR );
        auto   work1 = Array1D<double>( nC );
        auto   work2 = Array1D<double>( nC );
        auto   work3 = Array1D<double>( nC );

        auto weightMatrixSqrt = getWeightSquareRoot();

        // Evaluate the function at the starting point and calculate its norm.
        auto                         currentObjective = computeObjectiveValue( currentPoint );
        auto                         currentResiduals = computeResiduals( currentObjective );
        PointVectorValuePair<double> current( currentPoint, currentObjective );
        double                       currentCost = computeCost( currentResiduals );

        /*printf( "res0: " );
        for( int i = 0; i < currentResiduals.size(); i++ )
            printf( "%.10f ", currentResiduals[i] );
        printf( "\n" );*/

        // Outer loop.
        lmPar = 0;
        bool firstIteration = true;
        int  iter = 0;
        //final ConvergenceChecker<PointVectorValuePair> checker = getConvergenceChecker();
        //void* checker = nullptr;

        while( true )
        {
            ++iter;
            const PointVectorValuePair<double> previous = current;

            // QR decomposition of the jacobian matrix
            if( ! qrDecomposition( computeWeightedJacobian( currentPoint ) ) )
                return false;

            weightedResidual = weightMatrixSqrt.operate( currentResiduals );
            for( int i = 0; i < nR; i++ )
            {
                qtf[i] = weightedResidual[i];
            }

            // compute Qt.res
            qTy( qtf );

            // now we don't need Q anymore,
            // so let jacobian contain the R matrix with its diagonal elements
            for( int k = 0; k < solvedCols; ++k )
            {
                int pk = permutation[k];
                weightedJacobian[k][pk] = diagR[pk];
            }

            if( firstIteration )
            {
                // scale the point according to the norms of the columns
                // of the initial jacobian
                xNorm = 0;
                for( int k = 0; k < nC; ++k )
                {
                    double dk = jacNorm[k];
                    if( dk == 0 )
                    {
                        dk = 1.0;
                    }
                    double xk = dk * currentPoint[k];
                    xNorm += xk * xk;
                    diag[k] = dk;
                }
                xNorm = sqrt( xNorm );

                // initialize the step bound delta
                delta = ( xNorm == 0 ) ? initialStepBoundFactor :
                                         ( initialStepBoundFactor * xNorm );
            }

            // check orthogonality between function vector and jacobian columns
            double maxCosine = 0;
            if( currentCost != 0 )
            {
                for( int j = 0; j < solvedCols; ++j )
                {
                    int    pj = permutation[j];
                    double s = jacNorm[pj];
                    if( s != 0 )
                    {
                        double sum = 0;
                        for( int i = 0; i <= j; ++i )
                        {
                            sum += weightedJacobian[i][pj] * qtf[i];
                        }
                        maxCosine = std::max( maxCosine, fabs( sum ) / ( s * currentCost ) );
                    }
                }
            }
            if( maxCosine <= orthoTolerance )
            {
                // Convergence has been reached.
                setCost( currentCost );
                lastFit = current;
                return true;
            }

            // rescale if necessary
            for( int j = 0; j < nC; ++j )
            {
                diag[j] = std::max( diag[j], jacNorm[j] );
            }

            // Inner loop.
            for( double ratio = 0; ratio < 1.0e-4; )
            {

                // save the state
                for( int j = 0; j < solvedCols; ++j )
                {
                    int pj = permutation[j];
                    oldX[pj] = currentPoint[pj];
                }
                const double    previousCost = currentCost;
                Array1D<double> tmpVec = weightedResidual;
                weightedResidual = oldRes;
                oldRes = tmpVec;
                tmpVec = currentObjective;
                currentObjective = oldObj;
                oldObj = tmpVec;

                // determine the Levenberg-Marquardt parameter
                determineLMParameter( qtf, delta, diag, work1, work2, work3 );

                // compute the new point and the norm of the evolution direction
                double lmNorm = 0;
                for( int j = 0; j < solvedCols; ++j )
                {
                    int pj = permutation[j];
                    lmDir[pj] = -lmDir[pj];
                    currentPoint[pj] = oldX[pj] + lmDir[pj];
                    double s = diag[pj] * lmDir[pj];
                    lmNorm += s * s;
                }
                lmNorm = sqrt( lmNorm );
                // on the first iteration, adjust the initial step bound.
                if( firstIteration )
                {
                    delta = std::min( delta, lmNorm );
                }

                // Evaluate the function at x + p and calculate its norm.
                currentObjective = computeObjectiveValue( currentPoint );
                currentResiduals = computeResiduals( currentObjective );
                current = PointVectorValuePair<double>( currentPoint, currentObjective );
                currentCost = computeCost( currentResiduals );

                // compute the scaled actual reduction
                double actRed = -1.0;
                if( 0.1 * currentCost < previousCost )
                {
                    double r = currentCost / previousCost;
                    actRed = 1.0 - r * r;
                }

                // compute the scaled predicted reduction
                // and the scaled directional derivative
                for( int j = 0; j < solvedCols; ++j )
                {
                    int    pj = permutation[j];
                    double dirJ = lmDir[pj];
                    work1[j] = 0;
                    for( int i = 0; i <= j; ++i )
                    {
                        work1[i] += weightedJacobian[i][pj] * dirJ;
                    }
                }
                double coeff1 = 0;
                for( int j = 0; j < solvedCols; ++j )
                {
                    coeff1 += work1[j] * work1[j];
                }
                double pc2 = previousCost * previousCost;
                coeff1 /= pc2;
                double coeff2 = lmPar * lmNorm * lmNorm / pc2;
                double preRed = coeff1 + 2 * coeff2;
                double dirDer = -( coeff1 + coeff2 );

                // ratio of the actual to the predicted reduction
                ratio = ( preRed == 0 ) ? 0 : ( actRed / preRed );

                // update the step bound
                if( ratio <= 0.25 )
                {
                    double tmp =
                            ( actRed < 0 ) ? ( 0.5 * dirDer / ( dirDer + 0.5 * actRed ) ) : 0.5;
                    if( ( 0.1 * currentCost >= previousCost ) || ( tmp < 0.1 ) )
                    {
                        tmp = 0.1;
                    }
                    delta = tmp * std::min( delta, 10.0 * lmNorm );
                    lmPar /= tmp;
                }
                else if( ( lmPar == 0 ) || ( ratio >= 0.75 ) )
                {
                    delta = 2 * lmNorm;
                    lmPar *= 0.5;
                }

                // test for successful iteration.
                if( ratio >= 1.0e-4 )
                {
                    // successful iteration, update the norm
                    firstIteration = false;
                    xNorm = 0;
                    for( int k = 0; k < nC; ++k )
                    {
                        double xK = diag[k] * currentPoint[k];
                        xNorm += xK * xK;
                    }
                    xNorm = sqrt( xNorm );

#if 0
                        // tests for convergence.
                        if (checker != null && checker.converged(iter, previous, current)) {
                            setCost(currentCost);
                            return current;
                        }
#endif
                }
                else
                {
                    // failed iteration, reset the previous values
                    currentCost = previousCost;
                    for( int j = 0; j < solvedCols; ++j )
                    {
                        int pj = permutation[j];
                        currentPoint[pj] = oldX[pj];
                    }
                    tmpVec = weightedResidual;
                    weightedResidual = oldRes;
                    oldRes = tmpVec;
                    tmpVec = currentObjective;
                    currentObjective = oldObj;
                    oldObj = tmpVec;
                    // Reset "current" to previous values.
                    current = PointVectorValuePair<double>( currentPoint, currentObjective );
                }

                // Default convergence criteria.
                if( ( fabs( actRed ) <= costRelativeTolerance && preRed <= costRelativeTolerance
                            && ratio <= 2.0 )
                        || delta <= parRelativeTolerance * xNorm )
                {
                    setCost( currentCost );
                    lastFit = current;
                    return true;
                }

                // tests for termination and stringent tolerances
                // (2.2204e-16 is the machine epsilon for IEEE754)
                if( ( fabs( actRed ) <= 2.2204e-16 ) && ( preRed <= 2.2204e-16 )
                        && ( ratio <= 2.0 ) )
                {
                    m_failureReason = "TOO_SMALL_COST_RELATIVE_TOLERANCE";
                    return false;
                }
                else if( delta <= 2.2204e-16 * xNorm )
                {
                    m_failureReason = "TOO_SMALL_PARAMETERS_RELATIVE_TOLERANCE";
                    return false;
                }
                else if( maxCosine <= 2.2204e-16 )
                {
                    m_failureReason = "TOO_SMALL_ORTHOGONALITY_TOLERANCE";
                    return false;
                }
            }
        }
    }

private:
    /**
         * Solve a*x = b and d*x = 0 in the least squares sense.
         * <p>This implementation is a translation in Java of the MINPACK
         * <a href="http://www.netlib.org/minpack/qrsolv.f">qrsolv</a>
         * routine.</p>
         * <p>This method sets the lmDir and lmDiag attributes.</p>
         * <p>The authors of the original fortran function are:</p>
         * <ul>
         *   <li>Argonne National Laboratory. MINPACK project. March 1980</li>
         *   <li>Burton  S. Garbow</li>
         *   <li>Kenneth E. Hillstrom</li>
         *   <li>Jorge   J. More</li>
         * </ul>
         * <p>Luc Maisonobe did the Java translation.</p>
         *
         * @param qy array containing qTy
         * @param diag diagonal matrix
         * @param lmDiag diagonal elements associated with lmDir
         * @param work work array
         */
    void determineLMDirection( Array1D<double>& qy, Array1D<double>& diag, Array1D<double>& lmDiag,
            Array1D<double>& work )
    {

        // copy R and Qty to preserve input and initialize s
        //  in particular, save the diagonal elements of R in lmDir
        for( int j = 0; j < solvedCols; ++j )
        {
            int pj = permutation[j];
            for( int i = j + 1; i < solvedCols; ++i )
            {
                weightedJacobian[i][pj] = weightedJacobian[j][permutation[i]];
            }
            lmDir[j] = diagR[pj];
            work[j] = qy[j];
        }

        // eliminate the diagonal matrix d using a Givens rotation
        for( int j = 0; j < solvedCols; ++j )
        {

            // prepare the row of d to be eliminated, locating the
            // diagonal element using p from the Q.R. factorization
            int    pj = permutation[j];
            double dpj = diag[pj];
            if( dpj != 0 )
            {
                lmDiag.fill( j + 1, lmDiag.size(), 0 );
            }
            lmDiag[j] = dpj;

            //  the transformations to eliminate the row of d
            // modify only a single element of Qty
            // beyond the first n, which is initially zero.
            double qtbpj = 0;
            for( int k = j; k < solvedCols; ++k )
            {
                int pk = permutation[k];

                // determine a Givens rotation which eliminates the
                // appropriate element in the current row of d
                if( lmDiag[k] != 0 )
                {

                    double sin;
                    double cos;
                    double rkk = weightedJacobian[k][pk];
                    if( fabs( rkk ) < fabs( lmDiag[k] ) )
                    {
                        double cotan = rkk / lmDiag[k];
                        sin = 1.0 / sqrt( 1.0 + cotan * cotan );
                        cos = sin * cotan;
                    }
                    else
                    {
                        double tan = lmDiag[k] / rkk;
                        cos = 1.0 / sqrt( 1.0 + tan * tan );
                        sin = cos * tan;
                    }

                    // compute the modified diagonal element of R and
                    // the modified element of (Qty,0)
                    weightedJacobian[k][pk] = cos * rkk + sin * lmDiag[k];
                    double temp = cos * work[k] + sin * qtbpj;
                    qtbpj = -sin * work[k] + cos * qtbpj;
                    work[k] = temp;

                    // accumulate the tranformation in the row of s
                    for( int i = k + 1; i < solvedCols; ++i )
                    {
                        double rik = weightedJacobian[i][pk];
                        double temp2 = cos * rik + sin * lmDiag[i];
                        lmDiag[i] = -sin * rik + cos * lmDiag[i];
                        weightedJacobian[i][pk] = temp2;
                    }
                }
            }

            // store the diagonal element of s and restore
            // the corresponding diagonal element of R
            lmDiag[j] = weightedJacobian[j][permutation[j]];
            weightedJacobian[j][permutation[j]] = lmDir[j];
        }

        // solve the triangular system for z, if the system is
        // singular, then obtain a least squares solution
        int nSing = solvedCols;
        for( int j = 0; j < solvedCols; ++j )
        {
            if( ( lmDiag[j] == 0 ) && ( nSing == solvedCols ) )
            {
                nSing = j;
            }
            if( nSing < solvedCols )
            {
                work[j] = 0;
            }
        }
        if( nSing > 0 )
        {
            for( int j = nSing - 1; j >= 0; --j )
            {
                int    pj = permutation[j];
                double sum = 0;
                for( int i = j + 1; i < nSing; ++i )
                {
                    sum += weightedJacobian[i][pj] * work[i];
                }
                work[j] = ( work[j] - sum ) / lmDiag[j];
            }
        }

        // permute the components of z back to components of lmDir
        for( int j = 0; j < lmDir.size(); ++j )
        {
            lmDir[permutation[j]] = work[j];
        }
    }


    /**
         * Determine the Levenberg-Marquardt parameter.
         * <p>This implementation is a translation in Java of the MINPACK
         * <a href="http://www.netlib.org/minpack/lmpar.f">lmpar</a>
         * routine.</p>
         * <p>This method sets the lmPar and lmDir attributes.</p>
         * <p>The authors of the original fortran function are:</p>
         * <ul>
         *   <li>Argonne National Laboratory. MINPACK project. March 1980</li>
         *   <li>Burton  S. Garbow</li>
         *   <li>Kenneth E. Hillstrom</li>
         *   <li>Jorge   J. More</li>
         * </ul>
         * <p>Luc Maisonobe did the Java translation.</p>
         *
         * @param qy array containing qTy
         * @param delta upper bound on the euclidean norm of diagR * lmDir
         * @param diag diagonal matrix
         * @param work1 work array
         * @param work2 work array
         * @param work3 work array
         */
    void determineLMParameter( Array1D<double>& qy, double delta, Array1D<double>& diag,
            Array1D<double>& work1, Array1D<double>& work2, Array1D<double>& work3 )
    {
        int nC = weightedJacobian[0].size();

        // compute and store in x the gauss-newton direction, if the
        // jacobian is rank-deficient, obtain a least squares solution
        for( int j = 0; j < rank; ++j )
        {
            lmDir[permutation[j]] = qy[j];
        }
        for( int j = rank; j < nC; ++j )
        {
            lmDir[permutation[j]] = 0;
        }
        for( int k = rank - 1; k >= 0; --k )
        {
            int    pk = permutation[k];
            double ypk = lmDir[pk] / diagR[pk];
            for( int i = 0; i < k; ++i )
            {
                lmDir[permutation[i]] -= ypk * weightedJacobian[i][pk];
            }
            lmDir[pk] = ypk;
        }

        // evaluate the function at the origin, and test
        // for acceptance of the Gauss-Newton direction
        double dxNorm = 0;
        for( int j = 0; j < solvedCols; ++j )
        {
            int    pj = permutation[j];
            double s = diag[pj] * lmDir[pj];
            work1[pj] = s;
            dxNorm += s * s;
        }
        dxNorm = sqrt( dxNorm );
        double fp = dxNorm - delta;
        if( fp <= 0.1 * delta )
        {
            lmPar = 0;
            return;
        }

        // if the jacobian is not rank deficient, the Newton step provides
        // a lower bound, parl, for the zero of the function,
        // otherwise set this bound to zero
        double sum2;
        double parl = 0;
        if( rank == solvedCols )
        {
            for( int j = 0; j < solvedCols; ++j )
            {
                int pj = permutation[j];
                work1[pj] *= diag[pj] / dxNorm;
            }
            sum2 = 0;
            for( int j = 0; j < solvedCols; ++j )
            {
                int    pj = permutation[j];
                double sum = 0;
                for( int i = 0; i < j; ++i )
                {
                    sum += weightedJacobian[i][pj] * work1[permutation[i]];
                }
                double s = ( work1[pj] - sum ) / diagR[pj];
                work1[pj] = s;
                sum2 += s * s;
            }
            parl = fp / ( delta * sum2 );
        }

        // calculate an upper bound, paru, for the zero of the function
        sum2 = 0;
        for( int j = 0; j < solvedCols; ++j )
        {
            int    pj = permutation[j];
            double sum = 0;
            for( int i = 0; i <= j; ++i )
            {
                sum += weightedJacobian[i][pj] * qy[i];
            }
            sum /= diag[pj];
            sum2 += sum * sum;
        }
        double gNorm = sqrt( sum2 );
        double paru = gNorm / delta;
        if( paru == 0 )
        {
            // 2.2251e-308 is the smallest positive real for IEE754
            paru = 2.2251e-308 / std::min( delta, 0.1 );
        }

        // if the input par lies outside of the interval (parl,paru),
        // set par to the closer endpoint
        lmPar = std::min( paru, std::max( lmPar, parl ) );
        if( lmPar == 0 )
        {
            lmPar = gNorm / dxNorm;
        }

        for( int countdown = 10; countdown >= 0; --countdown )
        {

            // evaluate the function at the current value of lmPar
            if( lmPar == 0 )
            {
                lmPar = std::max( 2.2251e-308, 0.001 * paru );
            }
            double sPar = sqrt( lmPar );
            for( int j = 0; j < solvedCols; ++j )
            {
                int pj = permutation[j];
                work1[pj] = sPar * diag[pj];
            }
            determineLMDirection( qy, work1, work2, work3 );

            dxNorm = 0;
            for( int j = 0; j < solvedCols; ++j )
            {
                int    pj = permutation[j];
                double s = diag[pj] * lmDir[pj];
                work3[pj] = s;
                dxNorm += s * s;
            }
            dxNorm = sqrt( dxNorm );
            double previousFP = fp;
            fp = dxNorm - delta;

            // if the function is small enough, accept the current value
            // of lmPar, also test for the exceptional cases where parl is zero
            if( ( fabs( fp ) <= 0.1 * delta )
                    || ( ( parl == 0 ) && ( fp <= previousFP ) && ( previousFP < 0 ) ) )
            {
                return;
            }

            // compute the Newton correction
            for( int j = 0; j < solvedCols; ++j )
            {
                int pj = permutation[j];
                work1[pj] = work3[pj] * diag[pj] / dxNorm;
            }
            for( int j = 0; j < solvedCols; ++j )
            {
                int pj = permutation[j];
                work1[pj] /= work2[j];
                double tmp = work1[pj];
                for( int i = j + 1; i < solvedCols; ++i )
                {
                    work1[permutation[i]] -= weightedJacobian[i][pj] * tmp;
                }
                sum2 = 0;
            }
            for( int j = 0; j < solvedCols; ++j )
            {
                double s = work1[permutation[j]];
                sum2 += s * s;
            }
            double correction = fp / ( delta * sum2 );

            // depending on the sign of the function, update parl or paru.
            if( fp > 0 )
            {
                parl = std::max( parl, lmPar );
            }
            else if( fp < 0 )
            {
                paru = std::min( paru, lmPar );
            }

            // compute an improved estimate for lmPar
            lmPar = std::max( parl, lmPar + correction );
        }
    }
};


class LM_SOLVER_IMPL : public LevenbergMarquardtOptimizer
{
    int m_m, m_n;

public:
    LM_SOLVER_IMPL()
    {
        m_m = 0;
        m_n = 0;
    }

    virtual ~LM_SOLVER_IMPL() {};

    void AddParameter( LM_PARAMETER* aParam )
    {
        m_params.push_back( aParam );
        //printf("mm %d\n", m_m);
        aParam->m_paramIndex = m_m;
        m_m += aParam->LmGetDimension();
    }

    void AddEquation( LM_EQUATION* aEqn )
    {
        aEqn->m_eqnIndex = m_n;
        m_n += aEqn->LmGetEquationCount();
        m_equations.push_back( aEqn );
    }

    bool Solve();

    int GetEquationCount() const
    {
        return m_equations.size();
    }

    int GetParameterCount() const
    {
        return m_params.size();
    }


private:
    int totalParams() const
    {
        int nC = 0;
        for( auto param : m_params )
            nC += param->LmGetDimension();
        return nC;
    }

    int totalEqns() const
    {
        int nC = 0;
        for( auto eqn : m_equations )
            nC += eqn->LmGetEquationCount();
        return nC;
    }


    virtual const Matrix<double> computeJacobian( const Array1D<double>& params ) const override
    {
        int nC = totalParams();
        int nR = totalEqns();

        Matrix<double> jac( nR, nC );

        jac.zero();

        int pindex = 0;

        for( auto param : m_params )
        {
            for( int i = 0; i < param->LmGetDimension(); i++ )
            {
                param->LmSetValue( i, params[pindex] );
                pindex++;
            }
        }


        for( auto eqn : m_equations )
        {
            for( int p = 0; p < eqn->LmGetEquationCount(); p++ )
            {

                eqn->LmDFunc( jac[eqn->m_eqnIndex + p].getData(), p );
            }
        }

        //printf( "jac %.10f %.10f %.10f %.10f\n", jac[0][0], jac[0][1], jac[1][0], jac[1][1] );

        return jac;
    }


    virtual const Array1D<double> computeObjectiveValue(
            const Array1D<double>& params ) const override
    {
        int pindex = 0;
        //printf( " ComputeObjective : %d eqns\n", totalEqns() );
        Array1D<double> rv( totalEqns() );

        for( auto param : m_params )
        {
            for( int i = 0; i < param->LmGetDimension(); i++ )
            {
                //printf( "set param %d %.10f\n", pindex, params[pindex] );
                param->LmSetValue( i, params[pindex] );
                pindex++;
            }
        }


        for( auto eqn : m_equations )
        {
            //printf( "call index %d\n", eqn->m_eqnIndex );
            eqn->LmFunc( rv.getData() + eqn->m_eqnIndex );
        }
        return rv;
    }

    virtual Array1D<double> getTarget() const override
    {
        Array1D<double> rv( totalEqns() );
        for( int i = 0; i < totalEqns(); i++ )
            rv[i] = 0.0;
        return rv;
    }

    virtual Array1D<double> getStartPoint() const override
    {
        int pindex = 0;

        Array1D<double> rv( totalParams() );

        for( auto param : m_params )
        {
            for( int i = 0; i < param->LmGetDimension(); i++ )
            {
                rv[pindex] = param->LmGetInitialValue( i );
                pindex++;
            }
        }
        return rv;
    }


    std::vector<LM_EQUATION*>  m_equations;
    std::vector<LM_PARAMETER*> m_params;
};

bool LM_SOLVER_IMPL::Solve()
{
    int  pindex;
    auto result = doOptimize();


    if( !result )
    {
        printf("LM Fail: '%s'\n", m_failureReason.c_str() );
        return false;
    }

    pindex = 0;

    for( auto param : m_params )
    {
        for( int i = 0; i < param->LmGetDimension(); i++ )
        {
            param->LmSetValue( i, lastFit.point()[pindex] );
            //printf( "Param %d: %.10f \n", pindex, param->LmGetValue( i ) );
            pindex++;
        }
    }

    return true;
}

LM_SOLVER::LM_SOLVER()
{
    m_pimpl.reset( new LM_SOLVER_IMPL );
}

void LM_SOLVER::AddParameter( LM_PARAMETER* aParam )
{
    m_pimpl->AddParameter( aParam );
}

void LM_SOLVER::AddEquation( LM_EQUATION* aEqn )
{
    m_pimpl->AddEquation( aEqn );
}

bool LM_SOLVER::Solve()
{
    return m_pimpl->Solve();
}

int LM_SOLVER::GetEquationCount() const
{
    return m_pimpl->GetEquationCount();
}

int LM_SOLVER::GetParameterCount() const
{
    return m_pimpl->GetParameterCount();
}


#if 0
struct MyVec : public LM_PARAMETER
{
    virtual int LmGetDimension() override { return 2; }
    virtual void LmSetValue( int index, double value ) override
    {
        if(index == 1) y = value; else x=value;
    }
    
    virtual double LmGetValue( int index ) override
    {
        if(index ==1 ) return y; else return x;
    }
    
    virtual double LmGetInitialValue( int index ) override
    {
        return 0.0;
    }


    double x, y;
};

struct  MyEqn : public LM_EQUATION
{
    MyEqn(MyVec *v) :m_v(v) {}
    
    virtual int LmGetEquationCount() override { return 2; }

    virtual void LmFunc( double *x ) override
    {
        x[0] = 3* m_v->x + 2*m_v->y + 0.5;
        x[1] = m_v->x * m_v->x + m_v->y * m_v->y - 4;
        printf("err %.10f %.10f\n", x[0], x[1]);
    }

    virtual void LmDFunc( /*double *params,*/ double *dx, int equationIndex ) override
    {
        if(equationIndex == 0)
        {
            dx[0] = 3;
            dx[1] = 2;
        } else {
            dx[0] = 2*m_v->x;
            dx[1] = 2*m_v->y;
        }
    }
    MyVec *m_v;
};
#endif

#if 0
main()
{

    LM_SOLVER opt;
    MyVec d;
    opt.AddEquation( new MyEqn(&d ));
    opt.AddParameter( &d );
    opt.Solve();


    return 0;
}
#endif