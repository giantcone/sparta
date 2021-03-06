/* ----------------------------------------------------------------------
   SPARTA - Stochastic PArallel Rarefied-gas Time-accurate Analyzer
   http://sparta.sandia.gov
   Steve Plimpton, sjplimp@sandia.gov, Michael Gallis, magalli@sandia.gov
   Sandia National Laboratories

   Copyright (2014) Sandia Corporation.  Under the terms of Contract
   DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
   certain rights in this software.  This software is distributed under 
   the GNU General Public License.

   See the README file in the top-level SPARTA directory.
------------------------------------------------------------------------- */

#include "math.h"
#include "surf_react_global.h"
#include "input.h"
#include "update.h"
#include "comm.h"
#include "random_mars.h"
#include "random_park.h"
#include "math_extra.h"
#include "error.h"

using namespace SPARTA_NS;

/* ---------------------------------------------------------------------- */

SurfReactGlobal::SurfReactGlobal(SPARTA *sparta, int narg, char **arg) :
  SurfReact(sparta, narg, arg)
{
  if (narg != 4) error->all(FLERR,"Illegal surf_react global command");

  prob_destroy = input->numeric(FLERR,arg[2]);
  prob_create = input->numeric(FLERR,arg[3]);

  if (prob_destroy + prob_create > 1.0)
    error->all(FLERR,"Illegal surf_react gkibak command");

  // initialize RNG

  random = new RanPark(update->ranmaster->uniform());
  double seed = update->ranmaster->uniform();
  random->reset(seed,comm->me,100);
}

/* ---------------------------------------------------------------------- */

SurfReactGlobal::~SurfReactGlobal()
{
  delete random;
}

/* ---------------------------------------------------------------------- */

int SurfReactGlobal::react(Particle::OnePart *&ip, double *, 
                           Particle::OnePart *&jp)
{
  double r = random->uniform();

  // perform destroy reaction

  if (r < prob_destroy) {
    nsingle++;
    ip = NULL;
    return 1;
  }

  // perform create reaction
  // clone 1st particle to create 2nd particle
  // if add_particle performs a realloc:
  //   make copy of x,v with new species
  //   rot/vib energies will be reset by SurfCollide
  //   repoint ip to new particles data struct if reallocated

  if (r < prob_destroy+prob_create) {
    nsingle++;
    double x[3],v[3];
    int id = MAXSMALLINT*random->uniform();
    memcpy(x,ip->x,3*sizeof(double));
    memcpy(v,ip->v,3*sizeof(double));  
    Particle::OnePart *particles = particle->particles;
    int reallocflag = 
      particle->add_particle(id,ip->ispecies,ip->icell,x,v,0.0,0.0);
    if (reallocflag) ip = particle->particles + (ip - particles);
    jp = &particle->particles[particle->nlocal-1];
    return 1;
  }

  // no reaction

  return 0;
}
