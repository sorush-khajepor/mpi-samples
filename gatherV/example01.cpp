/*

We have system of particles. Each cpu computing some of them. The number
of particles on each cpu is not known at run-time. We want to write
forces acting on all particles in one file.
Here, we assume is a one-dimensional problem. So
Force has one element. 
*/

#include <iostream>
#include <vector>
#include <mpi.h>
using namespace std;

int main()
{
    MPI_Init(NULL, NULL);
    int rank, size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Lets mock a real simple system
    vector<double> force;
    if (rank == 0)
    {
        force = {0.1}; // Rank 0: only 1 particle
    }
    else if (rank == 1)
    {
        force = {4.2, 5.5, 6.1}; // Rank 1: 3 particles
    }
    else if (rank == 2)
    {
        force = {77, 88.1}; // Rank 2: 2 particles
    }

    // Sending particles count of each cpu to root
    int nlocal = force.size();

    std::vector<int> recvcounts;
    if (rank == 0)
        recvcounts.resize(size);

    MPI_Gather(&nlocal, 1, MPI_INT, recvcounts.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    // lets print recieve counts by root
    for (size_t i = 0; i < recvcounts.size(); i++)
    {
        cout << recvcounts[i] << " ";
    }
    if (rank == 0)
        cout << "\n";

    // Now we have to compute displacements
    int totlen = 0;
    std::vector<int> displs;
    std::vector<double> totalForce;

    if (rank == 0)
    {
        displs.resize(size);

        displs[0] = 0;
        totlen += recvcounts[0];

        for (int i = 1; i < size; i++)
        {
            totlen += recvcounts[i];
            displs[i] = displs[i - 1] + recvcounts[i - 1];
        }

        totalForce.resize(totlen);
    }

    // And finally,
    MPI_Gatherv(force.data(), force.size(), MPI_DOUBLE,
                totalForce.data(), recvcounts.data(), displs.data(), MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    // The intersting thing here is that all
    // the cpus run this part, but only root
    // has it full. totalForce of others is 
    // empty.
    for (auto &&f : totalForce)
    {
        cout << f << "\n";
    }

    MPI_Finalize();
}