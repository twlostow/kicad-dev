#include <cstdio>
#include <cmath>

#include <levmar.h>

#include "levenberg_marquardt.h"

//template<>
void LM_SOLVER::init()
{

}

void LM_SOLVER::evaluateFunc( double *p, double *hx, int m, int n, void *adata)
{
    auto self = reinterpret_cast<LM_SOLVER*> ( adata );
    printf("evalf: %p %p %d %d %p\n", p,hx, m, n, adata);
    int pindex = 0;

    for( auto param : self->m_params )
    {
        for( int i = 0; i < param->LmGetDimension(); i++ )
        {
            param->LmSetValue(i, p[pindex++]);
        }
    }

    for( auto eqn : self->m_equations )
    {
        eqn->LmFunc( *self->m_params.data() , hx + eqn->m_eqnIndex );
    }
}

void LM_SOLVER::evaluateJacobian( double *p, double *jac, int m, int n, void *adata)
{
    auto self = reinterpret_cast<LM_SOLVER*> ( adata );
    int pindex = 0;

    for( auto param : self->m_params )
    {
        for( int i = 0; i < param->LmGetDimension(); i++ )
        {
            param->LmSetValue(i, p[pindex++]);
        }
    }

    
    for( auto eqn : self->m_equations )
    {
        for( int i = 0; i < n; i++ ) 
            jac [ m * eqn->m_eqnIndex + i ] = 0.0;

        eqn->LmDFunc( *self->m_params.data() , jac + m * eqn->m_eqnIndex );
    }
}

bool LM_SOLVER::Solve()
{
    
    m_workmem = new double [ LM_DER_WORKSZ(m_m, m_n) ];

    double *p_vec = new double[ m_m ];
    double *x_vec = new double[ m_n ];
    int pindex = 0;

    for( auto param : m_params )
    {
        for( int i = 0; i < param->LmGetDimension(); i++ )
        {
            p_vec[pindex++] = param->LmGetValue( i );
        }
    }

    for ( int i = 0; i < m_n; i++ )
        x_vec[i] = 0.0;

    auto rv = dlevmar_der(
        evaluateFunc,
        evaluateJacobian,
        p_vec,
        x_vec,
        m_m,
        m_n,
        m_itmax,
        nullptr,
        nullptr,
        m_workmem,
        nullptr,
        this );


    printf("rv %d\n", rv);

    delete p_vec;
    delete x_vec;
    delete m_workmem;

    return true;
}
