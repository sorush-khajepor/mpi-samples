#include <mpi.h>
#include <stdio.h>
#include <vector>
#include <random>
#include <cmath>

using namespace std;

char TossCoin()
{
    static std::default_random_engine         e{};
    static std::uniform_int_distribution<int> d{0,1};
    return d(e)==0?'f':'t';
}

typedef vector<char> vchar;

struct Section{
    Section(vchar& _points)
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
        MPI_Isend(&tmpLeftPoint, 1, MPI_CHAR, leftRank, 0, MPI_COMM_WORLD, &reql);
        MPI_Isend(&tmpRightPoint, 1, MPI_CHAR, rightRank, 0, MPI_COMM_WORLD, &reqr);
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

        MPI_Wait( &reqr , MPI_STATUS_IGNORE);
        MPI_Wait( &reql , MPI_STATUS_IGNORE);

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
        cout<<step<<" "<< rank<<" "<<points[0]<<points[1]<<points[2]<<points[3]<<endl;
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
    MPI_Request reqr;
    MPI_Request reql;
};

int main(){
    MPI_Init(NULL, NULL);
    int rank,size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    
    int NumberOfAllPoints = 8;
    int n = NumberOfAllPoints/size;
    vchar points(n,'f');
    vchar allPoints;

    if (rank==0){
        double density = 0.25;
        int NoCars =  round(density * NumberOfAllPoints);
        allPoints.resize(NumberOfAllPoints);
        for(char& point:allPoints)
            point = 'f';
        
        size_t i=0;
        int sum=0;
        while (sum<NoCars)
        {
            auto tmp = allPoints[i];
            allPoints[i] = TossCoin();
            
            if (tmp!='t' && allPoints[i]=='t')
                sum++;
            else if (tmp=='t' && allPoints[i]!='t')
                sum--;

            i = (i+1)%allPoints.size();
        }
    }

    MPI_Scatter( &allPoints[0] , n , MPI_CHAR , &points[0] , n , MPI_CHAR , 0 , MPI_COMM_WORLD);        

    
    Section s(points);

    for (size_t i = 0; i < 6; i++)
    {
        s.Run();
        s.Display(i);
    }
    

    MPI_Finalize();
    return 0;
}
