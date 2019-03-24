#include <cmath>
#include <cstdio>

#include "levmar-2.6/levmar.h"

#include "levenberg_marquardt.h"

//template<>
void LM_SOLVER::init()
{
}

void LM_SOLVER::evaluateFunc( double* params, double* x, int m, int n, void* adata )
{
    auto self = reinterpret_cast<LM_SOLVER*>( adata );

    int pindex = 0;

    for( auto param : self->m_params )
    {
        for( int i = 0; i < param->LmGetDimension(); i++ )
        {
            param->LmSetValue( i, params[pindex] );
            // printf("Param %d: %.10f\n", pindex, param->LmGetValue( i ) );
            pindex++;
        }
    }


    for( auto eqn : self->m_equations )
    {
        eqn->LmFunc( /*params + eqn->m_eqnIndex,*/ x + eqn->m_eqnIndex );
    }
}

void LM_SOLVER::evaluateJacobian( double* params, double* jac, int m, int n, void* adata )
{
    auto self = reinterpret_cast<LM_SOLVER*>( adata );

    for( int i = 0; i < m * n; i++ )
        jac[i] = 0.0;

    int pindex = 0;

    for( auto param : self->m_params )
    {
        //printf("** Set Param %p\n", param );
        for( int i = 0; i < param->LmGetDimension(); i++ )
        {
            param->LmSetValue( i, params[pindex] );
            // printf("Param %d: %.10f\n", pindex, param->LmGetValue( i ) );
            pindex++;
        }
    }


    for( auto eqn : self->m_equations )
    {
        //printf( "** Do eqn %p [%d]\n", eqn, eqn->LmGetEquationCount() );

        for( int p = 0; p < eqn->LmGetEquationCount(); p++ )
        {

            //printf( "Subeqn %d\n", p );
            //int jac_index = m * ( eqn->m_eqnIndex + p );
            eqn->LmDFunc( &jac[m * ( eqn->m_eqnIndex + p )], p );

            //printf( "jacobian: \n" );
            for( int j = 0; j < m; j++ )
            {
                for( int i = 0; i < n; i++ )
                {
              //      printf( "%-10.3f", jac[j * m + i] );
                }
               // printf( "\n" );
            }
        }
    }

    for( int j = 0; j < m; j++ )
    {
        for( int i = 0; i < n; i++ )
        {
//            printf( "%-10.3f", jac[j * m + i] );
        }
  //      printf( "\n" );
    }
}

bool LM_SOLVER::Solve()
{

    m_workmem = new double[LM_DER_WORKSZ( m_m, m_n )];

    double* p_vec = new double[m_m * 10];
    double* x_vec = new double[m_n * 10];
    int     pindex = 0;


    for( auto param : m_params )
    {
        for( int i = 0; i < param->LmGetDimension(); i++ )
        {
            //printf( "Param %d: %.1f\n", pindex, param->LmGetValue( i ) );
            p_vec[pindex++] = param->LmGetValue( i );
        }
    }

    //printf( "Solve[%p]: params %d meas %d\n", this, m_m, m_n );

    for( int i = 0; i < m_n; i++ )
        x_vec[i] = 0.0;

    auto rv = dlevmar_der( evaluateFunc, evaluateJacobian, p_vec, x_vec, m_m, m_n, m_itmax, nullptr,
            nullptr, m_workmem, nullptr, this );


    //printf( "rv %d\n", rv );


    if( rv < 0 )
    {
      //  printf( "Solve failed.\n" );
        rv = false;
    }
    else
    {

        pindex = 0;

        for( auto param : m_params )
        {
            for( int i = 0; i < param->LmGetDimension(); i++ )
            {
                param->LmSetValue( i, p_vec[pindex] );
                //printf( "Param %d: %.10f %.10f\n", pindex, p_vec[pindex], param->LmGetValue( i ) );
                pindex++;
            }
        }
        rv = true;
    }

    delete[] p_vec;
    delete[] x_vec;
    delete[] m_workmem;

    return rv;
}
