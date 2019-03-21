#ifndef __LEVENBERG_MARQUARDT_H
#define __LEVENBERG_MARQUARDT_H

#include <functional>
#include <vector>

class LM_PARAMETER 
{
    friend class LM_SOLVER;

    public:
        virtual int LmGetDimension() = 0;
        virtual void LmSetValue( int index, double value ) = 0;
        virtual double LmGetValue( int index ) = 0;

        int LmGetIndex() const { printf("[lmGetIndex %p %d]\n", this, m_paramIndex ); return m_paramIndex; };

    protected:
        int m_paramIndex;
};

class LM_SOLVABLE
{
    friend class LM_SOLVER;

    public:
        //virtual int LmGetParameterCount() = 0;
        virtual int LmGetEquationCount() = 0;
        virtual void LmFunc( /*double *params,*/ double *x ) = 0;
        virtual void LmDFunc( /*double *params,*/ double *dx, int equationIndex ) = 0;

    protected:
        int m_eqnIndex;
};

class LM_SOLVER
{
public:
//    typedef void* USER_DATA;
//    typedef std::function<double(double *p, int index, int dindex, USER_DATA*)> DFUNC;
//    typedef std::function<double(double *p, int index, USER_DATA*)> FUNC;

    LM_SOLVER( int itmax_ = 100 )
    {
        m_itmax = itmax_;
        m_m = 0;
        m_n = 0;
        init();
    }

    void AddParameter ( LM_PARAMETER *aParam )
    {
        m_params.push_back( aParam );
        //printf("mm %d\n", m_m);
        aParam->m_paramIndex = m_m;
        printf("addParam %p dx %d\n", aParam, aParam->m_paramIndex);
        m_m += aParam->LmGetDimension();
    }

    void AddSolvable( LM_SOLVABLE* aSolvable )
    {
        aSolvable->m_eqnIndex = m_n;
        printf("solv %p eqnidx %d\n", aSolvable, aSolvable->m_eqnIndex);
        m_n += aSolvable->LmGetEquationCount();
        m_equations.push_back( aSolvable );
    }

    bool Solve();

    int GetSolvableCount() const
    {
        return m_equations.size();
    }

    int GetParameterCount() const
    {
        return m_params.size();
    }

private:

    void init();

    static void evaluateFunc( double *p, double *hx, int m, int n, void *adata);
    static void evaluateJacobian( double *p, double *j, int m, int n, void *adata);

    int m_m; // parameter vector dimension
    int m_n; // measurement vector dimension
    int m_itmax; // max number of iterations

    std::vector<LM_SOLVABLE*> m_equations;
    std::vector<LM_PARAMETER*> m_params;

    double *m_workmem;
};

#endif