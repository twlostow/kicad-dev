#ifndef __LEVENBERG_MARQUARDT2_H
#define __LEVENBERG_MARQUARDT2_H

#include <functional>
#include <vector>

class LM_PARAMETER 
{
    friend class LM_SOLVER;

    public:
        virtual int LmGetDimension() = 0;
        virtual void LmSetValue( int index, double value ) = 0;
        virtual double LmGetValue( int index ) = 0;
        virtual double LmGetInitialValue( int index ) = 0;

        int LmGetIndex() const { return m_paramIndex; };

    protected:
        int m_paramIndex;
};

class LM_EQUATION
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


#endif