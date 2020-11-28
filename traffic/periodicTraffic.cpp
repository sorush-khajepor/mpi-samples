#include <mpi.h>
#include <stdio.h>
#include <vector>
#include <random>
#include <cmath>

#include <algorithm>
#include <iterator>

using namespace std;
typedef vector<char> vchar;

string vcharToString(vchar& vector){
    std::string str; 
    for (char element:vector)
        str.push_back(element=='t'?'1':'-');
    return str;
}
// Randomely gives 't' or 'f'
char TossCoin()
{
    static std::default_random_engine         e{};
    static std::uniform_int_distribution<int> d{0,1};
    return d(e)==0?'f':'t';
}

/*
Section points:

     Left Boundary        Internal points          Right Boundary 

Left ghost   Left point                      Right point    Right ghost

    *            +          +        +           +            *

*/

// A road section each processor solves
struct RoadSection{
    RoadSection(vchar& _points)
    :points(_points)
    {
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);

        // Set neighboring ranks with
        // periodic boundaries
        leftRank = (rank+size-1)%size;
        rightRank = (rank+1)%size;
    }

    void ISendBoundaryPoints()
    {
        // left & right boundary points can be overridden during 
        // internal movement, so they are backed up in tmp variables.
        tmpLeftPoint = points.front();
        tmpRightPoint = points.back();
        MPI_Isend(&tmpLeftPoint, 1, MPI_CHAR, leftRank, 0, MPI_COMM_WORLD, &leftReq);
        MPI_Isend(&tmpRightPoint, 1, MPI_CHAR, rightRank, 0, MPI_COMM_WORLD, &rightReq);
    }
    void MoveInternalPoints(){
        char points_i = points[0];
        for (size_t i = 0; i < points.size()-1; i++)
        {
            if (points_i=='t' && points[i+1]!='t' ){
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
        MPI_Irecv(&rightGhost, 1, MPI_CHAR, rightRank, 0, MPI_COMM_WORLD, &req);
        MPI_Recv(&leftGhost, 1, MPI_CHAR, leftRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Wait(&req , MPI_STATUS_IGNORE);
    }
    void MoveBoundaryPoints(){
        // This release tmpLeftPoint, tmpRightPoint
        MPI_Wait( &rightReq , MPI_STATUS_IGNORE);
        MPI_Wait( &leftReq , MPI_STATUS_IGNORE);

        if (leftGhost=='t' && tmpLeftPoint!='t'){
                swap(leftGhost, tmpLeftPoint);
                points.front() = tmpLeftPoint;
        }
        if (tmpRightPoint=='t' && rightGhost!='t'){
                swap(tmpRightPoint, rightGhost);
                points.back() = tmpRightPoint;
        }
    }
    void Display(int step){
          
        cout<<"Step="<<step<<" Rank="<<rank<<" points="<< vcharToString(points)<<endl;
    }

    void Run(){
        ISendBoundaryPoints();
        MoveInternalPoints();
        RecvGhosts();
        MoveBoundaryPoints();
    }

private:
    char tmpLeftPoint;
    char tmpRightPoint;
    char rightGhost;
    char leftGhost;
    int rank;
    int size;
    int leftRank;
    int rightRank;
    vchar& points;
    MPI_Request rightReq;
    MPI_Request leftReq;
};

/*
Initialize all points randomely in one processor.
Each point is either true (contains a car)  or false (without a car).
density: percentage of cars (true points) in all points, should be =< 1.
allPoints: all points in the whole MPI world.
*/
void initAllPoints(double& density, vchar& allPoints, int& allPointsCount)
{
    int CarsCount =  round(density * allPointsCount);
    allPoints.resize(allPointsCount);
    for(char& point:allPoints)
        point = 'f';
    
    // loop over points define them by tossing a coin
    // unitl we have the required number of cars.
    size_t i=0;
    int sum=0;
    while (sum<CarsCount)
    {
        auto result = TossCoin();
        if (result=='t' && allPoints[i]!='t')
        { 
            allPoints[i] = result;
            sum++;
        }
        i = (i+1)%allPoints.size();
    }
};

int main(){
    MPI_Init(NULL, NULL);
    int rank,size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Number of car movement steps
    int steps = 6;


    // Totol number of points in MPI world
    int allPointsCount = 8;

    // Percentage of cars in all points
    double density = 0.25;

    int pointsCount = allPointsCount/size;

    // Points allocated to each processor
    vchar points(pointsCount,'f');
    
    // Only declare allPoints, so all processors know it exists 
    vchar allPoints;

    // Only rank 0 initializes allPoints
    if (rank==0){
        initAllPoints(density, allPoints, allPointsCount);
        cout<<" All Points = "<< vcharToString(allPoints) <<endl;
    }    

    // Rank 0 distributes points of each processor
    MPI_Scatter( &allPoints[0] , pointsCount , MPI_CHAR , &points[0] , pointsCount , MPI_CHAR , 0 , MPI_COMM_WORLD);        

    
    RoadSection section(points);

    // Show initial poitns of each section
    section.Display(0);

    // Move Cars
    for (auto i = 0; i < steps; i++)
    {
        section.Run();
        section.Display(i+1);
    }
    
    MPI_Finalize();
    return 0;
}
