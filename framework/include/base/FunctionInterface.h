#ifndef FUNCTIONINTERFACE_H
#define FUNCTIONINTERFACE_H

#include "InputParameters.h"
#include "ParallelUniqueId.h"

class SubProblem;
class Function;

/**
 * Inherit from this class at a very low level to make the getFunction method
 * available.
 */
class FunctionInterface
{
public:
  /**
   * @param func_warehouse Reference to the FunctionWarehouse stored by MooseSystem
   * @param params The parameters used by the object being instantiated. This
   *        class needs them so it can get the function named in the input file,
   *        but the object calling getFunction only needs to use the name on the
   *        left hand side of the statement "function = func_name"
   * @param tid The thread id used by this object, used by the warehouse
   */
  FunctionInterface(InputParameters & params);

  Function & getFunction(const std::string & name);

private:
  SubProblem & _problem;
  THREAD_ID _tid;
  InputParameters _params;
};

#endif //FUNCTIONINTERFACE_H_
