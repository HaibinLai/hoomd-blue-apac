// Copyright (c) 2009-2021 The Regents of the University of Michigan
// This file is part of the HOOMD-blue project, released under the BSD 3-Clause License.

#pragma once

#include "MuellerPlatheFlowEnum.h"
#include "hoomd/HOOMDMath.h"
#include "hoomd/ParticleGroup.h"
#include "hoomd/Updater.h"
#include "hoomd/Variant.h"
#include <pybind11/pybind11.h>

#include <cfloat>
#include <memory>

extern const unsigned int INVALID_TAG;
extern const Scalar INVALID_VEL;

//! By exchanging velocities based on their spatial position a flow is created.
/*! \ingroup computes
 */
class PYBIND11_EXPORT MuellerPlatheFlow : public Updater
    {
    public:
    //! Constructs the compute
    //!
    //! \param direction Indicates the normal direction of the slabs.
    //! \param N_slabs Number of total slabs in the simulation box.
    //! \param min_slabs Index of slabs, where the min velocity is searched.
    //! \param max_slabs Index of slabs, where the max velocity is searched.
    //! \note N_slabs should be a multiple of the DomainDecomposition boxes in that direction.
    //! If it is not, the number is rescaled and the user is informed.
    MuellerPlatheFlow(std::shared_ptr<SystemDefinition> sysdef,
                      std::shared_ptr<ParticleGroup> group,
                      std::shared_ptr<Variant> flow_target,
                      std::string slab_direction_str,
                      std::string flow_direction_str,
                      const unsigned int N_slabs,
                      const unsigned int min_slab,
                      const unsigned int max_slab,
                      Scalar flow_epsilon);

    //! Destructor
    virtual ~MuellerPlatheFlow(void);

    //! Take one timestep forward
    virtual void update(uint64_t timestep);

    Scalar getSummedExchangedMomentum(void) const
        {
        return m_exchanged_momentum;
        }

    unsigned int get_N_slabs(void) const
        {
        return m_N_slabs;
        }
    unsigned int get_min_slab(void) const
        {
        return m_min_slab;
        }
    unsigned int get_max_slab(void) const
        {
        return m_max_slab;
        }
    std::shared_ptr<Variant> getFlowTarget(void) const
        {
        return m_flow_target;
        }
    std::string getSlabDirection(void) const
        {
        return getStringFromDirection(m_slab_direction);
        }
    std::string getFlowDirection(void) const
        {
        return getStringFromDirection(m_flow_direction);
        }

    static std::string getStringFromDirection(const enum flow_enum::Direction direction)
        {
        if (direction == flow_enum::Direction::X)
            {
            return "X";
            }
        else if (direction == flow_enum::Direction::Y)
            {
            return "Y";
            }
        else if (direction == flow_enum::Direction::Z)
            {
            return "Z";
            }
        else
            {
            throw std::runtime_error("Direction must be X, Y, or Z");
            }
        }

    static enum flow_enum::Direction getDirectionFromString(std::string direction_str)
        {
        if (direction_str == "X")
            {
            return flow_enum::Direction::X;
            }
        else if (direction_str == "Y")
            {
            return flow_enum::Direction::Y;
            }
        else if (direction_str == "Z")
            {
            return flow_enum::Direction::Z;
            }
        else
            {
            throw std::runtime_error("Direction must be X, Y, or Z");
            }
        }

    void setMinSlab(const unsigned int slab_id);
    void setMaxSlab(const unsigned int slab_id);

    //! Determine, whether this part of the domain decomposition
    //! has particles in the min slab.
    bool hasMinSlab(void) const
        {
        return m_has_min_slab;
        }
    //! Determine, whether this part of the domain decomposition
    //! has particles in the max slab.
    bool hasMaxSlab(void) const
        {
        return m_has_max_slab;
        }

    //! Call function, if the domain decomposition has changed.
    void update_domain_decomposition(void);
    //! Get the ignored variance between flow target and summed flow.
    Scalar getFlowEpsilon(void) const
        {
        return m_flow_epsilon;
        }
    //! Get the ignored variance between flow target and summed flow.
    void setFlowEpsilon(const Scalar flow_epsilon)
        {
        m_flow_epsilon = flow_epsilon;
        }
    //! Trigger checks for orthorhombic checks.
    void force_orthorhombic_box_check(void)
        {
        m_needs_orthorhombic_check = true;
        }

    protected:
    //! Swap min and max slab for a reverse flow.
    //! More efficient than separate calls of setMinSlab() and setMaxSlab(),
    //! especially in MPI runs.
    void swap_min_max_slab(void);

    //! Group of particles, which are searched for the velocity exchange
    std::shared_ptr<ParticleGroup> m_group;

    virtual void search_min_max_velocity(void);
    virtual void update_min_max_velocity(void);

    //! Temporary variables to store last found min vel info.
    //!
    //! x: velocity y: mass z: tag as scalar.
    //! \note Transferring the mass is only necessary if velocities are updated in the ghost layer.
    //! This is only sometimes the case, but for the sake of simplicity it will be update here
    //! always. The performance loss should be only minimal.
    Scalar3 m_last_min_vel;

    //! Temporary variables to store last found max vel info
    //!
    //! x: velocity y: mass z: tag as scalar.
    //! \note Transferring the mass is only necessary if velocities are updated in the ghost layer.
    //! This is only sometimes the case, but for the sake of simplicity it will be update here
    //! always. The performance loss should be only minimal.

    Scalar3 m_last_max_vel;

    //! Direction perpendicular to the slabs.
    enum flow_enum::Direction m_slab_direction;
    //! Direction of the induced flow.
    enum flow_enum::Direction m_flow_direction;

    private:
    std::shared_ptr<Variant> m_flow_target;
    Scalar m_flow_epsilon;
    unsigned int m_N_slabs;
    unsigned int m_min_slab;
    unsigned int m_max_slab;

    Scalar m_exchanged_momentum;

    bool m_has_min_slab;
    bool m_has_max_slab;
    bool m_needs_orthorhombic_check;
    //! Verify that the box is orthorhombic.
    //!
    //! Returns if box is orthorhombic, but throws a runtime_error, if the box is not orthorhombic.
    void verify_orthorhombic_box(void);
#ifdef ENABLE_MPI
    struct MPI_SWAP
        {
        MPI_Comm comm;
        int rank;
        int size;
        int gbl_rank;     //!< global rank of zero in the comm.
        bool initialized; //!< initialized struct, manually set.
        MPI_SWAP()
            : comm(MPI_COMM_NULL), rank(MPI_UNDEFINED), size(MPI_UNDEFINED),
              gbl_rank(MPI_UNDEFINED), initialized(false)
            {
            }
        };
    struct MPI_SWAP m_min_swap;
    struct MPI_SWAP m_max_swap;
    void init_mpi_swap(struct MPI_SWAP* ms, const int color);
    void bcast_vel_to_all(struct MPI_SWAP* ms, Scalar3* vel, const MPI_Op op);
    void mpi_exchange_velocity(void);
#endif // ENABLE_MPI
    };

//! Exports the MuellerPlatheFlow class to python
void export_MuellerPlatheFlow(pybind11::module& m);
