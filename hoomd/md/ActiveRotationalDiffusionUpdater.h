// Copyright (c) 2009-2021 The Regents of the University of Michigan
// This file is part of the HOOMD-blue project, released under the BSD 3-Clause License.

/*! \file ActiveRotationalDiffusionUpdater.h
    \brief Declares an updater that actively diffuses particle orientations
*/

#ifdef __HIPCC__
#error This header cannot be compiled by nvcc
#endif

#include "ActiveForceCompute.h"
#include "hoomd/Updater.h"
#include "hoomd/Variant.h"

#include <memory>
#include <pybind11/pybind11.h>

#pragma once

/// Updates particle's orientations based on a given diffusion constant.
/** The updater accepts a variant rotational diffusion and updates the particle orientations of the
 * associated ActiveForceCompute's group (by calling m_active_force.rotationalDiffusion).
 *
 * Note: This was originally part of the ActiveForceCompute, and is separated to obey the idea that
 * force computes do not update the system directly, but updaters do. See GitHub issue (898). The
 * updater is just a shell that calls through to m_active_force due to the complexities of the logic
 * with the introduction of manifolds.
 *
 * If anyone has the time to do so, the implementation would be cleaner if moved to this updater.
 */
class PYBIND11_EXPORT ActiveRotationalDiffusionUpdater : public Updater
    {
    public:
    /// Constructor
    ActiveRotationalDiffusionUpdater(std::shared_ptr<SystemDefinition> sysdef,
                                     std::shared_ptr<Variant> rotational_diffusion,
                                     std::shared_ptr<ActiveForceCompute> active_force);

    /// Destructor
    virtual ~ActiveRotationalDiffusionUpdater();

    /// Get the rotational diffusion
    std::shared_ptr<Variant>& getRotationalDiffusion()
        {
        return m_rotational_diffusion;
        }

    /// Get the final box
    void setRotationalDiffusion(std::shared_ptr<Variant>& new_diffusion)
        {
        m_rotational_diffusion = new_diffusion;
        }

    /// Update box interpolation based on provided timestep
    virtual void update(uint64_t timestep);

    private:
    std::shared_ptr<Variant>
        m_rotational_diffusion; //!< Variant that determines the current rotational diffusion
    std::shared_ptr<ActiveForceCompute>
        m_active_force; //!< Active force to call rotationalDiffusion on
    };

/// Export the ActiveRotationalDiffusionUpdater to python
void export_ActiveRotationalDiffusionUpdater(pybind11::module& m);
