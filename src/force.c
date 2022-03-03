#include "prototypes.h"
#include "utilities.h"

void force(mdsys_t* sys) {
	double epot = 0.0;

	double sigma, sigma6;
	double c6, c12, rcsq;

	sigma = sys->sigma;
	sigma6 = sigma * sigma * sigma * sigma * sigma * sigma;

	c6 = 4.0 * sys->epsilon * sigma6;
	c12 = 4.0 * sys->epsilon * sigma6 * sigma6;
	rcsq = sys->rcut * sys->rcut;

	MPI_Bcast(sys->rx, sys->natoms, MPI_DOUBLE, 0, sys->mpicomm);
	MPI_Bcast(sys->ry, sys->natoms, MPI_DOUBLE, 0, sys->mpicomm);
	MPI_Bcast(sys->rz, sys->natoms, MPI_DOUBLE, 0, sys->mpicomm);

//	zero energy and forces
	/* azzero(sys->fx, sys->natoms); */
	/* azzero(sys->fy, sys->natoms); */
	/* azzero(sys->fz, sys->natoms); */
	/* azzero(sys->cx, sys->natoms); */
	/* azzero(sys->cy, sys->natoms); */
	/* azzero(sys->cz, sys->natoms); */

#ifdef _OPENMP
	#pragma omp parallel reduction(+ : epot)
#endif
	{
		double rx, ry, rz;
		double *cx, *cy, *cz;
		double rsq, ffac;
		double epot_local = 0.0;
		int i;
#ifdef _OPENMP
		int tid = omp_get_thread_num();
#else
		int tid = 0;
#endif

		cx = sys->cx + (tid * sys->natoms);
		azzero(cx, sys->natoms);
		cy = sys->cy + (tid * sys->natoms);
		azzero(cy, sys->natoms);
		cz = sys->cz + (tid * sys->natoms);
		azzero(cz, sys->natoms);

		for (i = sys->mpirank; i < sys->natoms - 1; i += sys->nsize) {
			if (((i - sys->mpirank) / sys->nsize) % sys->nthreads != tid)
				continue;
			for (int j = i + 1; j < (sys->natoms); ++j) {
				/* get distance between particle i and j */
				rx = pbc(sys->rx[i] - sys->rx[j], 0.5 * sys->box);
				ry = pbc(sys->ry[i] - sys->ry[j], 0.5 * sys->box);
				rz = pbc(sys->rz[i] - sys->rz[j], 0.5 * sys->box);
				rsq = rx * rx + ry * ry + rz * rz;

				/* compute force and energy if within cutoff */
				if (rsq < rcsq) {
					double r6, rsqinv;
					rsqinv = 1.0 / rsq;
					r6 = rsqinv * rsqinv * rsqinv;
					ffac = 12.0 * c12 * r6 * r6 * rsqinv - 6.0 * c6 * r6 * rsqinv;
					epot += (c12 * r6 * r6 - c6 * r6);
					cx[i] += rx * ffac;
					cy[i] += ry * ffac;
					cz[i] += rz * ffac;

					cx[j] -= rx * ffac;
					cy[j] -= ry * ffac;
					cz[j] -= rz * ffac;
				}
			}
		}
		epot += epot_local;
#ifdef _OPENMP
	#pragma omp barrier	 // sync everything
#endif
		i = 1 + (sys->natoms / sys->nthreads);
		int fromidx = tid * i;
		int toidx = fromidx + i;
		if (toidx > sys->natoms)
			toidx = sys->natoms;
		for (i = 1; i < sys->nthreads; ++i) {
			int offs = i * sys->natoms;
			for (int j = fromidx; j < toidx; ++j) {
				sys->cx[j] += sys->cx[offs + j];
				sys->cy[j] += sys->cy[offs + j];
				sys->cz[j] += sys->cz[offs + j];
			}
		}
	}
	sys->epot = epot;

	MPI_Reduce(sys->cx, sys->fx, sys->natoms, MPI_DOUBLE, MPI_SUM, 0, sys->mpicomm);
	MPI_Reduce(sys->cy, sys->fy, sys->natoms, MPI_DOUBLE, MPI_SUM, 0, sys->mpicomm);
	MPI_Reduce(sys->cz, sys->fz, sys->natoms, MPI_DOUBLE, MPI_SUM, 0, sys->mpicomm);
	MPI_Reduce(&epot, &sys->epot, 1, MPI_DOUBLE, MPI_SUM, 0, sys->mpicomm);
}

