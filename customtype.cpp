#include <mpi.h>
#include <stdio.h>
#include <stddef.h>

using namespace std;

struct Particle{
	int Index;
	double Location[3];
	int Neighbours;
}
;

int main(){
	MPI_Init(NULL, NULL);
/* create a type for struct car */
	Particle p;
	const int nitems=3;
	int blocklengths[3] = {1,3,1};
	MPI_Datatype types[3] = {MPI_INT, MPI_DOUBLE, MPI_INT};
	MPI_Datatype mpi_particle_type;
	MPI_Aint     offsets[3];

	offsets[0] = offsetof(Particle, Index);
	offsets[1] = offsetof(Particle, Location);
	offsets[2] = offsetof(Particle, Neighbours);

	MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_particle_type);
	MPI_Type_commit(&mpi_particle_type);


	int rank,size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	MPI_Request req;
	MPI_Status stat;
	Particle recv;
	Particle send;
	send.Index=2;
	send.Location[0]=50;
	send.Location[1]=51;
	send.Location[2]=52;
	send.Neighbours = 5;

	//cout<< left<<" "<<rank<<" "<<right<<endl;
	

	if (rank==0){		
        	MPI_Recv( &recv, 1 , mpi_particle_type, 1, 0 , MPI_COMM_WORLD, &stat);
	}
	if (rank==1){
   		MPI_Send(  &send, 1 , mpi_particle_type, 0 , 0 , MPI_COMM_WORLD);
	}

	if (rank==0)
	cout<<"rank"<<rank<<" data:"<<recv.Index<<recv.Location[0]<<recv.Location[1]<<recv.Location[2]<<recv.Neighbours<<endl;

	MPI_Finalize();
}

