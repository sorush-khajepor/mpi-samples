#include <mpi.h>
#include <chrono>
#include <thread>
#include <stdio.h>
#include <vector>
#include <random>
using namespace std;



typedef vector<int> vbool;
int NotRank=-1;
struct Section{
    Section(vbool& _points)
    :points(_points)
    {
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);

        leftRank = (rank-1)>=0?rank-1:NotRank;
        rightRank = (rank+1)<size?rank+1:NotRank;
    }

    void ISendBoundaryPoints()
    {
        // left & right boundary points can be overridden during 
        // internal movement, so they are backed up in tmp variables.
        tmpLeftPoint = points.front();
        tmpRightPoint = points.back();
        if (leftRank!=NotRank)
            MPI_Isend(&tmpLeftPoint, 1, MPI_C_BOOL, 
            leftRank, 0, MPI_COMM_WORLD, &reql);
        if (rightRank!=NotRank)
            MPI_Isend(&tmpRightPoint, 1, MPI_C_BOOL, rightRank, 0, MPI_COMM_WORLD, &reqr);
    }
    void MoveInternalPoints(){
        bool points_i=points[0];
        for (size_t i = 0; i < points.size()-1; i++)
        {
            if (points_i && !points[i+1] ){
                // Backing up points[i+1] for the next 
                // round which will be points[i].
                points_i = points[i+1];
                swap(points[i], points[i+1]);
            }
            else{
                points_i = points[i+1];
            }
        }
    }
    void RecvGhosts(){
        MPI_Request req;
        if (rightRank!=NotRank)
            MPI_Irecv(&rightGhost, 1, MPI_C_BOOL, rightRank, 0, MPI_COMM_WORLD, &req);
        if (leftRank!=NotRank)
            MPI_Recv(&leftGhost, 1, MPI_C_BOOL, leftRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (rightRank!=NotRank)
            MPI_Wait(&req , MPI_STATUS_IGNORE);
    }
    void MoveBoundaryPoints(){

        if (rightRank!=NotRank)
            MPI_Wait( &reqr , MPI_STATUS_IGNORE);
        if (leftRank!=NotRank)
            MPI_Wait( &reql , MPI_STATUS_IGNORE);

        if (leftGhost && !tmpLeftPoint && leftRank!=NotRank){
                swap(leftGhost, tmpLeftPoint);
                
                points.front() = tmpLeftPoint;
        }
        if (tmpRightPoint && !rightGhost && rightRank!=NotRank){
                swap(tmpRightPoint, rightGhost);
                points.back() = tmpRightPoint;
        }
    }
    void Display(int step){
        cout<<step<<" "<< rank<<" "<<points[0]<<points[1]<<points[2]<<points[3]<<endl;
    }

    void Run(){
        ISendBoundaryPoints();
        MoveInternalPoints();
        RecvGhosts();
        MoveBoundaryPoints();
    }

private:
    bool tmpLeftPoint;
    bool tmpRightPoint;
    bool rightGhost;
    bool leftGhost;
    int rank;
    int size;
    int leftRank;
    int rightRank;
    vbool& points;
    MPI_Request reqr;
    MPI_Request reql;
};

int main(){
    MPI_Init(NULL, NULL);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);


    vbool points(4,false);

    if (rank==0){
        points[0]=true;
        points[1]=true;
        points[2] = true;
    }
    Section s(points);

    for (size_t i = 0; i < 6; i++)
    {
        s.Run();
        s.Display(i);
    }
    

    MPI_Finalize();
    return 0;
}
