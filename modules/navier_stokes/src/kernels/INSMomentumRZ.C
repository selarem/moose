/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "INSMomentumRZ.h"

template<>
InputParameters validParams<INSMomentumRZ>()
{
  InputParameters params = validParams<INSMomentum>();
  return params;
}



INSMomentumRZ::INSMomentumRZ(const InputParameters & parameters) :
  INSMomentum(parameters)
{
}


Real INSMomentumRZ::computeQpResidual()
{
  // Base class residual contribution
  Real res_base = INSMomentum::computeQpResidual();

  if (_component == 0)
  {
    const Real r = _q_point[_qp](0);

    // If this is the radial component of momentum, there is an extra term for RZ.
    res_base += 2. * _mu * _u_vel[_qp] / (r*r) * _test[_i][_qp];

    // If the pressure is also integrated by parts, there is an extra term in RZ.
    if (_integrate_p_by_parts)
      res_base += -_p[_qp] / r * _test[_i][_qp];
  }

  return res_base;
}




Real INSMomentumRZ::computeQpJacobian()
{
  // Base class jacobian contribution
  Real jac_base = INSMomentum::computeQpJacobian();

  // If this is the radial component of momentum, there is an extra term for RZ.
  if (_component == 0)
  {
    const Real r = _q_point[_qp](0);
    jac_base += 2. * _mu * _phi[_j][_qp] * _test[_i][_qp] / (r*r);
  }

  return jac_base;
}




Real INSMomentumRZ::computeQpOffDiagJacobian(unsigned jvar)
{
  // Base class jacobian contribution
  Real jac_base = INSMomentum::computeQpOffDiagJacobian(jvar);

  // If we're getting the pressure Jacobian contribution, and we
  // integrated the pressure term by parts, there is an extra term for
  // RZ.
  if (jvar == _p_var_number && _component == 0 && _integrate_p_by_parts)
  {
    const Real r = _q_point[_qp](0);
    jac_base += -_phi[_j][_qp] / r * _test[_i][_qp];
  }

  return jac_base;
}
