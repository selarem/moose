/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// Navier-Stokes includes
#include "NSStagnationTemperatureBC.h"
#include "NS.h"

// FluidProperties includes
#include "IdealGasFluidProperties.h"

template<>
InputParameters validParams<NSStagnationTemperatureBC>()
{
  InputParameters params = validParams<NSStagnationBC>();
  params.addRequiredCoupledVar(NS::temperature, "temperature");
  params.addRequiredParam<Real>("desired_stagnation_temperature", "");
  return params;
}

NSStagnationTemperatureBC::NSStagnationTemperatureBC(const InputParameters & parameters) :
    NSStagnationBC(parameters),
    _temperature(coupledValue(NS::temperature)),
    _desired_stagnation_temperature(getParam<Real>("desired_stagnation_temperature"))
{
}

Real
NSStagnationTemperatureBC::computeQpResidual()
{
  // T_0 = T*(1 + 0.5*(gam-1)*M^2)
  Real computed_stagnation_temperature = _temperature[_qp] * (1. + 0.5 * (_fp.gamma() - 1.) * _mach[_qp] * _mach[_qp]);

  // Return the difference between the current solution's stagnation temperature
  // and the desired.  The Dirichlet condition asserts that these should be equal.
  return computed_stagnation_temperature - _desired_stagnation_temperature;
}
