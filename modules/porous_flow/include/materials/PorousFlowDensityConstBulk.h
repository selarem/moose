/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWDENSITYCONSTBULK_H
#define POROUSFLOWDENSITYCONSTBULK_H

#include "PorousFlowFluidPropertiesBase.h"

class PorousFlowDensityConstBulk;

template<>
InputParameters validParams<PorousFlowDensityConstBulk>();

/**
 * Material designed to calculate fluid density
 * from porepressure, assuming constant bulk modulus
 * for the fluid.
 */
class PorousFlowDensityConstBulk : public PorousFlowFluidPropertiesBase
{
public:
  PorousFlowDensityConstBulk(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

  /**
   * Density calucaled assuming a constant bulk modulus
   *
   * @param pressure gas pressure (Pa)
   * @return density (kg/m^3)
   */
  Real density(Real pressure) const;

  /**
   * Derivative of the density of a constant bulk density a function of
   * pressure.
   *
   * @param pressure gas pressure (Pa)
   * @return derivative of density (kg/m^3) with respect to pressure
   */
  Real dDensity_dP(Real pressure) const;

  /// density at zero porepressure
  const Real _dens0;

  /// constant bulk modulus
  const Real _bulk;

  /// Fluid phase density at the nodes
  MaterialProperty<Real> & _density_nodal;

  /// Old fluid phase density at the nodes
  MaterialProperty<Real> & _density_nodal_old;

  /// Derivative of fluid density wrt phase pore pressure at the nodes
  MaterialProperty<Real> & _ddensity_nodal_dp;

  /// Fluid phase density at the qps
  MaterialProperty<Real> & _density_qp;

  /// Derivative of fluid density wrt phase pore pressure at the qps
  MaterialProperty<Real> & _ddensity_qp_dp;
};

#endif //POROUSFLOWDENSITYCONSTBULK_H
