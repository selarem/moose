#ifndef PRINTDOFS_H
#define PRINTDOFS_H

#include "GeneralPostprocessor.h"

//Forward Declarations
class PrintDOFs;

template<>
InputParameters validParams<PrintDOFs>();

class PrintDOFs : public GeneralPostprocessor
{
public:
  PrintDOFs(const std::string & name, InputParameters parameters);
  
  virtual void initialize() {}
  virtual void execute() {}

  /**
   * This will return the degrees of freedom in the system.
   */
  virtual Real getValue();
};

#endif //PRINTDOFS_H
