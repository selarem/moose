/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "FauxGrainTracker.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<FauxGrainTracker>()
{
  InputParameters params = validParams<GrainTrackerInterface>();
  params.addClassDescription("Fake grain tracker object for cases where the number of grains is equal to the number of order parameters.");

  return params;
}


FauxGrainTracker::FauxGrainTracker(const InputParameters & parameters) :
    FeatureFloodCount(parameters),
    GrainTrackerInterface(),
    _tracking_step(getParam<int>("tracking_step"))
{
  // initialize faux data with identity map
  _faux_data.resize(_vars.size());
  for (unsigned int var_num = 0; var_num < _faux_data.size(); ++var_num)
    _faux_data[var_num] = std::make_pair(var_num, var_num);

  _faux_data_2.resize(_vars.size());
  for (unsigned int var_num = 0; var_num < _faux_data_2.size(); ++var_num)
    _faux_data_2[var_num] = var_num;
}

FauxGrainTracker::~FauxGrainTracker()
{
}

Real
FauxGrainTracker::getEntityValue(dof_id_type entity_id, FeatureFloodCount::FieldType field_type, unsigned int var_idx) const
{
  auto use_default = false;
  if (var_idx == std::numeric_limits<unsigned int>::max())
  {
    use_default = true;
    var_idx = 0;
  }

  mooseAssert(var_idx < _vars.size(), "Index out of range");

  switch (field_type)
  {
    case FieldType::UNIQUE_REGION:
    case FieldType::VARIABLE_COLORING:
    {
      auto entity_it = _entity_id_to_var_num.find(entity_id);

      if (entity_it != _entity_id_to_var_num.end())
        return entity_it->second;
      else
        return -1;
      break;
    }

    case FieldType::CENTROID:
    {
      if (_periodic_node_map.size())
        mooseDoOnce(mooseWarning("Centroids are not correct when using periodic boundaries, contact the MOOSE team"));

      // If this element contains the centroid of one of features, return it's index
      const auto * elem_ptr = _mesh.elemPtr(entity_id);
      for (unsigned int var_num = 0; var_num < _vars.size(); ++var_num)
      {
        const auto centroid = _centroid.find(var_num);
        if (centroid != _centroid.end())
          if (elem_ptr->contains_point(centroid->second))
            return 1;
      }

      return 0;
    }

    // We don't want to error here because this should be a drop in replacement for the real grain tracker.
    // Instead we'll just return zero and continue
    default:
      return 0;
  }

  return 0;
}

const std::vector<std::pair<unsigned int, unsigned int> > &
FauxGrainTracker::getElementalValues(dof_id_type /*elem_id*/) const
{
  mooseDeprecated("GrainTrackerInterface::getElementalValues() is deprecated use GrainTrackerInterface::getOpToGrainsVector() instead");

  return _faux_data;
}

const std::vector<unsigned int> &
FauxGrainTracker::getOpToGrainsVector(dof_id_type /*elem_id*/) const
{
  return _faux_data_2;
}

unsigned int
FauxGrainTracker::getNumberGrains() const
{
  return _variables_used.size();
}

Real
FauxGrainTracker::getGrainVolume(unsigned int grain_id) const
{
  const auto grain_volume = _volume.find(grain_id);
  mooseAssert(grain_volume != _volume.end(), "Grain " << grain_id << " does not exist in data structure");

  return grain_volume->second;
}

Point
FauxGrainTracker::getGrainCentroid(unsigned int grain_id) const
{
  const auto grain_center = _centroid.find(grain_id);
  mooseAssert(grain_center != _centroid.end(), "Grain " << grain_id << " does not exist in data structure");

  return grain_center->second;
}

void
FauxGrainTracker::initialize()
{
  _entity_id_to_var_num.clear();
  _variables_used.clear();
  if (_is_elemental)
  {
    _volume.clear();
    _vol_count.clear();
    _centroid.clear();
  }
}

void
FauxGrainTracker::execute()
{
  Moose::perf_log.push("execute()", "FauxGrainTracker");

  const MeshBase::element_iterator end = _mesh.getMesh().active_local_elements_end();
  for (MeshBase::element_iterator el = _mesh.getMesh().active_local_elements_begin(); el != end; ++el)
  {
    const Elem * current_elem = *el;

    // Loop over elements or nodes and populate the data structure with the first variable with a value above a threshold
    if (_is_elemental)
    {
      std::vector<Point> centroid(1, current_elem->centroid());
      _fe_problem.reinitElemPhys(current_elem, centroid, 0);

      for (unsigned int var_num = 0; var_num < _vars.size(); ++var_num)
      {
        Number entity_value = _vars[var_num]->sln()[0];

        if ((_use_less_than_threshold_comparison && (entity_value >= _threshold))
            || (!_use_less_than_threshold_comparison && (entity_value <= _threshold)))
        {
          _entity_id_to_var_num[current_elem->id()] = var_num;
          _variables_used.insert(var_num);
          _volume[var_num] += current_elem->volume();
          _vol_count[var_num]++;
          // Sum the centroid values for now, we'll average them later
          _centroid[var_num] += current_elem->centroid();
          break;
        }
      }
    }
    else
    {
      unsigned int n_nodes = current_elem->n_vertices();
      for (unsigned int i = 0; i < n_nodes; ++i)
      {
        const Node * current_node = current_elem->get_node(i);

        for (unsigned int var_num = 0; var_num < _vars.size(); ++var_num)
        {
          Number entity_value = _vars[var_num]->getNodalValue(*current_node);
          if ((_use_less_than_threshold_comparison && (entity_value >= _threshold))
              || (!_use_less_than_threshold_comparison && (entity_value <= _threshold)))
          {
            _entity_id_to_var_num[current_node->id()] = var_num;
            _variables_used.insert(var_num);
            break;
          }
        }
      }
    }
  }

  Moose::perf_log.pop("execute()", "FauxGrainTracker");
}

void
FauxGrainTracker::finalize()
{
  Moose::perf_log.push("finalize()", "FauxGrainTracker");

  _communicator.set_union(_variables_used);
  _communicator.set_union(_entity_id_to_var_num);

  if (_is_elemental)
    for (unsigned int var_num = 0; var_num < _vars.size(); ++var_num)
    {
      /**
       * Convert elements of the maps into simple values or vector of Real.
       * libMesh's _communicator.sum() does not work on std::maps
       */
      unsigned int vol_count;
      std::vector<Real> grain_data(4);

      const auto count = _vol_count.find(var_num);
      if (count != _vol_count.end())
        vol_count = count->second;

      const auto vol = _volume.find(var_num);
      if (vol != _volume.end())
        grain_data[0] = vol->second;

      const auto centroid = _centroid.find(var_num);
      if (centroid != _centroid.end())
      {
        grain_data[1] = centroid->second(0);
        grain_data[2] = centroid->second(1);
        grain_data[3] = centroid->second(2);
      }
        // combine centers & volumes from all MPI ranks
        gatherSum(vol_count);
        gatherSum(grain_data);
        _volume[var_num] = grain_data[0];
        _centroid[var_num] = {grain_data[1], grain_data[2], grain_data[3]};
        _centroid[var_num] /= vol_count;
    }

  Moose::perf_log.pop("finalize()", "FauxGrainTracker");
}

Real
FauxGrainTracker::getValue()
{
  return _variables_used.size();
}
