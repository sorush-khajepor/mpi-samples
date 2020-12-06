#include <mpi.h>
#include <stdio.h>
#include <vector>
using namespace std;


int main(){
	MPI_Init(NULL, NULL);

	int rank,size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	// 3x4 array
	vector<vector<double>> v;
	
	if (rank==0){
		v.push_back({0,1,2,3});
		v.push_back({4,5,6,7});
		v.push_back({8,9,10,11});

/* Rank0 
0 1 2  3
4 5 6  7
8 9 10 11
*/
	}else{
		v.push_back({0,0,0,0});
	}
int array_size[2] = {3,4};
int subarray_size[2] = {2,2};
int subarray_start[2] = {1,2}; // address of 6
MPI_Datatype subtype;

MPI_Type_create_subarray(2, array_size, subarray_size, subarray_start,
MPI_ORDER_C, MPI_DOUBLE, &subtype);
MPI_Type_commit(&subtype);

	double mm[4];
	if (rank==0)	
	   		MPI_Send( &v[0] , 1 , subtype , 1 , 0 , MPI_COMM_WORLD);
	else if(rank==1)
	        MPI_Recv( &mm[0], 4 , MPI_DOUBLE, 0, 0 , MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	std::cout<<"rank"<<rank<<endl;

	if (rank==1){
		// Displaying the 2D vector 
    for (int i = 0; i < 4; i++) { 
            std::cout << mm[i] << " "; 
    } 
		
	}

	MPI_Finalize();
}

