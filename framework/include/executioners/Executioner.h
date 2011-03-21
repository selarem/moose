#ifndef EXECUTIONER_H
#define EXECUTIONER_H

#include "MooseObject.h"

// System includes
#include <string>

class MooseMesh;
class Problem;

/**
 * Executioners are objects that do the actual work of solving your problem.
 *
 * In general there are two "sets" of Executioners: Steady and Transient.
 *
 * The Difference is that a Steady Executioner usually only calls "solve()"
 * for the NonlinearSystem once... where Transient Executioners call solve()
 * multiple times... i.e. once per timestep.
 */
class Executioner : public MooseObject
{
public:
  /**
   * Constructor
   *
   * @param name The name given to the Executioner in the input file.
   * @param parameters The parameters object holding data for the class to use.
   * @return Whether or not the solve was successful.
   */
  Executioner(const std::string & name, InputParameters parameters);

  virtual ~Executioner();

  /**
   * Pure virtual execute function MUST be overriden by children classes.
   * This is where the Executioner actually does it's work.
   */
  virtual void execute() = 0;

  virtual Problem & problem() = 0;

  void outputInitial(bool out_init) { _output_initial = out_init; }

protected:
  MooseMesh * _mesh;

  bool _output_initial;				/// output initial condition if true   


  /**
   * Override this for actions that should take place before execution
   */
  virtual void preExecute();

  /**
   * Override this for actions that should take place after execution
   */
  virtual void postExecute();

  /**
   * Override this for actions that should take place before execution
   */
  virtual void preSolve();

  /**
   * Override this for actions that should take place after execution
   */
  virtual void postSolve();
};


template<>
InputParameters validParams<Executioner>();

#endif //EXECUTIONER_H_
