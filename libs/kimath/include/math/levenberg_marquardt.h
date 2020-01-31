/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2018-2019 CERN
 * Author: Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef __LEVENBERG_MARQUARDT_H
#define __LEVENBERG_MARQUARDT_H

#include <functional>
#include <memory>
#include <vector>

class LM_SOLVER_IMPL;

class LM_PARAMETER 
{
    friend class LM_SOLVER;
    friend class LM_SOLVER_IMPL;

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
    friend class LM_SOLVER_IMPL;

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
    LM_SOLVER();


    void AddParameter ( LM_PARAMETER *aParam );
    void AddEquation( LM_EQUATION* aEqn );
    bool Solve();
    int GetEquationCount() const;
    int GetParameterCount() const;
private:
    std::shared_ptr<LM_SOLVER_IMPL> m_pimpl;
};


#endif